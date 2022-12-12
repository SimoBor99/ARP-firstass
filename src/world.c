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

void comunication_channel_insp ( const char* myfifo_insp, double pos, int fd) {
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
        	if (write(fd, p_str, strlen(p_str)+1)==-1)
        perror("Somenthing wrong in writing");
        }
}

void write_log(char * log_text, const char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a"); 
	if (fp_log==NULL)
		perror("Something went wrong"); 
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
	const char * filename = argv[1];
	char * logtxt = "";
	fd_set readfds;
	int retval, nRead;
	int fdx,fdz,fdix,fdiz;
	const char * myfifox = argv[5];
	const char * myfifoz = argv[4];
	const char * myfifoix = argv[2];
	const char * myfifoiz = argv[3];
	int vx = 0;
	int vz = 0;
	char mess[20];
	char messageP1[20], messageP2[20];
	const int TIMEOUT = 1; // seconds

	struct timeval tv;
	fdx = open(myfifox, O_RDONLY);
	if (fdx==0) {
		perror("Cannot open fifo");
		unlink(myfifox);
		exit(1);
	}
	fdz = open(myfifoz, O_RDONLY);
	if (fdz==0) {
		perror("Cannot open fifo");
		unlink(myfifoz);
		exit(1);
	}
	fdix = open(myfifoix, O_WRONLY);
	if (fdix==0) {
		perror("Cannot open fifo");
		unlink(myfifoix);
		exit(1);
	}
	fdiz = open(myfifoiz, O_WRONLY);
	if (fdiz==0) {
		perror("Cannot open fifo");
		unlink(myfifoiz);
		exit(1);
	}
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
        		        nRead = read(fdx, messageP1, 20);
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
            			nRead = read(fdz, messageP2, 20);
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
		close(fdiz);
		close(fdix);
		close(fdz);
        close(fdx);
        return 0;	
}

