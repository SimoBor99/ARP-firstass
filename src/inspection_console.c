// INCLUDING LIBRARIES

#include "./../include/inspection_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>
#include <signal.h>

// FUNCTION FOR WRITING ON LOG FILE

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	fclose(fp_log);
}

// MAIN FUNCTION

int main(int argc, char const *argv[])
{

	char *eptr;
	char posx_str[20];
	char posz_str[20];
	char *pidmx;
	char *pidmz;
	char pidcom[20];
	char pid_tmp[20] = "";
	int mx;
	int mz;
	int com;
	int check = 0;
	int check2 = 0;

    	// Utility variable to avoid trigger resize event on launch
    	int first_resize = TRUE;

    	// End-effector coordinates
    	float ee_x, ee_y;
    	float oldx, oldz;
    	
    	// FILE DESCRIPTORS
    	int fd_wix;
    	int fd_wiz;
    	int fd_mi;
    	int fd_ci;

	// STUFF NEEDED FOR THE SELECT FUNCTION

	int ret;
	struct timeval tv={0,0};
	fd_set rset;

    	// Initialize User Interface 
    	init_console_ui();

	// TAKING THE ARGUMENTS PASSED BY THE MASTER PROCESS

	char * filename = argv[1];
	/*char * world_insp_x = argv[2];
	char * world_insp_z = argv[3];
	char * master_insp = argv[4];*/

	// WE OPEN THE PIPES, CHECKING IF EVERYTHING GOES AS IT SHOULD
        	
        fd_wix = open(argv[2],O_RDONLY);
        if (fd_wix == -1) {
		perror("Error! Cannot open pipe!");
	}
        fd_wiz = open(argv[3],O_RDONLY);
        if (fd_wiz == -1) {
		perror("Error! Cannot open pipe!");
	}
	
	fd_mi = open(argv[4],O_RDONLY);
	if (fd_mi == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_ci = open(argv[5],O_RDONLY);
	if (fd_ci == -1) {
		perror("Error! Cannot open pipe!");
	}
	int i=15180;
    	// Infinite loop
    	while(TRUE)
		{
			
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

                	// STOP button pressed
                		if(check_button_pressed(stp_button, &event)) {
                    			mvprintw(LINES - 1, 1, "STP button pressed");
                    			refresh();
                    			sleep(1);
                    			if (kill(mx, SIGUSR1)==-1)
                    				perror("Cannot send signal");
                    			if (kill(mz, SIGUSR1)==-1)
                    				perror("Cannot send signal");
                    			if (kill(com, SIGUSR1)==-1)
                    				perror("Cannot send signal");
                    			for(int j = 0; j < COLS; j++) {
                        			mvaddch(LINES - 1, j, ' ');
                    			}
                		}

                	// RESET button pressed
                		else if(check_button_pressed(rst_button, &event)) {
                    			mvprintw(LINES - 1, 1, "RST button pressed");
                    			refresh();
                    			sleep(1);
                    			for(int j = 0; j < COLS; j++) {
                        			mvaddch(LINES - 1, j, ' ');
                    			}
                    			if (kill(mx, SIGUSR2)==-1)
                    				perror("Cannot send signal");
                    			if (kill(mz, SIGUSR2)==-1)
                    				perror("Cannot send signal");
                    			if (kill(com, SIGUSR2)==-1)
                    				perror("Cannot send signal");
                    				
                		}
            		}
        	}
        	
        	// WE TAKE THE COMMAND PID, PASSED BY THE COMMAND PROCESS
        	
		if (check2 == 0) {
			if(read(fd_ci, pidcom, 20) < 1) {
				perror("Error in reading");
			}
			com=atoi(pidcom);
			check2++;
		}
		
        	// WE TAKE THE TWO MOTORS' PIDS, PASSED BY THE MASTER
		if (check == 0) {
			if(read(fd_mi, pid_tmp, 20) < 1) {
				perror("Error in reading!");
			}
			pidmx=strtok(pid_tmp,",");
			pidmz=strtok(NULL, ",");
			mx=atoi(pidmx);
			mz=atoi(pidmz);
			check++;
		}
		
        	oldx = ee_x;
        	oldz = ee_y;
        
        	// NOW, WE USE THE SELECT TO READ FROM BOTH PIPES
        	
        	FD_ZERO(&rset);
        	FD_SET(fd_wix, &rset);
        	FD_SET(fd_wiz, &rset);
        	FD_SET(0,&rset);
        	ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);
        	
        	// IF THERE'S SOMETHING TO READ, WE READ IT!
        	
        	if (ret > 0) {
        		if (FD_ISSET(fd_wix, &rset)>0){
				read(fd_wix, posx_str, 20);
				ee_x = strtod(posx_str,&eptr);		// WE UPDATE THE X OF EE
				if (ee_x != oldx) {
					write_log("Current horizonthal position changed!",filename);
				}
			}

			if (FD_ISSET(fd_wiz, &rset)>0){
				read(fd_wiz, posz_str, 20);
				ee_y = strtod(posz_str,&eptr);		// WE UPDATE THE Z OF EE
				if (ee_y != oldz) {
					write_log("Current vertical position changed!",filename);
				}
			}
			
        	}
        	
        	
        	// Update UI
        	update_console_ui(&ee_x, &ee_y);
	}
	
	// WE CLOSE THE PIPES
        	
        if(close(fd_wix) == -1) {
			perror("Error! Cannot close pipe!");
		}
        if(close(fd_wiz) == -1) {
			perror("Error! Cannot close pipe!");
		}
	if(close(fd_mi) == -1) {
			perror("Error! Cannot close pipe!");
	}
	if(close(fd_ci) == -1) {
			perror("Error! Cannot close pipe!");
	}

    // Terminate
    endwin();
    return 0;
}
