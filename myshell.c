#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//----------------------------------------------------------------
// Data structure for path list
//----------------------------------------------------------------
struct Path{
	char* name;
	struct Path *next;
};

struct PathList{
	struct Path *head;
};


static inline void initList(struct PathList *pathList)
{
    pathList->head = NULL;
}


//---------------------------------------------------------------



//--------------------------------------------------------------
// Functions to manipulate pathlist
//--------------------------------------------------------------

// Check if pathlist is empty
static inline int isEmptyList(struct PathList *pathList)
{
    return (pathList->head == 0);
}

// Pop front path
void *popFront(struct PathList *pathList)
{
    if (isEmptyList(pathList))
        return NULL;

    struct Path *oldHead = pathList->head;
    pathList->head = oldHead->next;
    char* name = oldHead->name;
    free(oldHead);
}



// Traverse pathList, print each path with a colon unless last
void displayPaths(struct PathList *pathList){

	// If pathList is empty
	if(isEmptyList(pathList)){
		printf("Path List is empty\n");
		return;
	}

	struct Path *path = pathList->head;
	while (path->next) {
        	printf("%s", path->name);
		printf(":");
        	path = path->next;
    	}
	
	printf("%s\n", path->name);
	return;	
}


// Add a path to the pathList
void addPath(struct PathList *pathList, char *name){

	// make the new path that will go to the end of list
	struct Path *path = (struct Path *)malloc(sizeof(struct Path));
    	path->name = name;
    	path->next = NULL;

    	// if the list is empty, this path is the head
    	if (isEmptyList(pathList)) {
        	pathList->head = path;
		return;
    	}

    	// find the last path
    	struct Path *end = pathList->head;
    	while (end->next != NULL){
        	end = end->next;
	}

	// 'end' is the last node at this point
    	end->next = path;
	return;

}

// Remove path from pathList
void removePath(struct PathList *pathList, char* pathname){

	
	// check if list is empty
	if (isEmptyList(pathList)){
		printf("Path List is empty\n");
        	return;
	}

	// if path to be deleted is the head
	if(strcmp(pathname, pathList->head->name) == 0){
		struct Path *oldHead = pathList->head;
    		pathList->head = oldHead->next;
    		char* name = oldHead->name;
    		free(oldHead);
		return;
	}

	// Traverse rest of list
	struct Path *pathnext = pathList->head->next;
	struct Path *path = pathList->head;
	int found = 0;
	while(path->next){
		if(strcmp(pathname, pathnext->name) == 0){
			path->next = pathnext->next;
			free(pathnext);
			found = 1;
		}
	
		path = path->next;
		pathnext = path->next;
	}
	
	if(found == 0)
		printf("File not found in path\n");

	return;
}


// Clear all paths from list
void removeAllPaths(struct PathList *pathList)
{
    while (!isEmptyList(pathList))
        popFront(pathList);
}

// Search pathlist for executable
void searchPath(struct PathList *pathList, char** argv){

	struct Path *path = pathList->head;
	
	while(path){
		char* pathname = path->name;
		char temp[strlen(pathname) + strlen(argv[0]) +1];
		strcpy(temp, pathname);
		strcat(temp, "/");
		strcat(temp, argv[0]);		
		if(execv(temp, argv) != -1){
			return;
		}
		path = path->next;
	}	

	printf("Command: %s not found in current directory or path list\n", argv[0]);

}

//------------------------------------------------------------------
//------------------------------------------------------------------


//------------------------------------------------------------------
// EXECUTION FUNTIONS
//------------------------------------------------------------------


// Execute command and arguments
void execute(char **argv, struct PathList *pathList){


	int pid = fork();
	
	if(pid == -1){
		printf("Error forking\n");
		exit(1);
	}else if(pid == 0){ // CHILD PROCESS
		if(execv(argv[0], argv) == -1){
			searchPath(pathList, argv);
		}
	}else{              // PARENT PROCESS
		wait(NULL);
	}
		

}


// I/O Redirection
void redirect(char** command, char* file, int flag, struct PathList* pathList){

	int pid = fork();

	if (pid  == -1){
		printf("\nError forking");
                exit(1);
	}else if(pid == 0){

		// redirect input
		if(flag == 0){
			int fd0 = open(file, O_RDONLY);
			dup2(fd0, 0);
			close(fd0);
		}

		// redirect output
		else if(flag == 1){
			int fd1 = open(file, O_RDWR | O_CREAT, 0644);
			dup2(fd1, 1);
			close(fd1);
		}

		// redirect error
		else if(flag == 2){
			int fd2 = open(file, O_RDWR | O_CREAT, 0644);
			dup2(fd2, 2);
			close(fd2);
		}

		if(execv(command[0], command) == -1){
			searchPath(pathList, command);
		}
	}else{
		wait(NULL);
	}

}


// Pipeline
void pipeline(char** command, char** command2, int flag, struct PathList* pathList){

	int pipefd[2];
	if (pipe(pipefd)) {
		printf("Error making pipe\n");
		return;
	}

	int pid1;
	int pid2;
	
	// CHILD 1
	if((pid1 = fork()) == 0){
		dup2(pipefd[0], 0);
		close(pipefd[0]);
		close(pipefd[1]);
		if(execv(command2[0], command2) == -1){
                        searchPath(pathList, command2);
                }
	}

	// CHILD 2
	else if((pid2 = fork()) == 0) {
		dup2(pipefd[1], 1);
		close(pipefd[0]);
		if(execv(command[0], command) == -1){
                        searchPath(pathList, command);
                }
	}
	else{
		close(pipefd[1]);
		close(pipefd[0]);
		waitpid(pid1, NULL, 0);
		waitpid(pid2, NULL, 0);
		}
	return;

}

//------------------------------------------------------------------
//------------------------------------------------------------------



void main(void){

	// Initialize Path List
	struct PathList pathList;
	initList(&pathList);

	// Buffer for Command Line
	char line[4096];
	
	while(1){
		
		int skipexe = 0;

		//printf(">> ");
		char currentdir[1024];
                getcwd(currentdir, 1024);
                printf("%s $ ", currentdir);

		fgets(line, 4096, stdin);
		if(line[0] != '\n'){
		

			// count number of arguments in line
			int argc = 1;
			int i = 0;
        		for(i = 0; line[i] != '\0'; i++){
               		 	if(line[i] == ' '){
                        		argc++;
                		}
        		}

			// allocate size of argv array
			char** argv = malloc((sizeof(char*)*(argc + 1)));

			// parse the arguments and put them into argv
			int j = 0;
			char* argument = strtok(line, " \n");
			int flag = 1;
			while(argument != NULL){
				argv[j] = (char*)malloc(sizeof(char)*(strlen(argument)));
				strncpy(argv[j], argument, strlen(argument));
				argument = strtok(NULL, " \n");
				j++;
			}
		
			// check for redirection or pipeline
			int k;
			for(k = 0; k < argc; k++){
				
				// Found input redirect
				if(strcmp(argv[k], "<") == 0){
					int t;
					char* temp[k];
				
					// make an array for left side command and args
					for(t = 0; t < k; t++){
						temp[t] = argv[t];
					}
					skipexe = 1;
					redirect(temp, argv[k+1], 0, &pathList);
				}
				
				// Found output redirect
				else if(strcmp(argv[k], ">") == 0){
					int t;
                                        char* temp[k];
					// make an array for left side command and args

                                        for(t = 0; t < k; t++){
                                                temp[t] = argv[t];
                                       	}
					skipexe = 1;
                                        redirect(temp, argv[k+1], 1, &pathList);
				}
				
				// Found error redirect
				else if(strcmp(argv[k], "2>") == 0){
                                        int t;
                                        char* temp[k];
				
                                        // make an array for left side command and args
                                        for(t = 0; t < k; t++){
                                                temp[t] = argv[t];
                                        }
					skipexe = 1;
					redirect(temp, argv[k+1], 2, &pathList);
				}

				// Found pipe
				else if(strcmp(argv[k], "|") == 0){
                                        int t, s;
					int h = 0;
                                        char* temp[k];
					char* temp2[argc-k];                                        

					// make an array for left side command and args
					for(t = 0; t < k; t++){
                                                temp[t] = argv[t];
                                        }
					// make array for right side command and args
					for(s = k+1; (s < argc) && ((strcmp(argv[s], ">") != 0) && (strcmp(argv[s], "<") != 0) && (strcmp(argv[s], "2>") != 0) && (strcmp(argv[s], "|") != 0)) ; s++){
						temp2[h] = argv[s];
						h++;
                                        }
                                        skipexe = 1;
                                        pipeline(temp, temp2, 0, &pathList);
                                }

			}
		
			// if user enters exit, the shell exits
               		 if((strcmp(argv[0], "exit") == 0) && argc == 1){
                        	break;
                	}

               		 // if user enter cd, use chdir system call
               		 else if(strcmp(argv[0], "cd") == 0){
                        	if(chdir(argv[1]) == -1){
                                	printf("Directory does not exist\n");
                        	}
                	}
		
			// if user enters path with no arguments
			else if((strcmp(argv[0], "path") == 0) && argc == 1){
				displayPaths(&pathList);
			}

			// if user enters path +/- "path"
			else if((strcmp(argv[0], "path") == 0) && argc == 3){
				if((strcmp(argv[1], "+") == 0)){
					addPath(&pathList, argv[2]);
				}else if(strcmp(argv[1], "-") == 0){
					removePath(&pathList, argv[2]);	
				}
                	}

			// user enters invalid path format
			else if((strcmp(argv[0], "path") == 0) && argc == 2){ 
                        	printf("Invalid use of path\n");
                	}

			// execute program
			else{
				if(skipexe == 0)
					execute(argv, &pathList);
			}
	
			
		}	

		
	}

	removeAllPaths(&pathList);		
	exit(0);	

}		
