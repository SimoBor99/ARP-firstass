#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>

//function for calculating the estimate position
double estimate_position ( int velz, double delta_time) {
    return velz*delta_time;
}

//function for opening comunication channel to wolrd
void comunication_channel_world ( char* myfifo_world, double posx, int fd1) {
    char pz_str[20]="";
    //convert double to string
    sprintf( pz_str, "%f", posx);
    mkfifo(myfifo_world, 0666);
    if (!write(fd1, pz_str, strlen(pz_str)+1))
        perror("Somenthing wrong in writing");
}

//function for opening comunication channel to comand
double comunication_channel_comandz ( char* myfifo_comand, int fd2) {
    char velz_str[20]="";
    char format_string[20]="%f";
    double velz=0.0;
    mkfifo(myfifo_comandz, 0666);
    if (!read(fd2, velz_str, strlen(velz_str)+1))
        perror("Something wrong in reading");
    else 
        sscanf(velx_str, format_string, &velz);
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
     double delta_time=0.3;
     int fd1, fd2;
     while(1) {
        fd1=open(myfifo_world,O_WRONLY);
        fd2=open(myfifo_comand,O_RDONLY);
        velz=comunication_channel_comand(myfifo_comandz, fd2);
        posz=estimate_position(velx, delta_time);
        //TODO check if we are 0 or 100, we cannot accept, respectly, negative or positive velx
        comunication_channel_world(myfifo_world, posz, fd1);
        close(fd1);
        close(fd2);
    }
    return 0;
}