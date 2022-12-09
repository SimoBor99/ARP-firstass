#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/wait.h>
#include <string.h>

const double rnderr = 0.05;
char *eptr;
double resultx,resultz,old_resx,old_resz;

void comunication_channel_insp ( char* myfifo_insp, double pos, int fd) {
    char p_str[20]="";
    fd_set writefds;
    int retval;
    const int TIMEOUT = 1; // seconds

    struct timeval tv;
    
    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);
        	
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    //convert double to string
    sprintf( p_str, "%f", pos);
    //mkfifo(myfifo_world, 0666);
    retval = select(fd + 1, NULL, &writefds, NULL, &tv);
    if (retval == -1)
        {
        	perror("select()");
        	exit(EXIT_FAILURE);
        }

    else if (!retval) // not any input before timeout
        {
        	perror("NULLA WRITE");
	}
	
    else // there is incoming input
        {
        	if (!write(fd, p_str, strlen(p_str)+1))
        perror("Somenthing wrong in writing");
        }
}

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

double add_random_error(double r)
{
	r += rnderr;
	return r;
}

int main(int argc, char const *argv[])
{
	resultx = 0;
	resultz = 0;
	old_resx = 0;
	old_resz = 0;
	char * filename = argv[1];
	char * logtxt = "";
	fd_set readfds;
	int retval, nRead;
	int fdx,fdz,fdix,fdiz;
	char * myfifox = "/tmp/myfifo_worldx";
	char * myfifoz = "/tmp/myfifo_worldz";
	mkfifo(myfifox,0666);
	mkfifo(myfifoz,0666);
	char * myfifoix = "/tmp/myfifo_inspx";
	char * myfifoiz = "/tmp/myfifo_inspz";
	mkfifo(myfifoix,0666);
	mkfifo(myfifoiz,0666);
	int vx = 0;
	int vz = 0;
	char mess[80];
	char messageP1[80], messageP2[80];
	const int TIMEOUT = 1; // seconds

	struct timeval tv;
	fdx = open(myfifox, O_RDONLY);
		fdz = open(myfifoz, O_RDONLY);
		fdix = open(myfifoix, O_WRONLY);
		fdiz = open(myfifoiz, O_WRONLY);
	while(1)
	{
		sleep(0.5);
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
        		continue;
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
        	        		char tmpx[20]="";
    					sprintf(tmpx, "%f", resultx);
        	        		printf("Provider X: %s\n", tmpx);
        	        		fflush(stdout);
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
        	        		char tmpz[20]="";
    					sprintf(tmpz, "%f", resultz);
        	        		printf("Provider Z: %s\n", tmpz);
            	        		fflush(stdout);
            	    		}
            		}
        	}
        	comunication_channel_insp(myfifoix,resultx,fdix);
        	comunication_channel_insp(myfifoiz,resultz,fdiz);
        	
        	
        }
        close(fdx);
        	close(fdz);
        	close(fdix);
        	close(fdiz);
        return 0;	
}
