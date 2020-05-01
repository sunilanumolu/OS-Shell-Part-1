// C Program to design a shell in Linux
#include<sys/utsname.h>
#include<dirent.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<sys/stat.h>
#include<pwd.h>
#include<grp.h>
#include<readline/readline.h>
#include<readline/history.h>


#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
 
// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")
 
// Greeting shell during startup
void init_shell()
{
    clear();
    printf("\n\n\n\n******************"
        "************************");
    printf("\n\n\n\t****MY SHELL****");
    printf("\n\n\t-USE AT YOUR OWN RISK-");
    printf("\n\n\n\n*******************"
        "***********************");
    char* username = getenv("USER");
    printf("\n\n\nUSER is: %s", username);
    printf("\n");
    sleep(1);
    clear();
}
 
// Function to take input
int takeInput(char* str)
{
    char* buf;
 
    buf = readline("$ ");
    if (strlen(buf) != 0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}
 
// Function to print Current Directory.
void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    char* username = getenv("USER");
    printf("\n%s@Ubuntu:%s", username, cwd);
}
 
void handler(int sig)
{
    int status;
    pid_t wpid = waitpid(-1, &status, WNOHANG);
    if (wpid > 0 && WIFEXITED(status) == 0)
    {
        fprintf(stderr, "\nProcess with pid: %d exited normally\n", wpid);
    }
    if (wpid > 0 && WIFSIGNALED(status) == 0) {
        fprintf(stderr, "\nProcess with pid: %d interrupted by signal %d\n", wpid, sig);
    }
}

// Function where the system command is executed
void execArgs(char** parsed)
{
    int bg = 0;
    if(parsed[0][strlen(parsed[0])-1] == *"&"){
        bg = 1;
        signal(SIGCHLD,handler);
        parsed[0][strlen(parsed[0])-1] = *"\0";
    }
    // Forking a child
    for(int i=0;i<MAXLIST && parsed[i]!=NULL;i++){
        if(!strcmp(parsed[i],"&")){
            signal(SIGCHLD,handler);
            bg = 1;
        }
    }
    pid_t pid = fork(); 
 
    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        // waiting for child to terminate
        if(bg == 0){
           wait(NULL); 
    }
        return;
    }
}
 

// Help command builtin
void openHelp()
{
    puts("\n***WELCOME TO MY SHELL HELP***"
        "\nCopyright @ Sunil"
        "\n-Use the shell at your own risk..."
        "\nList of Commands supported:"
        "\n>cd"
        "\n>ls"
        "\n>exit"
        "\n>all other general commands available in UNIX shell"
        "\n>pipe handling"
        "\n>improper space handling");
 
    return;
}

void printlist(char tmp[],char name[],int l,int file){
  struct stat sfile;
  struct passwd *pw;
  struct group *gr;
  char path[1000];
  char Time[1000];
  strcpy(path,tmp);
  if(!file&&path[l-1]!='/'){
    path[l]='/';
    path[l+1]=0;
    strcat(path,name);
    // printf("path is:%s\n",path);
    // printf("hello in printlist\n");
  }
  if(stat(path,&sfile)==0){
    char t[100];
    char perms[11];
    if(S_ISREG(sfile.st_mode)) perms[0]='-';
        else if(S_ISDIR(sfile.st_mode)) perms[0]='d';
        else if(S_ISFIFO(sfile.st_mode)) perms[0]='|';
        else if(S_ISSOCK(sfile.st_mode)) perms[0]='s';
        else if(S_ISCHR(sfile.st_mode)) perms[0]='c';
        else if(S_ISBLK(sfile.st_mode)) perms[0]='b';
        else perms[0]='l';  // S_ISLNK
    perms[1] = (sfile.st_mode & S_IRUSR) ? 'r':'-';
        perms[2] = (sfile.st_mode & S_IWUSR) ? 'w':'-';
        perms[3] = (sfile.st_mode & S_IXUSR) ? 'x':'-';
        perms[4] = (sfile.st_mode & S_IRGRP) ? 'r':'-';
        perms[5] = (sfile.st_mode & S_IWGRP) ? 'w':'-';
        perms[6] = (sfile.st_mode & S_IXGRP) ? 'x':'-';
        perms[7] = (sfile.st_mode & S_IROTH) ? 'r':'-';
        perms[8] = (sfile.st_mode & S_IWOTH) ? 'w':'-';
        perms[9] = (sfile.st_mode & S_IXOTH) ? 'x':'-';
        perms[10] = '\0';
    printf("%s",perms);
    printf(" ");
    printf("%lu ",sfile.st_nlink);
    pw = getpwuid(sfile.st_uid);
    gr = getgrgid(sfile.st_gid);
    printf("%s %s ",pw->pw_name,gr->gr_name);
    printf("%-5ld ",sfile.st_size);
    strcpy(t,ctime(&sfile.st_mtime));
    t[16]='\0';
    printf("%s ",t);
    printf("%s\n",name);
    // fprintf(stderr,"Failed to access the %s file\n",path);
  }
  else
  {
    fprintf(stderr,"Failed to access the %s file\n",path);
  }
  return;
}

void pinfo(char *temp){
    FILE *fd;
    char add[100] = "/proc/", text[10000],exe[100],state,size[10],extra_arr[100] = "a";
    printf("Pid -- %d\n",getpid());
    strcat(add,temp);
    strcpy(exe,add);
    strcat(exe,"/exe");
    strcat(add,"/status");
    fd = fopen(add, "r");
    while(fgets(text,100,fd)){
        if(text[0] == *"S" && text[1] == *"t" && text[2] == *"a" && text[3] == *"t" && text[4] == *"e")printf("Process Status -- %c\n",text[7]);
        if(text[0] == *"V" && text[1] == *"m" && text[2] == *"S" && text[3] == *"i" && text[4] == *"z" && text[5] == *"e")
        {
            for(int i=0;i<10;i++)size[i] = text[11+i];
            printf("Memory -- %s",size);
        }
    }
    printf("Executable -- ");
    int ret = readlink(exe,extra_arr,100);
    printf("%s\n",extra_arr);
}

// Function to execute builtin commands
int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 8, i, switchOwnArg = 0;
    char* ListOfOwnCmds[NoOfOwnCmds];
    char* ListOflsCmds[3];
    char* username;
    char cwd[1024];
    struct dirent *dir;
    DIR *d;
 
    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";
    ListOfOwnCmds[4] = "pwd";
    ListOfOwnCmds[5] = "echo";
    ListOfOwnCmds[6] = "ls";
    ListOfOwnCmds[7] = "pinfo";

    ListOflsCmds[0] = "-a";
    ListOflsCmds[1] = "-l";
    ListOflsCmds[2] = "-la";

    for (i = 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i + 1;
            break;
        }
    }
 
    switch (switchOwnArg) {
    case 1:
        printf("\nGoodbye\n");
        exit(0);
    case 2:
        chdir(parsed[1]);
        return 1;
    case 3:
        openHelp();
        return 1;
    case 4:
        username = getenv("USER");
        printf("\nHello %s.\nMind that this is "
            "not a place to play around."
            "\nUse help to know more..\n",
            username);
        return 1;
    case 5:
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        return 1;
    case 6:
        for(i=1; i < MAXLIST && parsed[i] != NULL; i++)
            printf("%s ", parsed[i]);
        printf("\n");
        return 1;
    case 7:
        if (parsed[1]==NULL){
        d=opendir(".");
        if(d!=NULL){
        while((dir=readdir(d))!=NULL){
          if(dir->d_name[0]!='.'){
            printf("%s\n",dir->d_name);
            }
        }
        }
        else{
            printf("cannot access the contents of the present directory\n");
         }
        }
        else if (strcmp(parsed[1], ListOflsCmds[0] ) == 0){
        d=opendir(".");
        if(d!=NULL){
        while((dir=readdir(d))!=NULL){
            printf("%s\n",dir->d_name);
        }
        }
        else{
            printf("cannot access the contents of the present directory\n");
         }
        }
        else if (strcmp(parsed[1], ListOflsCmds[1] ) == 0){
            d=opendir(".");
            if(d!=NULL){
            while((dir =readdir(d))!=NULL){
            if(dir->d_name[0]!='.'){
            printlist(".",dir->d_name,1,0);
            }
           }
          }
          else{
            printf("cannot access the contents of the present directory\n");
         }
        }
        else if (strcmp(parsed[1], ListOflsCmds[2] ) == 0){
            d=opendir(".");
            if(d!=NULL){
            while((dir =readdir(d))!=NULL){
            printlist(".",dir->d_name,1,0);
           }
          }
          else{
            printf("cannot access the contents of the present directory\n");
         }
        }
        return 1;
        case 8:
             if(parsed[1] == NULL){
            char *temp = (char *)malloc(100 * sizeof(char));
            sprintf(temp, "%d", getpid());
            pinfo(temp);
        }
        else {
            pinfo(parsed[1]);
        }
        return 1;
    default:
        break;
    }
 
    return 0;
}

// function for parsing command words
void parseSpace(char* str, char** parsed)
{
    int i;
 
    for (i = 0; i < MAXLIST; i++) {
        parsed[i] = strsep(&str, " ");
 
        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[i]) == 0)
            i--;
    }
}
 
int processString(char* str, char** parsed)
{
 
    parseSpace(str, parsed);
 
    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1;
}
 
int main()
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    int execFlag = 0;
    init_shell();
 
    while (1) {
        // print shell line
        printDir();
        // take input
        if (takeInput(inputString))
            continue;
        // process
        execFlag = processString(inputString,parsedArgs);
        // execflag returns zero if there is no command
        // or it is a builtin command,
        // 1 if it is a simple command
 
        // execute
        if (execFlag == 1)
            execArgs(parsedArgs);
 
    }
    return 0;
}
