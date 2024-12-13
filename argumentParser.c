/* © Ahmed Sellami */

#include "argumentParser.h"
#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#define MAX_ARGS 100

int parametersNumber;
char* parameters[MAX_ARGS];

bool isKeyValue(char* argument){
	if(*argument!='-')
		return false;
	int index=1;

	while(*(argument+index)!='\0'){
		if(*(argument+index)=='=')
			return true;
		index++;
	}
	return false;
}

int initArgumentParser(int argc, char* argv[]) {
	bool inKeyValues=false;
	for(int i=0;i<argc;++i){
		if(inKeyValues && !isKeyValue(*(argv+i))){
			errno=22;
			return -1;
		}
		if(!inKeyValues && isKeyValue(*(argv+i))){
			inKeyValues=true;
		}
		*(parameters+i)=*(argv+i);
	}
	parametersNumber=argc;
	return 0;
	// errno: EINVAL = 22 
}

char* getCommand(void) {
	return *parameters;
}

int getNumberOfArguments(void) {
	int result=0;
	for(int i=1;i<parametersNumber;++i){
		if(isKeyValue(*(parameters+i)))
			break;
		result++;
	}
	return result;
}

char* getArgument(int index) {
	int n=getNumberOfArguments();
	if(index>=n || (index<0 && index*-1>n))
		return NULL;
	return index>=0?*(parameters+index+1):*(parameters+n+index+1);
}

char* getValueForOption(char* keyName) {
	int index=getNumberOfArguments()+1;
	if(index>=parametersNumber)
		return NULL;
	int i;
	bool match;
	char* parameter;
	for(;index<parametersNumber;++index){
		match=false;
		parameter=*(parameters+index);
		int length=strlen(parameter),keyIndex=0;
		i=0;
		for(;i<length;++i){
			if(*(parameter+i)=='=' && !match){
				match=true;
				continue;
			}
			if(*(parameter+i)=='-' && !match)
				continue;
			if(match){
				return parameter+i;
			}else{
				if(!match && *(parameter+i)!=*(keyName+keyIndex))
					break;
				keyIndex++;
			}
		}
		if(match)
			break;
	}
	if(match)
		return parameter+i;
	return NULL;
}
