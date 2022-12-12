#include "./../include/command_utilities.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>


void comunication_channel_insp ( const char* myfifo_insp, double pos, int fd) {
    char p_str[20]="";
    //convert double to string
    sprintf( p_str, "%f", pos);
    //mkfifo(myfifo_world, 0666);
    if (write(fd, p_str, strlen(p_str)+1)==-1)
        perror("Somenthing wrong in writing");
}

void write_log( char * log_text, const char * fn)
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

int main(int argc, char const *argv[])
{
    if (argc==0) {
        printf("One argument expected!");
		return -1;
	}
    const char * filename = argv[1];
    char * logtxt = "";
    int fdx,fdz, fdinsp;
    int pid=getpid();
    char cpid[100];
    const char * myfifox = argv[2];
    const char * myfifoz = argv[3];
    //char * myfifoinsp="/tmp/myfifoinsp";
    double vx = 0.0;
    double vz = 0.0;
    char mess[80];
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();
    
    // Infinite loop
    fdx = open(myfifox,O_WRONLY);
    if (fdx==0) {
         perror("Cannot open pipe");
        unlink(myfifox);
        exit(1);
    }
    fdz = open(myfifoz,O_WRONLY);
    if (fdz==0) {
         perror("Cannot open pipe");
        unlink(myfifoz);
        exit(1);
    }

    /*fdinsp=open(myfifoinsp,O_WRONLY);
    sprintf(cpid, "%d", pid);
    if (read(fdinsp,cpid,100)==-1)
        perror("Something went wrong");
    close(fdinsp);*/
    while(TRUE)
	{	
        // Get mouse/resize commands in non-blocking mode...
        sleep(0.5);
        int cmd = getch();

        // If user resizes screen, re-draw UI
        if(cmd == KEY_RESIZE) {
            if(first_resize) {
                first_resize = FALSE;
            }
            else {
                reset_console_ui();
            }
        }
        // Else if mouse has been pressed
        else if(cmd == KEY_MOUSE) {

            // Check which button has been pressed...
            if(getmouse(&event) == OK) {

                // Vx-- button pressed
                if(check_button_pressed(vx_decr_btn, &event)) {
                    vx--;
                    mvprintw(LINES - 1, 1, "Horizontal Speed Decreased");
                    sprintf(mess,"%f",vx);
                    //fdx = open(myfifox,O_WRONLY);
                    comunication_channel_insp(myfifox, vx, fdx);
                    logtxt = "Decrease horizonthal speed";
                    write_log(logtxt,filename);
                    refresh();
                    //close(fdx);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    vx++;
                    sprintf(mess,"%f",vx);
                    //fdx = open(myfifox,O_WRONLY);
                    comunication_channel_insp(myfifox, vx, fdx);
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    logtxt = "Increase horizonthal speed";
                    write_log(logtxt,filename);
                    refresh();
                    //close(fdx);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx stop button pressed
                else if(check_button_pressed(vx_stp_button, &event)) {
                    vx = 0;
                    mvprintw(LINES - 1, 1, "Horizontal Motor Stopped");
                    sprintf(mess,"%f",vx);
                    //fdx = open(myfifox,O_WRONLY);
                    comunication_channel_insp(myfifox, vx, fdx);
                    logtxt = "Stop horizonthal speed";                 
                    write_log(logtxt,filename);
                    refresh();
                    //close(fdx);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz-- button pressed
                else if(check_button_pressed(vz_decr_btn, &event)) {
                    vz--;
                    mvprintw(LINES - 1, 1, "Vertical Speed Decreased");
                    sprintf(mess,"%f",vz);
                    //fdz = open(myfifoz,O_WRONLY);
                    comunication_channel_insp(myfifoz, vz, fdz);
                    logtxt = "Decrease vertical speed";
                    write_log(logtxt,filename);
                    refresh();
                    //close(fdz);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz++ button pressed
                else if(check_button_pressed(vz_incr_btn, &event)) {
                    vz++;
                    mvprintw(LINES - 1, 1, "Vertical Speed Increased");
                    sprintf(mess,"%f",vz);
                    //fdz = open(myfifoz,O_WRONLY);
                    comunication_channel_insp(myfifoz, vz, fdz);
                    logtxt = "Increase vertical speed";
                    write_log(logtxt,filename);
                    refresh();
                    //close(fdz);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vz stop button pressed
                else if(check_button_pressed(vz_stp_button, &event)) {
                    vz = 0;
                    mvprintw(LINES - 1, 1, "Vertical Motor Stopped");
                    sprintf(mess,"%f",vz);
                    //fdz = open(myfifoz,O_WRONLY);
                    comunication_channel_insp(myfifoz, vz, fdz);
                    logtxt = "Stop vertical speed";
                    write_log(logtxt,filename);
                    refresh();
                    //close(fdz);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
        refresh();
	}
    // Terminate
    close(fdz);
    close(fdx);
    endwin();
    return 0;
}
