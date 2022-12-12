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
double old_pos = 0.0;
double posz=0.0;

void write_log(char * log_text, const char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");
    if (fp_log==NULL)
        perror("Something went wrong in open log_file");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

//function for calculating the estimate position; delta_time is 30Hz
double estimate_position ( int velz, double delta_time) {
    return velz*delta_time;
}
//signal handler for stopping; mz crashes when we are inside the stop_handlerz
void stop_handlerz (int signum) {
    signal(SIGUSR1, stop_handlerz);
    write_log("Signal called", "p.txt");
    velz=0.0;
}

//function for opening comunication channel to world
void comunication_channel_world ( char* myfifo_world, double posz, int fd1) {
    char pz_str[20]="";
    //convert double to string
    sprintf( pz_str, "%f", posz);
    //mkfifo(myfifo_world, 0666);
    if (write(fd1, pz_str, strlen(pz_str)+1)==-1)
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
    if (argc==0) {
        printf("One argument expected!");
		return -1;
	}
     const char * filename = argv[1];
     char * logtxt = "";
    //open comunication channel to world process
     char * myfifo_world = "/tmp/myfifo_worldz";
     if (mkfifo(myfifo_world, 0666) != 0)
      perror("Cannot create fifo_world. Already existing?");
     //open comunication channel to comand process
     char * myfifo_comandz = "/tmp/myfifo_comandz";
     char input[20]="";
     double delta_time1=pow(30, -1);
     double delta_time2 = 1;
     double increment = 0.0;
     int fd1, fd2;
     char s[100];
     fd1=open(myfifo_world,O_WRONLY);
     if (fd1==0) {
        perror("Cannot open pipe");
        unlink(myfifo_world);
        exit(1);
    }
    fd2=open(myfifo_comandz,O_RDONLY);
    if (fd2==0) {
        perror("Cannot open pipe");
        unlink(myfifo_comandz);
        exit(1);
    }
    int l=0;
    l=getpid();
    sprintf(s, "%d", l);
    write_log(s, "c.txt");
     while(1) {
        sleep(1);
        velz=comunication_channel_comandz(velz, myfifo_comandz, fd2);
        old_pos = posz;
        if (signal(SIGUSR1, stop_handlerz)==SIG_ERR)
            perror("\ncan't catch the SIGUSR1");
        if (((int) posz==40 && velz>0) || ((int) posz==0 && velz<0))
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
    //we have to check unlink later
    //unlink(myfifo_world);
    close(fd2);
    //unlink(myfifo_comandz);
    return 0;
}
