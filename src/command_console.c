// including libraries
#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <signal.h>

double vx = 0.0;
double vz = 0.0;

// variables for reset handling
int n_motor_reset = 0;
int stop_reset = 0;
int resetting = 0;
int debugger = 0;

// file descriptors  	
int fd_cmx;
int fd_cmz;
int fd_cpmx;
int fd_cpmz;
int fd_ci;

//function for writing in log file
void write_log(char * log_text, const char * fn) {
	
	FILE *fp_log;
	fp_log = fopen(fn,"a"); 
	if (fp_log==NULL) {
		perror("Cannot read the file"); 
	} 
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	fclose(fp_log);
}

//function fo unable the input from user
 void reset_handler(int signum) {
 	
	// if signal is sigurs2, the reset button has been pressed on the inspection
	if (signum==SIGUSR2) {

		// if a resetting is ongoing, we don't do anything
 		if (resetting == 1) {
 			if (stop_reset != 0) {
 				debugger = 1;
			}
 		}

		// if no reset is ongoing, we start the reset procedure
 		else {
 			vx=0.0;
 			vz=0.0;
 			char reset_x1[20];
			char reset_z1[20];
			sprintf(reset_x1,"%f",vx);

			// we send a velocity equal to zero to the motors 
            if (write(fd_cmx, reset_x1, 20)==-1) {
                perror("Something wrong happened in writing!");
            }
            sprintf(reset_z1,"%f",vz);
            if (write(fd_cmz, reset_z1, 20)==-1) {
                perror("Something wrong happened in writing!");
            }
            resetting = 1;
 			while(1) {

				// if a stop has been pressed during the reset, we cancel the reset
 				if (stop_reset!= 0) {
 					stop_reset = 0;
 					sleep(1);
					break;
				}

				// if both motors have finished, we terminate the reset and send a velocity equal to zero to the motors
 				if (n_motor_reset == 2) {
 					n_motor_reset = 0;
 					vx=0.0;
 					vz=0.0;
 					char reset_x[20];
					char reset_z[20];
					sprintf(reset_x,"%f",vx);
                	if (write(fd_cmx, reset_x, 20)==-1) {
                   		perror("Something wrong happened in writing!");
                	}
                	sprintf(reset_z,"%f",vz);
                	if (write(fd_cmz, reset_z, 20)==-1) {
                    	perror("Something wrong happened in writing!");
                	}
                	break;
 				}
 				if (debugger == 1) {
 					debugger = 0;
 					break;
 				}
 			}
 		}
 	}

	// if signal is sigalrm, a motor has terminated the reset
 	else if (signum == SIGALRM) {
 		n_motor_reset++;
 	}
 }

void stop_handler(int signum) {
	
	if(signum==SIGUSR1) {
		vx=0.0;
		vz=0.0;
		char stop_x[20];
		char stop_z[20];
		sprintf(stop_x,"%f",vx);

		//send vx to command
        if (write(fd_cmx, stop_x, 20)==-1) {
            perror("Something wrong happened in writing!");
        }
        sprintf(stop_z,"%f",vz);

		//send vz to command
        if (write(fd_cmz, stop_z, 20)==-1) {
            perror("Something wrong happened in writing!");
        }
        stop_reset = 1;
	}
}


// main function
int main(int argc, char const *argv[]) {
    	
    const char * filename = argv[1];
    	
    char messx[20];
    char messz[20];
    	
    // process pid
    int ppid = getpid();
    char ppid_str[20];
    sprintf(ppid_str,"%d",ppid);

    // utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // initialize User Interface 
    init_console_ui();

	// we open the file descriptors and check if they are valid
	// open file descriptor from command to mx
    fd_cmx = open(argv[2],O_WRONLY);
    if (fd_cmx ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[2]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	// open file descriptor from command to mz
    fd_cmz = open(argv[3],O_WRONLY);
    if (fd_cmz ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[3]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	
	// open file descriptor from command to inspection
	fd_ci = open(argv[4],O_WRONLY);
    	if (fd_ci ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[4]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	
	// open file descriptor from command to mx, but now for sending pid
	fd_cpmx=open(argv[5], O_WRONLY);
	if (fd_cpmx==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[5]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	
	// open file descriptor from command to mz, but now for sending pid
	fd_cpmz=open(argv[6], O_WRONLY);
	if (fd_cpmz==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[6]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	
	// we send the command pid to the two motors, so that when the two motors finish their reset, they can send a signal to the command
	if (write(fd_cpmx, ppid_str, 20)==-1) {
		perror("Something went wrong in writing"); 
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	if (write(fd_cpmz, ppid_str, 20)==-1) {
		perror("Something went wrong in writing");
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
		
    struct sigaction s_stop, s_reset;
    memset(&s_stop, 0, sizeof(s_stop));
    memset(&s_reset, 0, sizeof(s_reset));
    s_stop.sa_handler=&stop_handler;
    s_reset.sa_handler=&reset_handler;

	// signal for stop button -> set velocities to zero
    if (sigaction(SIGUSR1, &s_stop, NULL)==-1) { 
    	perror("Can't catch the signal");
    }
    
	// signal for reset completed -> commands rehabilitated
	if (sigaction(SIGALRM, &s_reset, NULL)==-1) { 
    	perror("Can't catch the signal");
    }
    
	// signal for reset button -> command blocked
	if (sigaction(SIGUSR2, &s_reset, NULL)==-1) {
    	perror("Can't catch the signal");
	}
    	
    int check = 0;
    
    // infinite loop
    while(TRUE) {
		if (stop_reset != 0) {
			stop_reset = 0;
		}

        // get mouse/resize commands in non-blocking mode...
        int cmd = getch();
		if (resetting != 0) {
			sleep(2);
			resetting = 0;
			continue;
		}

        // if user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
        	if(first_resize) {
                first_resize = FALSE;
   	    	}
   	        else {
   	            reset_console_ui();
   	         	}
        }

        // else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

        // check which button has been pressed...
        if(getmouse(&event) == OK) {

        // vx-- button pressed
        if(check_button_pressed(vx_decr_btn, &event)) {
            mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
            refresh();
						
			// decrease the x velocity
            vx = vx - 1.0;
            sprintf(messx,"%f",vx);
						
			// send vx to mx
            if (write(fd_cmx, messx, strlen(messx)+1)==-1) {
                perror("Something wrong happened in writing!");
            }
						
			// write on log file
            write_log("Vx --",filename);
            sleep(1);
            for(int j = 0; j < COLS; j++) {
            	 mvaddch(LINES - 1, j, ' ');
            }
        }

        // vx++ button pressed
        else if(check_button_pressed(vx_incr_btn, &event)) {
            mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
            refresh();
						
			// increase the x velocity
            vx = vx + 1.0;
            sprintf(messx,"%f",vx);
						
			// send vx to mx
            if (write(fd_cmx, messx, strlen(messx)+1)==-1) {
                perror("Something wrong happened in writing!");
            }
						
			// write on log file
            write_log("Vx ++",filename);
            sleep(1);
            for(int j = 0; j < COLS; j++) {
                mvaddch(LINES - 1, j, ' ');
            }
        }

        // vx stop button pressed
        else if(check_button_pressed(vx_stp_button, &event)) {
            mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
            refresh();
						
			// stop the motor x, by setting vx to 0
            vx = 0.0;
            sprintf(messx,"%f",vx);
						
			// send vx to mx
            if (write(fd_cmx, messx, strlen(messx)+1)==-1) {
                perror("Something wrong happened in writing!");
            }
						
			// write on log file
            write_log("Vx stop",filename);
            sleep(1);
            for(int j = 0; j < COLS; j++) {
                 mvaddch(LINES - 1, j, ' ');
            }
                	}

                	// vz--button pressed
                	else if(check_button_pressed(vz_decr_btn, &event)) {
                	    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                	    refresh();
						
						// decrease z velocity
                	    vz = vz - 1.0;
                	    sprintf(messz,"%f",vz);
						
						// send vz to mz
                	    if (write(fd_cmz, messz, strlen(messz)+1)==-1) {
                	    	perror("Something wrong happened in writing!");
                	    }
						
						// write on log file
                	    write_log("Vz--",filename);
                	    sleep(1);
                	    for(int j = 0; j < COLS; j++) {
                	        mvaddch(LINES - 1, j, ' ');
                	    }
                	}

                	// vz++ button pressed
                	else if(check_button_pressed(vz_incr_btn, &event)) {
                	    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                	    refresh();
						
						// increase z velocity
                	    vz = vz + 1.0;
                	    sprintf(messz,"%f",vz);
						
						// send vz to mz
                	    if (write(fd_cmz, messz, strlen(messz)+1)==-1) {
                	    	perror("Something wrong happened in writing!");
                	    }
						
						// write on log file
                	    write_log("Vz++",filename);
                	    sleep(1);
	                    for(int j = 0; j < COLS; j++) {
	                        mvaddch(LINES - 1, j, ' ');
        	            }
        	        }

        	        // vz stop button pressed
        	        else if(check_button_pressed(vz_stp_button, &event)) {
        	            mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
        	            refresh();
						
						// stop the motor z, by setting vz to 0
        	            vz = 0.0;
                	    sprintf(messz,"%f",vz);

						// send vz to mz
                	    if (write(fd_cmz, messz, strlen(messz)+1)==-1) {
                	    	perror("Something wrong happened in writing!");
                	    }

						// write on log file
                	    write_log("Vz stop",filename);
        	            sleep(1);
        	            for(int j = 0; j < COLS; j++) {
        	                mvaddch(LINES - 1, j, ' ');
        	            }
        	        }
        	    }
			}
        	
			if (check == 0) {
				if(write(fd_ci, ppid_str, 20)==-1) {
					perror("Error in write!");
					if (kill(getppid(), SIGUSR1)==-1) {
						perror("Cannot send signal!");
						exit(1);
					}
				}
				if(close(fd_ci) == -1) {
					perror("Error! Cannot close pipe!");
				}
				check++;
			}
        	refresh();
		}
	
	// we close the pipes
	if(close(fd_cmx) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_cmz) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_cpmx) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_cpmz) == -1) {
		perror("Error! Cannot close pipe!");
	}
	
    // Terminate
    endwin();
    return 0;
}
