#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>


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

  int status;
  while (1) {
    
  }
  
  printf ("Main program exiting with status %d\n", status);
  return 0;
}

