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
double posx=0.0;
double old_pos = 0.0;

void write_log(char * log_text, const char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}
//signal handler for stopping; mx crashes when we are inside the stop_handlerx
void stop_handlerx (int signum) {
    signal(SIGUSR1, stop_handlerx);
    velx=0.0;
}

//function for calculating the estimate position; delta_time is 30Hz
double estimate_position ( int velx, double delta_time) {
    return velx*delta_time;
}

//function for opening comunication channel to world
void comunication_channel_world (double posx, int fd1) {
    char px_str[20]="";
    
    sprintf( px_str, "%f", posx);
    //mkfifo(myfifo_world, 0666);
    if (write(fd1, px_str, strlen(px_str)+1)==-1)
        perror("Somenthing wrong in writing");
}

//function for opening comunication channel to comand
double comunication_channel_comandx (int fd2) {
    fd_set readfds;
    int retval;
    const int TIMEOUT = 1; // seconds

    struct timeval tv;
    
    char *eptr;
    char velx_str[20]="";
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
        	if (read(fd2, velx_str, 20) == -1) {
        		perror("Something wrong in reading");
        		}	
    		else {
    			//perror("READ OK");
        		velx = strtod(velx_str, &eptr);
        	}
        }
    return velx;
}

int main(int argc, char const *argv[]) {
    if (argc==0) {
        printf("One argument expected!");
		return -1;
	}
     const char * filename = argv[1];
     char * logtxt = "";
    //open comunication channel to world process
     const char * myfifo_world = argv[2];
     const char * myfifo_comandx = argv[3];
     char input[20]="";
     char l[100];
     double delta_time1=pow(30, -1);
     double delta_time2 = 1;
     int fd1, fd2, fd3;
     char s[100];
     int y=getpid();
     sprintf(s, "%d", y);
     write_log(s, "c.txt");
     fd1=open(myfifo_world,O_WRONLY);
     if (fd1==0) {
        perror("Cannot open fifo_world");
        unlink(myfifo_world);
        exit(1);
    }
     fd2=open(myfifo_comandx,O_RDONLY);
     if (fd2==0) {
        perror("Cannot open fifo_comandx");
        unlink(myfifo_comandx);
        exit(1);
    }
     while(1) {
        velx=comunication_channel_comandx(fd2);
        old_pos = posx;
        if (signal(SIGUSR1, stop_handlerx)==SIG_ERR)
            perror("\ncan't catch the SIGUSR1");
        if (((int) posx==40 && velx>0) || ((int) posx==0 && velx<0))
            velx=comunication_channel_comandx(fd2);
        posx+=estimate_position(velx, delta_time2);
        if (old_pos != posx) {
        	logtxt="Current horizonthal position just changed!";
        	write_log(logtxt,filename);
        }
        comunication_channel_world(posx, fd1);
      
    }
    close(fd2);
    close(fd1);
    return 0;
}
