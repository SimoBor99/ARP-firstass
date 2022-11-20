#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

FILE *fp1;
FILE *fp2;
char str[80];

int try(char *filepath)
{
    struct stat filestat;

    stat(filepath,&filestat);
    /* newline included in ctime() output */
    //printf(" File access time %s",
            //ctime(&filestat.st_atime)
          //);
    printf(" File modify time %s",
            ctime(&filestat.st_mtime)
          );
    //printf("File changed time %s",
            //ctime(&filestat.st_ctime)
          //);

    return(0);
}

    
int main() {

    int pid = fork();

    if(pid == -1) {
        perror("Errore! Fork fallita!");
        return -1;
    }

    else if(pid == 0) {
        while(true) {
            fp1 = fopen("test3.txt", "a");
            if(fp1 == NULL) {
                printf("CREA FIGLIO?");
            }
            else {
                printf("FIGLIO C'é");
                fclose(fp1);
                break;
            }
        }
        while(true){
        	fp1 = fopen("test3.txt", "a");
        	//printf("ORA PRINTO!");
        	fputs("1", fp1);
        	//printf("PRINTATO!");
        	fclose(fp1);
        }
        return 0;
    }

    else {
        printf("Proviamo a leggere sto log file: \n");
        while(true) {
       	printf("PRIMA");
            fp2 = fopen("test3.txt", "r");
            if(fp2 == NULL) {
                printf("CREA PADRE?");
            }
            else {
                printf("PADRE C'é");
                fclose(fp2);
                break;
            }
        }
        char *fpt = "test3.txt";
        while(true) {
            fp2 = fopen("test3.txt", "r");
            //printf("PROVO");
            try(fpt);
            //printf("SIUM");
            fgets(str, 60, fp2);
            //printf(str);
            //i++;
            fclose(fp2);
        }
        return 0;
    }

}
