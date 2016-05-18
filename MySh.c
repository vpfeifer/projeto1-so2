#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096
#define MAX_ARGS 32

struct command
{
  char **argv;
};

//Assinaturas das funções.
void prompt();
void prepareArguments(char *command, char ** args);
void getCommands(char *command);
void execute(struct command list[]);
void handle_signal(int signo);
int changeDirectory(char* directory);
void createProcess (int in, int out, struct command *list);
void executePipes (int n, struct command *list);

int countCommands = 0;
struct command list[MAX_ARGS];

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
	getCommands(command);

    	//se o comando for 'exit', sai do programa.
    	if (!strcmp(list[0].argv[0], "exit")) exit(0);
    
	//implementação do cd
	if (!strcmp(list[0].argv[0], "cd")){
           changeDirectory(list[0].argv[1]);
        }else{
    	//executa o comando
	execute(list);	
	}
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
void prepareArguments(char * command, char **args){
	char *arg;
	int k = 0;
	arg = strtok(command," ");
	while(arg != NULL){
		args[k] = arg;
		k++;
		arg = strtok(NULL," ");
	}
}
/*
Separa os comandos
*/
void getCommands(char *command){
	char *commands[MAX_ARGS];
	char **tokens;
	int i = 0;
  	char *cmd;
	cmd = strtok(command,"|");
	while(cmd != NULL){	
		commands[i] = cmd;
		i++;
		cmd = strtok(NULL,"|");
	}
	int j = 0;
	while(j < i)
	{
		tokens = malloc(sizeof(char *[MAX_ARGS]));
		prepareArguments(commands[j], tokens);
		list[j].argv = tokens;
		j++;
	}
	countCommands = i;
}

/*
 Função para executar os comandos.
*/
void execute(struct command list[]){
  pid_t  pid;
  int    status;
  int	 fd[2];

  if ((pid = fork()) < 0) {
    printf("Runtime Error : Couldn't fork a child process.\n");
    exit(1);
  } else if (pid == 0) {   
    executePipes(countCommands,list);
    countCommands=0;   
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

/*
Cria um processo e executa o comando
*/
void createProcess (int in, int out, struct command *list)
{
  pid_t pid;
  int status;
  if ((pid = fork ()) == 0)
    {
      if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

      if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }

      if(execvp(list->argv[0], (char * const *)list->argv) < 0){
	perror("Erro ao executar comando");
	exit(1);	
	}
    }
}

/*
Executa n pipes, onde n é a quantidade de comandos na lista de comandos list.
*/
void executePipes (int n, struct command *list)
{
  int i;
  pid_t pid;
  int in, fd [2];

  in = 0;

  for (i = 0; i < n - 1; ++i)
    {
      pipe (fd);
      createProcess(in, fd [1], list + i);
      close (fd [1]);
      in = fd [0];
    }

  if (in != 0)
    dup2 (in, 0);

  if(execvp(list[i].argv[0], (char * const *)list[i].argv) < 0){
	perror("Erro ao executar comando");
	exit(1);
  }
}
