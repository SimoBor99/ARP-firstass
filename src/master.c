#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include <signal.h>
#include <string.h>	

FILE *fp[5];
//function that read the log file of our porcesses
char* readlogfile(char *filepath) {
    struct stat filestat;
    char * filemod;
    if (filemod==NULL) 
      perror("Invalid pointer");
    char timemod[200];
    char time[100];
    char format_string_date[100]="%s %s  %s %s %s";
    char day[80];
    char month[80];
    char num_day[80];
    char year[80];
    if (stat(filepath,&filestat)==-1)
        perror("Something went wrong");
    filemod=ctime(&filestat.st_mtime);
    sscanf(filemod, format_string_date, day, month, num_day, time, year);
    return time;
}

int timecheck( char *filetime) {
  time_t rawtime;
  struct tm * timeinfo;
  char y[100];
  char format_string_time[100]="%d:%d:%d";
  if ( timeinfo==NULL || format_string_time==NULL)
    perror("Invalid pointer");
  char actual_time[100];
  int actual_hour=0;
  int actual_minutes=0;
  int actual_seconds=0;
  int file_minutes=0;
  int file_seconds=0;
  int file_hour=0;
  char t[20], r[20], f[20];
  time(&rawtime);
  timeinfo=localtime( &rawtime);
  //actual time
  strftime(actual_time, 100, "%H:%M:%S", timeinfo);
  //convertion of actual and last modify file time
  sscanf(actual_time, format_string_time, &actual_hour, &actual_minutes, &actual_seconds);
  sscanf(filetime, format_string_time, &file_hour, &file_minutes, &file_seconds);
  sprintf(t, "%d", file_hour);
  sprintf(r, "%d", file_minutes);
  sprintf(f, "%d", file_seconds);
  if (actual_minutes != file_minutes) {
  	if (actual_minutes == 0) {
  		actual_minutes = 60;
  	}
  	if (abs(actual_minutes-file_minutes)>1) {
  		return 1;
  	}
  	else {
  		if ((actual_seconds-file_seconds)>= 0) {
  			return 1;
  		}
  		else {
  			return 0;
  		}
  	}
  }
  else {
  	return 0;
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

// FINE NUOVE FUNZIONI

int spawn(const char * program, char * arg_list[]) {

  pid_t child_pid = fork();

  if(child_pid < 0) {
    perror("Error while forking...");
    return 1;
  }

  else if(child_pid != 0) {
    return child_pid;
  }

  else {
    if(execvp (program, arg_list) == 0);
    perror("Exec failed");
    return 1;
  }
}

int kill_processes( int pid, int status) {
  if (kill(pid, SIGINT)==-1) {
    perror("Something went wrong in kill");
    return -1;
  }
  if (waitpid(pid, &status, 0)==-1) {
    perror("Error in waitpid");
    return -1;
  }
  return 0;
}

int main() {

  //BLOCCO DI CODICE TEMPORANEO
  
  char *file_name[][100]={{"log_command.txt"}, {"log_inspection.txt"}, {"log_mx.txt"}, {"log_mz.txt"}, {"log_world.txt"}, {"controll.txt"}};
  for (int i=0; i<6; i++)
  {
      while(1) {
      	fp[i]=fopen(*file_name[i], "w+");
        if(fp[i] == NULL) {
        	//printf("CREA PADRE?");
        }
        else {
        	//printf("PADRE C'é");
                fclose(fp[i]);
                break;
        }
     }
     //printf("%d",i);
  }
  
  //FINE BLOCCO DI CODICE TEMPORANEO
  
  //INIZIALIZZAZIONI NUOVE
  char * sec[5];
  char * min[5];
  char tmp[10];
  //FINE INIZIALIZZAZIONI NUOVE
  
  //list of all processes
  char *arg_lists[][100]= {{"/usr/bin/konsole", "-e", "./command", *file_name[0], NULL}, {"/usr/bin/konsole", "-e", "./inspection", *file_name[1], NULL}, 
  {"/usr/bin/konsole", "-e", "./mx", *file_name[2], NULL }, {"/usr/bin/konsole", "-e", "./mz", *file_name[3], NULL }, {"/usr/bin/konsole", "-e", "./world", *file_name[4], NULL }};
  //list of processes path
  char *process_path[][100]={{"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {"/usr/bin/konsole"}};
  //array that collects all pid
  int all_pid[5];
  for ( int i=0; i<5; i++)
    all_pid[i]=spawn(*process_path[i], arg_lists[i]);
  char * time_filemod[5];
  if ( time_filemod==NULL)
  	perror("ssss");
  int status;
  while (1) {
    for (int i=0; i<5; i++)
      fp[i]=fopen(*file_name[i], "r");
    if (fp[0]==NULL || fp[1]==NULL || fp[2]==NULL || fp[3]==NULL || fp[4]==NULL) 
        perror("Error in read files");
    for ( int i=0; i<5; i++) {
      time_filemod[i]=readlogfile(*file_name[i]);
      }
    //check if all processes are idle more than 60 seconds
    if (timecheck(time_filemod[0])==1 && timecheck(time_filemod[1])==1 && timecheck(time_filemod[2])==1 && timecheck(time_filemod[3])==1 && timecheck(time_filemod[4])==1) {
      //kill all child processes
      for(int i=0; i<5; i++) {
        sleep(0.5);
        kill_processes(all_pid[i], status);
        }
      //process kill itself
      //kill_processes(all_pid[0], status);
    for (int i = 0; i < 5; i++) {
    	fclose(fp[i]);
    }
    printf ("Main program exiting with status %d\n", status);
    exit(1);
    }
    for (int i = 0; i < 5; i++) {
    	fclose(fp[i]);
    }
  }
  printf ("Main program exiting with status %d\n", status);
  return 0;
}
