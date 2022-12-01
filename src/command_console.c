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

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

int main(int argc, char const *argv[])
{
    char * filename = "log_command.txt";
    char * logtxt = "";
    int fdx,fdz;
    char * myfifox = "/tmp/myfifo_comandx";
    char * myfifoz = "/tmp/myfifo_comandz";
    mkfifo(myfifox,0666);
    mkfifo(myfifoz,0666);
    double vx = 0.0;
    double vz = 0.0;
    char mess[80];
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize User Interface 
    init_console_ui();
    
    // Infinite loop
    while(TRUE)
	{	
	//fdx = open(myfifox,O_WRONLY);
    	//fdz = open(myfifoz,O_WRONLY);
        // Get mouse/resize commands in non-blocking mode...
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
                    fdx = open(myfifox,O_WRONLY);
                    if (write(fdx,mess,80) == -1) {
                    	perror("Error in writing!");
                    }
                    logtxt = "Decrease horizonthal speed";
                    write_log(logtxt,filename);
                    refresh();
                    close(fdx);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }

                // Vx++ button pressed
                else if(check_button_pressed(vx_incr_btn, &event)) {
                    vx++;
                    sprintf(mess,"%f",vx);
                    fdx = open(myfifox,O_WRONLY);
                    if (write(fdx,mess,80) == -1) {
                    	perror("Error in writing!");
                    }
                    mvprintw(LINES - 1, 1, "Horizontal Speed Increased");
                    logtxt = "Increase horizonthal speed";
                    write_log(logtxt,filename);
                    refresh();
                    close(fdx);
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
                    fdx = open(myfifox,O_WRONLY);
                    if (write(fdx,mess,80) == -1) {
                    	perror("Error in writing!");
                    }
                    logtxt = "Stop horizonthal speed";                 
                    write_log(logtxt,filename);
                    refresh();
                    close(fdx);
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
                    fdz = open(myfifoz,O_WRONLY);
                    if (write(fdz,mess,80) == -1) {
                    	perror("Error in writing!");
                    }
                    logtxt = "Decrease vertical speed";
                    write_log(logtxt,filename);
                    refresh();
                    close(fdz);
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
                    fdz = open(myfifoz,O_WRONLY);
                    if (write(fdz,mess,80) == -1) {
                    	perror("Error in writing!");
                    }
                    logtxt = "Increase vertical speed";
                    write_log(logtxt,filename);
                    refresh();
                    close(fdz);
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
                    fdz = open(myfifoz,O_WRONLY);
                    if (write(fdz,mess,80) == -1) {
                    	perror("Error in writing!");
                    }
                    logtxt = "Stop vertical speed";
                    write_log(logtxt,filename);
                    refresh();
                    close(fdz);
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }
	//close(fdx);
	//close(fdz);
        refresh();
	}

    // Terminate
    endwin();
    return 0;
}
