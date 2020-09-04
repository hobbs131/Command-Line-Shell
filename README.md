# Command-Line-Shell
  - Simple, quasi-command line shell. The shell is less functional than full fledged standard shells such as bash.
  - Has properties that distinguish it from other shells such as recalling output for any child process.
  - Variety of system calls implemented to accomplish tasks

# Features/Commands
  - help: lists all commands
  - exit: exits the program
  - list: list all the jobs that have been started giving information on each
  - pause nanos secs: pause for the given number of nanoseconds and seconds
  - output-for int: print the output for the given job number
  - output-all: print the output for all jobs
  - wait-for int: wait until the given job number finishes
  - wait-all: wait for all jobs to finish
  - command arg1...: non-built-in is run as job
 
# Usage
  - Download source code, switch to directory containing source code, and compile using the command 'make'
  - Run using command: ./commando
  - Features are listed above

# Demo
 
 ![command](https://user-images.githubusercontent.com/60115853/92271617-c16b2980-eead-11ea-8068-9afd079da3bc.png)
