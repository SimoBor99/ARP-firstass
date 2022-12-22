// including libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>
#include <math.h>
#include <signal.h>

double velx=0.0;
double posx = 0.0;
double old = 0.0;
double delta=pow(30, -1);
char pid_cmd[20];
int pidcmd;
int fd_mxw;
int stop_reset = 0;
char velx_str[20];
char posx_str[20];
int resetting = 0;
int debugger = 0;

// function for writing on log file
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

// stop_handler
void stop_handler(int signum) {
	
	if(signum==SIGUSR1) {

		// set the velocity to 0
		velx=0.0;
		char t[20];
		sprintf(t, "%f", velx);
		stop_reset = 1;
	}
}

void reset_handler(int signum) {

	// if we are already resetting, we do nothing
	if (resetting == 1) {
		sleep(2);
		if (stop_reset == 1) {
			debugger = 1;
		}
	}

	// if no resetting is ongoing, we start the reset
	else {
		sleep(2);
		resetting = 1;
		velx=1.0;
		sprintf (posx_str, "%f", posx);
		if (write(fd_mxw, posx_str, 20)==-1) {
			perror("Cannot write!");
		}

		// if the motor is not in the zero position, we start moving towards that position
		if (posx!=0.0) {
			while (posx>=0.0) {
				if (stop_reset != 0) {
					break;
				}

				// we calculate the new position and send it to the world process
				posx=posx-(velx*delta);
				sprintf (posx_str, "%f", posx);
				if (write(fd_mxw, posx_str, 20)==-1) {
					perror("Cannot write!");
				}
				if (debugger == 1) {
					debugger = 0;
					velx = 0.0;
					break;
				}
				usleep(30000);
			}

			// if a stop has been pressed during reset, the reset is cancelled
			if (stop_reset != 0) {
				stop_reset = 0;
				velx = 0.0;
				sleep(1);
			}

			// the reset has finished, so we send a sigalrm to the command process
			else {
				velx = 0.0;
				posx=0.0;
				sprintf (posx_str, "%f", posx);
				if (write(fd_mxw, posx_str, 20)==-1) {
					perror("Cannot write!");
				}
				if (kill(pidcmd, SIGALRM)==-1) {
					perror("Cannot send the signal");
				}
				sleep(1);
			}
		}

		// we are already at the zero position, so we send a sigalrm to the command process
		else {
			velx = 0.0;
			posx=0.0;
			sprintf (posx_str, "%f", posx);
			if (write(fd_mxw, posx_str, 20)==-1) {
				perror("Cannot write!");
			}
			if (kill(pidcmd, SIGALRM)==-1) {
				perror("Cannot send the signal");
			}
			sleep(1);
		}
	}
}

// main function
int main(int argc, char const *argv[]) {

	const char * filename = argv[1];

	char *eptr;
	const double delta = pow(30,-1);
	char debug[20];

	// variables for the select function
	struct timeval tv={0,0};
	int ret;
	fd_set rset;

	// file descriptors
	int fd_cmx;
	int fd_cpmx;

	// we open the pipes
	// open file descriptor from command to mx
	fd_cmx = open(argv[2],O_RDONLY);
	if (fd_cmx ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[2]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	// open file descriptor from mx to world
	fd_mxw = open(argv[3],O_WRONLY);
	if (fd_mxw ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[3]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	// open file descriptor from command to mx, but for receiving pid
	fd_cpmx = open(argv[4],O_RDONLY);
	if (fd_cpmx ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[4]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	if (read(fd_cpmx, pid_cmd, 20)==-1)  {
		perror("Something wrong in reading");
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	// extract the command pid;
	pidcmd=atoi(pid_cmd);

	// define two signal, for stop and reset
	struct sigaction s_stop, s_reset;
	memset(&s_stop, 0, sizeof(s_stop));
	memset(&s_reset, 0, sizeof(s_reset));
    s_stop.sa_handler=&stop_handler;
    s_reset.sa_handler=&reset_handler;

	// check if the two signal handler are fine
    if (sigaction(SIGUSR1, &s_stop, 0)==-1) {
    	perror("Can't catch the signal");
	}
    if (sigaction(SIGUSR2, &s_reset, NULL)==-1) {
    	perror("Can't catch the signal");
	}
	
	while(1) {
		if (stop_reset != 0) {
			stop_reset = 0;
		}
		if (resetting != 0) {
			resetting = 0;
		}
		FD_ZERO(&rset);
        FD_SET(fd_cmx, &rset);
        ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

		if (ret == -1) {
			perror("Error in select()!");
		}

        // if the pipe is ready, we read from it
		else if(ret>=0){
			if (FD_ISSET(fd_cmx, &rset)>0){
				read(fd_cmx, velx_str, 20);
			}
			velx = strtod(velx_str,&eptr);
		}
		
		// we calculate the new position
		old = posx;
		posx = posx + (velx * delta);

		// if we are out of bounds, we set the position to the minimum/maximum possible position value
		if (posx < 0.0) {
			posx = 0.0;
		}
		if (posx > 39.0) {
			posx = 39.0;
		}
		
		if (posx != old) {
			write_log("Current position changed!",filename);
		}
		
		// now we send the position to the world process
		sprintf(posx_str,"%f",posx);
		if (!write(fd_mxw, posx_str, strlen(posx_str)+1)) {
        		perror("Somenthing wrong in writing");
		}
		
		usleep(30000);
	}
	
	// we close the pipes
	if(close(fd_cmx) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_mxw) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_cpmx) == -1) {
		perror("Error! Cannot close pipe!");
	}
	return 0;
}
