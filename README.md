# ARP First assignment

Installing and running
----------------------------------------------
Before running the program with **shell file**, it is fundamental to install one library:
**lncurses**.

For doing that, you can run on the terminal the following command:

``` sudo apt-get install libncurses-dev```

After doing that, it is the time to create the binary of all 6 processes and run the programm. Before running it, you need to make **first_ass.sh** executable, by using on the terminal:

``` chmod u+x first_ass.sh```

You can find **first_ass.sh** into ```add_materials``` folder, but, when launched, this file must be in the same folder of the project. 

Then launch it to create all binary and execute the program, by the command:

```./first_ass.sh```

Once you have done this, the program is successfully launched!

Description of the program
----------------------------------------------
This program simulates a simple hoist with 2 degrees of freedom.
The hoist can move horizonthally and vertically, in a range between 0-39 (horizonthally) and 0-9 (vertically).
After starting, the program spawns two console windows: the command console and the inspection console.
In the command console, the user can input some command by pressing one of the 6 buttons present on the interface. These commands can increase, decrease or stop the velocity of the along one of the two axis where the hoist is able to move.
In the inspection console, the user can see where the hoist end-effector is placed in space and its coordinates. The user has also the possibility to press 2 buttons: the reset button, which takes the hoist back to its original position, and the stop button, which immediately stops the hoist movement.
Last but not least, if the state of the program doesn't change (hoist doesn't move, user doesn't press any button) for 1 minute, the program will automatically terminate its own execution.

Development of the program
----------------------------------------------

The program is developed with 6 different processes:

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
If the stop button is pressed, a signal is sent to the command process and to the 2 motors processes, which set the velocities of both the x and z axis to 0 in the 3 processes.
If the reset button is pressed, a signal is sent to the command process and to the 2 motor processes. The signal sent to the command process blocks the input given by the user, so the command process enter a while loop which in which it does nothing, until the reset procedure has concluded. The signal sent to the two motors processes makes them enter into a while loop, in which the 2 coordinates of the end-effector are continously checked and if the position is different from 0, a negative velocity is set and mantained, until it reaches 0.
After both motors have reached the 0 position, the 2 velocities are set to 0 and the command console inputs are rehabilitated.
In the loop, this process also uses the select() instruction the check if something is readable from the world process (there are 2 pipes, because the world process send to the inspection process both the x position and the z position) and if it is the case, the value(s) position read are changed.
To conclude, the process updates the graphic interface the end of every loop.

* **Motor Processes**
These 2 processes are identical, so they are included in the same section.
These processes are not opened in konsole, so they are invisible to the user. They have to read the current velocities of hoist, which are setted by the user thanks to the grapichal interface of **Inspection** process. Then they have to calculate the ideal position of end effector, and send it to the world process; actually the position are two: one for the x axis and another for the z axis.
There are also signal handler implemented here, in order to manage the signals sent by the **inspection** process.

* **world process**
This process takes the position values for the x-axe and the z-axe from the motors processes (a select() is used to do so). Then, this process add to these two positions a little random error, in order to simulate better a real-world problem, where the theorethical value and the real value are usually not exactly the same. After that, the two real position values are sent through a pipe to the **inspection** process, which will take these values and display them into its graphical interface.
