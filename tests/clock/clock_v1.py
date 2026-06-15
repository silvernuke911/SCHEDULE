import curses
import time
from datetime import datetime

def big_clock(stdscr):
    curses.curs_set(0)  # Hide cursor
    stdscr.nodelay(1)   # Non-blocking input
    
    # 7-segment patterns for digits 0-9 and colon
    digits = {
        '0': [' ███ ', '█   █', '█   █', '█   █', ' ███ '],
        '1': ['  █  ', ' ██  ', '  █  ', '  █  ', ' ███ '],
        '2': [' ███ ', '    █', ' ███ ', '█    ', ' ███ '],
        '3': [' ███ ', '    █', '  ██ ', '    █', ' ███ '],
        '4': ['█   █', '█   █', ' ███ ', '    █', '    █'],
        '5': [' ███ ', '█    ', ' ███ ', '    █', ' ███ '],
        '6': [' ███ ', '█    ', ' ███ ', '█   █', ' ███ '],
        '7': [' ███ ', '    █', '   █ ', '  █  ', ' █   '],
        '8': [' ███ ', '█   █', ' ███ ', '█   █', ' ███ '],
        '9': [' ███ ', '█   █', ' ███ ', '    █', ' ███ '],
        ':': ['     ', '  █  ', '     ', '  █  ', '     '],
        ' ': ['     ', '     ', '     ', '     ', '     ']
    }
    
    while True:
        stdscr.clear()
        now = datetime.now()
        time_str = now.strftime("%H:%M:%S")
        date_str = now.strftime("%a, %B %d, %Y")
        
        height, width = stdscr.getmaxyx()
        
        # Display each digit (5 rows tall)
        start_row = max(0, (height - 7) // 2)
        start_col = max(0, (width - len(time_str)*6) // 2)
        
        for row in range(5):
            line = ""
            for char in time_str:
                line += digits.get(char, digits[' '])[row] + " "
            stdscr.addstr(start_row + row, start_col, line)
        
        # Display date
        stdscr.addstr(start_row + 6, start_col, f"  {date_str}")
        
        stdscr.refresh()
        time.sleep(0.1)  # Update 10 times per second
        
        # Exit on 'q'
        if stdscr.getch() == ord('q'):
            break

# Run it
curses.wrapper(big_clock)