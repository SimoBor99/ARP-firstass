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

* **Command Console Process**
This process shows a graphic interface with the 6 input buttons mentioned above. The process initailize the velocities of the x and z axis at 0. Then, it passes its PID to the inspection console process, which will be used to implement signals between the 2 processes. After this, the process enters in and endless loop, where it checks if a button has been pressed and, if that is the case, it increments the correspondent velocity (x or z) and sends it to its correspondent process (the motor x process or the motor z process).
In this process is also implemented a signal handler, which reset to 0 both velocities.
This process write in its log file whenever a button is pressed.

* **Inpection Console Process**
This process shows a graphic interface with the hoist end-effector, its coordinates and the 2 emergency buttons mentioned above. The process is an infinite loop, in which the process check if one of the two emergency buttons has been pressed:

- If the stop button is pressed, a signal is sent to the command process and to the 2 motors processes, which set the velocities of both the x and z axis to 0 in the 3 processes.

- If the reset button is pressed, a signal is sent to the command process and to the 2 motor processes. The signal sent to the command process blocks the input given by the user, so the command process enter a while loop which in which it does nothing, until the reset procedure has concluded. The signal sent to the two motors processes makes them enter into a while loop, in which the 2 coordinates of the end-effector are continously checked and if the position is different from 0, a negative velocity is set and mantained, until it reaches 0.

After both motors have reached the 0 position, the 2 velocities are set to 0 and the command console inputs are rehabilitated.

In the loop, this process also uses the select() instruction the check if something is readable from the world process (there are 2 pipes, because the world process send to the inspection process both the x position and the z position) and if it is the case, the value(s) position read are changed.
To conclude, the process updates the graphic interface the end of every loop.

* **Motor Processes**
These 2 processes are identical, so they are included in the same section.
These processes are not opened in konsole, so they are invisible to the user. They 