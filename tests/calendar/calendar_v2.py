import calendar
from datetime import datetime
import os

TERMINAL_WIDTH = 112
date_width = TERMINAL_WIDTH // 7

def borderline(style, connect=True):
    if connect:
        border_major_char = '═'
        border_minor_char = '─'
    else:
        border_major_char = '='
        border_minor_char = '-'
    if style in ('major', '='):
        print(border_major_char * TERMINAL_WIDTH)
    elif style in ('minor', '-'):
        print(border_minor_char * TERMINAL_WIDTH)

# Sample tasks dictionary: {day: [(task, priority), ...]}
# Priority order: 1 (highest) to 5 (lowest)
sample_tasks = {
    1: [
        ("Finish report", 1),
        ("Meeting with team", 2),
        ("Call client", 3),
        ("Update docs", 4)  # Will show as "*"
    ],
    2: [
        ("Gym workout", 4),
        ("Buy groceries", 3)
    ],
    5: [
        ("Project deadline", 1),
        ("Submit assignment", 2)
    ],
    8: [
        ("Dentist appointment", 2)
    ],
    9: [
        ("Team lunch", 4),
        ("Code review", 3),
        ("Prepare presentation", 2),
        ("Send emails", 5)  # Will show as "*"
    ],
    12: [
        ("Birthday party", 5)
    ],
    15: [
        ("Budget planning", 2),
        ("Pay bills", 1),
        ("Renew subscription", 4)
    ],
    17: [
        ("Conference call", 1),
        ("Flight booking", 2),
        ("Hotel reservation", 3),
        ("Car rental", 4),
        ("Pack luggage", 5)  # Will show as "*"
    ],
    20: [
        ("Doctor checkup", 2)
    ],
    25: [
        ("Monthly review", 3),
        ("Team meeting", 2)
    ],
    28: [
        ("Movie night", 5),
        ("Clean house", 4),
        ("Laundry", 3)
    ]
}

def get_tasks_for_day(day, tasks_dict):
    """Return sorted tasks for a given day, sorted by priority (ascending)."""
    if day in tasks_dict:
        # Sort by priority (1 = highest)
        return sorted(tasks_dict[day], key=lambda x: x[1])
    return []

def format_task_for_display(task_tuple, max_len=9):
    """Format a task for display in calendar cell."""
    task, priority = task_tuple
    # Truncate if too long
    if len(task) > max_len:
        task = task[:max_len-2] + ".."
    
    return f"{task:<{date_width}}"


now = datetime.now()
year, month = now.year, now.month
day_today = now.day 

os.system('cls' if os.name == 'nt' else 'clear')

calendar.setfirstweekday(calendar.SUNDAY)
cal = calendar.monthcalendar(year, month)
month_name = calendar.month_name[month]
weekdays = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"]

# Header
borderline('=')
month_year = f'{month_name} {year}'
print(f'{month_year:^{TERMINAL_WIDTH}}')
borderline('=')

# Weekday headers
weekday_line = "".join(f'{day:^{date_width}}' for day in weekdays)
print(weekday_line)
# Calendar dates
for week in cal:
    # First: Print dates row
    weekdate_line = ""
    for date in week:
        if date == 0:
            weekdate_line += " " * date_width
        elif date == day_today:
            date = f'[{date}]'
            weekdate_line += f'{date:^{date_width}}'
        else:
            weekdate_line += f'{date:^{date_width}}'
    borderline('-')
    print(weekdate_line)
    borderline('-')
    
    # Second: Print tasks for this week (up to 3 rows)
    # Prepare 3 rows for tasks
    task_rows = ["", "", ""]
    
    for col, date in enumerate(week):
        if date != 0:
            tasks = get_tasks_for_day(date, sample_tasks)
            if tasks:
                # Get first 3 tasks (sorted by priority)
                for i in range(3):
                    if i < len(tasks):
                        if i == 2 and len(tasks) > 3:
                            # Show "***" on third line if more than 3 tasks
                            task_display = "***"
                            task_display = f"{task_display:^{date_width}}"
                        else:
                            task_display = format_task_for_display(tasks[i], max_len=date_width-1)
                        # Center in cell
                        task_rows[i] += f'{task_display:^{date_width}}'
                    else:
                        task_rows[i] += " " * date_width
            else:
                # No tasks, add empty space
                for i in range(3):
                    task_rows[i] += " " * date_width
    
    for row in task_rows:
        print(row)
borderline('=')

# # Legend
# print("\n" + " " * 20 + "LEGEND: ! = Urgent  # = Important  + = Normal  - = Low  . = Optional")
# print(" " * 30 + "* = More than 3 tasks for that day")