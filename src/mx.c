// INCLUDING LIBRARIES

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

// FUNCTION FOR WRITING ON LOG FILE
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

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

//stop_handler
void stop_handler(int signum) {
	if(signum==SIGUSR1) {
		velx=0.0;
		char t[20];
		sprintf(t, "%f", velx);
		stop_reset = 1;
	}
}

void reset_handler(int signum) {
	if (resetting == 1) {
		sleep(2);
		if (stop_reset == 1) {
			debugger = 1;
		}
	}
	else {
		sleep(2);
		resetting = 1;
		velx=1.0;
		sprintf (posx_str, "%f", posx);
		if (write(fd_mxw, posx_str, 20)==-1) {
			perror("Cannot write!");
		}
		if (posx!=0.0) {
			while (posx>=0.0) {
				if (stop_reset != 0) {
					break;
				}
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
				sleep(1);
			}
			if (stop_reset != 0) {
				//resetting = 0;
				stop_reset = 0;
				velx = 0.0;
				sleep(1);
			}
			else {
				//resetting = 0;
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
		else {
			velx = 0.0;
			posx=0.0;
			//resetting = 0;
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

// MAIN FUNCTION

int main(int argc, char const *argv[]) {

	char * filename = argv[1];

	char *eptr;
	const double delta = pow(30,-1);
	char debug[20];

	// STUFF FOR THE SELECT

	struct timeval tv={0,0};
	int ret;
	fd_set rset;

	// FILE DESCRIPTORS
	
	int fd_cmx;
	int fd_cpmx;

	// TAKING THE ARGUMENTS PASSED BY THE MASTER PROCESS
	
	/*char * filename = argv[1];
	char * com_mx = argv[2];
	char * mx_world = argv[3];*/

	// WE OPEN THE PIPES

	fd_cmx = open(argv[2],O_RDONLY);
	if (fd_cmx == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_mxw = open(argv[3],O_WRONLY);
	if (fd_mxw == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_cpmx = open(argv[4],O_RDONLY);
	if (fd_cpmx == -1) {
		perror("Error! Cannot open pipe!");
	}
	if (read(fd_cpmx, pid_cmd, 20)==-1)
		perror("Something wrong in reading");
	pidcmd=atoi(pid_cmd);
	struct sigaction s_stop, s_reset;
	memset(&s_stop, 0, sizeof(s_stop));
	memset(&s_reset, 0, sizeof(s_reset));
    	s_stop.sa_handler=&stop_handler;
    	s_reset.sa_handler=&reset_handler;
    	if (sigaction(SIGUSR1, &s_stop, 0)==-1)
    		perror("Can't catch the signal");
    	if (sigaction(SIGUSR2, &s_reset, NULL)==-1)
    		perror("Can't catch the signal");
	
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

        	// IF THE PIPE IS READY, WE READ FROM IT

		else if(ret>=0){
			if (FD_ISSET(fd_cmx, &rset)>0){
				read(fd_cmx, velx_str, 20);
			}
			velx = strtod(velx_str,&eptr);
		}
		
		old = posx;
		posx = posx + (velx * delta);
		if (posx < 0.0) {
			posx = 0.0;
			//velx = 0.0;
		}
		if (posx > 39.0) {
			posx = 39.0;
			//velx = 0.0;
		}
		
		if (posx != old) {
			write_log("Current position changed!",filename);
		}
		
		// NOW WE SEND THE POSITION TO THE WORLD PROCESS
		
		sprintf(posx_str,"%f",posx);
		if (!write(fd_mxw, posx_str, strlen(posx_str)+1)) {
        		perror("Somenthing wrong in writing");
		}
		
		sleep(1);
	}
	
	// WE CLOSE THE PIPES
	
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
