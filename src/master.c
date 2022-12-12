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
#include <fcntl.h>

const int dim=100;
FILE *fp[5];
//function that read the log file of our processes

const char* readlogfile(char *filepath) {
    struct stat filestat;
    char day[dim];
    char month[dim];
    char time[dim];
    char num_day[dim];
    char year[dim];
    if (stat(filepath,&filestat)==-1)
        perror("Something went wrong");
    char *filemod=ctime(&filestat.st_mtime);
    sscanf(filemod, "%s %s  %s %s %s", day, month, num_day, time, year);
    const char const *ptime=&time[0];
    //printf("%s\n", ptime);
    return ptime;
}

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");
  if (fp_log==NULL)
    perror("Error in open file");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

int timecheck( const char const *filetime) {
  time_t rawtime;
  char current_time[dim];
  char file_time[dim];
  int current_hour=0;
  int current_minutes=0;
  int current_seconds=0;
  int file_minutes=0;
  int file_seconds=0;
  int file_hour=0;
  //printf("D: %s\n", filetime);
  time(&rawtime);
  struct tm *timeinfo=localtime( &rawtime);
  //actual time
  strftime(current_time, dim, "%H:%M:%S", timeinfo);
  //convertion of current and last modify file time
  memcpy(file_time, filetime, sizeof(filetime));
  sscanf(current_time, "%d:%d:%d", &current_hour, &current_minutes, &current_seconds);
  sscanf(file_time, "%d:%d:%d", &file_hour, &file_minutes, &file_seconds);
  if (current_minutes != file_minutes) {
  	if (current_minutes == 0) {
  		current_minutes = 60;
  	}
  	if (abs(current_minutes-file_minutes)>1) {
  		return 1;
  	}
  	else {
  		if ((current_seconds-file_seconds)>= 0) {
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
  if (kill(pid, SIGTERM)==-1) {
    perror("Something went wrong in kill");
    return -1;
  }
  if (waitpid(pid, &status, 0)==-1) {
    perror("Error in waitpid");
    return -1;
  }
  return 0;
}
//some functions that I have to implement later
/*void open_file(const char** const file_name) {
      for ( int i=0; i<5; i++) {
        fp[i]=fopen(*file_name[i], "r");
        if (fp[i]==NULL)
          perror("Somethin went wrong in opening file");
      }
}

void close_file(const char** const file_name, FILE **fp) {
  for ( int i=0; i<5; i++)
    fclose(fp[i]);
}*/

int check_pipe( char *myfifo) {
  if (mkfifo(myfifo, 0666)!=0) {
    perror("Cannot open fifo. Already exists?");
    exit(EXIT_FAILURE);
  }
  return 0;
}

int main() {
  int fd;
  char write_pidmx[dim];
  char write_pidmz[dim];
  char *myfifo[][100]={{"/tmp/myfifo_insp"}, {"/tmp/myfifo_inspx"}, {"/tmp/myfifo_inspz"}, {"/tmp/myfifo_comandx"}, {"/tmp/myfifo_comandz"}, {"/tmp/myfifo_worldz"}, {"/tmp/myfifo_worldx"}};
  char *file_name[][100]={{"log_command.txt"}, {"log_inspection.txt"}, {"log_mx.txt"}, {"log_mz.txt"}, {"log_world.txt"}};
  //list of all processes
  char *arg_lists[][100]= {{"/usr/bin/konsole", "-e", "./command", *file_name[0], *myfifo[3], *myfifo[4], NULL}, {"/usr/bin/konsole", "-e", "./inspection", *file_name[1], *myfifo[1], *myfifo[2], *myfifo[0], NULL}, 
  {"./mx", *file_name[2], *myfifo[3], *myfifo[6],  NULL }, {"./mz", *file_name[3], *myfifo[4], *myfifo[5], NULL }, {"./world", *file_name[4], *myfifo[1], *myfifo[2], *myfifo[5], *myfifo[6], NULL }};
  //list of processes path
  char *process_path[][100]={{"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {"./mx"}, {"./mz"}, {"./world"}};
  //array that collects all pid
  for (int i=0; i<5; i++)
  {
      while(1) {
      	fp[i]=fopen(*file_name[i], "a");
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
  for ( int i=0; i<7; i++)
    check_pipe(*myfifo[i]);
  int all_pid[5];
  for ( int i=0; i<5; i++)
    all_pid[i]=spawn(*process_path[i], arg_lists[i]);
  fd=open(*myfifo[0], O_WRONLY);
  if (fd==0) {
    perror("Cannot open fifo");
    unlink(*myfifo[0]);
    exit(1);
  }
  sprintf(write_pidmx, "%d", all_pid[2]);
  sprintf(write_pidmz, "%d", all_pid[3]);
  strcat(write_pidmx, ",");
  strcat(write_pidmx, write_pidmz);
  if (write(fd, write_pidmx, dim)==-1)
    perror("Something went wrong");
  write_log(write_pidmx, "inspection_log.txt");
  int status;
  const char * time_filemod[5];
  while (1) {
    for (int i=0; i<5; i++) {
    fp[i]=fopen(*file_name[i], "r");
      if(fp[i] == NULL) {
        perror("Error in opening file!");
      }
  }
    for ( int i=0; i<5; i++)
      time_filemod[i]=readlogfile(*file_name[i]);
    //check if all processes are idle more than 60 seconds
    if (timecheck(time_filemod[0])==1 && timecheck(time_filemod[1])==1 && timecheck(time_filemod[2])==1 && timecheck(time_filemod[3])==1 && timecheck(time_filemod[4])==1) {
      //kill all child processes
      for(int i=0; i<5; i++) {
        sleep(0.5);
        kill_processes(all_pid[i], status);
        }
      //process kill itself
      //
      for (int i = 0; i < 5; i++)
    	  fclose(fp[i]);
      close(fd);
      for ( int i=0; i<7; i++)
        unlink(*myfifo[i]);
      printf ("Main program exiting with status %d\n", status);
      exit(1);
    }
    for ( int i=0; i<5; i++)
      fclose(fp[i]);

  }
  /*for ( int i=0; i<7; i++)
        unlink(*myfifo[i]);*/
  return 0;
}
