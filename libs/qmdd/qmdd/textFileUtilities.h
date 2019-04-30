#ifndef textFileUtilities_H
#define textFileUtilities_H

#include <stdio.h>

FILE *openTextFile(char*,char);
char getch(FILE*);
char getnbch(FILE*);
char getstr(FILE*,char*);
int getstr(const char*,char*);
char getline(FILE*,char*);
char processChar(char);
int getint(FILE*);
void skip2eof(FILE*);
void skip2eol(FILE*);
#endif
