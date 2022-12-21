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
// FUNCTION FOR WRITING ON LOG FILE
void write_log(char * log_text, const char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	if (fp_log==NULL)
		perror("Cannot read the file"); 
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
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
	if (resetting == 1) {
		sleep(2);
		if (stop_reset == 1) {
			debugger = 1;
		}
	}
	else {
		sleep(2);
		resetting = 1;
		velz=1.0;
		sprintf (posz_str, "%f", posz);
		if (write(fd_mzw, posz_str, 20)==-1) {
			perror("Cannot write!");
		}
		if (posz!=0.0) {
			while (posz>=0.0) {
				if (stop_reset != 0) {
					break;
				}
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
			if (stop_reset != 0) {
				//resetting = 0;
				stop_reset = 0;
				velz = 0.0;
				sleep(1);
			}
			else {
				velz = 0.0;
				posz=0.0;
				//resetting = 0;
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

// MAIN FUNCTION

int main(int argc, char const *argv[]) {

	const char * filename = argv[1];

	char *eptr;
	const double delta = pow(30,-1);
	char debug[20];

	// STUFF FOR THE SELECT

	struct timeval tv={0,0};
	int ret;
	fd_set rset;

	// FILE DESCRIPTORS
	
	int fd_cmz;
	int fd_cpmz;


	// WE OPEN THE PIPES
		
	fd_cmz = open(argv[2],O_RDONLY);
	if (fd_cmz == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[2]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	fd_mzw = open(argv[3],O_WRONLY);
	if (fd_mzw == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[3]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
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
	//extract the command pid
	pidcmd=atoi(pid_cmd);
	//catch two signal, for stop and reset
	struct sigaction s_stop, s_reset;
	memset(&s_stop, 0, sizeof(s_stop));
	memset(&s_reset, 0, sizeof(s_reset));
    	s_stop.sa_handler=&stop_handler;
    	s_reset.sa_handler=&reset_handler;
		//check if two signal handlers are fine
    	if (sigaction(SIGUSR1, &s_stop, NULL)==-1)
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
        	FD_SET(fd_cmz, &rset);
        	ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);

		if (ret == -1) {
			perror("Error in select()!");
		}

        	// IF THE PIPE IS READY, WE READ FROM IT

		else if(ret>=0){
			if (FD_ISSET(fd_cmz, &rset)>0){
				read(fd_cmz, velz_str, 20);
			}
			velz = strtod(velz_str,&eptr);
		}
		
		old = posz;
		posz = posz + (velz * delta);
		if (posz < 0.0) {
			posz = 0.0;
			//velz = 0.0;
		}
		if (posz > 9.0) {
			posz = 9.0;
			//velz = 0.0;
		}
		
		if (posz != old) {
			write_log("Current position changed!",filename);
		}
		
		// NOW WE SEND THE POSITION TO THE WORLD PROCESS
		
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
	
	// WE CLOSE THE PIPES
		
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
