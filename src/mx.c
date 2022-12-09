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
double estimate_position ( int velx, double delta_time) {
    return velx*delta_time;
}

//function for opening comunication channel to world
void comunication_channel_world ( char* myfifo_world, double posx, int fd1) {
    char px_str[20]="";
    
    sprintf( px_str, "%f", posx);
    //mkfifo(myfifo_world, 0666);
    if (!write(fd1, px_str, strlen(px_str)+1))
        perror("Somenthing wrong in writing");
}

//function for opening comunication channel to comand
double comunication_channel_comandx (double velx, char* myfifo_comandx, int fd2) {
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
    			perror("READ OK");
        		velx = strtod(velx_str, &eptr);
        	}
        }
    return velx;
}

int main(int argc, char const *argv[]) {
     char * filename = argv[1];
     char * logtxt = "";
     double old_pos = 0.0;
    //open comunication channel to world process
     char * myfifo_world = "/tmp/myfifo_worldx";
     //open comunication channel to comand process
     char * myfifo_comandx = "/tmp/myfifo_comandx";
     char input[20]="";
     char l[100];
     double posx=0.0;
     double velx=0.0;
     double delta_time2=pow(30, -1);
     double delta_time1 = 1;
     int fd1, fd2;
     double o = 0;
     fd1=open(myfifo_world,O_WRONLY);
        fd2=open(myfifo_comandx,O_RDONLY);
     while(1) {
     	//sleep(0.0033);
        if (fd2 == 0){
        	perror("Error in comunication!!");
        }
        velx=comunication_channel_comandx(velx, myfifo_comandx, fd2);
        old_pos = posx;
        while (((int) posx==40 && velx>0) || ((int) posx==0 && velx<0))
            velx=comunication_channel_comandx(velx, myfifo_comandx, fd2);
        o=estimate_position(velx, delta_time2);
        posx=posx+o;
        sprintf(l, "%f", o);
        if (old_pos != posx) {
        	logtxt="Current horizonthal position just changed!";
        	write_log(logtxt,filename);
        }
        comunication_channel_world(myfifo_world, posx, fd1);
      
    }
    close(fd1);
        close(fd2);
    return 0;
}
