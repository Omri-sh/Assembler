#include "project.h"
#include "global.h"

/*
This program made by Omri Shkedi, ID: 308523620
This program implements interpreter for ASM and translate the code to 32Base - Muzar.
This file contain the global declaration on variable and reference for parsing the files and creating the output.
The program receive the input from the command line arguments which the suffix of the file - just the name.

Note: the program convert the output to 2 digit's 32 Muzar base. if you would like to insert input that converts to bigger digit - edit the MUZAR_ARRAY_SIZE define in project.h as you would like.
*/

unsigned int g_dataArr[DATA_ARR_SIZE]; /*store the data*/
unsigned int g_codeArr[CODE_ARR_SIZE]; /*store the code*/
int g_IC; /*Instruction counter*/
int g_DC; /*Data counter*/
symbolTable g_labelTable[LABEL_TABLE_SIZE]; /*store the labels in a table*/
int g_labelIndex; /*label table index*/
exTable g_exTable[LABEL_TABLE_SIZE]; /*store the extern label in a table*/
int g_exTableIndex; /*extern table index*/
bool g_error = FALSE; /*indicator for errors*/
bool hasEntry; /*indicator for entry labels*/
char muzarBase[] = {'!', '@', '#', '$', '%', '^', '&', '*', '<', '>', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'}; /*32 Muzar bse which defined in the guide*/

cmdStruct cmds[] = {
	/* table of all commands which holds: string, code, number of arguments, permittable params in source destination.
	the 4 permittable params are: immediate, direct, struct access, register. 1 means permitted 0 means not permitted. */
	{"mov", 0, 2, {{1,1,1,1}, {0,1,1,1}}},
	{"cmp", 1, 2, {{1,1,1,1}, {1,1,1,1}}}, 
	{"add", 2, 2, {{1,1,1,1}, {0,1,1,1}}}, 
	{"sub", 3, 2, {{1,1,1,1}, {0,1,1,1}}}, 
	{"not", 4, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"clr", 5, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"lea", 6, 2, {{0,1,1,0}, {0,1,1,1}}}, 
	{"inc", 7, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"dec", 8, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"jmp", 9, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"bne", 10, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"red", 11, 1, {{0,0,0,0}, {0,1,1,1}}}, 
	{"prn", 12, 1, {{0,0,0,0}, {1,1,1,1}}}, 
	{"jsr", 13, 1, {{0,0,0,0}, {0,1,1,0}}}, 
	{"rts", 14, 0, {{0,0,0,0}, {0,0,0,0}}}, 
	{"stop", 15, 0, {{0,0,0,0}, {0,0,0,0}}}
};


int main(int argc, char *argv[])
{
	int argcount = 1;
	printf("---Assembler---\n"); 
	if(argc == 1) /*check if there is not args*/
		printf("No file has been entered.\n");
	else /*if there is args*/
	{
		while(--argc > 0) /*running on all the args*/
		{
			FILE *fp ,*objfp, *entfp, *extfp; /*FILE pointers to the input and output files*/
			char filename[4][50]; /*store files name*/
			hasEntry = FALSE;
			if(strchr(argv[argcount], '.') != NULL) /*if the file name contains a dot*/
			{
				fprintf(stderr,"Error in file name '%s' which contains a dot.\n", argv[argcount]);
				++argcount;
				continue;
			}
			strcpy(filename[0],argv[argcount]); /*copy a file name from args*/
			strcpy(filename[1],argv[argcount]); /*copy a file name from args*/
			strcpy(filename[2],argv[argcount]); /*copy a file name from args*/
			strcpy(filename[3],argv[argcount]); /*copy a file name from args*/
			strcat(filename[0], READ_FILE); /*add read file suffix*/
			strcat(filename[1], OBJ_FILE); /*add ob file suffix*/
			strcat(filename[2], EXTERN_FILE); /*add ext file suffix*/
			strcat(filename[3], ENTRY_FILE); /*add ent file suffix*/
			fp = fopen(filename[0], "r"); /*open input file*/
			if(fp == NULL) /*if the file doesn't exist*/
			{
				fprintf(stderr,"Can't open the file: %s.\n",filename[0]);
				++argcount;
				continue;
			}
			printf("Compiling %s file.\n", filename[0]);
			parsing(fp); /*parsing the file*/
			if(g_error == FALSE) /*if there is not error after parsing*/
			{
				objfp = fopen(filename[1], "wb"); /*create and open ob file*/
				if(objfp == NULL) /*if there is an error in creating the file*/
				{
					fprintf(stderr,"Can't open the file: %s.\n",filename[1]);
					++argcount;
					continue;
				}
				printf("Creating a %s file.\n", filename[1]);
				writeObject(objfp); /*write the ob file*/
				fclose(objfp); /*close the ob file*/
				if(g_exTableIndex > 0) /*if there is EXTERN labels*/
				{
					extfp = fopen(filename[2], "wb"); /*create and open ext file*/
					if(extfp == NULL) /*if there is an error in creating the file*/
					{
						fprintf(stderr,"Can't open the file: %s.\n",filename[2]);
						++argcount;
						continue;
					}
					printf("Creating a %s file.\n", filename[2]);
					writeExtern(extfp); /*write the ext file*/
					fclose(extfp); /*close the ext file*/
				}
				if(hasEntry == TRUE) /*if there is ENTRY labels*/
				{
					entfp = fopen(filename[3], "wb"); /*create and open ent file*/
					if(objfp == NULL) /*if there is an error in creating the file*/
					{
						fprintf(stderr,"Can't open the file: %s.\n",filename[3]);
						++argcount;
						continue;
					}
					printf("Creating a %s file.\n", filename[3]);
					writeEntry(entfp); /*write the ent file*/
					fclose(entfp); /*close the ent file*/
				}
				printf("Compilation is successfully done.\n");
			}
			fclose(fp); /*close the input file*/
			++argcount; /*next arg*/
		}
	}
	printf("---Exiting Assembler---\n"); 
	return 1;
}
