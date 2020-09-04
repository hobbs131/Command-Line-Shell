#include "commando.h"

cmd_t *cmd_new(char *argv[]){
  cmd_t *new_cmd = malloc(sizeof(cmd_t));
  int end_arg = 0;
  // Make string copies to argv
  for (int i = 0; argv[i] != NULL && i < ARG_MAX; i++){
    new_cmd->argv[i] = strdup(argv[i]);
    end_arg++;
  }
  new_cmd->argv[end_arg] = NULL;
  strncpy(new_cmd->name, argv[0], NAME_MAX); // ADDED BY TA Chase for Debugging
  new_cmd->finished = 0;
  snprintf(new_cmd->str_status,STATUS_LEN,"%s","INIT");
  //Set remaining fields to defaults
  new_cmd->pid = -1;
  new_cmd->out_pipe[0] = -1;
  new_cmd->out_pipe[1] = -1;
  new_cmd->status = -1;
  new_cmd->output = NULL;
  new_cmd->output_size = -1;
  return new_cmd;

}

// Allocates a new cmd_t with the given argv[] array. Makes string
// copies of each of the strings contained within argv[] using
// strdup() as they likely come from a source that will be
// altered. Ensures that cmd->argv[] is ended with NULL. Sets the name
// field to be the argv[0]. Sets finished to 0 (not finished yet). Set
// str_status to be "INIT" using snprintf(). Initializes the remaining
// fields to obvious default values such as -1s, and NULLs.

void cmd_free(cmd_t *cmd){
  for(int i = 0; cmd->argv[i] != NULL; i++){
    free(cmd->argv[i]);
  }
  if(cmd->output != NULL){
    free(cmd->output);
  }
  free(cmd);
}
// Deallocates a cmd structure. Deallocates the strings in the argv[]
// array. Also deallocats the output buffer if it is not
// NULL. Finally, deallocates cmd itself.

void cmd_start(cmd_t *cmd){

  pipe(cmd->out_pipe);
  snprintf(cmd->str_status,STATUS_LEN,"%s","RUN");
  pid_t child = fork();

  // Child code
  if (child == 0){
    // Redirect standard input, close write end of pipe, and execute indicated program
    dup2(cmd->out_pipe[PWRITE], STDOUT_FILENO);
    close(cmd->out_pipe[PREAD]);
    execvp(cmd->name, cmd->argv);
  }
  // Parent code
  cmd->pid = child;
  // Close write end of pipe
  close(cmd->out_pipe[PWRITE]);
}

// Forks a process and starts executes command in cmd in the process.
// Changes the str_status field to "RUN" using snprintf().  Creates a
// pipe for out_pipe to capture standard output.  In the parent
// process, ensures that the pid field is set to the child PID. In the
// child process, directs standard output to the pipe using the dup2()
// command. For both parent and child, ensures that unused file
// descriptors for the pipe are closed (write in the parent, read in
// the child).

void cmd_update_state(cmd_t *cmd, int block){
  // If command is finished, do not check again
  if (cmd->finished == 1){
    return;
  }
  int status;
  pid_t return_value = waitpid(cmd->pid,&status,block);

  // Error has occurred, exit with non-zero status
  if (return_value == -1){
    exit(1);
  }
  // No status change. Nothing left to do -- continue program execution
  else if (return_value == 0){
    return;
  }
  // There is a state change
  else{
    // Child process has exited. Retrieve its return code (exit_status), change its status, set finished field, and
    // Read contents of pipe using cmd_fetch_output
    if (WIFEXITED(status)){
      int exit_status = WEXITSTATUS(status);
      cmd->status = exit_status;
      snprintf(cmd->str_status,STATUS_LEN,"EXIT(%d)",cmd->status);
      cmd->finished = 1;
      printf("@!!! %s[#%d]: %s\n", cmd->name, cmd->pid, cmd->str_status);
      cmd_fetch_output(cmd);
    }
  }
}

// If the finished flag is 1, does nothing. Otherwise, updates the
// state of cmd.  Uses waitpid() and the pid field of command to wait
// selectively for the given process. Passes block (one of DOBLOCK or
// NOBLOCK) to waitpid() to cause either non-blocking or blocking
// waits.  Uses the macro WIFEXITED to check the returned status for
// whether the command has exited. If so, sets the finished field to 1
// and sets the cmd->status field to the exit status of the cmd using
// the WEXITSTATUS macro. Calls cmd_fetch_output() to fill up the
// output buffer for later printing.
//
// When a command finishes (the first time), prints a status update
// message of the form
//
// @!!! ls[#17331]: EXIT(0)
//
// which includes the command name, PID, and exit status.

char *read_all(int fd, int *nread){
  int cur_pos = 0;
  int max_size = 1024;
  char *input = calloc(max_size, max_size * sizeof(char) + 1);
  int n_read;

  while(1){
    // reallocate buffer if buffer runs out of space
    if(cur_pos >= max_size){
      max_size = max_size * 2;
      input = realloc(input, max_size);
      // Assure nothing has gone wrong during reallocation
      if(input == NULL){
        exit(1);
      }
    }
    // Read from given file descriptor, fd
    int max_read = max_size - cur_pos;
    n_read = read(fd, input+cur_pos, max_read);

    // No more input to be read, set nread to total bytes read
    if(n_read == 0){
      *nread = cur_pos;
      break;
    }
    // Read has failed, exit
    else if(n_read == -1){
      exit(1);
    }
    // Advance input buffer position
    cur_pos += n_read;
  }
  // Null terminate as requested
  input[max_size - 1] = '\0';
  return input;
  }
// Reads all input from the open file descriptor fd. Stores the
// results in a dynamically allocated buffer which may need to grow as
// more data is read.  Uses an efficient growth scheme such as
// doubling the size of the buffer when additional space is
// needed. Uses realloc() for resizing.  When no data is left in fd,
// sets the integer pointed to by nread to the number of bytes read
// and return a pointer to the allocated buffer. Ensures the return
// string is null-terminated. Does not call close() on the fd as this
// is done elsewhere.

void cmd_fetch_output(cmd_t *cmd){
  // Command hasn't finished continue executing program (input loop)
  if (cmd->finished == 0){
      printf("%s[#%d] not finished yet ", cmd->name, cmd->pid);
      return;
  }
  // Read bytes from the pipe, set output, and close the pipe
  char *output = read_all(cmd->out_pipe[PREAD],&cmd->output_size);
  cmd->output = output;
  close(cmd->out_pipe[PREAD]);
}
// If cmd->finished is zero, prints an error message with the format
//
// ls[#12341] not finished yet
//
// Otherwise retrieves output from the cmd->out_pipe and fills
// cmd->output setting cmd->output_size to number of bytes in
// output. Makes use of read_all() to efficiently capture
// output. Closes the pipe associated with the command after reading
// all input.

void cmd_print_output(cmd_t *cmd){
  // If output is not ready, continue program execution.
  if (cmd->output == NULL){
    printf("%s[#%d] : output not ready\n", cmd->name, cmd->pid);
  }
  // If output is ready, write to screen.
  else{
    write(STDOUT_FILENO, cmd->output, cmd->output_size);
  }
}
// Prints the output of the cmd contained in the output field if it is
// non-null. Prints the error message
//
// ls[#17251] : output not ready
//
// if output is NULL. The message includes the command name and PID.
