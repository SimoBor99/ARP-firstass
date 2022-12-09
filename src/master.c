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
      return -1;
    char format_string_date[100]="%s, %s, %s, %s, %s";
    char day[80];
    char month[80];
    char num_day[80];
    char year[80];
    if (stat(filepath,&filestat)==-1)
        perror("Something went wrong");
    filemod=ctime(&filestat.st_mtime);
    int i = 0;
    char * time = strtok(filemod, " ");
    while (i < 3) {
    	time = strtok(NULL, " ");
    	i++;
    }
    //time of last file modify
    char * try = time;
    return try;
}
//check if proces is idle more than 60 seconds
int timecheck( char *filetime, char * minutes, char * seconds) {
  time_t rawtime;
  struct tm * timeinfo;
  char format_string_time[100]="%d, %d, %d";
  if ( timeinfo==NULL || format_string_time==NULL)
    perror("Invalid pointer");
  char actual_time[100];
  char * pnt = &actual_time[0];
  char * at;
  int actual_minutes=0;
  int actual_seconds=0;
  int file_minutes=0;
  int file_seconds=0;
  time(&rawtime);
  timeinfo=localtime( &rawtime);
  //actual time
  strftime(actual_time, 100, "%H %M %S", timeinfo);
  //convertion of actual and last modify file time
  char * h = strtok(pnt," ");
  char * m = strtok(NULL," ");
  char * s = strtok(NULL," ");
  actual_minutes = atoi(m);
  actual_seconds = atoi(s);
  file_minutes = atoi(minutes);
  file_seconds = atoi(seconds);
  //check if 1 minute has gone
  if (actual_minutes != file_minutes) {
  	if (actual_minutes == 0) {
  		actual_minutes = 60;
  	}
  	if (abs(actual_minutes-file_minutes) > 1) {
  		return 1;
  	}
  	else {
  		if (actual_seconds - file_seconds >= 0) {
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

// NUOVE FUNZIONI PER ESTRARRE MINUTI E ORE

char* ext_min(char* times) {
	int i = 0;
	char * min = strtok(times, ":");
	while (i < 1) {
		min = strtok(NULL, ":");
    		i++;
	}
	return min;
}

char* ext_sec(char* times) {
	int i = 0;
	char * sec = strtok(times, ":");
	while (i < 2) {
		sec = strtok(NULL, ":");
    		i++;
	}
	return sec;
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
  
  char *file_name[][100]={{"log_command.txt"}, {"log_inspection.txt"}, {"log_mx.txt"}, {"log_mz.txt"}, {"log_world.txt"}};
  for (int i=0; i<5; i++)
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
      strcpy(tmp,time_filemod[i]);
      //printf("%s",tmp);
      //printf("\n");
      min[i] = ext_min(tmp);
      sec[i] = ext_sec(time_filemod[i]);
      }
    //check if all processes are idle more than 60 seconds
    if (timecheck(time_filemod[0],min[0],sec[0])==1 && timecheck(time_filemod[1],min[1],sec[1])==1 && timecheck(time_filemod[2],min[2],sec[2])==1 && timecheck(time_filemod[3],min[3],sec[3])==1 && timecheck(time_filemod[4],min[4],sec[4])==1) {
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
