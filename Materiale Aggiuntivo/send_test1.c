
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
  
int main() 
{ 
    int fd1; 
  
    char * myfifo = "/tmp/myfifo"; 
    mkfifo(myfifo, 0666); 
  
    char str1[80] = "QUA è MR 1";
    int n1, n2;
    double mean;

    while (1) 
    { 
        fd1 = open(myfifo,O_WRONLY); 
        sleep(1);
        write(fd1, str1, 80); 
     	close(fd1);
    } 
    return 0; 
} 
