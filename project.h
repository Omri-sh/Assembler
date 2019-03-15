#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
This file contains all the defined which used in the program, the data structures and prototype of functions.
*/

#define CMD_NAME_SIZE 5 /*size of commands' name*/
#define LABEL_NAME_SIZE 31 /*size of label's name allowed*/
#define LABEL_TABLE_SIZE 2000 /*size of label table - how much labels allowed*/
#define DATA_ARR_SIZE 2000 /*size of DATA table - how much data allowed*/
#define CODE_ARR_SIZE 2000 /*size of code table - how much code allowed*/
#define START_ADD 100 /*the starting address which defined for entering datas and codes*/
#define LINE_SIZE 80 /*size of line allowed*/
#define READ_FILE ".as" /* extention for read assembly file */
#define OBJ_FILE ".ob" /* extention of write object file */
#define EXTERN_FILE ".ext" /* extention of write external file */
#define ENTRY_FILE ".ent" /* extention of write entry file */
#define MUZAR_BASE_SIZE 32 /*the base of muzar is 32*/
#define MAX_CMD_CODES 15 /*how many commands has been defined*/
#define OPC_SHIFT 6 /*place of OPC bits in the address*/
#define SOURCE_SHIFT 4 /*place of source bits in the address*/
#define DESTINATION_SHIFT 2 /*place of destination bits in the address*/
#define REG_SOURCE_SHIFT 6 /*place of register source bits in the address*/
#define REG_DESTINATION_SHIFT 2 /*place of register destination bits in the address*/
#define CODE_WITHOUT_AER_SIZE 8 /*size CODE address which A.E.R bits at the end for data or labels*/
#define DATA_ADD_SIZE 10 /*size of DATA address for data types*/
#define MUZAR_ARRAY_SIZE 2 /*the size of the array which store the converation to 32base*/
#define MAX_VALUE_ALLOWED_DATA 511 /*max values allowed in 10 bits address*/
#define MIN_VALUE_ALLOWED_DATA -512 /*min values allowed in 10 bits address*/
#define MAX_VALUE_ALLOWED_IMMIDIATE 127 /*max values allowed in 10 bits address*/
#define MIN_VALUE_ALLOWED_IMMIDIATE -128 /*min values allowed in 10 bits address*/

typedef enum {ABSOLUTE = 0, EXTERNAL, RELOCATABLE} encode; /*AER bits*/
typedef enum {SPACE = 0, COMMA, DATA_SNT, STRING_SNT, STRUCT_SNT, ENTRY, EXTERN, LABEL, INVALID_LABEL, NUMBER, STRING, STRUCT_FIRST, STRUCT_SECOND, INVALID_STRUCT, IMMIDIATE, WRONG_IMMIDIATE,RELATIVE, CMD, REGISTER, OTHER} wordType; /*types of words*/
typedef enum {CODE = 0, DATA} sntType; /*is the sentence is CODE or DATA*/
typedef enum {FALSE = 0, TRUE} bool; /*boolean for TRUE and FALSE*/
typedef enum {IMMIDIATE_ADD = 0, DIRECT_ADD, STRUCT_ADD, REG_ADD} addType; /*addressing types*/

typedef struct wordStruct /*struct to store each word which read from the line*/
{
	char str[LINE_SIZE+1]; /*store the string of the word*/
	wordType type; /*which type of word is it*/
	int num; /*if it's a number stores the number as integer*/
	int reg; /*if it's a register stores it's number*/
}wordStruct;

typedef struct symbolTable /*struct to store all the labels*/
{
	wordStruct label; /*store the label as a word struct*/
	int labelAddress; /*store the address of the label*/
	sntType type; /*is the label DATA or CODE*/
	bool isEntry; /*is the label defined as entry*/
}symbolTable;

typedef struct exTable /*struct to store external labels*/
{
	wordStruct label;/*store the label as a word struct*/
	sntType type; /*is the label DATA or CODE*/
	int exAddress[LABEL_TABLE_SIZE]; /*store all the address which shows in the input*/
	int exAddNum; /*index for the array which stores the address*/
}exTable;

typedef struct cmdStruct /* struct for the command in ASM */
{
	char name[CMD_NAME_SIZE]; /* command name */
	int code; /* the code of the command (0-15) */
	int oper; /* how many operands for this command (0-2) */
	int permit[2][4]; /* table of 2*4 permittable source/destinations operands: immediate, direct, struct Access, register */
}cmdStruct;

/*all explanations written before each function*/

void parsing(FILE*);
int labelSearch(char[]);
void printError(int, char*);
int cmdSearch(char[]);
void wordParsing(wordStruct*,char[], int,int*);
int isLabel(wordStruct*);
int isNum(wordStruct*);
void cmdProcess(char[],int,int, int, int, int);
int cmdProcess2(wordStruct*,int, int, int, int, int);
void writeObject(FILE*);
void writeExtern(FILE*);
void writeEntry(FILE*);
void dtom(unsigned int, char[]);
unsigned int twosComplement(int, int);
int extSearch(char str[]);




