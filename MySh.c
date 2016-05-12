#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096
#define MAX_ARGS 32

//Assinaturas das funções.
void prompt();
void prepareArguments(char *command, char ** arguments);
void execute(char **arguments);
void handle_signal(int signo);
int changeDirectory(char* directory);

int main(){
  char command[BUFFER_SIZE];
  char *arguments[MAX_ARGS];

  //Tratamento para CTRL+C e CTRL+Z
  signal(SIGINT, handle_signal);
  signal(SIGTSTP, handle_signal);
  
  do{
    prompt();
    if(fgets(command, sizeof(command), stdin) == NULL)
    { //Ctrl+D
      printf("exit\n");
      exit(0);
    }
    
    if(strcmp(command, "\n"))
    {
    	strtok(command, "\n");
    
    	//Separa os argumentos
    	prepareArguments(command,arguments);

    	//se o comando for 'exit', sai do programa.
    	if (!strcmp(arguments[0], "exit")) exit(0);
    
	//implementação do cd
	if (!strcmp(arguments[0], "cd")){
           changeDirectory(arguments[1]);
        }else
    	//executa o comando
    	execute(arguments);
     }
  }while(1);
  
  return 0;
}

/*
Replace de string
*/
char * replace_str(char *str, char *orig, char *rep)
{
  static char buffer[BUFFER_SIZE];
  char *p;

  if(!(p = strstr(str, orig)))
    return str;

  strncpy(buffer, str, p-str);
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}

/*
 Método para imprimir o prompt.
*/
void prompt(){
  char *userName;
  char host[BUFFER_SIZE];
  char dir[BUFFER_SIZE];
  char *path;

  //busca o nome do usuário
  userName = getenv("USER");

  //busca o nome do host
  gethostname(host, BUFFER_SIZE);

  //busca o diretório em que o usuário se encontra
  if (getcwd(dir, sizeof(dir)) == NULL) perror("Runtime error : can't execute cwd");
  path = dir;
  path = replace_str(path, "/home/", "");
  path = replace_str(path, userName, "");

  printf("[MySh]%s@%s:~%s/$ ", userName, host, path);
  fflush(stdout);
}

/*
  Função para separa o comando e os argumentos.
*/
void prepareArguments(char * command, char ** arguments, char ** argumentsArray){
  while (*command != '\0') 
  {
    while (*command == ' ' || *command == '\t' || *command == '\n') 
      *command++ = '\0';

    *arguments++ = command;

    while (*command != '\0' && *command != ' ' && *command != '\t' && *command != '\n') command++;
  }
  *arguments = '\0'; 
}

/*
 Função para executar os comandos.
*/
void execute(char **arguments, char **argumentsArray){
  pid_t  pid;
  int    status;
  int	 fd[2];

  if ((pid = fork()) < 0) {
    printf("Runtime Error : Couldn't fork a child process.\n");
    exit(1);
  } else if (pid == 0) {          
    /*if (execvp(arguments[0], arguments) < 0) {
      perror("Error");
      exit(1);
    }*/
      pipe(fd);

	  if(!fork()){
		dup2(fd[1],1);
		if (execlp(argumentsArray[0][0], argumentsArray[0],NULL) < 0) {
	      		perror("Error pipe 1");
	      		exit(1);
	    	}
	  }
	  
	  dup2(fd[0], 0);//1 é o identificador do fluxo STDOUT
	  close(fd[1]);
	  
	  if (execlp(argumentsArray[1][0], argumentsArray[1],NULL) < 0) {
	      perror("Error pipe 2");
	      exit(1);
	  }
  } else {
    while (wait(&status) != pid);
  }
}

/*
 Função para manipular CTRL+C e CTRL+Z
*/
void handle_signal(int signo){
 
}

/*
 Função para executar o cd
*/
int changeDirectory(char* directory){
  if (directory == NULL || directory[0] == '/' || directory[0] == '~') {
    char homePath[BUFFER_SIZE] = "/home/";
    char* user = getenv("USER");
    strcat(homePath, user);
    chdir(homePath);
  } else {
    if (chdir(directory) != 0) {
      perror("Error");
    }
  }
  return 1;  
}
