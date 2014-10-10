#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

char * HOME_VAR;
char * PATH_VAR;
char * CWD;
char * DIRS[256];
char * HOME_DIR;

int MAX_INPUT_SIZE = 1024;

/* Show a fancy ASCII art thing on startup just because*/
void showSplashScreen(){
	printf(" _____                        _____  _            _  _\n|  _  |                      /  ___|| |          | || |\n| | | | ___   ___  __ _  _ __\\ `--. | |__    ___ | || |\n| | | |/ __| / __|/ _` || '__|`--. \\| '_ \\  / _ \\| || |\n\\ \\_/ /\\__ \\| (__| (_| || |  /\\__/ /| | | ||  __/| || |\n \\___/ |___/ \\___|\\__,_||_|  \\____/ |_| |_| \\___||_||_|\n");
}

/* Loads the profile file and parses the HOME and PATH variables into HOME_VAR and PATH_VAR*/
void loadProfile(){
	FILE *profile;
	profile = fopen("profile", "r");
	if(profile == NULL){
		fprintf(stderr, "Couldn't open profile, Exiting\n");
		exit(1);
	}
	PATH_VAR = malloc(512*sizeof(char));
	HOME_VAR = malloc(512*sizeof(char));
	char line1[512];
	char line2[512];
	fgets(line1,512,profile);
	fgets(line2,512,profile);
	fclose(profile);
	if(line1[0]=='P'){
		strcpy(PATH_VAR, line1);
		strcpy(HOME_VAR, line2);
	}
	else if(line1[0]=='H'){
		strcpy(HOME_VAR, line1);
		strcpy(PATH_VAR, line2);
	}
	else{
		fprintf(stderr, "Error in profile file, No PATH_VAR or HOME_VAR variables could be found\nEnsure there is no whitespace before first variable declaration\n");
		exit(1);
	}
}

/* helper function to return current working directory as string */
void updateCwd(){
	char cwdBuff[1024];
	CWD = getcwd(cwdBuff, sizeof(cwdBuff));
}

/* Gets the current working directory and displays it as the prompt*/
void showPrompt(){
	updateCwd();
	printf("%s > ", CWD);
}

/* Parse the directories in PATH_VAR into the DIRS array*/
void parsePath(){
	int i = 0;
	for(i=0; i<256; i++){
		DIRS[i] = NULL;
	}
	char * tok;
	tok = strtok(PATH_VAR, "PATH_VAR=");
	tok = strtok(tok, ":");
	int count = 0;
	while(tok!=NULL){
		DIRS[count] = malloc(strlen(tok)*sizeof(char));
		strcpy(DIRS[count], tok);
		count++;
		tok = strtok(NULL, ":");
	}

}

/* Put the home directory in HOME_VAR into the HOME_DIR variable*/
void parseHome(){
	if(HOME_DIR!=NULL){
		free(HOME_DIR);
	}
	HOME_DIR = malloc((strlen(HOME_VAR)-5)*sizeof(char));
	int c = 5;
	for(c=5; c<strlen(HOME_VAR); c++){
		HOME_DIR[c-5]=HOME_VAR[c];
	}
	
}

/* run the correct program for the tokens given */
void runProgram(char * programName , char **  args){
	char * directoryName = DIRS[0];
	int counter = 0;
	while(directoryName!=NULL){
		DIR * directory;
		directory = opendir(directoryName);
		if(directory!=NULL){
			struct dirent * dirEntry; 
			dirEntry = readdir(directory);
			//iterate over programs in each directory until find one that matches programName
			while(dirEntry!=NULL){
				char * filename = dirEntry->d_name;
				if(strcmp(programName, filename) == 0){
					char * fileToExec = malloc(1024*sizeof(char));
					strcpy(fileToExec, directoryName);
					strcat(fileToExec, "/");
					strcat(fileToExec, filename);
					int i;
					pid_t forkedPid = fork();
					if(forkedPid == 0) {
						i = execv(fileToExec, args);
						if(i < 0) {
							printf("%s: program failed\n", args[0]);
							exit(1);		
						}
					} else {
						int status;
    					waitpid(forkedPid, &status, 0);
    					return;
					}
					
					//free(fileToExec);
				}
				dirEntry = readdir(directory); 
			}
		}
		else{
			printf("The directory %s does not exist, check your PATH variable\n", directoryName);
		}
		counter++;
		directoryName = DIRS[counter];
	}
	//if we get here no program was executed, so tell the user
	printf("No such command: %s\n", programName);
}

/* parse a line of user input into tokens */
void parseInput(char * input){
	char * tokens[256];
	char * tok;
	tok = strtok(input, " ");
	if (tok==NULL){
		//just empty input so no point continuing
		return;
	}
	int counter = 0;
	while(tok!=NULL){
		tokens[counter] = malloc(strlen(tok)*sizeof(char));
		strcpy(tokens[counter], tok);
		tok = strtok(NULL, " ");
		counter++;
	}
	char * args[counter+1];
	int i = 0;
	for(i=0; i<counter; i++){
		args[i] = malloc(strlen(tokens[i])*sizeof(char));
		strcpy(args[i], tokens[i]);
		free(tokens[i]);
	}
	args[counter] = malloc(sizeof(char)); //hacky fix
	args[counter] = NULL;
	runProgram(args[0], args);
}


/* Read a line of user input */
void readLine(){
	char charIn = '\0';
	showPrompt();
	int counter = 0;
	char * input = malloc(MAX_INPUT_SIZE*sizeof(char));
	while(charIn != '\n'){
		input[counter] = charIn;
		if(charIn!='\0'){
			counter++;
		}
		charIn = getchar();
	}
	input[counter] = '\0';
	//printf("%s\n", input);
	parseInput(input);
	free(input);
	readLine();
}


/* Fire up the shell :) */
void init(){
	showSplashScreen();
	CWD = malloc(1024*sizeof(char));
	updateCwd();
	loadProfile();
	parsePath();
	parseHome();
	readLine();
}



int main(int argc, char *argv[], char *envp[]){
	init();
}