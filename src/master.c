#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include<signal.h>

FILE *fp[5];
//function that read the log file of our porcesses
char* readlogfile(char *filepath) {
    struct stat filestat;
    char* filemod;
    if (filemod==NULL) 
      return -1;
    char format_string_date[100]="%s, %s, %s, %s, %s";
    char day[80];
    char month[80];
    char num_day[80];
    char time[80];
    char year[80];
    if (stat(filepath,&filestat)==-1)
        perror("Something went wrong");
    filemod=ctime(&filestat.st_mtime);
    sscanf(filemod, format_string_date, day, month, num_day, time, year);
    //time of last file modify
    return time;
}
//check if proces is idle more than 60 seconds
int timecheck( char *filetime) {
  time_t rawtime;
  struct tm * timeinfo;
  char format_string_time[100]="%d, %d, %d";
  if ( timeinfo==NULL || format_string_time==NULL)
    perror("Invalid pointer");
  char actual_time[100];
  int actual_hour=0;
  int actual_minutes=0;
  int actual_seconds=0;
  int hours=0;
  int minutes=0;
  int seconds=0;
  time(&rawtime);
  timeinfo=localtime( &rawtime);
  //actual time
  strftime(actual_time, 100, " %H %M %S", timeinfo);
  //convertion of actual and last modify file time
  sprintf(actual_time, format_string_time, &actual_hour, &actual_minutes, &actual_seconds);
  sprintf(filetime, format_string_time, &hours, &minutes, &seconds);
  //check if 1 minute has gone
  if ( abs(actual_minutes-minutes)>=1 && abs(actual_seconds-seconds)>=0 || abs(actual_minutes-minutes)>=2) 
    return 1;
  return 0;
}


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
//function that pass our file name and pid to processes
int send_pid_file ( const char * program, int pid, char *filename) {
    char pids[100];
    sprintf(pids,"%d", pid);
    if (execlp(program, pids, filename)==0) {
        perror("Error in execlp");
        return -1;
    }
    return 0;
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
  //list of all processes
  char *arg_lists[][100]= {{"/usr/bin/konsole", "-e", "./command", NULL}, {"/usr/bin/konsole", "-e", "./inspection", NULL}, 
  {"./mx", NULL }, {"./mz", NULL }, {"./world", NULL }};
  //list of processes path
  char *process_path[][100]={{"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {NULL}, {NULL}, {NULL}};
  //array that collects all pid
  int all_pid[5];
  for ( int i=0; i<5; i++)
    all_pid[i]=spawn(*process_path[i], arg_lists[i]);
  //set file names
  char *file_name[][100]={"log_command.txt", "log_inspection.txt", "log_mx.txt", "log_mz.txt", "log_world.txt"};
  for ( int i=0; i<5; i++)
    send_pid_file(*process_path[i], all_pid[0], *file_name[i]);
  char *time_filemod[5]={0};
  int status;
  while (1) {
    for (int i=0; i<5; i++)
      fp[i]=fopen(*file_name[i], "r");
    if (fp[0]==NULL || fp[1]==NULL || fp[2]==NULL || fp[3]==NULL || fp[4]==NULL) 
        perror("Error in read files");
    for ( int i=0; i<5; i++)
      time_filemod[i]=readlogfile(*file_name[i]);
    //check if all processes are idle more than 60 seconds
    if (timecheck(time_filemod[0])==1 && timecheck(time_filemod[1])==1 && timecheck(time_filemod[2])==1 && timecheck(time_filemod[3])==1 && timecheck(time_filemod[4])==1) {
      //kill all child processes
      for(int i=0; i<5; i++)
        kill_processes(all_pid[i], status);
      //process kill itself
    fcloseall();
    exit(1);
    }
    fcloseall();
  }
  printf ("Main program exiting with status %d\n", status);
  return 0;
}
