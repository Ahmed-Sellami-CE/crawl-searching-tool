/* © Ahmed Sellami */

#ifndef ARGUMENTPARSER_H
#define ARGUMENTPARSER_H

int initArgumentParser(int argc, char* argv[]);
char* getCommand(void);
char* getValueForOption(char* keyName);
int getNumberOfArguments(void);
char* getArgument(int index);
#endif // ARGUMENTPARSER_H
