import calendar
from datetime import datetime
import os

TERMINAL_WIDTH = 105
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

now = datetime.now()
year, month = now.year, now.month  # Hardcoded to January for testing
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
    borderline('-')
    
    weekdate_line = ""
    for date in week:
        if date == 0:
            weekdate_line += " " * date_width
        elif date == day_today:
            date = f'[{date}]'
            weekdate_line += f'{date:^{date_width}}'
        else:
            weekdate_line += f'{date:^{date_width}}'
    print(weekdate_line)
    
    borderline('-')
    print('\n' * 3)  # Space for task lines

borderline('=')