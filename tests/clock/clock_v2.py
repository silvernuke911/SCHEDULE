import time
import os
from datetime import datetime

def blocky_clock():
    # Block characters for better visibility
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
    
    try:
        while True:
            os.system('cls' if os.name == 'nt' else 'clear')
            now = datetime.now()
            time_str = now.strftime("%H:%M:%S")
            date_str = now.strftime("%a, %B %d, %Y")
            
            print("\n" + " " * 10 + "LIVE CLOCK\n")
            
            # Display time
            for row in range(5):
                line = " " * 10
                for char in time_str:
                    line += digits.get(char, digits[' '])[row] + " "
                print(line)
            
            print("\n" + " " * 12 + date_str)
            print("\n" + " " * 10 + "Ctrl+C to stop")
            
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n" + " " * 10 + "Clock stopped.")

blocky_clock()