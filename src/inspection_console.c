#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include "./../include/inspection_utilities.h"

char *eptr;
double resultx,resultz,old_resx,old_resz;
const int dim=80;

void write_log(char * log_text, const char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a"); 
	if (fp_log==NULL)
		perror("Something wrong in open log file"); 
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	fclose(fp_log);
}

int main(int argc, char const *argv[]) {   
	if (argc==0) {
        printf("One argument expected!");
		return -1;
	}
	const char *myfifo_insp=argv[4];
    char mxmz[dim];
    int fd1, fd2;
    char mx[dim];
    char mz[dim];
    char pidc[dim];
    fd_set readfds;
    int retval, nRead;
    double mm;
    double kk;
	fd1=open(myfifo_insp, O_RDONLY);
	if (fd1==0) {
		perror("Cannot open fifo");
		unlink(myfifo_insp);
		exit(1);
	}
	if (read(fd1, mxmz, dim)==-1)
		perror("Something went wrong in reading");
	sscanf(mxmz, "%[^,],%s", mx, mz);
    write_log(mx, "inspection_log.txt");
    write_log(mz, "inspection_log.txt");
	int pmx=atoi(mx);
	int pmz=atoi(mz);
	close(fd1);
	//this select it will be used for reading the pid of command;
    /*fd1=open(myfifo_insp, O_RDONLY);
    fd2=open(myfifoinsp, O_RDONLY);
    FD_ZERO(&readfds);
        	FD_SET(fd1, &readfds);
        	FD_SET(fd2, &readfds);
        	
        	tv.tv_sec = TIMEOUT;
        	tv.tv_usec = 0;
        	retval = select(
            (fd1 > fd2) ? fd1 + 1 : fd2 + 1,
            &readfds, NULL, NULL, &tv);
		if (retval == -1)
        	{
        		perror("select()");
        		exit(EXIT_FAILURE);
        	}

        	else if (!retval) // not any input before timeout
        	{
        		continue;
		}
		
		else // there is incoming input
        	{
        		// check where is it coming from
        		if (FD_ISSET(fd1, &readfds))
        		{
        		        nRead = read(fd1, mxmz, 100);
        		        if (nRead < 0)
        		        {
        		        	perror("read provider 1");
        	        		exit(EXIT_FAILURE);
        	        	}
        	        	else
        	        	{
        	        		 mm= strtod(mxmz, &eptr);
        				}
        	        		char tmpx[100]="";
    					sprintf(tmpx, "%f", mm);
        	        	sscanf(mxmz, "%[^,],%s", mx, mz);
                        write_log(mx, "inspection_log.txt");
                        write_log(mz, "inspection_log.txt");
                        int pmx=atoi(mx);
                        int pmz=atoi(mz);
        	        	}
        	    	}
	
            		if(FD_ISSET(fd2, &readfds))
            		{
            			nRead = read(fd2, pidc, 100);
            			if (nRead < 0)
            	    		{
            	        		perror("read provider 2");
            	        		exit(EXIT_FAILURE);
            	    		}
            	    		else
            	    		{
            	        		kk = strtod(pidc, &eptr);
            	        		
        				}
        	        		char tmpz[100]="";
    					sprintf(tmpz, "%f", kk);
            	    		}
            		}
        	}
    close(fd1);
    close(fd2);*/
    resultx = 0;
    resultz = 0;
    old_resx = 0;
    old_resz = 0;
    const char * filename = argv[1];
    char * logtxt = "";
    int fdx,fdz;
    //fd_set readfds;
    //int retval, nRead;
    const char * myfifox = argv[2];
    const char * myfifoz = argv[3];
    char mess[dim];
    char messageP1[dim], messageP2[dim];
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
	if ( fdx==0) {
		unlink(myfifox);
		exit(1);
	}
    fdz = open(myfifoz, O_RDONLY);
	if ( fdz==0) {
		unlink(myfifoz);
		exit(1);
	}
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
        	       		char tmpx[dim];
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
        	        	char tmpz[dim];
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
                    mvprintw(LINES - 1, 1, "STP button pressed");
                    refresh();
                    sleep(1);
                    for(int j = 0; j < COLS; j++) {
                        mvaddch(LINES - 1, j, ' ');
                    }
					//here we send 2 signal to mz and mx
                    if (kill( pmx, SIGUSR1)==-1)
                        perror("Something wrong in stop");
					sleep(0.5);
					if (kill(pmz, SIGUSR1)==-1)
						perror("Something wrong in stop");
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
                        perror("Something wrong in reset");*/
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
	close(fdz);
	close(fdx);
    return 0;
}

