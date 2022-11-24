
#include <stdio.h> 
#include <string.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <stdlib.h>
  
int main() 
{ 
    int fd2; 
  
    char * myfifo2 = "/tmp/myfifo2"; 
    mkfifo(myfifo2, 0666); 
  
    char str1[80] = "QUA è MR 2 ";
    int n1, n2;
    double mean;

    while (1) 
    { 
        fd2 = open(myfifo2,O_WRONLY); 
        sleep(5);
        write(fd2, str1, 80); 
     	close(fd2);
    } 
    return 0; 
} 
