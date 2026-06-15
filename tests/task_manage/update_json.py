import os 
from datetime import datetime 
import json 

TASKS_FILE = r"C:\Users\verci\Documents\code\Task Scheduler\tests\task_manage\tasks.json"
NEXT_ID_FILE = r'C:\Users\verci\Documents\code\Task Scheduler\tests\task_manage\next_id.json'

def convert_to_5digit():
    """Convert existing tasks.json to 5-digit format."""
    if not os.path.exists(TASKS_FILE):
        print(f"{TASKS_FILE} not found.")
        return
    
    with open(TASKS_FILE, 'r') as f:
        tasks = json.load(f)
    
    # Convert to list and sort by creation date
    tasks_list = list(tasks.values())
    tasks_list.sort(key=lambda x: datetime.strptime(x["created"], "%Y-%m-%d %H:%M"))
    
    # Create new dictionary with 5-digit IDs
    new_tasks = {}
    for i, task in enumerate(tasks_list, 1):
        new_id = f"{i:05d}"  # 00001, 00002, etc.
        task["id"] = new_id  # Update ID field
        new_tasks[new_id] = task
    
    # Save converted tasks
    with open(TASKS_FILE, 'w') as f:
        json.dump(new_tasks, f, indent=2, default=str)
    
    # Update next_id.json
    next_num = len(new_tasks) + 1  # Next integer to use
    with open(NEXT_ID_FILE, 'w') as f:
        json.dump({'next_id': next_num}, f, indent=2)
    
    print(f"Converted {len(new_tasks)} tasks to 5-digit format.")
    print(f"Next ID will be: {next_num:05d} ({next_num})")

# Run the conversion
if __name__ == "__main__":
    convert_to_5digit()