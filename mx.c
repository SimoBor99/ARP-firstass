#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>

//function for calculating the estimate position
double estimate_position ( int velx, double delta_time) {
    return velx*delta_time;
}

//function for opening comunication channel to wolrd
void comunication_channel_world ( char* myfifo_world, double posx, int fd1) {
    char px_str[20]="";
    //convert double to string
    sprintf( px_str, "%f", posx);
    mkfifo(myfifo_world, 0666);
    if (!write(fd1, px_str, strlen(px_str)+1))
        perror("Somenthing wrong in writing");
}

//function for opening comunication channel to comand
double comunication_channel_comand ( char* myfifo_comand, int fd2) {
    char velx_str[20]="";
    char format_string[20]="%f";
    double velx=0.0;
    mkfifo(myfifo_comand, 0666);
    if (!read(fd2, velx_str, strlen(velx_str)+1))
        perror("Something wrong in reading");
    else 
        sscanf(velx_str, format_string, &velx);
    return velx;
}

int main() {
    //open comunication channel to world process
     char * myfifo_world = "/tmp/myfifo_world";
     //open comunication channel to comand process
     char * myfifo_comand = "/tmp/myfifo_comand";
     char input[20]="";
     double posx=0.0;
     double velx=0.0;
     double delta_time=0.3;
     int fd1, fd2;
     while(1) {
        fd1=open(myfifo_world,O_WRONLY);
        fd2=open(myfifo_comand,O_RDONLY);
        velx=comunication_channel_comand(myfifo_comand, fd2);
        posx=estimate_position(velx, delta_time);
        //TODO check if we are 0 or 100, we cannot accept, respectly, negative or positive velx
        comunication_channel_world(myfifo_world, posx, fd1);
        close(fd1);
        close(fd2);
    }
    return 0;
}
