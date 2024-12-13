/* © Ahmed Sellami */

#include <stdio.h>
#include <stdlib.h>
#include "argumentParser.h"
#include <sys/stat.h>
#include <dirent.h>
#include <libgen.h>
#include <fnmatch.h>
#include <regex.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

void show_line_context(char* path,int index,int context){
	FILE *fptr;
	fptr = fopen(path,"r");
	if(fptr!=NULL){
		char* line = NULL;
		size_t len=0;
		int i=0;
		while(getline(&line,&len,fptr)!=-1){
			i++;
			if((i >= index-context) && (i <= index+context))
				printf("%s:%d:%s",path,i,line);
		}
	}
	fclose(fptr);
}

bool show_based_on_size(off_t fileSize,off_t size,int sizeMode){
	return (sizeMode == -1)
	|| (sizeMode == 2 && size == fileSize)
	|| (sizeMode == 1 && fileSize < size)
	|| (sizeMode == 0 && fileSize > size);
}

bool typeValueTest(char* type_value){
	if(type_value != NULL && (strlen(type_value) != 1 || (*type_value != 'f' && *type_value != 'd') )){
		errno = EINVAL;
		fprintf(stderr, "-type argument must be d or f\n");
		return false;
	}
	return true;
}

bool regexValueTest(char* regex_value, regex_t* regp){
	if(regex_value != NULL && regcomp(regp,regex_value,REG_EXTENDED) != 0){
		regfree(regp);
		errno = EINVAL;
		fprintf(stderr,"ERROR: invalid file regex\n");
		return false;
	}
	return true;
}

static void crawl(char path[], int maxDepth, const char pattern[], char type,
                  int sizeMode, off_t size, regex_t * line_regex,int context) {
	struct stat buffer;
	int status;

	status = lstat(path,&buffer);
	if(status == 0){
		if(S_ISREG(buffer.st_mode) !=0 && (type == 'f' || type == 'a')) {
			if((pattern != NULL && fnmatch(pattern,basename(path),0) == 0) || pattern == NULL){
				if(show_based_on_size(buffer.st_size,size,sizeMode)){
					if(line_regex == NULL ){
						printf("%s\n",path);
					}else{
						FILE *fptr;
						fptr = fopen(path,"r");
						if(fptr != NULL){
							char* line = NULL;
							size_t len = 0;
							int i = 0;
							while(getline(&line,&len,fptr) != -1){
								i++;
								if(regexec(line_regex, line, 0, NULL, 0) == 0){
									if(context==0)
										printf("%s:%d:%s",path,i,line);
									else
										show_line_context(path,i,context);
								}
							}
						}
						fclose(fptr);
					}
				}
			}
		}else if(S_ISDIR(buffer.st_mode) != 0){
			struct dirent *entry;
			DIR *dp = opendir(path);
			if(dp != NULL){
				if((pattern == NULL && line_regex == NULL) && (type == 'd' || type == 'a') 
					&& show_based_on_size(buffer.st_size,size,sizeMode)){
					printf("%s\n",path);
				}
				if(maxDepth!=0){
			    	while ((entry = readdir(dp))) {
			    		if(strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0){
			    			if(path[strlen(path)-1] == '/')
			    				path[strlen(path)-1] = '\0';
			        		char new_path[strlen(path)+strlen(entry->d_name)+2];
			        		strcpy(new_path,path);
			        		strcat(new_path,"/");
			        		strcat(new_path,entry->d_name);
			        		new_path[strlen(path)+strlen(entry->d_name)+2] = '\0';
			        		crawl(new_path,maxDepth-1,pattern,type,sizeMode,size,line_regex,context);
			    		}
			    	}
		    	}
		    	closedir(dp);
			}
		}
	}else{
		if(path[strlen(path)-1] == '/'){
			path[strlen(path)-1] = '\0';
			status = lstat(path,&buffer);
			if(status == 0){
				errno = 20;
				path[strlen(path)] = '/';
				perror(path);
			}else{
				errno = 2;

			}
			path[strlen(path)] = '/';
			perror(path);
			return;
		}
		errno = 2;
		perror(path);
	}

}




int main(int argc, char *argv[]) {
	if ( initArgumentParser(argc, argv) ) {
		perror("Arguments not accepted by argsInit");
		exit(EXIT_FAILURE);
	}

	// number of arguments == number of files/directories to find
	int number_of_files = getNumberOfArguments();
	if(number_of_files==0){ // if no arguments are given: it should give an error to inform the user how to use our command.
		errno = EINVAL;
		fprintf(stderr, "Usage: ./crawl path... [-maxdepth=n] [-name=pattern] [-type={d,f}] [-size=[+-]n] [-line=regex]\n"); // ANMERKUNG: Hier habt ihr `-context` vergessen.
    	exit(EXIT_FAILURE);
	}

	// get the type of file to search for. 
	char* type_value = getValueForOption("type");
	char type;
	if(!typeValueTest(type_value)){ // this will insure that only files and directories are acceptable
		exit(EXIT_FAILURE);
	}

	char* regex_value = getValueForOption("line");
	regex_t regp;
	
	if(!regexValueTest(regex_value,&regp)) // this will insure that the regular expression is valid.
		exit(EXIT_FAILURE);

	char* size_value = getValueForOption("size");
	int size_mode;
	off_t size;
	if(size_value == NULL){
		size_mode = -1; // if no size is set
	}else{
		if(*size_value == '+' || *size_value == '-'){
			size_mode = *size_value == '+' ? 0  // to check if the searched value is bigger
			: 1;  // to check if the searched value is smaller
			size = (off_t) strtol(size_value+1,NULL,10);
		}else{
			size_mode = 2; // to look for the searched value
			size = (off_t) strtol(size_value,NULL,10);
		}
	}

	char* context_value = getValueForOption("context");
	int context=0;
	if(regex_value != NULL && context_value!=NULL){
		context=strtol(context_value,NULL,10);
	}

	type=type_value==NULL?'a':*type_value;
	char* max_depth_value = getValueForOption("maxdepth"); 
	int max_depth = max_depth_value != NULL ? (int) strtol(max_depth_value,NULL,10) : -1;
	char* name_pattern = getValueForOption("name");
	
	for(int i=0;i<number_of_files;++i){
		crawl(getArgument(i),max_depth,name_pattern,type,size_mode,size,regex_value == NULL ? NULL : &regp,context);
	}

	if(regex_value!=NULL)
		regfree(&regp);
}