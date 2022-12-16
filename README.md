# ARP First assignment

Installing and running
----------------------------------------------
Before running the program with **konsole file**, it is fundamental to install one library:
*lncurses.

For doing that, you can run on the terminal the following command:

``` sudo apt-get install libncurses-dev```

After doing that, it is the time to crete the binary of all 6 processes, because the script konsole can be executed if and only if there are the binary files (which are the executables).
Then, the program is ready to be run. All you need to do is going with the terminal into the folder where you put the bins and type the command:

``` ./master```

And the program will start!

Description of the program
----------------------------------------------
This program simulates a simple hoist with 2 degrees of freedom.
The hoist can move horizonthally and vertically, in a range between 0-40 (horizonthally) and between 0-10 (vertically).
After starting, the program spawns two console windows: the command console and the inspection console.
In the command console, the user can input some command by pressing one of the 6 buttons present on the interface. These commands cen increase, decrease or stop the velocity of the along one of the two axis where the hoist is able to move.
In the inspection console, the user can see where the hoist end-effector is placed in space and its coordinates. The user has also the possibility to press 2 buttons: the reset button, which takes the hoist back to its original position, and the stop button, which immediately stops the hoist movement.
Last but not least, if the state of the program doesn't change (hoist doesn't move, user doesn't press any button) for 1 minute, the program will automatically terinate its own execution.

Deployment of the program
----------------------------------------------

The program is deployed with 6 different processes:

* **Master Process**
This is the process which starts everything: after starting its execution, this process spawns 5 other processes, passing them the arguments they need for communicating through each other and for writing on their log files.
After that, the process starts to control the last modify time of all the log files of the others 5 processes. Then, the process takes the current time and compares it with the 5 modify times.
If every log file has been modified more than 1 minute ago, this process sends a kill signal to all the other processes, unlink all the pipes used during the execution and then terminates itself.

