#include<stdio.h> // printf()
#include<stdlib.h> // malloc()
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
int kapish_execute(char **args);


void kapish_loop(void){
  char *line;
  char **args;
  int status;

  do {
    printf("?_");
    line = kapish_read_line();      // Read line
    args = kapish_split_line(line); // Split the line into args and execute args
    status = kapish_execute(args);  // Determine when to execute

    free(line);
    free(args);
  } while (status);
}

char *kapish_read_line(void){
  int buffer_size = 512;
  int position = 0;
  char *buffer = malloc(sizeof(char) * buffer_size);  // Allocate memory
  int c;

  if(!buffer){
    fprintf(stderr, "Error: Failed to allocate memory.\n");
    exit(EXIT_FAILURE);
  }

  while(1){
    c = getchar();              // Read a character

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

char ** kapish_split_line(char *line){
  int buffer_size = 64, position = 0;
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

int kapish_launch(char **args){
  pid_t pid;
//  pid_t wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("kapish");
    }
    // Never going to get here if new process is loaded correctly
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
  "exit"
};

int (*builtin_func[]) (char **) = {
  &kapish_cd,
  &kapish_help,
  &kapish_exit
};

int kapish_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/
int kapish_cd(char **args){
  if (args[1] == NULL) {
    fprintf(stderr, "kapish: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("kapish");
    }
  }
  return 1;
}

int kapish_help(char **args){
  int i;
  printf("Possum Nuada's kapish\n");
  printf("Type program names and arguments, and hit enter.\n");
  printf("The following are built in:\n");

  for (i = 0; i < kapish_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  printf("Use the man command for information on other programs.\n");
  return 1;
}

int kapish_exit(char **args){
  return 0;
}

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
  static const char filename[] = "file.txt";
   FILE *file = fopen ( filename, "r" );
   if ( file != NULL )
   {
      char line [ 128 ]; /* or other suitable maximum line size */
      while ( fgets ( line, sizeof line, file ) != NULL ) /* read a line */
      {
         fputs ( line, stdout ); /* write the line */
      }
      fclose ( file );
   }
   else
   {
      perror ( filename ); /* why didn't the file open? */
   }

  // Run command loop
  kapish_loop();

  // Perform any shutdown/cleanup

  return EXIT_SUCCESS;
}
