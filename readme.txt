==========================================================
==========================================================
### -------------------------------------
[ TASK SCHEDULER COMMAND LINE INTERFACE TOOL ]
### -------------------------------------
==========================================================
==========================================================
Alright, so here's the plan.

Make a cli scheduler. kinda like vim
big screen showing information
then a small command line at the bottom left, like: 
tasks\ >

and on the bottom right, show the current [page], like TASK, HOME, CALENDAR, FOCUS, HELP

written in c, tasks.c, compiled to tasks.exe, 
add path to cmd so we can just typle tasks and run it there
use colors and shit

start at home

Home page is big clock showing time and date.
and list of tasks for the day. show overdue, and show task today. ordered by priority

tasks are stored in a separate .json file; see below


==========================================================
		 Command list  
==========================================================
>home, hm					;go back to home page

>help, h, ?					;print the help menu with the flags and commands and stuff
	
>clock, c
	-set --time military	;set the time to military format
	-set --time civilian	;set the time to civilian format
	-set --date				;set the date to ymd, dmy, or mdy
			YMD
			MDY
			DMY
	
>calendar, cal				;graphical calendar with tasks. 
	-month, -mon, -m		;choose which month to show, default current
	-year, -yr, -y		    ;show the entire year, default current
	-week, -wk, -w			;show the week, default current
	-day, -d				;show the days tasks
							;default behaviour is month, and shows the days tasks on the side (selected)

>task, tasklist, t			;show tasks
	-default				;default display, tasks for the week (sorted)
	-todo,   -td			;show unfinished
	-overdue,-od, -late, -l	;show overdue
	-done,   -d				;show done
	-add,	 -a				;add task     -enter task field
	-remove, -rm			;remove task
	-edit	 -e				;edit task    ;edit task field
	-all	 -a				;show all tasks
	-sort	 -s				;sort the tasks
		default						;sort by deadline, then by priority, then by status
		creation					;by time of creation
		priority                    ;by priority (then by deadline), 
		deadline	[default]       ;by deadline
		status						;by status (done or not done and all that)
	-setting						;some settings
		done-invisible				;do not show done

							task field is :
						
							task = {
								"id": 1,
								"task": "Complete physics 195 PS2 assignment",
								"task_abv":"P195 PS2"							\\ 8 chars max
								"priority": "urgent",  # urgent, important, normal, non-urgent, optional
								"status": "not_started",  # not_started, in_progress, done, partial
								"deadline": "2024-12-25 23:59",  # YYYY-MM-DD HH:MM format
								"created": "2024-11-20 10:00",
								"completed": None,  # Timestamp when done
								"category": "Study",  # Optional: for filtering
								"description": ""  # Optional description field
							}

>focus				;enter focus mode; just a countdown clock[big], current time, the task at hand, and all that
					;exit by typing >done or >pause. Will update the task file to done or partially done
					;required argument of task id or task name to run
					;throw error message if not
	-set-n	[task#] ;task number from task list (default)
	-set-id [taskid];argument is task id or task number
					
		
>exit, q, quit		;exit the application
>history			;show command history
	-[n]			;start with command [n]
>lock				;lock the program, only allow entrance with username and password
					;check externally, keep locked as long as not unlocked
	-add_usr		;add new user and password
	-remove_usr,rm_user ; remove user
	-
					;obfuscate this shit
==========================================================
DISPLAY SCREEN
==========================================================
-------------------------------------------------------
|												   	  |
|												   	  |
|												   	  |
|					DISPLAY						   	  |
|												   	  |
|												   	  |
|												   	  |
-------------------------------------------------------
tasks\>										  [ HOME ]

tasks\> is for commands
[  ] shows the mode;


==========================================================
   HOME PAGE
==========================================================

				   #    #####         #   #    #           #####  #####
				  ##    #        #    #   #   ##      #    #   #  #
				   #    #####         #####    #           #   #  #####
				   #    #   #    #        #    #      #    #   #  #   #
				  ###   #####             #   ###          #####  #####										// this counts up

								Mon, 15-06-2026 

 Task List:
	ITEM    	TASK      PRIORITY       STATUS      DEADLINE           TIME REMAINING
	000001   [title1]      HIGH           [ ]    2026-02-17 14:30      0-00-00  2:30:15    
	000021   [title2]     NORMAL          [x]    2026-02-17 18:00	   0-00-00  6:00:15   
	000022	 [title3]      LOW			  [*]    2026-02-21 18:00	   0-00-04 18:00:00	      // counts down
	000021   [title4]     NORMAL          [ ]    2026-02-24 15:00	   0-00-00  6:00:15   
	000022	 [title5]      LOW			  [ ]    2026-02-26 16:00	   0-00-04 18:00:00	 
	                                   ...

tasks\>|																	   [ HOME ]

============================================================
CALENDAR MODE
------------------
		-month mode (default)
		===================================================
						February 2025
		---------------------------------------------------
		Sun		 Mon     Tue	 Wed 	Thu		Fri		Sat
		---------------------------------------------------
		 1		 2		  3		  4		  5		  6		  7
		---------------------------------------------------
		[Task 1]	 [Task 2]		[Task 4]
					 [Task 3]
		---------------------------------------------------
		 8		 9     	10		 11		 12		 13		 14
		---------------------------------------------------
				  [Task 5]
		          [Task 6]
					 *									    * means more tasks (max 3 task per date displayed
		---------------------------------------------------
		15		  16	  17	 18		 19		 20		 21  // just color selected date red or green or sum shit
		---------------------------------------------------  // use arrow keys to navigate, or use command line
				[Task 8]									 // >17 for example shows the calendar day mode for feb17
				[Task 9]
		---------------------------------------------------		
		22		 23		 24		 25		 26		 27		 28
		---------------------------------------------------
				[Task 10]

		Date Selected: Tue,February 17, 2025
		Task Due:	[Task 8]
					[Task 9]


		-year mode (current year), input --2027 if want new year or something
		===================================================
								2025
		===================================================
		   1       2       3       4       5       6  
		[ JAN ] [ FEB ] [ MAR ]	[ APR ] [ MAY ] [ JUN ] 	// same, just use red or green to color selected
		   7       8       9       10      11      12			 
		[ JUL ] [ AUG ] [ SEP ] [ OCT ] [ NOV ] [ DEC ]     // use arrows to navigate, or use cmd >jun or >6

		Month Selected: June, 2025
		Overview:   16 tasks:
		              4 done, 0 late, 0 partially done
	    -day mode
		----------------------------------------------------
					Tue, February 17, 2026
		----------------------------------------------------
		TIME			TASKS		  STATUS
		 6:00			
		 7:00
		 8:00
		 9:00
		10:00			[task1]			[x]
		11:00
		12:00
		13:00
		14:00			[task2]			[*]
		15:00
		16:00
		17:00
		18:00			[task3]			[ ]
		19:00
		20:00
		21:00
		22:00
		23:00
		24:00			[task4]			[ ]

		Selected task: Task2
		Some task information here
		[x] means done, [*] partially done, [ ] not done. either late or not yet done
		[l] means late  
[ ] = Not started
[*] = In progress/Partial
[x] = Complete
[!] = Overdue
[~] = Optional

============================================================
TASK MODE
============================================================

-- sample display!
	ITEM    	TASK      PRIORITY       STATUS      DEADLINE           TIME REMAINING
	000001   [title1]      HIGH           [ ]    2026-02-17 14:30      0-00-00  2:30:15    //time remaining updates every second
	000021   [title2]     NORMAL          [x]    2026-02-17 18:00	   0-00-00  6:00:15    //color code priority too
	000022	 [title3]      LOW			  [*]    2026-02-21 18:00	   0-00-04 18:00:00	   //for overdue, just display the negative	



>task add       // field
Task Name			:
Task Abbreviation   :				//max 8 chars
Task Priority		:				// urgent, high,normal,low,optional,none [6,5,4,3,2,1,0]
Task Status			:               //default to not done if left blank   (late,not done, partial, done) [-1,0,1,2]
Task Deadline	
		Date[D-M-Y] :				//DMY date. if no y, assume current year. 
		Time[H:M]   :				//time in hm, if h>12, assume military. if am/pm added, then do that. save to military anw
Category			:				//optional, leave blank if none
Notes				:				//optional, leave blank if none

Confirm?			:				//free to go back and edit. Enter saves it


// same with edit, present the information and allow the user to edit it
// Upon saving, generate a new json entry, id number is 1 more than the last. creation date is now, 
// completed is blank until status is marked "done", 

============================================================
FOCUS MODE
============================================================


	TIME REMAINING:
									0 y 00 m 01 d

				   #    #####         #   #    #           #####  #####
				  ##    #        #    #   #   ##      #    #   #  #
				   #    #####         #####    #           #   #  #####   // constantly refreshes
				   #    #   #    #        #    #      #    #   #  #   #
				  ###   #####             #   ###          #####  #####		
			
							      02-17-2026  13:21:15						// current time

	TASK NAME:
				[name 1]


tasks\>																		[ FOCUS ]

\\ when typed >exit, exit and go back to home

============================================================
============================================================
============================================================
============================================================
============================================================
former plan; take ideas from this, but renew; THIS IS OLD, REFER TO NEW
I want a python text based terminal based scheduler which has these tasks

Scheduler
	TIME:
	// big clock digital that matches local time and date // updates all the time
	FOCUS MODE:
		Select task (prompt user item number)
		Display Current time, Target time and time remaining
	TASKS:
		TODO
			ITEM 	TASK      PRIORITY       STATUS      DEADLINE      TIME REMAINING
THE TIME REMAINING FIELD IS CONSTANTLY UPDATED;
TIME REMAINING FOR OVERDUE IS --:--	

			EDIT 
		DONE
			TASK 	STATUS TIME DATE
	CALENDAR:
		SHOW CALENDAR WITH TASKS 

		===================================================
				February 2025
		Sun	 Mon     Tue	 Wed 	Thu	Fri	Sat
		---------------------------------------------------
		 1	  2	  3	  4	  5	  6	  7
		---------------------------------------------------
		[Task 1]  [Task 2]		[Task 4]
			  [Task 3]
		---------------------------------------------------
		 8	  9	 10	 11	 12	 13	 14
		---------------------------------------------------
			  [Task 5]
		          [Task 6]
			     *					    * means more tasks (max 3 task per date displayed
		---------------------------------------------------
		15	 16	 [17]	 18	 19	 20	 21
		---------------------------------------------------
				[Task 8]
				[Task 9]
		---------------------------------------------------		
		22	 23	 24	 25	 26	 27	 28
		---------------------------------------------------
			[Task 10]

		Date Selected: Tue,February 17, 2025
		Task Due:	[Task 8]
				[Task 9]


task edit history
commands:
hm - returns to home (shows the time and date)
task 
	default to todo list
	--todo - display todo list (update the time)
	--done - display the done tasks

logic: display item in order of priority and deadline date; deadline date takes precedence, then priority level

priority: urgent; important; [normal (change this]; non-urgent; optional;
status; done, partial, not started
when task is done, move it to the done task section
focus 
	focus mode
	prompt user todo item number 
	prompt user to type "DONE"

calendar: large calendar showing the current month and the tasks lined up for each date;

alright, now let's work on the task management. 

we have the add task, edit task, and remove task. This are prompted (have a parser for word 'task' and associated flags)
task --add 
task --edit
task --remove

if we are prompted to add task, ask:
Task Name
Priority (0-5), or urgent, optional, normal, important, urgent (interchangeable)
deadline:
	Date: MM/DD/YYYY (YYYY is optional, assume current year)
	Time: HH:MIN AM/PM (if HH higher than 12, assume military time)
In a separate file, make a dict where the following is recorded
	ITEM 	TASK      PRIORITY       STATUS      DEADLINE      TIME REMAINING

task = {
    "id": 1,
    "task": "Complete physics assignment",
    "priority": "urgent",  # urgent, important, normal, non-urgent, optional
    "status": "not_started",  # not_started, in_progress, done, partial
    "deadline": "2024-12-25 23:59",  # YYYY-MM-DD HH:MM format
    "created": "2024-11-20 10:00",
    "completed": None,  # Timestamp when done
    "estimated_hours": 2.5,
    "category": "Study",  # Optional: for filtering
    "notes": ""  # Optional notes field
}


Add that to the dict, and set status to Not Started. [Time remaining will be for later, set to --- for now]
status; done, partial, not started, late;

If prompted to edit: prompt which part to edit, then save changes. We can edit all fields. 
if prompted to remove: Prompt either the task name or item name.

If the status is Done, set it to done. If the status is late (current date is past deadline), set it to late. 
Now, after that, we have these commands

task --show
	Show all not done tasks. Order by: Date first, then by priority. task id will be set by its order on this tasks.
task --late 
	Filter by late (order by date and priority)
task --done
	Show all done (order by latest date) (id of these is negative number, ordered by -1 from latest to -[whatever number] on the oldest)
task --all
	Show all tasks, both done and not done and others
task --priority
	Filter by priority (late and not done, by prio, then by date)



if task more than 20, add ... at the end. 
if --all, order by creation date.

update command 
update once a minute.  

clock, timer (for set time), task timer
miniclock

====================== SUGGESTIONS
╔══════════════════════════════════════════════════════════════════════════════╗
║                                  HOME                                        ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                                                              ║
║    ┌─────────────────────────┐      ┌─────────────────────────────────────┐  ║
║    │                         │      │          TODAY'S TASKS              │  ║
║    │       ██╗  ██╗          │      ├─────────────────────────────────────┤  ║
║    │       ██║  ██║          │      │ ID │ TASK              │ STATUS     │  ║
║    │       ███████║ 14:23:45 │      │────┼───────────────────┼────────── ─│  ║
║    │       ╚════██║          │      │ 1  │ Complete physics  │ [URGENT]   │  ║
║    │            ██║          │      │ 2  │ Buy groceries     │ [normal]   │  ║
║    │            ╚═╝          │      │ 3  │ Call dentist      │ [LOW]      │  ║
║    │                         │      └─────────────────────────────────────┘  ║
║    │       Monday, June 15   │                                               ║
║    │       2026              │      ┌─────────────────────────────────────┐  ║
║    └─────────────────────────┘      │          OVERDUE TASKS              │  ║
║                                     ├─────────────────────────────────────┤  ║
║                                     │ ID │ TASK              │ DAYS LATE  │  ║
║                                     │────┼───────────────────┼────────────│  ║
║                                     │ 5  │ Submit report     │   2 days   │  ║
║                                     │ 8  │ Pay bills         │   5 days   │  ║
║                                     └─────────────────────────────────────┘  ║
║                                                                              ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  tasks\> _                                                        [ HOME ]   ║
╚══════════════════════════════════════════════════════════════════════════════╝

======== calendar layout
╔══════════════════════════════════════════════════════════════════════════════╗
║                               CALENDAR - June 2026					       ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                                                              ║
║    ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┐      ┌─────────────────────┐  ║
║    │ Sun │ Mon │ Tue │ Wed │ Thu │ Fri │ Sat │      │ Selected: 2026-06-17│  ║
║    ├─────┼─────┼─────┼─────┼─────┼─────┼─────┤      ├─────────────────────┤  ║
║    │     │  1  │  2  │  3  │  4  │  5  │  6  │      │ Tasks Due:          │  ║
║    ├─────┼─────┼─────┼─────┼─────┼─────┼─────┤      │ • Physics PS2       │  ║
║    │  7  │  8  │  9  │ 10  │ 11  │ 12  │ 13  │      │ • Meeting at 14:00  │  ║
║    ├─────┼─────┼─────┼─────┼─────┼─────┼─────┤      └─────────────────────┘  ║
║    │ 14  │ 15  │ 16  │ 17  │ 18  │ 19  │ 20  │                               ║
║    ├─────┼─────┼─────┼─────┼─────┼─────┼─────┤      Legend:                  ║
║    │ 21  │ 22  │ 23  │ 24  │ 25  │ 26  │ 27  │      [x] = Has tasks          ║
║    ├─────┼─────┼─────┼─────┼─────┼─────┼─────┤      [*] = Overdue tasks      ║
║    │ 28  │ 29  │ 30  │     │     │     │     │                               ║
║    └─────┴─────┴─────┴─────┴─────┴─────┴─────┘                               ║
║                                                                              ║
║    Commands: >next, >prev, >goto 17, >today                                  ║
║                                                                              ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  tasks\> _                                                        [CALENDAR] ║
╚══════════════════════════════════════════════════════════════════════════════╝

╔══════════════════════════════════════════════════════════════════════════════╗
║                               TASK LIST (42 total)                           ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                                                              ║
║  ┌────┬──────────────────────────┬──────────┬────────┬──────────────────┐    ║
║  │ ID │ TASK                     │ PRIORITY │ STATUS │ DEADLINE         │    ║
║  ├────┼──────────────────────────┼──────────┼────────┼──────────────────┤    ║
║  │ 1  │ Complete physics PS2     │ ███ URG  │ [ ]    │ 2026-06-17 23:59 │    ║
║  │ 2  │ Buy groceries            │ ██ HIGH  │ [x]    │ 2026-06-15 20:00 │    ║
║  │ 3  │ Call dentist             │ █ NORM   │ [*]    │ 2026-06-20 09:00 │    ║
║  │ 4  │ Clean room               │ ░ LOW    │ [ ]    │ 2026-06-22       │    ║
║  │ 5  │ Read chapter 5           │ ░ OPT    │ [-]    │ No deadline      │    ║
║  ├────┼──────────────────────────┼──────────┼────────┼──────────────────┤    ║
║  │                     Showing 5 of 42 tasks                            │    ║
║  └──────────────────────────────────────────────────────────────────────┘    ║
║                                                                              ║
║  Commands: >add, >edit 1, >done 3, >delete 2, >next, >prev                   ║
║                                                                              ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  tasks\> _                                                          [ TASK ] ║
╚══════════════════════════════════════════════════════════════════════════════╝


╔══════════════════════════════════════════════════════════════════════════════╗
║                               FOCUS MODE                                     ║
╠══════════════════════════════════════════════════════════════════════════════╣
║                                                                              ║
║                              ┌─────────────────┐                             ║
║                              │                 │                             ║
║                              │    02:15:33     │  (countdown timer)          ║
║                              │                 │                             ║
║                              └─────────────────┘                             ║
║                                                                              ║
║                              Current Task:                                   ║
║                              ┌─────────────────────────────────────────┐     ║
║                              │ 1. Complete physics PS2                 │     ║
║                              │    Deadline: 2026-06-17 23:59           │     ║
║                              │    Priority: URGENT                     │     ║
║                              └─────────────────────────────────────────┘     ║
║                                                                              ║
║                              Time: 14:23:45                                  ║
║                              Date: 2026-06-15                                ║
║                                                                              ║
║                              >pause  (pause timer)                           ║
║                              >done  (mark complete and exit)                 ║
║                              >quit  (exit without saving)                    ║
║                                                                              ║
╠══════════════════════════════════════════════════════════════════════════════╣
║  tasks\> _                                                         [ FOCUS ] ║
╚══════════════════════════════════════════════════════════════════════════════╝
