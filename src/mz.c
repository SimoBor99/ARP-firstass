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
double comunication_channel_comandz ( char* myfifo_comandz, int fd2) {
    char *eptr;
    char velz_str[20]="";
    double velz=0.0;
    //mkfifo(myfifo_comandz, 0666);
    if (read(fd2, velz_str, 20) == -1) {
        perror("Something wrong in reading");
        }
    else {
        velz = strtod(velz_str, &eptr);
        }
    return velz;
}

int main() {
     double old_pos = 0.0;
     char * filename = "log_mz.txt";
     char * logtxt = "";
    //open comunication channel to world process
     char * myfifo_world = "/tmp/myfifo_worldz";
     //open comunication channel to comand process
     char * myfifo_comandz = "/tmp/myfifo_comandz";
     char input[20]="";
     double posz=0.0;
     double velz=0.0;
     double delta_time1=pow(30, -1);
     double delta_time2=1;
     int fd1, fd2;
     while(1) {
     	sleep(1);
        fd1=open(myfifo_world,O_WRONLY);
        fd2=open(myfifo_comandz,O_RDONLY);
        if (fd2 == 0){
        	perror("Error in comunication!!");
        }
        velz=comunication_channel_comandz(myfifo_comandz, fd2);
        old_pos = posz;
        printf("%f",posz);
        printf("%f",old_pos);
        //posz+=(1/4)*estimate_position(velz, delta_time2);
        while (((int) posz==40 && velz>0) || ((int) posz==0 && velz<0))
            velz=comunication_channel_comandz(myfifo_comandz, fd2);
        posz+=(1/4)*estimate_position(velz, delta_time2);
        if (old_pos != posz) {
        	logtxt="Current vertical position just changed!";
        	write_log(logtxt,filename);
        }
        comunication_channel_world(myfifo_world, posz, fd1);
        close(fd1);
        close(fd2);
    }
    return 0;
}
