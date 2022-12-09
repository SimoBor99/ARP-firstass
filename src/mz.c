#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>
#include <math.h>

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

//function for calculating the estimate position; delta_time is 30Hz
double estimate_position ( int velz, double delta_time) {
    return velz*delta_time;
}

//function for opening comunication channel to world
void comunication_channel_world ( char* myfifo_world, double posz, int fd1) {
    char pz_str[20]="";
    //convert double to string
    sprintf( pz_str, "%f", posz);
    //mkfifo(myfifo_world, 0666);
    if (!write(fd1, pz_str, strlen(pz_str)+1))
        perror("Somenthing wrong in writing");
}

//function for opening comunication channel to comand
double comunication_channel_comandz (double velz, char* myfifo_comandz, int fd2) {
    fd_set readfds;
    int retval;
    const int TIMEOUT = 1; // seconds

    struct timeval tv;
    
    char *eptr;
    char velz_str[20]="";
    FD_ZERO(&readfds);
    FD_SET(fd2, &readfds);
        	
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    retval = select(fd2 + 1, &readfds, NULL, NULL, &tv);
    if (retval == -1)
        {
        	perror("select()");
        	exit(EXIT_FAILURE);
        }

    else if (!retval) // not any input before timeout
        {
        	perror("NULLA READ");
	}
	
    else // there is incoming input
        {
        	if (read(fd2, velz_str, 20) == -1) {
        		perror("Something wrong in reading");
        		}	
    		else {
    			perror("READ OK");
        		velz = strtod(velz_str, &eptr);
        	}
        }
    return velz;
}

int main(int argc, char const *argv[]) {
     double old_pos = 0.0;
     char * filename = argv[1];
     char * logtxt = "";
    //open comunication channel to world process
     char * myfifo_world = "/tmp/myfifo_worldz";
     //open comunication channel to comand process
     char * myfifo_comandz = "/tmp/myfifo_comandz";
     char input[20]="";
     double posz=0.0;
     double velz=0.0;
     double delta_time2=pow(30, -1);
     double delta_time1 = 1;
     double increment = 0.0;
     int fd1, fd2;
     fd1=open(myfifo_world,O_WRONLY);
        fd2=open(myfifo_comandz,O_RDONLY);
     while(1) {
        //sleep(0.0033);
        if (fd2 == 0){
        	perror("Error in comunication!!");
        }
        velz=comunication_channel_comandz(velz, myfifo_comandz, fd2);
        old_pos = posz;
        while (((int) posz==40 && velz>0) || ((int) posz==0 && velz<0))
            velz=comunication_channel_comandz(velz, myfifo_comandz, fd2);
        increment=estimate_position(velz, delta_time2);
        increment/=4;
        posz+=increment;
        if (old_pos != posz) {
        	logtxt="Current vertical position just changed!";
        	write_log(logtxt,filename);
        }
        comunication_channel_world(myfifo_world, posz, fd1);
    }
    close(fd1);
    close(fd2);
    return 0;
}
