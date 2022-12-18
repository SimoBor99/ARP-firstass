// INCLUDING FUNCTION

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
#include <errno.h> 

FILE *fp[5];

// FUNCTION FOR WRITING ON LOG FILE

void write_log(char * log_text, char * fn)
{
	FILE *fp_log;
	fp_log = fopen(fn,"a");  
	fputs(log_text, fp_log);
	fputs("\n", fp_log);
	//perror("Error in something!"); 
	fclose(fp_log);
}

// FUNCTION FOR READING LAST MODIFY TIME FROM LOG FILES (MASTER IS ALSO WATCHDOG)

char* readlogfile(char *filepath, int k) {
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
    char * try = time;
    return try;
}

// FUNCTION THAT TAKES THE CURRENT TIME AND CONFRONT IT WITH THE LAST MODIFY TIME OF A FILE

int timecheck( char *filetime, char * minutes, char * seconds, int j) {
    char * str = "";
    if (j == 0) {
    	str = "COMMAND: ";
    }
    if (j == 1) {
    	str = "INSPECTION: ";
    }
    if (j == 2) {
    	str = "MX: ";
    }
    if (j == 3) {
    	str = "MZ: ";
    }
    if (j == 4) {
    	str = "WORLD: ";
    }
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
  			write_log(str, "master.txt");
    			write_log("2 O PIU MINUTI", "master.txt");
  			return 1;
  		}
  		else {
  			if (actual_seconds - file_seconds >= 0) {
  				write_log(str, "master.txt");
    				write_log("1 MINUTO E PIU SECONDI", "master.txt");
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

// FUNCTIONS FOR EXTRACTING MINUTES AND SECONDS FROM THE LAST MODIFY TIME OF A LOG FILE

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

// FUNCTION FOR SPAWNING A NEW PROCESS

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

// FUNCTION FOR KILLING A PROCESS, GIVEN ITS PID

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

// FUNCTION FOR CREATING A PIPE AND CONTROLLING IF EVERYTHING GOES RIGHT

void create_and_check(const char * name) {

	if(mkfifo(name, 0666)==-1) {
	
        	if (errno != EEXIST) {
          		perror("Error creating named fifo\n");
          		exit(-1);
		}
	}
}

int main() {

	int all_pid[5];
	
	char * sec[5];
  	char * min[5];
  	int cool[5];
	
	char *file_name[][100]={{"log_command.txt"}, {"log_inspection.txt"}, {"log_mx.txt"}, {"log_mz.txt"}, {"log_world.txt"}};

	char * com_mx = "/tmp/fifo_com-mx";		// DECLARE ALL PIPES NAMES
	char * com_mz = "/tmp/fifo_com-mz";
	char * mx_world = "/tmp/fifo_mx-world";
	char * mz_world = "/tmp/fifo_mz-world";
	char * world_insp_x = "/tmp/fifo_world-insp-x";
	char * world_insp_z = "/tmp/fifo_world-insp-z";
	char * master_insp = "/tmp/fifo_master-insp";
	char * comm_insp = "/tmp/fifo_comm-insp";
	char * com_mx2="/tmp/fifo_com-mx2";
	char * com_mz2="/tmp/fifo_com-mz2";
	
	int fd_mi;
  
	create_and_check(com_mx);			// CREATE ALL THE PIPES NEEDED
	create_and_check(com_mz);
	create_and_check(mx_world);
	create_and_check(mz_world);
	create_and_check(world_insp_x);
	create_and_check(world_insp_z);
	create_and_check(master_insp);
	create_and_check(comm_insp);
	create_and_check(com_mx2);
	create_and_check(com_mz2);

	char *arg_lists[][100]= {{"/usr/bin/konsole", "-e", "./command", *file_name[0], com_mx, com_mz, comm_insp, com_mx2, com_mz2, NULL}, {"/usr/bin/konsole", "-e", "./inspection", *file_name[1], world_insp_x, world_insp_z, master_insp, comm_insp, NULL}, {"./mx", *file_name[2], com_mx, mx_world, com_mx2, NULL }, {"./mz", *file_name[3], com_mz, mz_world, com_mz2, NULL }, {"./world", *file_name[4], mx_world, mz_world, world_insp_x, world_insp_z, NULL }};
  	
  	//list of processes path
  	
  	char *process_path[][100]={{"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {"./mx"}, {"./mz"}, {"./world"}};

	for ( int i=0; i<5; i++) {
    		all_pid[i]=spawn(*process_path[i], arg_lists[i]);
    	}
	
	int status;
	char writepmx[20];
	char writepmz[20];
	//convert pids mx and mz from char to int
	sprintf(writepmx, "%d", all_pid[2]);
	sprintf(writepmz, "%d", all_pid[3]);
	//concatenate the two pids
	strcat(writepmx, ",");
	strcat(writepmx, writepmz);
	strcat(writepmx, ",");
	write_log(writepmx,"master.txt");
	fd_mi=open(master_insp, O_WRONLY);

	if (fd_mi==0) {
		perror("Cannot open m_insp fifo");
		//unlink(master_insp);
	}
	//write the value on pipe
	if (write(fd_mi, writepmx, 20)==-1) {
		perror("Write failure");
	}
	char * time_filemod[5];
  	if ( time_filemod==NULL) {
  		perror("Something went wrong initializing timefilemod");
  	}
	
  	for (int i=0; i<5; i++)
  	{
      		while(1) {
      			fp[i]=fopen(*file_name[i], "w+");
        			if(fp[i] == NULL) {
        				printf("Waiting for file creation...");
        			}
        			else {
        				printf("File crerated successfully!");
                			fclose(fp[i]);
        	        		break;
        			}
     		}
  	}
	
	while (1) {
    		for (int i=0; i<5; i++) {
      			fp[i]=fopen(*file_name[i], "r");
      		}
    		if (fp[0]==NULL || fp[1]==NULL || fp[2]==NULL || fp[3]==NULL || fp[4]==NULL) {
        		perror("Error in read files");
        	}
    		for ( int i=0; i<5; i++) {
      			char tmp[10];
      			time_filemod[i]=readlogfile(*file_name[i],i);
      			strcpy(tmp,time_filemod[i]);
      			min[i] = ext_min(tmp);
      			sec[i] = ext_sec(time_filemod[i]);
      			char tmp1[80];
      			char tmp2[80];
      			strcpy(tmp1, min[i]);
      			strcpy(tmp2, sec[i]);
      			cool[i] = timecheck(time_filemod[i],tmp1,tmp2,i);
     		}
     		
    //check if all processes are idle more than 60 seconds
    
     		if (cool[0] + cool[1] + cool[2] + cool[3] + cool[4] == 5) {
     		
      //kill all child processes
      
      			for(int i=0; i<5; i++) {
        			kill_processes(all_pid[i], status);
        			sleep(0.5);
        		}
        		
        		// WE UNLINK ALL THE PIPES
        		
        		close(fd_mi);
        		
      			if(unlink(com_mx) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(com_mz) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(mx_world) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(mz_world) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(world_insp_x) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(world_insp_z) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(master_insp) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(comm_insp) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(com_mx2) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
      			if(unlink(com_mz2) == -1) {
      				perror("An error happened in unlinking a pipe!");
      			}
        		
      //process kill itself
      
      			kill_processes(all_pid[0], status);
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
	
	waitpid(all_pid[0], &status, 0);
  	waitpid(all_pid[1], &status, 0);
  	waitpid(all_pid[2], &status, 0);
  	waitpid(all_pid[3], &status, 0);
  	waitpid(all_pid[4], &status, 0);
	
	printf ("Main program exiting with status %d\n", status);
	return 0;
}

