#define _GNU_SOURCE
#include <stdio.h> // printf
#include <stdlib.h> // malloc()
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

void kapish_loop(void);
char *kapish_read_line(void);
char ** kapish_split_line(char *line);
int kapish_launch(char **args);
int kapish_cd(char **args);
int kapish_help(char **args);
int kapish_exit(char **args);
int kapish_num_builtins();
int kapish_cd(char **args);
int kapish_help(char **args);
int kapish_exit(char **args);
int kapish_setenv(char **args);
int kapish_unsetenv(char **args);
int kapish_execute(char **args);
int kapish_printenv();

extern char **environ;   // environment variables

/*
  Read lines from input and execute
*/
void kapish_loop(void){
  char *line;
  char **args;
  int status;

  do {
    printf("? ");
    line = kapish_read_line();      // Read line
    args = kapish_split_line(line); // Split the line into args and execute args
    status = kapish_execute(args);  // Determine when to execute

    free(line);                     // Free line and args
    free(args);
  } while (status);
}

/*
  Read line from input and return it
*/
char *kapish_read_line(void){
  int buffer_size = 512;
  int position = 0;
  char *buffer = malloc(sizeof(char) * buffer_size);  // Allocate memory
  int c;
  int first = 1;

  if(!buffer){
    fprintf(stderr, "Error: Failed to allocate memory.\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    c = getchar();              // Read a character

    if(first == 1 && c == EOF) exit(EXIT_SUCCESS);
    first = 0;

    if(c == EOF || c =='\n') {  // If end of file of new line
      buffer[position] = '\0';  // Add null terminator
      return buffer;            // Return buffer
    } else {                    // Else
      buffer[position] = c;     // Put c in buffer
    }
    position++;

    if(position >= buffer_size) {
      buffer_size += 512;
      buffer = realloc(buffer, buffer_size);
      if(!buffer){
        fprintf(stderr, "Allocating error\n" );
        exit(EXIT_FAILURE);
      }
    }
  }
}

/*
  Split given line and return as tokens
*/
char ** kapish_split_line(char *line){
  int buffer_size = 512, position = 0;
  char **tokens = malloc(buffer_size * sizeof(char*));
  char *token;

  if(!tokens){
    fprintf(stderr, "Alocation Error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, " \t\r\n\a");
  while(token != NULL){
    tokens[position] = token;
    position++;
    if(position>=buffer_size){
      buffer_size+=64;
      tokens = realloc(tokens, buffer_size * sizeof(char*));
      if(!tokens){
        fprintf(stderr, "Allocation Error\n");
        exit(EXIT_FAILURE);
      }
    }
    token = strtok(NULL, " \t\r\n\a");
  }
  tokens[position] = NULL;
  return tokens;
}

/*
  Forks a child process
*/
int kapish_launch(char **args){
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("kapish");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    // Error forking
    perror("kapish");
  } else {
    // Parent process
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


/*
  List of builtin commands, followed by their corresponding functions.
*/
char *builtin_str[] = {
  "cd",
  "help",
  "exit",
  "setenv",
  "unsetenv",
  "printenv"
};

int (*builtin_func[]) (char **) = {
  &kapish_cd,
  &kapish_help,
  &kapish_exit,
  &kapish_setenv,
  &kapish_unsetenv,
  &kapish_printenv
};

/*
  Returns the number of built-in functions
*/
int kapish_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Built-in function implementations
  See README for details on each function
*/
int kapish_cd(char **args){
  if (args[1] == NULL) {
    chdir(getenv("HOME"));
  } else {
    if (chdir(args[1]) != 0) {
      perror("kapish");
    }
  }
  return 1;
}

int kapish_help(char **args){
  printf("Possum Nuada's kapish\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (int i = 0; i < kapish_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("See the README file for more information about the built-in commands.\n");
  printf("Use the man command for information on other programs.\n");
  return 1;
}

int kapish_exit(char **args){
  return 0;
}

int kapish_setenv(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "kapish: expected argument to setenv\n");
  }else if(args[2] == NULL){
    fprintf(stderr, "kapish: expected value for %s\n", args[1]);
  }else{
    int set = setenv(args[1], args[2], 1);
    if(set == -1){
      fprintf(stderr, "kapish: failed to set %s to %s\n", args[1], args[2]);
    }
  }
  return 1;
}

int kapish_unsetenv(char **args){
  if(args[1] == NULL){
    fprintf(stderr, "kapish: expected argument to unsetenv\n");
  }else{
    int unset = unsetenv(args[1]);
    if(unset == -1){
      fprintf(stderr, "kapish: failed to unset %s\n", args[1]);
    }
  }
  return 1;
}

int kapish_printenv() {
  int i = 1;
  char *s = *environ;

  for (; s; i++) {
      printf("%s\n", s);
      s = *(environ+i);
  }

  return 1;
}

/*
  Executes the provided command
*/
int kapish_execute(char **args){
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < kapish_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

 return kapish_launch(args);
}

int main(int argc, char **argv){
  // Load config files, if there are any
  char **args;
  static const char filename[] = ".kapishrc";
  FILE *file = fopen ( filename, "r" );
  if ( file != NULL ){
    char line [512];
    while ( fgets ( line, sizeof line, file ) != NULL ){ // Read file line by line
       printf("? ");
       fputs ( line, stdout );         // write the line
       args = kapish_split_line(line); // Split the line into args and execute args
       kapish_execute(args);  // Determine when to execute
    }
    fclose ( file );
  }
  else{
    perror ( filename ); // Error opening file
  }

  // Run command loop
  kapish_loop();

  return EXIT_SUCCESS;
}
