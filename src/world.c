// INCLUDE LIBRARIES

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>

// FUNCTION FOR ADDING A RANDOM ERROR TO THE POSITION

double add_rnd_err(double or_val) {
	double err = (rand()%(1)) - (rand()%(1));
	double fin_val = or_val + err;
	return fin_val;
}

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

// MAIN FUNCTION

int main(int argc, char const *argv[]) {

	const char * filename = argv[1];

	char *eptr;
	double oldx = 0.0;
	double oldz = 0.0;
	double posx = 0.0;
	double posz = 0.0;
	char posx_str[20];
	char posz_str[20];
	char real_posx_str[20];
	char real_posz_str[20];

	struct timeval tv={0,0};
	int ret;
	fd_set rset;

	// FILE DESCRIPTORS
	
	int fd_mxw;
	int fd_mzw;
	int fd_wix;
	int fd_wiz;

	// WE OPEN THE PIPES
		
	fd_mxw = open(argv[2],O_RDONLY);
	if (fd_mxw ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[2]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	fd_mzw = open(argv[3],O_RDONLY);
	if (fd_mzw ==0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[3]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	fd_wix = open(argv[4],O_WRONLY);
	if (fd_wix == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[4]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}
	fd_wiz = open(argv[5], O_WRONLY);
	if (fd_wiz == 0) {
		perror("Error! Cannot open pipe!");
		unlink(argv[5]);
		if (kill(getppid(), SIGUSR1)==-1) {
			perror("Cannot send signal!");
			exit(1);
		}
	}

	while(1) {
	
		// SETTING PARAMETERS FOR SELECT
	
		FD_ZERO(&rset);
        	FD_SET(fd_mxw, &rset);
        	FD_SET(fd_mzw, &rset);
        	ret = select(FD_SETSIZE, &rset, NULL, NULL, &tv);
        	
        	oldx = posx;
        	oldz = posz;
        	
        	// IF THERE'S SOMETHING TO READ, WE READ IT!
        	
        	if (ret > 0) {
        		if (FD_ISSET(fd_mxw, &rset)>0){
				if(read(fd_mxw, posx_str, 20) == -1) {
					perror("Cannot read from pipe!");
					if (kill(getppid(), SIGUSR1)==-1) {
						perror("Cannot send signal!");
						exit(1);
					}
				}
				posx = strtod(posx_str,&eptr);
			}

			if (FD_ISSET(fd_mzw, &rset)>0){
				if(read(fd_mzw, posz_str, 20) == -1) {
					perror("Cannot read from pipe!");
					if (kill(getppid(), SIGUSR1)==-1) {
						perror("Cannot send signal!");
						exit(1);
					}
				}
				posz = strtod(posz_str,&eptr);
			}
			
        	}
		if (posx != oldx) {
			posx = add_rnd_err(posx);
			write_log("Current horizonthal position changed!",filename);
		}
		if (posz != oldz) {
			posz = add_rnd_err(posz);
			write_log("Current vertical position changed!",filename);
		}	
		
		sprintf(real_posx_str,"%f",posx);
		sprintf(real_posz_str,"%f",posz);
		
		// WE PASS THE REAL POSITION VALUES TO THE INSPECTION, SO THAT IT CAN DESPLAY THE POSITION
		
		if (write(fd_wix, real_posx_str, strlen(real_posx_str)+1)==-1) {
        		perror("Somenthing wrong in writing");
				if (kill(getppid(), SIGUSR1)==-1) {
					perror("Cannot send signal!");
					exit(1);
				}
		}
		
		if (write(fd_wiz, real_posz_str, strlen(real_posz_str)+1)==-1) {
        		perror("Somenthing wrong in writing");
				if (kill(getppid(), SIGUSR1)==-1) {
					perror("Cannot send signal!");
					exit(1);
				}
		}
			
		usleep(30000);
	}
	
	// WE CLOSE THE PIPES
		
	if(close(fd_mxw) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_mzw) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_wix) == -1) {
		perror("Error! Cannot close pipe!");
	}
	if(close(fd_wiz) == -1) {
		perror("Error! Cannot close pipe!");
	}
	
	return 0;
}
