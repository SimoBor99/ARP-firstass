#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>
#include <math.h>

//function for calculating the estimate position; delta_time is 30Hz
double estimate_position ( int velz, double delta_time) {
    return velz*delta_time;
}

//function for opening comunication channel to world
void comunication_channel_world ( char* myfifo_world, double posx, int fd1) {
    char pz_str[20]="";
    //convert double to string
    sprintf( pz_str, "%f", posx);
    mkfifo(myfifo_world, 0666);
    if (!write(fd1, pz_str, strlen(pz_str)+1))
        perror("Somenthing wrong in writing");
}

//function for opening comunication channel to comand
double comunication_channel_comandz ( char* myfifo_comandz, int fd2) {
    char velz_str[20]="";
    double velz=0.0;
    mkfifo(myfifo_comandz, 0666);
    if (!read(fd2, velz_str, strlen(velz_str)+1))
        perror("Something wrong in reading");
    else 
        sprintf(velz_str, "%f", &velz);
    return velz;
}

int main() {
    //open comunication channel to world process
     char * myfifo_world = "/tmp/myfifo_world";
     //open comunication channel to comand process
     char * myfifo_comandz = "/tmp/myfifo_comandz";
     char input[20]="";
     double posz=0.0;
     double velz=0.0;
     double delta_time=pow(30, -1);
     int fd1, fd2;
     while(1) {
        fd1=open(myfifo_world,O_WRONLY);
        fd2=open(myfifo_comandz,O_RDONLY);
        velz=comunication_channel_comandz(myfifo_comandz, fd2);
        posz+=(1/4)*estimate_position(velz, delta_time);
        while (((int) posz==40 && velz>0) || ((int) posz==0 && velz<0))
            velz=comunication_channel_comandz(myfifo_comandz, fd2);
        comunication_channel_world(myfifo_world, posz, fd1);
        close(fd1);
        close(fd2);
    }
    return 0;
}
