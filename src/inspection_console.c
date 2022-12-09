#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>
#include "./../include/inspection_utilities.h"

char *eptr;
double resultx,resultz,old_resx,old_resz;

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
{   char *myfifo_insp="/tmp/myfifo_insp";
    char l[100];
    int fd;
    char format[100]="%s-%s";
    char *t;
    char *o;
    fd=open(myfifo_insp, O_RDONLY);
    if (fd==0)
     perror("Something wrong");
    if (read(fd, l, 100)==-1)
        perror("Error in read");
    sscanf(l, format, t, o);
     write_log(t, "inspection_log.txt");
      write_log(o, "inspection_log.txt");
    resultx = 0;
    resultz = 0;
    old_resx = 0;
    old_resz = 0;
    char * filename = argv[1];
    char * logtxt = "";
    int fdx,fdz;
    fd_set readfds;
    int retval, nRead;
    char * myfifox = "/tmp/myfifo_inspx";
    char * myfifoz = "/tmp/myfifo_inspz";
    char mess[80];
    char messageP1[80], messageP2[80];
    const int TIMEOUT = 1; // seconds
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    struct timeval tv;

    // End-effector coordinates
    float ee_x, ee_y;

    // Initialize User Interface 
    init_console_ui();

    // Infinite loop
    fdx = open(myfifox, O_RDONLY);
    fdz = open(myfifoz, O_RDONLY);
    while(TRUE)
    {
	
	FD_ZERO(&readfds);
        FD_SET(fdx, &readfds);
        FD_SET(fdz, &readfds);
        
        tv.tv_sec = TIMEOUT;
        tv.tv_usec = 0;
        retval = select(
            (fdx > fdz) ? fdx + 1 : fdz + 1,
            &readfds, NULL, NULL, &tv);
	if (retval == -1)
        {
        	perror("select()");
        	exit(EXIT_FAILURE);
        }
        else if (!retval) // not any input before timeout
       	{
        	printf("fedffeef");
	}
		
	else // there is incoming input
        {
        	// check where is it coming from
        	if (FD_ISSET(fdx, &readfds))
        	{
        	        nRead = read(fdx, messageP1, 80);
        	        if (nRead < 0)
        	        {
        	        	perror("read provider 1");
        	       		exit(EXIT_FAILURE);
        	       	}
        	       	else
        	       	{
        	       		//perror(messageP1);
        	       		old_resx = resultx;
        	       		resultx = strtod(messageP1, &eptr);
        	       		if (old_resx != resultx) {
        				logtxt="Current horizothal position just changed!";
        				write_log(logtxt,filename);
        			}
        			else {
        				write_log(messageP1,filename);
        			}
        	       		char tmpx[20]="";
    				sprintf(tmpx, "%f", resultx);
        	       		//printf("Provider X: %s\n", tmpx);
        	       		//fflush(stdout);
        	       	}
        	}
	
            	if(FD_ISSET(fdz, &readfds))
            	{
            		nRead = read(fdz, messageP2, 80);
            		if (nRead < 0)
            	   	{
            	        	perror("read provider 2");
            	        	exit(EXIT_FAILURE);
            	    	}
            	    	else
            	    	{
            	    		//perror(messageP2);
            	    		old_resz = resultz;
            	        	resultz = strtod(messageP2, &eptr);
            	        	if (old_resx != resultx) {
        				logtxt="Current vertical position just changed!";
        				write_log(logtxt,filename);
        			}
        			else {
        				write_log(messageP2,filename);
        			}
        	        	char tmpz[20]="";
    				sprintf(tmpz, "%f", resultz);
        	        	//printf("Provider Z: %s\n", tmpz);
            	        	//fflush(stdout);
            	    	}
            	}
        }
        
	{	
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

                // STOP button pressed
                if(check_button_pressed(stp_button, &event)) {
                    /*mvprintw(LINES - 1, 1, "STP button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }*/
                    /*if (kill()==-1)
                        perror("Something wrong in stop");*/
                }

                // RESET button pressed
                else if(check_button_pressed(rst_button, &event)) {
                    /*mvprintw(LINES - 1, 1, "RST button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }*/
                    /*if (kill()==-1)
                        perror("Something wrong in stop");*/
                }
            }
        }
        ee_x = resultx;
        ee_y = resultz;        
        // Update UI
        update_console_ui(&ee_x, &ee_y);
	}
    }
    // Terminate
    endwin();
    close(fdx);
        close(fdz);
    return 0;
}
