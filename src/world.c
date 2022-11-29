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
double resultx,resultz;

double add_random_error(double r)
{
	r += rnderr;
	return r;
}

int main()
{
	fd_set readfds;
	int retval, nRead;
	int fdx,fdz;
	char * myfifox = "/tmp/myfifo_worldx";
	char * myfifoz = "/tmp/myfifo_worldz";
	mkfifo(myfifox,0666);
	mkfifo(myfifoz,0666);
	int vx = 0;
	int vz = 0;
	char mess[80];
	char messageP1[80], messageP2[80];
	const int TIMEOUT = 5; // seconds

	struct timeval tv;
	while(1)
	{
		fdx = open(myfifox, O_RDONLY);
		fdz = open(myfifoz, O_RDONLY);
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
		printf("EHILAAAA");
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
        	        		resultx = strtod(messageP1, &eptr);
        	        		resultx /= 2;
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
            	        		resultz = strtod(messageP2, &eptr);
        	        		resultz /= 2;
        	        		char tmpz[20]="";
    					sprintf(tmpz, "%f", resultz);
        	        		printf("Provider X: %s\n", tmpz);
            	        		fflush(stdout);
            	    		}
            		}
        	}
        	close(fdx);
        	close(fdz);
        }
        return 0;	
}
