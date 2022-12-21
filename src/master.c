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

//array of file pointers
FILE *fp[5];

//array of pid
int all_pid[5];
int status;

// Declare all pipe names
char * com_mx = "/tmp/fifo_com-mx";		
char * com_mz = "/tmp/fifo_com-mz";
char * mx_world = "/tmp/fifo_mx-world";
char * mz_world = "/tmp/fifo_mz-world";
char * world_insp_x = "/tmp/fifo_world-insp-x";
char * world_insp_z = "/tmp/fifo_world-insp-z";
char * master_insp = "/tmp/fifo_master-insp";
char * comm_insp = "/tmp/fifo_comm-insp";
char * com_mx2="/tmp/fifo_com-mx2";
char * com_mz2="/tmp/fifo_com-mz2";


// FUNCTION FOR READING LAST MODIFY TIME FROM LOG FILES (MASTER IS ALSO WATCHDOG)

char* readlogfile(char *filepath, int k) {
    struct stat filestat;
    char * filemod;			
    if (filemod==NULL) 
      perror("Invalid pointer!");
    char format_string_date[100]="%s, %s, %s, %s, %s";
    char day[80];
    char month[80];
    char num_day[80];
    char year[80];
    if (stat(filepath,&filestat)==-1)
        perror("Something went wrong");
	//filemod contains data and time of last modify
    filemod=ctime(&filestat.st_mtime);
    int i = 0;
    char * time = strtok(filemod, " ");
	//here we extract what we need, only the time
    while (i < 3) {
    	time = strtok(NULL, " ");
    	i++;
    }
    char * try = time;
    return try;
}

// FUNCTION THAT TAKES THE CURRENT TIME AND CONFRONT IT WITH THE LAST MODIFY TIME OF A FILE

int timecheck( char *filetime, char * minutes, char * seconds, int j) {
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

// FUNCTION FOR KILLING A PROCESS, GIVEN ITS PID

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

// function for unlink pipes when user press ctrl+c
void unlink_handler (int signum) {
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
	exit(1);
}

// handler for managing something wrong in other processes
void error_handler (int signum) {
	if (signum==SIGUSR1) {
		for (int i=0; i<5; i++) {
			sleep(0.5);
			kill_processes(all_pid[i], status);
		}
	
		if(unlink(com_mx) == -1) {
      		perror("An error happened in unlinking a pipe!");
    	}
    	if (unlink(com_mz) == -1) {
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
		exit(1);
	}

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
	
	char * sec[5];
  	char * min[5];
  	int check_inactivity[5];
	//list of log files
	char *file_name[][100]={{"log_command.txt"}, {"log_inspection.txt"}, {"log_mx.txt"}, {"log_mz.txt"}, {"log_world.txt"}};

	
	int fd_mi;
	// CREATE ALL THE PIPES NEEDED
	create_and_check(com_mx);			
	create_and_check(com_mz);
	create_and_check(mx_world);
	create_and_check(mz_world);
	create_and_check(world_insp_x);
	create_and_check(world_insp_z);
	create_and_check(master_insp);
	create_and_check(comm_insp);
	create_and_check(com_mx2);
	create_and_check(com_mz2);
	// list of arguments of all processes
	char *arg_lists[][100]= {{"/usr/bin/konsole", "-e", "./bin/command", *file_name[0], com_mx, com_mz, comm_insp, com_mx2, com_mz2, NULL}, {"/usr/bin/konsole", "-e", "./bin/inspection", *file_name[1], world_insp_x, world_insp_z, master_insp, comm_insp, NULL}, {"./bin/mx", *file_name[2], com_mx, mx_world, com_mx2, NULL }, {"./bin/mz", *file_name[3], com_mz, mz_world, com_mz2, NULL }, {"./bin/world", *file_name[4], mx_world, mz_world, world_insp_x, world_insp_z, NULL }};
  	
  	//list of processes path
  	char *process_path[][100]={{"/usr/bin/konsole"}, {"/usr/bin/konsole"}, {"./bin/mx"}, {"./bin/mz"}, {"./bin/world"}};
	//Signal for unlink pipes when user press ctrl+c
	struct sigaction s_unlink, s_error;
	memset(&s_unlink, 0, sizeof(s_unlink));
	memset(&s_error, 0, sizeof(s_error));
	s_unlink.sa_handler=&unlink_handler;
	s_error.sa_handler=&error_handler;
	//catch signal handler
	if (sigaction(SIGINT, &s_unlink, 0)==-1)
    	perror("Can't catch the signal");
	if (sigaction(SIGUSR1, &s_error, 0)==-1)
    	perror("Can't catch the signal");
	for (int i=0; i<5; i++)
  	{
      	fp[i]=fopen(*file_name[i], "w");
        if(fp[i] == NULL) {
        	perror("Error in creating file");
        }
        else {
            fclose(fp[i]);
        }
     	
  	}
	for ( int i=0; i<5; i++) {
    		all_pid[i]=spawn(*process_path[i], arg_lists[i]);
    	}
	
	// HERE, WE SEND THE 2 MOTORS PIDS TO THE INSPECTION PROCESS
	
	char writepmx[20];
	char writepmz[20];
	//convert pids mx and mz from char to int
	sprintf(writepmx, "%d", all_pid[2]);
	sprintf(writepmz, "%d", all_pid[3]);
	//concatenate the two pids
	strcat(writepmx, ",");
	strcat(writepmx, writepmz);
	strcat(writepmx, ",");
	//send pids to inspection process
	fd_mi=open(master_insp, O_WRONLY);

	if (fd_mi==0) {
		perror("Cannot open m_insp fifo");
		if (unlink(master_insp)==-1)
			perror("Cannot unlink the pipe");
	}
	//write the value on pipe
	if (write(fd_mi, writepmx, 20)==-1) {
		perror("Write failure");
	}
	if (close(fd_mi)==-1)
		perror("Cannot close the file descriptor");
	//array of last modify on log file
	char * time_filemod[5];
  	if ( time_filemod==NULL) {
  		perror("Something went wrong initializing timefilemod");
  	}
	
	
	while (1) {
			//open file descriptor
    		for (int i=0; i<5; i++) {
      			fp[i]=fopen(*file_name[i], "r");
      		}
			//check if they are opened correctly 
    		if (fp[0]==NULL || fp[1]==NULL || fp[2]==NULL || fp[3]==NULL || fp[4]==NULL) {
        		perror("Error in read files");
        	}
    		for ( int i=0; i<5; i++) {
      			char tmp[10];
				//save for each process, the last modify
      			time_filemod[i]=readlogfile(*file_name[i],i);
      			strcpy(tmp,time_filemod[i]);
				//extract minutes and seconds from the last modify
      			min[i] = ext_min(tmp);
      			sec[i] = ext_sec(time_filemod[i]);
      			char tmp1[80];
      			char tmp2[80];
      			strcpy(tmp1, min[i]);
      			strcpy(tmp2, sec[i]);
				//save for each process, the current time
      			check_inactivity[i] = timecheck(time_filemod[i],tmp1,tmp2,i);
     		}
     		
    //check if all processes are idle more than 60 seconds
    
     		if (check_inactivity[0] + check_inactivity[1] + check_inactivity[2] + check_inactivity[3] + check_inactivity[4] == 5) {
     		
      //kill all child processes
      
      			for(int i=0; i<5; i++) {
        			kill_processes(all_pid[i], status);
        			sleep(0.5);
        		}
        		
        		// WE UNLINK ALL THE PIPES
        		
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
        		
      			//process kill itself, but before it closse all file descriptor
      
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
	//wait that all child terms
	waitpid(all_pid[0], &status, 0);
  	waitpid(all_pid[1], &status, 0);
  	waitpid(all_pid[2], &status, 0);
  	waitpid(all_pid[3], &status, 0);
  	waitpid(all_pid[4], &status, 0);
	
	printf ("Main program exiting with status %d\n", status);
	return 0;
}

