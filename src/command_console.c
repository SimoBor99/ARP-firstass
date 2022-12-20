// INCLUDING LIBRARIES

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

// FUNCTION FOR WRITING ON LOG FILE
double vx = 0.0;
double vz = 0.0;

// STUFF FOR THE RESET HANDLING
int n_motor_reset = 0;
int stop_reset = 0;
int resetting = 0;
int debugger = 0;

// FILE DESCRIPTORS
    	
int fd_cmx;
int fd_cmz;
int fd_cpmx;
int fd_cpmz;
int fd_ci;

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}
 void reset_handler(int signum) {
 	if (signum==SIGUSR2) {
 		if (resetting == 1) {
 			if (stop_reset != 0) {
 				debugger = 1;
			}
 		}
 		else {
 			vx=0.0;
 			vz=0.0;
 			char reset_x1[20];
			char reset_z1[20];
			sprintf(reset_x1,"%f",vx);
                	if (write(fd_cmx, reset_x1, 20) < 1) {
                		perror("Something wrong happened in writing!");
                	}
                	sprintf(reset_z1,"%f",vz);
                	if (write(fd_cmz, reset_z1, 20) < 1) {
                	   	perror("Something wrong happened in writing!");
                	}
                	resetting = 1;
 			while(1) {
 				if (stop_reset != 0) {
 					stop_reset = 0;
 					sleep(1);
					break;
				}
 				//write_log("RESETTING","log_command.txt");
 				if (n_motor_reset == 2) {
 					n_motor_reset = 0;
 					write_log("RESET COMPLETED","bebug_com.txt");
 					vx=0.0;
 					vz=0.0;
 					char reset_x[20];
					char reset_z[20];
					sprintf(reset_x,"%f",vx);
                			if (write(fd_cmx, reset_x, 20) < 1) {
                   				perror("Something wrong happened in writing!");
                			}
                			sprintf(reset_z,"%f",vz);
                			if (write(fd_cmz, reset_z, 20) < 1) {
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
 		resetting = 0;
 	}
 	else if (signum == SIGALRM) {
 		//write_log("EHILA","log_command.txt");
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
                if (write(fd_cmx, stop_x, 20) < 1) {
                    	perror("Something wrong happened in writing!");
                }
                sprintf(stop_z,"%f",vz);
                if (write(fd_cmz, stop_z, 20) < 1) {
                    	perror("Something wrong happened in writing!");
                }
                stop_reset = 1;
	}
}

//MAIN FUNCTION

int main(int argc, char const *argv[])
{
    	
    	char * filename = argv[1];
    	
    	char messx[20];
    	char messz[20];
    	
    	// PROCESS PID
    	
    	int ppid = getpid();
    	char ppid_str[20];
    	sprintf(ppid_str,"%d",ppid);
    	// Utility variable to avoid trigger resize event on launch
    	int first_resize = TRUE;

    	// Initialize User Interface 
    	init_console_ui();
    
	// TAKING THE ARGUMENT PASSED BY THE MASTER PROCESS
	
	/*char * filename = argv[1];
	char * com_mx = argv[2];
	char * com_mz = argv[3];*/
    
    	// WE OPEN THE PIPES
    	
    	fd_cmx = open(argv[2],O_WRONLY);
    	if (fd_cmx == -1) {
		perror("Error! Cannot open pipe!");
	}
    	fd_cmz = open(argv[3],O_WRONLY);
    	if (fd_cmz == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_ci = open(argv[4],O_WRONLY);
    	if (fd_ci == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_cpmx=open(argv[5], O_WRONLY);
	if (fd_cpmx== -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_cpmz=open(argv[6], O_WRONLY);
	if (fd_cpmz== -1) {
		perror("Error! Cannot open pipe!");
	}
	
	// WE SEND THE COMMAND PID TO THE TWO MOTORS, SO THAT WHEN THE TWO MOTORS FINISH THEIR RESET, THEY CAN SEND A SIGNAL TO THE COMMAND
	
	if (write(fd_cpmx, ppid_str, 20)==-1)
		perror("Something went wrong in writing"); 
	if (write(fd_cpmz, ppid_str, 20)==-1)
		perror("Something went wrong in writing");
		
    	struct sigaction s_stop, s_reset;
    	memset(&s_stop, 0, sizeof(s_stop));
    	memset(&s_reset, 0, sizeof(s_reset));
    	s_stop.sa_handler=&stop_handler;
    	s_reset.sa_handler=&reset_handler;
    	if (sigaction(SIGUSR1, &s_stop, NULL)==-1) { // SIGNAL FOR STOP BUTTON -> SET VELOCITIES TO ZERO
    		perror("Can't catch the signal");
    	}
    	if (sigaction(SIGALRM, &s_reset, NULL)==-1) { // SIGNAL FOR RESET COMPLETED -> COMMANDS REHABILITATED
    		perror("Can't catch the signal");
    	}
    	if (sigaction(SIGUSR2, &s_reset, NULL)==-1) // SIGNAL FOR RESET BUTTON -> COMMANDS BLOCKED
    		perror("Can't catch the signal");
    	
    	int check = 0;
    
    	// Infinite loop
    	while(TRUE)
		{
		if (stop_reset != 0) {
			stop_reset = 0;
		}
        	// Get mouse/resize commands in non-blocking mode...
        	int cmd = getch();

        	// If user resizes screen, re-draw UI
        	if(cmd == KEY_RESIZE) {
        		if(first_resize) {
                	first_resize = FALSE;
   	         	}
   	         	else {
   	             		reset_console_ui();
   	         	}
        	}
        	// Else if mouse has been pressed
        	else if(cmd == KEY_MOUSE) {

            	// Check which button has been pressed...
            	if(getmouse(&event) == OK) {

                	// Vx++ button pressed
                	if(check_button_pressed(vx_decr_btn, &event)) {
                	    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                	    refresh();
                	    vx = vx - 1.0;
                	    sprintf(messx,"%f",vx);
                	    if (write(fd_cmx, messx, strlen(messx)+1) < 1) {
                	    	perror("Something wrong happened in writing!");
                	    }
                	    write_log("Vx --",filename);
                	    sleep(1);
                	    for(int j = 0; j < COLS; j++) {
                	        mvaddch(LINES - 1, j, ' ');
                	    }
                	}

                	// Vx-- button pressed
                	else if(check_button_pressed(vx_incr_btn, &event)) {
                	    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                	    refresh();
                	    vx = vx + 1.0;
                	    sprintf(messx,"%f",vx);
                	    if (write(fd_cmx, messx, strlen(messx)+1) < 1) {
                	    	perror("Something wrong happened in writing!");
                	    }
                	    write_log("Vx ++",filename);
                	    sleep(1);
               		    for(int j = 0; j < COLS; j++) {
                	        mvaddch(LINES - 1, j, ' ');
                	    }
                	}

                	// Vx stop button pressed
                	else if(check_button_pressed(vx_stp_button, &event)) {
                	    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                	    refresh();
                	    vx = 0.0;
                	    sprintf(messx,"%f",vx);
                	    if (write(fd_cmx, messx, strlen(messx)+1) < 1) {
                	    	perror("Something wrong happened in writing!");
                	    }
                	    write_log("Vx stop",filename);
                	    sleep(1);
                	    for(int j = 0; j < COLS; j++) {
                	        mvaddch(LINES - 1, j, ' ');
                	    }
                	}

                	// Vz++ button pressed
                	else if(check_button_pressed(vz_decr_btn, &event)) {
                	    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                	    refresh();
                	    vz = vz - 1.0;
                	    sprintf(messz,"%f",vz);
                	    if (write(fd_cmz, messz, strlen(messz)+1) < 1) {
                	    	perror("Something wrong happened in writing!");
                	    }
                	    write_log("Vz--",filename);
                	    sleep(1);
                	    for(int j = 0; j < COLS; j++) {
                	        mvaddch(LINES - 1, j, ' ');
                	    }
                	}

                	// Vz-- button pressed
                	else if(check_button_pressed(vz_incr_btn, &event)) {
                	    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                	    refresh();
                	    vz = vz + 1.0;
                	    sprintf(messz,"%f",vz);
                	    if (write(fd_cmz, messz, strlen(messz)+1) < 1) {
                	    	perror("Something wrong happened in writing!");
                	    }
                	    write_log("Vz++",filename);
                	    sleep(1);
	                    for(int j = 0; j < COLS; j++) {
	                        mvaddch(LINES - 1, j, ' ');
        	            }
        	        }

        	        // Vz stop button pressed
        	        else if(check_button_pressed(vz_stp_button, &event)) {
        	            mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
        	            refresh();
        	            vz = 0.0;
                	    sprintf(messz,"%f",vz);
                	    if (write(fd_cmz, messz, strlen(messz)+1) < 1) {
                	    	perror("Something wrong happened in writing!");
                	    }
                	    write_log("Vz stop",filename);
        	            sleep(1);
        	            for(int j = 0; j < COLS; j++) {
        	                mvaddch(LINES - 1, j, ' ');
        	            }
        	        }
        	    }
        	}
        	if (vx != 0.0) {
        		char t[20];
        		sprintf(t,"%f",vx);
        		write_log(t,"bebug_com.txt");	
        	}
        	// WE TAKE THE TWO MOTORS' PIDS, PASSED BY THE MASTER
        	
		if (check == 0) {
			if(write(fd_ci, ppid_str, 20) < 1) {
				perror("Error in write!");
			}
			check++;
		}
        	refresh();
	}
	
	// WE CLOSE THE PIPES
	
	if(close(fd_cmx) == -1) {
			perror("Error! Cannot close pipe!");
		}
	if(close(fd_cmz) == -1) {
			perror("Error! Cannot close pipe!");
		}
	if(close(fd_ci) == -1) {
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
