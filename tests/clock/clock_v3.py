import time
import os
from datetime import datetime

def blocky_clock(military_time=False):
    digits = {
        '0': ["█████", 
              "█   █", 
              "█   █", 
              "█   █", 
              "█████"],

        '1': ["  █  ", 
              " ██  ", 
              "  █  ", 
              "  █  ", 
              "█████"],

        '2': ["█████", 
              "    █", 
              "█████", 
              "█    ", 
              "█████"],

        '3': ["█████", 
              "    █", 
              " ████", 
              "    █", 
              "█████"],

        '4': ["█   █", 
              "█   █", 
              "█████", 
              "    █", 
              "    █"],

        '5': ["█████", 
              "█    ", 
              "█████", 
              "    █", 
              "█████"],

        '6': ["█████", 
              "█    ", 
              "█████", 
              "█   █", 
              "█████"],

        '7': ["█████", 
              "    █", 
              "   █ ", 
              "  █  ", 
              "██   "],

        '8': ["█████", 
              "█   █", 
              "█████", 
              "█   █", 
              "█████"],

        '9': ["█████", 
              "█   █", 
              "█████", 
              "    █", 
              "█████"],

        ':': ["     ", 
              "  █  ", 
              "     ", 
              "  █  ", 
              "     "],
              
        ' ': ["     ", 
              "     ", 
              "     ", 
              "     ", 
              "     "]
    }
    
    TERMINAL_WIDTH = 80
    CHAR_SPACING = 2
    ENABLE_BORDER = False

    try:
        while True:
            os.system('cls' if os.name == 'nt' else 'clear')
            now = datetime.now()
            
            if military_time:
                time_str = now.strftime("%H:%M:%S")
                am_pm = ""
            else:
                hour = now.hour
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
                
                minute = now.minute
                second = now.second
                time_str = f"{hour_12:02d}:{minute:02d}:{second:02d}"
            
            date_str = now.strftime("%A, %B %d, %Y")
            
            # Center and display time
            if ENABLE_BORDER:
                print("="*TERMINAL_WIDTH)
                print()
            for row in range(5):
                line = ""
                for char in time_str:
                    line += digits.get(char, digits[' '])[row] + " "*CHAR_SPACING
                
                if not military_time:
                    if row == 2:
                        line += " "*CHAR_SPACING + am_pm
                    else:
                        line += " "*CHAR_SPACING + "  "
                
                line_length = len(line)
                padding_left = (TERMINAL_WIDTH - line_length) // 2
                print(" " * padding_left + line)
            
            # Center and display date
            date_length = len(date_str)
            date_padding = (TERMINAL_WIDTH - date_length) // 2
            print("\n" + " " * date_padding + date_str)
            if ENABLE_BORDER:
                print("\n" + "="*TERMINAL_WIDTH)
            time.sleep(1/60)
    except KeyboardInterrupt:
        print("\nClock stopped.")

if __name__ == "__main__":
    USE_MILITARY_TIME = False 
    blocky_clock(military_time=USE_MILITARY_TIME)