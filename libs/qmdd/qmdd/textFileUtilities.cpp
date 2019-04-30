// text file input output routines 
#include "textFileUtilities.h"

FILE *openTextFile(char *fname,char mode)
{
  FILE *file;
  
    if(mode=='r') file = fopen(fname, "rt"); // open the file for reading
    else file = fopen(fname, "w"); // open the file for writing
    return(file);
}

char getch(FILE *infile)
  {
// fetch one character / convert lowercase to uppercase
  char ch;
  int retval = fscanf(infile,"%c",&ch);
  if(retval != 1) {
	  return -1;
  }
  if(ch==13){
    fscanf(infile,"%c",&ch);
  }
  return processChar(ch);
}

char getline(FILE *infile, char x[]){

	char ch;
	int i = 0;

	do
	{
		ch=getch(infile);
		x[i]=ch;
		i++;
	} while(ch!='\n' && i < 511);

	x[i]=0;
	return(ch);
}

char processChar(char ch){

	if(ch==11) ch='\n';
	  if(ch>='a'&&ch<='z') ch=ch-'a'+'A'; // convert lowercase letters to uppercase
	  return(ch);

}

char getnbch(FILE *infile)
{
// get next blank character  
  char ch;
  
  while(' '==(ch=getch(infile)));
  return(ch);
}

char getstr(FILE *infile,char x[])
{
// store next token in x (string encapsulated by {","," ","\n"}), return last character of token  
  char ch;
  int i;
  do
  {
    ch=getch(infile);
  } while(ch==','||ch==' '||ch=='\n');
  i=0;
  while(ch!=','&&ch!=' '&&ch!='\n')
  {
    x[i]=ch;
    i++;
    ch=getch(infile);
  }
  x[i]=0;
  return(ch);
}

int getstr(const char line[],char x[])
{
// store next token in x (string encapsulated by {","," ","\n"}), return last character of token
  char ch;
  int i,j = 0;
  do
  {
    ch=processChar(line[j++]);
  } while(ch==','||ch==' '||ch=='\n');
  i=0;
  while(ch!=','&&ch!=' '&&ch!='\n')
  {
    x[i]=ch;
    i++;
    ch=processChar(line[j++]);
  }
  x[i]=0;
  return(j);
}

int getint(FILE *infile)
{
 // read integer from stream 
  char ch;
  int i;
  while(' '==(ch=getch(infile)));
  i=0;
  while(ch!=','&&ch!=' '&&ch!='\n')
  {
    i=i*10+ch-'0';
    ch=getch(infile);
  }
  return(i);
}

void skip2eof(FILE *infile)
{
// skips until it reaches end of file
  char ch;
  while(EOF!=fscanf(infile,"%c",&ch));
}

void skip2eol(FILE *infile)
{
// skips until it reads a '\n'
  char ch;
  do{
    ch=getch(infile);
    if(feof(infile)) return;
  } while(ch!='\n');
}


