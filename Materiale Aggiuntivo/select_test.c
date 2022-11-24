
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

int main() 
{
	fd_set readfds;
	int retval, nRead;
	int fd1, fd2;    
	char * myfifo = "/tmp/myfifo"; 
	mkfifo(myfifo, 0666);
	char * myfifo2 = "/tmp/myfifo2"; 
	mkfifo(myfifo2, 0666);
	char messageP1[80], messageP2[80];
	const int TIMEOUT = 5; // seconds

	struct timeval tv;
	while(1)
	{
		fd1 = open(myfifo, O_RDONLY);
		fd2 = open(myfifo2, O_RDONLY);
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
		printf("EHILAAAA");
        	{
        		// check where is it coming from
        		if (FD_ISSET(fd1, &readfds))
        		{
        		        nRead = read(fd1, messageP1, 80);
        		        if (nRead < 0)
        		        {
        		        	perror("read provider 1");
        	        		exit(EXIT_FAILURE);
        	        	}
        	        	else
        	        	{
        	        		printf("Provider 1: %s\n", messageP1);
        	        		fflush(stdout);
        	        	}
        	    	}
	
            		if(FD_ISSET(fd2, &readfds))
            		{
            			nRead = read(fd2, messageP2, 80);
            			if (nRead < 0)
            	    		{
            	        		perror("read provider 2");
            	        		exit(EXIT_FAILURE);
            	    		}
            	    		else
            	    		{
            	        		printf("Provider 2: %s\n", messageP2);
            	        		fflush(stdout);
            	    		}
            		}
        	}
        	close(fd1);
        	close(fd2);
        }
        return 0;
}
