#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

FILE *fp1;
FILE *fp2;
char str[80];

int main() {

    int pid = fork();

    if(pid == -1) {
        perror("Errore! Fork fallita!");
        return -1;
    }

    else if(pid == 0) {
        while(true) {
            fp1 = fopen("test3.txt", "w");
            if(fp1 == NULL) {
                printf("CREA FIGLIO?");
            }
            else {
                printf("FIGLIO C'é");
                break;
            }
        }
        printf("ORA PRINTO!");
        fputs("1", fp1);
        printf("PRINTATO!");
        fclose(fp1);
        return 0;
    }

    else {
        printf("Proviamo a leggere sto log file: \n");
        while(true) {
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
        //int i = 0;
        while(true) {
            fp2 = fopen("test3.txt", "r");
            printf("SIUM");
            fgets(str, 60, fp2);
            printf(str);
            //i++;
            fclose(fp2);
        }
        return 0;
    }

}
