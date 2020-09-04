#include "commando.h"

int main(int argc, char *argv[]){
  // Environmental variable COMMANDO_ECHO
  char* COMMANDO_ECHO = getenv("COMMANDO_ECHO");
  setvbuf(stdout, NULL, _IONBF, 0);
  cmdcol_t *cmd_col = malloc(sizeof(cmdcol_t));
  cmd_col->size = 0;
  // Input loop
  while(1){
    int ntoks;
    char *tokens[ARG_MAX];
    char buffer[MAX_LINE];

    printf("@> ");
    char *input = fgets(buffer,MAX_LINE,stdin);
    // If no input remains, print and break out of loop as requested.
    if(input == NULL){
      printf("\nEnd of input");
      break;
    }

    // If echoing is enabled, print the output as requested.
    if((argc > 1 && strcmp(argv[1], "--echo") == 0) || COMMANDO_ECHO != NULL ){
      printf("%s", input);
    }
    parse_into_tokens(input,tokens,&ntoks);
    // If there are no tokens, jump to end of loop
    if(tokens[0] == NULL){
      continue;
    }
    // If/else statements for built-ins/non-built-in to be run as a job. Each statement self-described by help command
    // Uses atoi() to convert strings to ints and strcmp() to recognize given command
    if (strcmp(tokens[0], "help") == 0){
      printf("COMMANDO COMMANDS\n");
      printf("help               : show this message\n");
      printf("exit               : exit the program\n");
      printf("list               : list all jobs that have been started giving information on each\n");
      printf("pause nanos secs   : pause for the given number of nanseconds and seconds\n");
      printf("output-for int     : print the output for given job number\n");
      printf("output-all         : print output for all jobs\n");
      printf("wait-for int       : wait until the given job number finishes\n");
      printf("wait-all           : wait for all jobs to finish\n");
      printf("command arg1 ...   : non-built-in is run as a job\n");
    }
    else if (strcmp(tokens[0],"exit") == 0){
      break;
    }
    else if(strcmp(tokens[0],"list") == 0){
      cmdcol_print(cmd_col);
    }
    else if(strcmp(tokens[0], "pause") == 0){
      pause_for(atoi(tokens[1]), atoi(tokens[2]));
    }
    else if(strcmp(tokens[0], "output-for") == 0){
      int job_number = atoi(tokens[1]);
      printf("@<<< Output for %s[#%d] (%d bytes):\n",cmd_col->cmd[job_number]->name,cmd_col->cmd[job_number]->pid,cmd_col->cmd[job_number]->output_size);
      printf("----------------------------------------\n");
      cmd_print_output(cmd_col->cmd[job_number]);
      printf("----------------------------------------\n");
    }
    else if(strcmp(tokens[0], "output-all") == 0){
      for(int i = 0; i < cmd_col->size; i++){
        printf("@<<< Output for %s[#%d] (%d bytes):\n",cmd_col->cmd[i]->name,cmd_col->cmd[i]->pid,cmd_col->cmd[i]->output_size);
        printf("----------------------------------------\n");
        cmd_print_output(cmd_col->cmd[i]);
        printf("----------------------------------------\n");
      }
    }
    else if (strcmp(tokens[0], "wait-for") == 0){
      int wait_job_number = atoi(tokens[1]);
      cmd_update_state(cmd_col->cmd[wait_job_number],DOBLOCK);
    }

    else if(strcmp(tokens[0], "wait-all") == 0){
      cmdcol_update_state(cmd_col,DOBLOCK);
    }
    else{
      // Allocate, add to cmd array, and start a cmd.
      cmd_t *cmd = cmd_new(tokens);
      cmdcol_add(cmd_col,cmd);
      cmd_start(cmd);
    }
    // Update state at end of input loop as requested
    cmdcol_update_state(cmd_col,NOBLOCK);
  }
  // Free all allocated memory and exit program.
  cmdcol_freeall(cmd_col);
  free(cmd_col);
  exit(1);
}
