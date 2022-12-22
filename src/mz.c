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

double velz=0.0;
double posz = 0.0;
double old = 0.0;
double delta=pow(30, -1);
char pid_cmd[20];
int pidcmd;
int fd_mzw;
int stop_reset = 0;
char velz_str[20];
char posz_str[20];
int resetting = 0;
int debugger = 0;

// function for writing on log file
void write_log(char * log_text, const char * fn) {
	
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	if (fp_log==NULL)  {
		perror("Cannot read the file"); 
	}
	fputs(log_text, fp_log);
	fputs("\n", fp_log); 
	fclose(fp_log);
}

//stop handler
void stop_handler(int signum) {
	if(signum==SIGUSR1) {

		//set the velocity to 0
		velz=0.0;
		char t[20];
		sprintf(t, "%f", velz);
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
		velz=1.0;
		sprintf (posz_str, "%f", posz);
		if (write(fd_mzw, posz_str, 20)==-1) {
			perror("Cannot write!");
		}

		// if the motor is not in the zero position, we start moving towards that position
		if (posz!=0.0) {
			while (posz>=0.0) {
				if (stop_reset != 0) {
					break;
				}

				// we calculate the new position and send it to the world process
				posz=posz-(velz*delta);
				sprintf (posz_str, "%f", posz);
				if (write(fd_mzw, posz_str, 20)==-1) {
					perror("Cannot write!");
				}
				if (debugger == 1) {
					debugger = 0;
					velz = 0.0;
					break;
				}
				usleep(30000);
			}

			// if a stop has been pressed during reset, the reset is cancelled
			if (stop_reset != 0) {
				//resetting = 0;
				stop_reset = 0;
				velz = 0.0;
				sleep(1);
			}

			// the reset has finished, so we send a sigalrm to the command process
			else {
				velz = 0.0;
				posz=0.0;
				sprintf (posz_str, "%f", posz);
				if (write(fd_mzw, posz_str, 20)==-1) {
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
			velz = 0.0;
			posz=0.0;
			//resetting = 0;
			sprintf (posz_str, "%f", posz);
			if (write(fd_mzw, posz_str, 20)==-1) {
				perror("Cannot write!");
			}
			sleep(1);
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

	// stuff for select
	struct timeval tv={0,0};
	int ret;
	fd_set rset;

	// file descriptors
	int fd_cmz;
	int fd_cpmz;


	// we open the pipes
	// open file descriptor from command to mz	
	fd_cmz = open(argv[2],O_RDONLY);
	if (fd_cmz == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[2]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	// open file descriptor from mz to world
	fd_mzw = open(argv[3],O_WRONLY);
	if (fd_mzw == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[3]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	// open file descriptor from command to mz, but for receiving pid
	fd_cpmz = open(argv[4],O_RDONLY);
	if (fd_cpmz == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[4]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	if (read(fd_cpmz, pid_cmd, 20)==-1) {
		perror("Something wrong in reading");
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	
	// extract the command pid
	pidcmd=atoi(pid_cmd);
	
	// define two signal, for stop and reset
	struct sigaction s_stop, s_reset;
	memset(&s_stop, 0, sizeof(s_stop));
	memset(&s_reset, 0, sizeof(s_reset));
    s_stop.sa_handler=&stop_handler;
    s_reset.sa_handler=&reset_handler;
	
	// check if two signal handlers are fine
    if (sigaction(SIGUSR1, &s_stop, NULL)==-1) {
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
        	FD_SET(fd_cmz, &rset);
        	ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

		if (ret == -1) {
			perror("Error in select()!");
		}

		// if the pipe is ready, we read from it
		else if(ret>=0){
			if (FD_ISSET(fd_cmz, &rset)>0){
				read(fd_cmz, velz_str, 20);
			}
			velz = strtod(velz_str,&eptr);
		}
		
		// we calculate the new position
		old = posz;
		posz = posz + (velz * delta);

		// if we are out of bounds, we set the position to the minimum/maximum possible position value
		if (posz < 0.0) {
			posz = 0.0;
		}
		if (posz > 9.0) {
			posz = 9.0;
		}
		
		if (posz != old) {
			write_log("Current position changed!",filename);
		}
		
		// now we send the position to the world process
		sprintf(posz_str,"%f",posz);
		if (write(fd_mzw, posz_str, strlen(posz_str)+1)==-1) {
        	perror("Somenthing wrong in writing");
			if (kill(getppid(), SIGUSR1)==-1) {
				perror("Cannot send signal!");
				exit(1);
			}
		}
		usleep(30000);
	}
	
	// we close all the pipes
	if(close(fd_cmz) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_mzw) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_cpmz) == -1) {
		perror("Error! Cannot close pipe!");
	}
	
	return 0;
}
