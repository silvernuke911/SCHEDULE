import json
import os
import sys
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Any

# ========== CONFIGURATION ==========
TASKS_FILE = "tasks.json"
NEXT_ID_FILE = "next_id.json"
TERMINAL_WIDTH = 80

# Priority mapping
PRIORITY_MAP = {
    '0': 'optional', 'optional': 'optional',
    '1': 'non-urgent', 'non-urgent': 'non-urgent',
    '2': 'normal', 'normal': 'normal',
    '3': 'important', 'important': 'important',
    '4': 'urgent', 'urgent': 'urgent',
    '5': 'urgent'
}

PRIORITY_VALUES = {'optional': 0, 'non-urgent': 1, 'normal': 2, 'important': 3, 'urgent': 4}

# ========== PROMPT SYSTEM ==========
def prompt(address=True, lower=True, yes_no=False):
    """Custom prompt function."""
    if address:
        print(r'C:Vercil\Scheduler', end='\n')
    if yes_no:
        result = yes_no_query('> ')
        # log_to_cmdlog('Y' if result else 'N')  # You can add logging later
        return result  # Boolean
    user_input = input('> ').strip()
    # log_to_cmdlog(user_input)  # You can add logging later
    if lower:
        user_input = user_input.lower()
    return user_input

def yes_no_query(prompt_text):
    """Simple yes/no query."""
    while True:
        response = input(prompt_text).strip().lower()
        if response in ('y', 'yes'):
            return True
        elif response in ('n', 'no'):
            return False
        print("Please enter 'y' or 'n'")

# ========== FILE OPERATIONS ==========
def load_tasks() -> Dict[int, Dict]:
    """Load tasks from JSON file."""
    if os.path.exists(TASKS_FILE):
        with open(TASKS_FILE, 'r') as f:
            data = json.load(f)
            return {int(k): v for k, v in data.items()}
    return {}

def save_tasks(tasks: Dict[int, Dict]) -> None:
    """Save tasks to JSON file."""
    with open(TASKS_FILE, 'w') as f:
        json.dump(tasks, f, indent=2)

def load_next_id() -> int:
    """Load next task ID."""
    if os.path.exists(NEXT_ID_FILE):
        with open(NEXT_ID_FILE, 'r') as f:
            return json.load(f).get('next_id', 1)
    return 1

def save_next_id(next_id: int) -> None:
    """Save next task ID."""
    with open(NEXT_ID_FILE, 'w') as f:
        json.dump({'next_id': next_id}, f, indent=2)

# ========== TASK OPERATIONS ==========
def create_task(task_name: str, priority: str, deadline_str: str, 
                estimated_hours: float = 0, category: str = "", notes: str = "") -> Dict:
    """Create a new task dictionary."""
    now = datetime.now()
    
    deadline_dt = parse_datetime(deadline_str)
    if not deadline_dt:
        print("Invalid deadline format. Use MM/DD/YYYY HH:MM AM/PM or similar.")
        return None
    
    priority = PRIORITY_MAP.get(priority.lower(), priority.lower())
    if priority not in PRIORITY_VALUES:
        print(f"Invalid priority. Must be one of: {list(PRIORITY_VALUES.keys())}")
        return None
    
    status = "not_started"
    if deadline_dt < now:
        status = "late"
    
    task = {
        "id": 0,
        "task": task_name,
        "priority": priority,
        "status": status,
        "deadline": deadline_dt.strftime("%Y-%m-%d %H:%M"),
        "created": now.strftime("%Y-%m-%d %H:%M"),
        "completed": None,
        "estimated_hours": estimated_hours,
        "category": category,
        "notes": notes
    }
    
    return task

def add_task(task_data: Dict) -> int:
    """Add a new task and return its ID."""
    tasks = load_tasks()
    next_id = load_next_id()
    
    task_data["id"] = next_id
    tasks[next_id] = task_data
    
    save_tasks(tasks)
    save_next_id(next_id + 1)
    
    return next_id

def edit_task(task_id: int, field: str, new_value: Any) -> bool:
    """Edit a specific field of a task."""
    tasks = load_tasks()
    
    if task_id not in tasks:
        print(f"Task ID {task_id} not found.")
        return False
    
    if field == "priority":
        new_value = PRIORITY_MAP.get(str(new_value).lower(), new_value.lower())
        if new_value not in PRIORITY_VALUES:
            print(f"Invalid priority. Must be one of: {list(PRIORITY_VALUES.keys())}")
            return False
    
    elif field == "deadline":
        deadline_dt = parse_datetime(new_value)
        if not deadline_dt:
            print("Invalid deadline format.")
            return False
        new_value = deadline_dt.strftime("%Y-%m-%d %H:%M")
        
        if deadline_dt < datetime.now():
            tasks[task_id]["status"] = "late"
        elif tasks[task_id]["status"] == "late":
            tasks[task_id]["status"] = "not_started"
    
    elif field == "status":
        if new_value == "done":
            tasks[task_id]["completed"] = datetime.now().strftime("%Y-%m-%d %H:%M")
        elif new_value != "done" and tasks[task_id]["status"] == "done":
            tasks[task_id]["completed"] = None
    
    tasks[task_id][field] = new_value
    save_tasks(tasks)
    return True

def remove_task(task_id: Optional[int] = None, task_name: Optional[str] = None) -> bool:
    """Remove a task by ID or name."""
    tasks = load_tasks()
    
    if task_id:
        if task_id in tasks:
            del tasks[task_id]
            save_tasks(tasks)
            return True
        else:
            print(f"Task ID {task_id} not found.")
            return False
    
    elif task_name:
        found_ids = []
        for tid, task in tasks.items():
            if task_name.lower() in task["task"].lower():
                found_ids.append(tid)
        
        if not found_ids:
            print(f"No task found with name containing '{task_name}'.")
            return False
        
        if len(found_ids) == 1:
            del tasks[found_ids[0]]
            save_tasks(tasks)
            return True
        else:
            print(f"Multiple tasks found with '{task_name}'. Please use task ID.")
            for tid in found_ids:
                print(f"  ID {tid}: {tasks[tid]['task']}")
            return False
    
    return False

# ========== PARSING & FORMATTING ==========
def parse_datetime(dt_str: str) -> Optional[datetime]:
    """Parse various datetime formats."""
    now = datetime.now()
    
    formats = [
        "%m/%d/%Y %I:%M %p",
        "%m/%d/%Y %H:%M",
        "%m/%d/%Y",
        "%m/%d %I:%M %p",
        "%m/%d %H:%M",
        "%m/%d",
    ]
    
    for fmt in formats:
        try:
            dt = datetime.strptime(dt_str, fmt)
            if fmt in ["%m/%d", "%m/%d %I:%M %p", "%m/%d %H:%M"]:
                dt = dt.replace(year=now.year)
            if fmt in ["%m/%d/%Y", "%m/%d"]:
                dt = dt.replace(hour=23, minute=59)
            return dt
        except ValueError:
            continue
    
    return None

def calculate_time_remaining(deadline_str: str) -> str:
    """Calculate time remaining until deadline."""
    try:
        deadline = datetime.strptime(deadline_str, "%Y-%m-%d %H:%M")
        now = datetime.now()
        
        if deadline < now:
            return "OVERDUE"
        
        delta = deadline - now
        
        if delta.days > 0:
            return f"{delta.days}d {delta.seconds // 3600}h"
        elif delta.seconds // 3600 > 0:
            return f"{delta.seconds // 3600}h {delta.seconds % 3600 // 60}m"
        else:
            return f"{delta.seconds // 60}m"
    except:
        return "---"

# ========== DISPLAY FUNCTIONS ==========
def display_tasks(tasks_list: List[Dict], title: str = "TASKS") -> None:
    """Display tasks in a formatted table."""
    if not tasks_list:
        print(f"\nNo {title.lower()} found.\n")
        return
    
    print(f"\n{'=' * TERMINAL_WIDTH}")
    print(f"{title:^{TERMINAL_WIDTH}}")
    print(f"{'=' * TERMINAL_WIDTH}")
    
    header = f"{'ID':<4} {'TASK':<25} {'PRIORITY':<12} {'STATUS':<12} {'DEADLINE':<16} {'TIME LEFT':<10}"
    print(header)
    print("-" * TERMINAL_WIDTH)
    
    for task in tasks_list:
        time_left = calculate_time_remaining(task["deadline"])
        
        status = task["status"]
        if status == "late":
            status = f"LATE"
        elif status == "done":
            status = f"DONE"
        
        task_line = f"{task['id']:<4} {task['task'][:23]:<25} {task['priority'].upper():<12} {status:<12} {task['deadline'][5:16]:<16} {time_left:<10}"
        print(task_line)
    
    print(f"{'=' * TERMINAL_WIDTH}")
    print(f"Total: {len(tasks_list)} task(s)\n")

def filter_and_sort_tasks(status_filter: Optional[str] = None, 
                          priority_filter: Optional[str] = None,
                          show_all: bool = False) -> List[Dict]:
    """Filter and sort tasks based on criteria."""
    tasks = load_tasks()
    
    if not tasks:
        return []
    
    tasks_list = []
    for task_id, task in tasks.items():
        if not show_all and task["status"] == "done" and status_filter != "done":
            continue
        
        if status_filter and task["status"] != status_filter:
            continue
        
        if priority_filter and task["priority"] != priority_filter:
            continue
        
        tasks_list.append(task)
    
    tasks_list.sort(key=lambda x: (
        x["deadline"],
        -PRIORITY_VALUES[x["priority"]]
    ))
    
    return tasks_list

# ========== INTERACTIVE FUNCTIONS ==========
def add_task_interactive() -> None:
    """Interactive task addition."""
    print("\n" + "=" * TERMINAL_WIDTH)
    print("ADD NEW TASK".center(TERMINAL_WIDTH))
    print("=" * TERMINAL_WIDTH)
    
    print(r'C:\Scheduler', end=' ')
    task_name = input("Task Name: ").strip()
    if not task_name:
        print("Task name cannot be empty.")
        return
    
    print("\nPriority levels:")
    print("  0/optional    - Optional")
    print("  1/non-urgent  - Non-urgent")
    print("  2/normal      - Normal")
    print("  3/important   - Important")
    print("  4/urgent      - Urgent")
    
    print(r'C:\Scheduler', end=' ')
    priority = input("Priority (0-4 or name): ").strip()
    
    print(r'C:\Scheduler', end=' ')
    deadline = input("Deadline (MM/DD[/YYYY] [HH:MM] [AM/PM]): ").strip()
    
    print(r'C:\Scheduler', end=' ')
    estimated = input("Estimated hours (optional, press Enter for 0): ").strip()
    estimated_hours = float(estimated) if estimated else 0.0
    
    print(r'C:\Scheduler', end=' ')
    category = input("Category (optional): ").strip()
    
    print(r'C:\Scheduler', end=' ')
    notes = input("Notes (optional): ").strip()
    
    task_data = create_task(task_name, priority, deadline, estimated_hours, category, notes)
    if task_data:
        task_id = add_task(task_data)
        print(f"\n✓ Task added successfully with ID: {task_id}")

def edit_task_interactive(task_id: int) -> None:
    """Interactive task editing."""
    tasks = load_tasks()
    
    if task_id not in tasks:
        print(f"Task ID {task_id} not found.")
        return
    
    task = tasks[task_id]
    
    print("\n" + "=" * TERMINAL_WIDTH)
    print(f"EDIT TASK {task_id}".center(TERMINAL_WIDTH))
    print("=" * TERMINAL_WIDTH)
    
    print(f"Current task: {task['task']}")
    print("\nFields you can edit:")
    print("  1. task         - Task name")
    print("  2. priority     - Priority level")
    print("  3. deadline     - Due date and time")
    print("  4. status       - Current status")
    print("  5. estimated_hours - Time estimate")
    print("  6. category     - Category/tag")
    print("  7. notes        - Additional notes")
    
    print(r'C:\Scheduler', end=' ')
    field = input("Enter field name or number to edit: ").strip().lower()
    
    field_map = {
        '1': 'task', '2': 'priority', '3': 'deadline',
        '4': 'status', '5': 'estimated_hours', '6': 'category', '7': 'notes'
    }
    
    if field in field_map:
        field = field_map[field]
    
    if field not in task:
        print(f"Invalid field: {field}")
        return
    
    print(f"\nCurrent value: {task[field]}")
    
    if field == "status":
        print("\nAvailable statuses: not_started, in_progress, partial, done, late")
    
    print(r'C:\Scheduler', end=' ')
    new_value = input(f"New {field}: ").strip()
    
    if edit_task(task_id, field, new_value):
        print(f"\n✓ Task {task_id} updated successfully.")

def remove_task_interactive(identifier: str) -> None:
    """Interactive task removal."""
    try:
        task_id = int(identifier)
        if remove_task(task_id=task_id):
            print(f"\n✓ Task {task_id} removed successfully.")
    except ValueError:
        if remove_task(task_name=identifier):
            print(f"\n✓ Task with name containing '{identifier}' removed successfully.")

# ========== MAIN COMMAND PARSER ==========
def parse_command(user_input: str) -> None:
    """Parse and execute commands with noun-verb syntax."""
    parts = user_input.strip().split()
    if not parts:
        return
    
    noun = parts[0].lower()
    
    if noun == "task":
        if len(parts) < 2:
            print("Usage: task --add | --edit | --remove | --show | --late | --done | --all | --priority")
            return
        
        verb = parts[1]
        args = parts[2:] if len(parts) > 2 else []
        
        if verb == "--add":
            add_task_interactive()
        
        elif verb == "--edit":
            if not args:
                print("Usage: task --edit <task_id>")
                return
            try:
                task_id = int(args[0])
                edit_task_interactive(task_id)
            except ValueError:
                print("Task ID must be a number.")
        
        elif verb == "--remove":
            if not args:
                print("Usage: task --remove <task_id_or_name>")
                return
            remove_task_interactive(args[0])
        
        elif verb == "--show":
            tasks = filter_and_sort_tasks(status_filter=None)
            display_tasks(tasks, "ACTIVE TASKS")
        
        elif verb == "--late":
            tasks = filter_and_sort_tasks(status_filter="late")
            display_tasks(tasks, "LATE TASKS")
        
        elif verb == "--done":
            tasks = filter_and_sort_tasks(status_filter="done")
            tasks.sort(key=lambda x: x.get("completed", ""), reverse=True)
            for i, task in enumerate(tasks, 1):
                task["display_id"] = -i
            display_tasks(tasks, "COMPLETED TASKS")
        
        elif verb == "--all":
            tasks = filter_and_sort_tasks(show_all=True)
            display_tasks(tasks, "ALL TASKS")
        
        elif verb == "--priority":
            if not args:
                print("Usage: task --priority <urgent|important|normal|non-urgent|optional>")
                return
            priority = PRIORITY_MAP.get(args[0].lower(), args[0].lower())
            if priority not in PRIORITY_VALUES:
                print(f"Invalid priority. Must be one of: {list(PRIORITY_VALUES.keys())}")
                return
            tasks = filter_and_sort_tasks(priority_filter=priority)
            display_tasks(tasks, f"{priority.upper()} PRIORITY TASKS")
        
        else:
            print(f"Unknown task command: {verb}")
            print("Available: --add, --edit, --remove, --show, --late, --done, --all, --priority")
    
    elif noun == "calendar":
        # You can add calendar commands here
        print("Calendar functionality coming soon...")
        # Example: calendar --show [month] [year]
        # Example: calendar --today
    
    elif noun == "clock":
        # You can add clock commands here
        print("Clock functionality coming soon...")
        # Example: clock --start
        # Example: clock --stop
    
    elif noun == "help" or noun == "?":
        print_help()
    
    elif noun == "exit" or noun == "quit":
        print("Goodbye!")
        sys.exit(0)
    
    else:
        print(f"Unknown command: {noun}")
        print("Available commands: task, calendar, clock, help, exit")

def print_help() -> None:
    """Print help information."""
    print("\n" + "=" * TERMINAL_WIDTH)
    print("SCHEDULER HELP".center(TERMINAL_WIDTH))
    print("=" * TERMINAL_WIDTH)
    
    print("\nTASK COMMANDS:")
    print("  task --add                   - Add a new task")
    print("  task --edit <id>             - Edit an existing task")
    print("  task --remove <id|name>      - Remove a task by ID or name")
    print("  task --show                  - Show active tasks")
    print("  task --late                  - Show late tasks")
    print("  task --done                  - Show completed tasks")
    print("  task --all                   - Show all tasks")
    print("  task --priority <level>      - Filter by priority")
    
    print("\nOTHER COMMANDS:")
    print("  calendar [--show]            - Show calendar")
    print("  clock [--start]              - Start/stop clock")
    print("  help, ?                      - Show this help")
    print("  exit, quit                   - Exit program")
    
    print("\n" + "=" * TERMINAL_WIDTH)

# ========== MAIN LOOP ==========
def main() -> None:
    """Main program loop."""
    os.system('cls' if os.name == 'nt' else 'clear')
    
    print("=" * TERMINAL_WIDTH)
    print("TERMINAL SCHEDULER".center(TERMINAL_WIDTH))
    print("=" * TERMINAL_WIDTH)
    print("Type 'help' or '?' for available commands")
    print("=" * TERMINAL_WIDTH)
    
    while True:
        try:
            user_input = prompt(address=True, lower=False, yes_no=False)
            if user_input:
                parse_command(user_input)
        except KeyboardInterrupt:
            print("\n\nUse 'exit' or 'quit' to close the program.")
        except EOFError:
            print("\n\nGoodbye!")
            sys.exit(0)

if __name__ == "__main__":
    main()