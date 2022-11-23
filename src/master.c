#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/stat.h>
#include <math.h>
#include<signal.h>

FILE *fp1;
FILE *fp2;
FILE *fp3;
FILE *fp4;
FILE *fp5;
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
int timecheck( char* filetime) {
  time_t rawtime;
  struct tm * timeinfo;
  char format_string_time[100]="%d, %d, %d";
  if ( timeinfo==NULL || format_string_time==NULL)
    return -1;
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
  strftime(actual_time, 100, " %H:%M:%S", timeinfo);
  //convertion of actual and last modify file time
  sprintf(actual_time, format_string_time, &actual_hour, &actual_minutes, &actual_seconds);
  sprintf(filetime, format_string_time, &hours, &minutes, &seconds);
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
//function that pass filename and pid to processes
int send_pid_file ( const char * program, int pid, char *filename) {
    char pids[100];
    sprintf(pids,"%d", pid);
    if (execlp(program, pids, filename)==0) {
        perror("Error in execlp");
        return -1;
    }
    return 0;
}

int main() {

  char * arg_list_command[] = { "/usr/bin/konsole", "-e", "./bin/command", NULL };
  char * arg_list_inspection[] = { "/usr/bin/konsole", "-e", "./bin/inspection", NULL };
  char * arg_list_mx[] = { "/usr/bin/mx", "-e", "./bin/mx", NULL };
  char * arg_list_mz[] = { "/usr/bin/mz", "-e", "./bin/mz", NULL };
  char * arg_list_world[] = { "/usr/bin/world", "-e", "./bin/world", NULL };
  pid_t pid_cmd = spawn("/usr/bin/konsole", arg_list_command);
  pid_t pid_insp = spawn("/usr/bin/konsole", arg_list_inspection);
  pid_t pid_mx=spawn("/usr/bin/mx", arg_list_mx);
  pid_t pid_mz=spawn("/usr/bin/mz", arg_list_mz);
  pid_t pid_world=spawn("/usr/bin/world", arg_list_world);
  //set file names
  char *file_name[]={"log_command.txt", "log_inspection.txt", "log_mx.txt", "log_mz.txt", "log_world.txt"};
  //serial calls of send_pid_file
  send_pid_file ( arg_list_command[2], pid_cmd, file_name[0]);
  send_pid_file ( arg_list_inspection[2], pid_insp, file_name[1]);
  send_pid_file ( arg_list_mx[2], pid_mx, file_name[2]);
  send_pid_file ( arg_list_mz[2], pid_mz, file_name[3]);
  send_pid_file ( arg_list_world[2], pid_world, file_name[4]);
  char filemod[5]={0};
  int status;
  if (filemod==NULL)
    return -1;
  while (1) {
    fp1=fopen("log_command.txt", "r");
    fp2=fopen("log_inspection.txt", "r");
    fp3=fopen("log_mx.txt", "r");
    fp4=fopen("log_mz.txt", "r");
    fp5=fopen("log_world.txt", "r");
    if (fp1==NULL || fp2==NULL || fp3==NULL || fp4==NULL || fp5==NULL) 
        perror("Error in read files");
    filemod[0]=readlogfile(file_name[0]);
    filemod[1]=readlogfile(file_name[1]);
    filemod[2]=readlogfile(file_name[2]);
    filemod[3]=readlogfile(file_name[3]);
    filemod[4]=readlogfile(file_name[4]);
    //check if all processes are idle more than 60 seconds
    if (timecheck(filemod[0])==1 && timecheck(filemod[1])==1 && timecheck(filemod[2])==1 && timecheck(filemod[3])==1 && timecheck(filemod[4])==1) {
      //kill all child processes
      kill(pid_cmd, SIGINT);
      waitpid(pid_cmd, &status, 0);
      kill(pid_insp, SIGINT);
      waitpid(pid_insp, &status, 0);
      kill(pid_mx, SIGINT);
      waitpid(pid_mx, &status, 0);
      kill(pid_mz, SIGINT);
      waitpid(pid_mz, &status, 0);
      kill(pid_world, SIGINT);
      waitpid(pid_world, &status, 0) ;
      //process kill itself
      fcloseall();
      exit(1);
    }
    fcloseall();
  }
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

