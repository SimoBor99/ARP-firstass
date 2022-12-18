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
// FUNCTION FOR WRITING ON LOG FILE

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

void stop_handler(int signum) {
	if(signum==SIGUSR1) {
		velz=0.0;
		char t[20];
		sprintf(t, "%f", velz);
		write_log(t, "log_mz.txt");
	}
}
void reset_handler(int signum) {
	velz=2;
	double pos=posz;
	char reset_mz[20];
	if (pos!=0) {
	while (pos>0) {
		pos=pos-(velz*delta);
		sprintf (reset_mz, "%f", pos);
		write_log(reset_mz, "log_mz.txt");
		if (write(fd_mzw, reset_mz, 20)==-1)
			perror("Cannot write!");
		
	pos=0.0;
	sprintf (reset_mz, "%f", pos);
	if (write(fd_mzw, reset_mz, 20)==-1)
			perror("Cannot write!");
	if (kill(pidcmd, SIGUSR1)==-1)
		perror("Cannot send the signal");
	}
	}
}

// MAIN FUNCTION

int main(int argc, char const *argv[]) {

	char * filename = argv[1];

	char velz_str[20];
	char posz_str[20];
	char *eptr;
	const double delta = pow(30,-1);
	char debug[20];

	// STUFF FOR THE SELECT

	struct timeval tv={0,0};
	int ret;
	fd_set rset;

	// INITIALIZE POSITION STUFF
	
	double posz = 0.0;
	double old = 0.0;

	// FILE DESCRIPTORS
	
	int fd_cmz;
	int fd_cpmz;

	// TAKING THE ARGUMENTS PASSED BY THE MASTER PROCESS
	
	/*char * filename = argv[1];
	char * com_mz = argv[2];
	char * mz_world = argv[3];*/

	// WE OPEN THE PIPES
		
	fd_cmz = open(argv[2],O_RDONLY);
	if (fd_cmz == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_mzw = open(argv[3],O_WRONLY);
	if (fd_mzw == -1) {
		perror("Error! Cannot open pipe!");
	}
	fd_cpmz=open(argv[4], O_RDONLY);
	if (fd_cpmz == -1) {
		perror("Error! Cannot open pipe!");
	}
	if (read(fd_cpmz, pid_cmd, 20)==-1)
		perror("Something wrong in reading");
	pidcmd=atoi(pid_cmd);
	struct sigaction s_stop, s_reset;
	memset(&s_stop, 0, sizeof(s_stop));
	memset(&s_reset, 0, sizeof(s_reset));
    	s_stop.sa_handler=&stop_handler;
    	s_reset.sa_handler=&reset_handler;
    	if (sigaction(SIGUSR1, &s_stop, NULL)==-1)
    		perror("Can't catch the signal");
    	if (sigaction(SIGUSR2, &s_reset, NULL)==-1)
    		perror("Can't catch the signal");

	while(1) {
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
		if (!write(fd_mzw, posz_str, strlen(posz_str)+1)) {
        		perror("Somenthing wrong in writing");
		}
		
		sleep(1);
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
