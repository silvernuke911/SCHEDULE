import time
import os
import sys
from datetime import datetime

borderchar = '═'
solid_block = '█'
at_char = '@'
digits = {
    '0': ["@@@@@", 
          "@   @", 
          "@   @", 
          "@   @", 
          "@@@@@"],

    '1': ["  @  ", 
          " @@  ", 
          "  @  ", 
          "  @  ", 
          "@@@@@"],

    '2': ["@@@@@", 
          "    @", 
          "@@@@@", 
          "@    ", 
          "@@@@@"],

    '3': ["@@@@@", 
          "    @", 
          " @@@@", 
          "    @", 
          "@@@@@"],

    '4': ["@   @", 
          "@   @", 
          "@@@@@", 
          "    @", 
          "    @"],

    '5': ["@@@@@", 
          "@    ", 
          "@@@@@", 
          "    @", 
          "@@@@@"],

    '6': ["@@@@@", 
          "@    ", 
          "@@@@@", 
          "@   @", 
          "@@@@@"],

    '7': ["@@@@@", 
          "    @", 
          "   @ ", 
          "  @  ", 
          " @   "],

    '8': ["@@@@@", 
          "@   @", 
          "@@@@@", 
          "@   @", 
          "@@@@@"],

    '9': ["@@@@@", 
          "@   @", 
          "@@@@@", 
          "    @", 
          "@@@@@"],

    ':': ["     ", 
          "  @  ", 
          "     ", 
          "  @  ", 
          "     "],

    ' ': ["     ", 
          "     ", 
          "     ", 
          "     ", 
          "     "]
}

MILITARY_TIME = False
TERMINAL_WIDTH = 105
TERMINAL_HEIGHT = 30
CHAR_SPACING = 2
ENABLE_BORDER = True

try:
    # Clear once at the beginning
    os.system('cls' if os.name == 'nt' else 'clear')
    
    while True:
        now = datetime.now()
        
        if MILITARY_TIME:
            time_str = now.strftime("%H:%M:%S")
            am_pm = ""
        else:
            hour = now.hour
            minute = now.minute
            second = now.second
            if hour == 0:
                hour_12 = 12
                am_pm = "AM"
            elif hour < 12:
                hour_12 = hour
                am_pm = "AM"
            elif hour == 12:
                hour_12 = 12
                am_pm = "PM"
            else:
                hour_12 = hour - 12
                am_pm = "PM"
            time_str = f"{hour_12:02d}:{minute:02d}:{second:02d}"
        
        date_str = now.strftime("%A, %B %d, %Y")
        
        # Move cursor to top-left position
        sys.stdout.write('\033[H')
        
        if ENABLE_BORDER:
            sys.stdout.write(borderchar * TERMINAL_WIDTH + '\n\n')
        
        # Display time lines
        for row in range(5):
            line = ""
            for char in time_str:
                line += digits.get(char, digits[' '])[row] + " " * CHAR_SPACING
            
            if not MILITARY_TIME:
                if row == 2:
                    line += " " * CHAR_SPACING + am_pm
                else:
                    line += " " * CHAR_SPACING + "  "

            line_str = f'{line:^{TERMINAL_WIDTH}}'
            # Clear the line and write new content
            sys.stdout.write('\033[K')  # Clear entire line
            sys.stdout.write(line_str + '\n')
        
        # Display date
        sys.stdout.write('\033[K')  # Clear line
        date_str = f'{date_str:^{TERMINAL_WIDTH}}'
        sys.stdout.write("\n" + date_str + '\n')
        
        if ENABLE_BORDER:
            sys.stdout.write('\033[K')  # Clear line
            sys.stdout.write("\n" + borderchar * TERMINAL_WIDTH + '\n')
        
        sys.stdout.flush()
        time.sleep(1/60)
        
except KeyboardInterrupt:
    print("\nClock stopped.")

