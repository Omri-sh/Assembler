#include "project.h"
#include "global.h"

/*
This file contain the functions for parsing a word
*/

/*This function parsing a word in a line. the function receives a wordStruct to store the word at the end, the current line and its length and current position in the line*/
void wordParsing(wordStruct *word,char line[], int lineLength,int *current)
{
	int i = 0;
	wordType type = OTHER;
	bool labelPotential = FALSE; /*indicator for potential label*/
	while(line[*current] != '\0' && (line[*current] == ' ' || line[*current] == '\t')) /*skip whitespaces and tabs*/
		(*current)++;
	while(*current < lineLength && line[*current] != ' ' && line[*current] != '\t' && line[*current] != ',' && line[*current] != ':') /*runs until get a space, comma or a colon */
	{
		if(i == 0 && line[*current] == '#') /*if starts with # it's immidiate addressing*/
			type = IMMIDIATE;
		else if(i == 0 && line[*current] == '"') /*if it's start with " it's a string*/
		{
			type = STRING;
			(*current)++; /*skip the '"'*/
			while(*current < lineLength && line[*current] != '"') /*runs of the rest of the string*/
			{
				word->str[i] = line[*current]; /*store the char in the word*/
				i++;
				(*current)++;
			}
			if(line[*current] == '"')
			{
				word->str[i] = line[*current]; /*store the char in the word*/
				i++;
				(*current)++;
			}
			break;
		}
		else
		{
			word->str[i] = line[*current]; /*store the char in the word without the '#', '"'*/
			i++;
		}
		(*current)++;
	}
	if(i == 0 && line[*current] == ',') /*store the comma in the word*/
	{
		word->str[i] = line[*current];
		(*current)++;
		i++;
	}
	if(i > 0 && line[*current] == ':' && type != IMMIDIATE && type != STRING) /*if end with a colon - maybe a label*/
	{
		labelPotential = TRUE;
		(*current)++;
	}
	word->str[i] = (char) '\0'; /*add '\0' to the end of the word*/
	if(i == 0) /*line conains only spcaes*/
		word->type = SPACE; /*classified as SPACE*/
	else if(labelPotential == TRUE)/*check if valid label*/
	{
		if(isLabel(word)) /*if it's a valid name label*/
		{
			if(cmdSearch(&word->str[0]) >= 0) /*check if the label is defined as a name of a command*/
				word->type = INVALID_LABEL;
			else if(i == 2 && word->str[0] == 'r' && word->str[1] >='0' && word->str[1] <='7')
				word->type = INVALID_LABEL;
			else
				word->type = LABEL; /*it's a label*/
		}
		else /*if it's not a valid label*/
			word->type = INVALID_LABEL;
	}
	else if(i == 2 && word->str[0] == 'r' && word->str[1] >='0' && word->str[1] <='7') /*if the word contains 2 chars, first is 'r' and the second is a number between 0-7 it's a register*/
	{
		word->type = REGISTER; /*it's a register*/
		word->reg = (word->str[1] - '0'); /*the word struct contains the register number*/
	}	
	else if(type == IMMIDIATE) /*it's immidiate addressing */
	{
		if(isNum(word)) /*followed by a numer*/
			word->type = IMMIDIATE;
		else /*wrong use of '#'*/
			word->type = WRONG_IMMIDIATE;
	}
	else if(isNum(word))
		word->type = NUMBER;
	else if(i > 1 && type == STRING && word->str[i-1] == '"') /*it's a string only if contain more than 1 char(because of " and '\0')*/
	{
		word->type = STRING; /*it's a string*/
		word->str[i-1] = '\0'; /*The string will be without the '"'*/
	}
	else if(strcmp(&word->str[0], ".string") == 0)
		word->type = STRING_SNT; /*it's a ".string".*/
	else if(strcmp(&word->str[0], ".data") == 0)
		word->type = DATA_SNT; /*it's a ".data".*/
	else if(strcmp(&word->str[0], ".extern") == 0)
		word->type = EXTERN; /*it's an ".extern"*/
	else if(strcmp(&word->str[0], ".entry") == 0)
		word->type = ENTRY; /*it's an ".entry"*/
	else if(strcmp(&word->str[0], ".struct") == 0)
		word->type = STRUCT_SNT; /*it's a ".struct".*/
	else if(strchr(&word->str[0], '.') != NULL) /*check if there is a '.' - maybe a struct in a instruction  */
	{
		int dotPos = 0;
		while(word->str[dotPos] != '.') /*search the '.'*/
			dotPos++;
		word->str[dotPos] = '\0';
		if(isLabel(word) == TRUE) /*check if it's a valid label*/
		{
			if(i - dotPos > 2) /*if there is more than 1 digit after the dot*/
				word->type = INVALID_STRUCT;
			else if(word->str[dotPos+1] == '1')/*check which field is defined*/
				word->type = STRUCT_FIRST;
			else if(word->str[dotPos+1] == '2')
				word->type = STRUCT_SECOND;
			else
				word->type = INVALID_STRUCT;
		}
		else /*if the struct is not a valid label name*/
			word->type = INVALID_STRUCT;
		word->str[dotPos] = '.';
	}
	else if(word->str[0] == ',')
		word->type = COMMA; /*it's a comma*/
	else if(cmdSearch(&word->str[0]) > -1)/*runs through the command struct list*/
		word->type = CMD; /*it's a known command*/
	else
		word->type = OTHER; /*nothing found. could be a label in the line*/
}

/*
The function gets a wordstruct and check if it's a label
*/
int isLabel(wordStruct *word)
{
	int pos = 0;
	if(!isalpha(word->str[pos]))/*check if the first char is a letter*/
		return FALSE;
	pos++;
	while(pos < LABEL_NAME_SIZE && isalnum(word->str[pos]))/*check if the label contain letters and numbers only*/
		pos++;
	if(word->str[pos] == '\0')/*if we reached the null terminator all the string is valid label*/
		return TRUE;
	return FALSE;/*the string is longer than the maximum size of a label*/
}
	
/*
This function checks if the received word is a number and if so it will be inserted to a word dec variable.
*/
int isNum(wordStruct *word)
{
	int pos = 0, sign = 1, num = 0, mul = 10;
	if(word->str[pos] == '-') /*check if it's a negative number*/
	{
		sign = -1;
		pos++;
	}
	if(word->str[pos] == '+') /*check if the number starts with '+'*/
		pos++;
	while(word->str[pos] != '\0') /*translate a string to a number*/
	{
		if(!(isdigit(word->str[pos]))){
			return FALSE;}
		num = num *mul +(word->str[pos] - '0');
		pos++;
	}
	word->num = num * sign; /*saves the result in the word*/
	return TRUE;
	
}	

