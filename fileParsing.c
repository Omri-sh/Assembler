#include "project.h"
#include "global.h"

/*
This file parsing a input file in 2 passes as defined in the guide and refernce for parsing commands 
*/


/*This function parsing a file by passing it twice. The function receives a file pointer and parsing each word in each line and stores the translation in the code and data arrays.*/
void parsing(FILE *fp)
{
	char line[LINE_SIZE+1]; /*stores the line*/
	int i;
	int c; /*indicator for ends of file*/
	int lineLength; /*store the size of a line*/
	int current; /*index for current position in the line*/
	int linesCounter = 0; /*counter of lines in the file*/
	bool isSymbol = FALSE; /*indicator for a label*/
	wordStruct word,tmpword; /*stores words*/
	g_IC = START_ADD; /*set start address for code array*/
	g_DC = 0; /*set start index in data array*/
	g_labelIndex = 0; /*set start index in label table*/
	g_exTableIndex = 0; /*set start index in extern table*/
	hasEntry = FALSE; 
	while(c != EOF) /*first pass*/
	{
		c = getc(fp);
		for(i = 0;c != EOF && c != '\n'; i++) /*The loop insert the line to the array as long it's not passes the allowd length of the line.*/
		{
			if(i < LINE_SIZE) /*if we not passes the allowed length of the line*/
				line[i] = (char) c; /*insert to the line array*/
			c = getc(fp);
		}
		linesCounter++; /*update lines counter*/
		if(i == 0 || line[0] == ';') /*skip empty or comment line*/
			continue;
		if(i >= LINE_SIZE) /*if the line is longer than allowed, skips the line*/
		{
			printError(linesCounter, "The line is longer than allowed.");
			continue;
		}
		lineLength = i; /*update line length indicator*/
		line[i] = '\0'; /*add '\0' to the end of the line*/
		current = 0;
		isSymbol = FALSE; /*reset label indicator*/
		while(line[current] != '\0' && (line[current] == ' ' || line[current] == '\t')) /*skip whitespaces and tabs*/
			current++;
		if(current == lineLength || line[current] == ';') /*skip line if has only whitespaces or tabs , or comment line*/
			continue;
		while(current < lineLength) /*runs again on the line*/
		{
			wordParsing(&word,line,lineLength,&current); /*get a word in the line*/
			if(word.type == LABEL) /*check if the word is label*/
			{
				if(extSearch(&word.str[0]) >= 0) /*check if the label is already defined as extern*/
					printError(linesCounter, "This label name is already defined as extern.");
				else if(labelSearch(&word.str[0]) >= 0) /*check if the label is already defined*/
					printError(linesCounter, "This label name is already defined.");
				else
					isSymbol = TRUE; /*update label indicator*/
				tmpword = word; /*save the label word*/
				wordParsing(&word,line,lineLength,&current); /*get the next word after the label*/
			}
			else if(word.type == INVALID_LABEL) /*check if the word is invalid label name*/
			{
				printError(linesCounter, "Label is defined as a invalid label name.");
				tmpword = word; /*save the label word*/
				wordParsing(&word,line,lineLength,&current); /*get the next word after the label*/
			}
			if(word.type == DATA_SNT || word.type == STRING_SNT || word.type == STRUCT_SNT) /*if the word is ".data", ".string", ".struct"*/
			{
				if(isSymbol) /*if it's after a label*/
				{
					if(labelSearch(&tmpword.str[0]) < 0) /*if the label was not defined*/
					{
						g_labelTable[g_labelIndex].label = tmpword; /*save the word in the label table*/
						g_labelTable[g_labelIndex].labelAddress = g_DC; /*insert the current free spaced in the memory*/
						g_labelTable[g_labelIndex].type = DATA; /*update the type of the line*/
						g_labelTable[g_labelIndex].isEntry = FALSE; /*set label as not entry*/
						g_labelIndex++;
					}
					else
						printError(linesCounter, "This label was already defined.");
				}
				if(word.type == STRING_SNT)
				{
					wordParsing(&word,line,lineLength,&current); /*get the next word after the ".string"*/
					if(word.type == SPACE) /*check if no string after '.string'*/
						printError(linesCounter,"Must be a string after '.string'");
					else if(word.type == STRING)/*if it's a string after the ".string"*/
					{
						i = 0;
						while(word.str[i] != '\0') /*runs on the word*/
						{
							g_dataArr[g_DC+i] = (unsigned int)word.str[i]; /*store the string in the Data array*/
							i++;
						}
						g_dataArr[g_DC+i] = (unsigned int)'\0'; /*add '\0' to the end*/
						g_DC += i+1; /*update the DC indicator*/
					}
					else
						printError(linesCounter,"Not a string.");
					wordParsing(&word,line,lineLength,&current); /*get the next word after the string*/
					if(word.type != SPACE)/*if it's a string after the ".string"*/
						printError(linesCounter,"'.string' gets only 1 parameter.");
				}
				else if(word.type == STRUCT_SNT)
				{
					wordParsing(&word,line,lineLength,&current); /*get the next word after the ".struct"*/
					if(word.type == NUMBER)/*if it's a string after the ".string"*/
					{
						if(word.num <= MAX_VALUE_ALLOWED_DATA && word.num >= MIN_VALUE_ALLOWED_DATA) /*if the number is between allowed values*/
						{
							 if(word.num < 0) /*if negative*/
								g_dataArr[g_DC++] = twosComplement(word.num, DATA); /*store the two's complement of thenumber in the data array*/
							else /*if positive*/
								g_dataArr[g_DC++] = word.num; /*store the number in the data array*/
							wordParsing(&word,line,lineLength,&current); /*get the next word*/
							if(word.type != COMMA)
							{
								printError(linesCounter, "Comma should saparate structs fields.");
								break;
							}
							wordParsing(&word,line,lineLength,&current); /*get the next word*/
							if(word.type == STRING)/*if it's a string after the number*/
							{
								i = 0;
								while(word.str[i] != '\0') /*runs on the word*/
								{
									g_dataArr[g_DC+i] = (unsigned int)word.str[i]; /*store the string in the Data array*/
									i++;
								}
								g_dataArr[g_DC+i] = '\0'; /*add '\0' to the end*/
								g_DC += i+1; /*update the DC indicator*/
							}
							else
								printError(linesCounter,"exepted a string after a number in '.struct'.");
							wordParsing(&word,line,lineLength,&current); /*get the next word*/
							if(word.type != SPACE)/*if there is something after the string*/
								printError(linesCounter, "'.struct' has only 2 fields.");
						}
						else /*if the value is larger or smaller than allowed*/
						{
							if(word.num > MAX_VALUE_ALLOWED_DATA) /*check if the value is larger than allowed in 10 bits by two's complement*/
								printError(linesCounter, "The value is larger than allowed.");
							else /*check if the value is smaller than allowed in 10 bits by two's complement*/
								printError(linesCounter, "The value is smaller than allowed.");
						}
					}
					else
						printError(linesCounter,"exepted a number after '.struct'.");
				}
				else /*it's DATA_SNT*/
				{
					bool previousComma = FALSE; /*indicator which indicate that last word was a comma*/
					wordParsing(&word,line,lineLength,&current); /*get the next word after ".data"*/
					if(word.type == SPACE) /*check if no numbers after '.data'*/
						printError(linesCounter,"Must be a number after '.data'");
					else /*if there is a somthing after the '.data'*/
					{
						while(word.type == NUMBER)
						{
							previousComma = FALSE;
							if(word.num > MAX_VALUE_ALLOWED_DATA) /*check if the value is larger than allowed in 10 bits by two's complement*/
							{
								printError(linesCounter, "The value is larger than allowed.");
								g_dataArr[g_DC++] = 0;
								wordParsing(&word,line,lineLength,&current); /*get the next word after the number*/
							}
							if(word.num < MIN_VALUE_ALLOWED_DATA) /*check if the value is smaller than allowed in 10 bits by two's complement*/
							{
								printError(linesCounter, "The value is smaller than allowed.");
								g_dataArr[g_DC++] = 0;
								wordParsing(&word,line,lineLength,&current); /*get the next word after the number*/
							}
							if(word.num < 0) /*if negative*/
								g_dataArr[g_DC++] = twosComplement(word.num, DATA); /*store the two's complement of thenumber in the data array*/
							else /*if positive*/
								g_dataArr[g_DC++] = word.num; /*store the number in the data array*/
							wordParsing(&word,line,lineLength,&current); /*get the next word after the number*/
							if(word.type != COMMA)/*comma should saparate*/
								break;
							previousComma = TRUE;
							wordParsing(&word,line,lineLength,&current); /*get the next word after the number*/
						}
						if(word.type != SPACE)
							printError(linesCounter, "Should be numbers after '.data', Missing ',' between numbers.");
						if(word.type == SPACE && previousComma == TRUE)
							printError(linesCounter, "Should be numbers after ',' in '.data'.");
					}
				}
				current = lineLength; /*end of line - get the next line*/
				continue;			
			}
			if(word.type == EXTERN || word.type == ENTRY) /*if ".entry" or ".extern"*/
			{
				if(word.type == EXTERN) /*if ".extern"*/
				{
					wordParsing(&word,line,lineLength,&current); /*get the next word after the ".extern"*/
					wordParsing(&tmpword,line,lineLength,&current); /*get the next word after the label*/
					if(tmpword.type != SPACE)
						printError(linesCounter, "'.extern' has only 1 operand.");
					else if(labelSearch(&word.str[0]) >= 0) /*check if the label was defined already as internal label*/
						printError(linesCounter, "Internal label cannot be defined as extern.");
					else if(extSearch(&word.str[0]) >= 0) /*check if the label is already defined as extern*/
						printError(linesCounter, "This extern is already defined as extern.");
					else if(isLabel(&word)) /*check if valid label*/
					{
						g_exTable[g_exTableIndex].label = word; /*store the label word*/
						g_exTable[g_exTableIndex].exAddNum = 0; /*no extern address*/
						g_exTableIndex++;
					}
					else
						printError(linesCounter, "Must be an allowed label type after '.extern'.");
				}
				else /*if ".entry"*/
				{
					wordParsing(&word,line,lineLength,&current); /*get a word in the line*/
					wordParsing(&tmpword,line,lineLength,&current); /*get the next word after the label*/
					if(tmpword.type != SPACE) /*error - ".entry" can have only 1 operand*/
						printError(linesCounter, ".entry can get only 1 operand.");
					if(word.type == CMD || isLabel(&word) == FALSE)
						printError(linesCounter, "Invalid label name after '.entry'.");
					if(word.type == SPACE) /*error - ".entry" can have only 1 operand*/
						printError(linesCounter, "Must be a label after '.entry'.");
				}
				current = lineLength;/*end of line - get the next line*/
				continue;
			}
			if(isSymbol) /*If there is a symbol it's belongs to code line*/
			{
				g_labelTable[g_labelIndex].label = tmpword; /*store the label in the label table*/
				g_labelTable[g_labelIndex].type = CODE; /*classify as a CODE*/
				g_labelTable[g_labelIndex].labelAddress = g_IC; /*update the address*/
				g_labelTable[g_labelIndex].isEntry = FALSE; /*set label as not entry*/
				g_labelIndex++;
			}
			if(word.type != SPACE)
			{
				i = cmdSearch(&word.str[0]); /*search command in cmds*/
				if(i >= 0) /*if found*/
					cmdProcess(line,lineLength, current, i,linesCounter,1); /*proccess thecommand*/
				else /*if not found*/
					printError(linesCounter, "Command has not been found.");
				current = lineLength;
				continue;
			}
			
		}
			
	}/*end of first pass*/
	if(g_error == TRUE)/*if error was found - exit function*/
	{ 
		fprintf(stderr, "Files wasn't created for this file.\n");
		return;
	}
	
	for(i = 0; i < g_labelIndex; i++) /*updating data labels in the label table a final address with IC*/
		if(g_labelTable[i].type == DATA)
			g_labelTable[i].labelAddress +=g_IC;

	rewind(fp);/*reset to the start of the file*/
	linesCounter = 0; /*reset lines counter*/
	g_IC = START_ADD; /*reset starting address for code array*/
	c = 0;
	while(c != EOF) /*second pass*/
	{
		c = getc(fp);
		for(i = 0; i < LINE_SIZE && c != EOF && c != '\n'; i++) /*The loop insert the line to the array. If the line is longer than maximum allowed it's stops.*/
		{
			line[i] = (char) c;
			c = getc(fp);
		}
		linesCounter++;
		if(i == 0 || line[0] == ';') /*skip empty or comment line*/
			continue;
		lineLength = i; /*update line length indicator*/
		line[i] = '\0'; /*add '\0' to the end of the line*/
		current = 0;
		while(line[current] != '\0' && (line[current] == ' ' || line[current] == '\t')) /*skip whitespaces and tabs*/
			current++;
		
		if(current == lineLength || line[current] == ';') /*skip line if has only whitespaces or tabs , or comment line*/
			continue;
		while(current < lineLength) /*runs again on the line*/
		{
			wordParsing(&word,line,lineLength,&current); /*get a word in the line*/
			if(word.type == LABEL) /*check if the word is label*/
			{
				tmpword = word; /*store label word*/
				wordParsing(&word,line,lineLength,&current); /*get the next word after the label*/
			}
			if(word.type == DATA_SNT || word.type == STRING_SNT || word.type == STRUCT_SNT || word.type == EXTERN) /*if the word is ".data", ".string", ".struct", ".extern"*/
			{	
				current = lineLength;
				continue; /*not need to be changed*/	
			}	
			if(word.type == ENTRY) /*if ".entry"*/
			{
				wordParsing(&word,line,lineLength,&current); /*get a word in the line*/
				if((i = extSearch(&word.str[0])) >= 0) /*check if the label is already defined as extern*/
					printError(linesCounter, "This label name is already defined as extern.");
				else if(i == -1) /*if the loop runs on all the extern label and found nothing*/
				{
					i = labelSearch(&word.str[0]); /*search the label of ".entry"*/
					if(i >= 0) /*if found*/
					{
						g_labelTable[i].isEntry = TRUE; /*tag as ".entry" label*/
						hasEntry = TRUE; /*update entry indicator*/
					}
					else /*if label was not found*/
						printError(linesCounter, "Entry label has not been found.");
				}
				current = lineLength;
				continue; /*get next line*/
			}
			if(word.type != SPACE) /*if it's not of the above in a command*/
			{
				i = cmdSearch(&word.str[0]); /*search commands in cmds*/
				if(i >= 0)
					cmdProcess(line,lineLength, current, i,linesCounter,2);
				else
					printError(linesCounter, "Command has not been found.");
				current = lineLength;
				continue;
			}
			
		}
			
	}/*end of second pass*/
	if(g_error == TRUE)/*if error was found - exit function*/
		fprintf(stderr, "Files wasn't created for this file.\n");
}

/*
This function gets a char array contains a label name and search it in the label table. if found returns its index, else returns -1
*/
int labelSearch(char str[])
{
	int i;
	for(i = 0; i < g_labelIndex; i++) /*runs through the label table*/
	{
		if(strcmp(str,&g_labelTable[i].label.str[0]) == 0) /*if found the label returns the index in the label table*/
			return i;
	}
		return -1; /*if wasn't found return -1*/
}

/*
This function gets a char array contains a extern name and search it in the extern table. if found returns its index, else returns -1
*/
int extSearch(char str[])
{
	int i;
	for(i = 0; i < g_exTableIndex; i++) /*the loop runs on the extern label*/
	{
		if(strcmp(g_exTable[i].label.str, str) == 0) /*if found the label returns the index in the extern table*/
			return i;
	}
	return -1; /*if wasn't found return -1*/
}

/*
this function get the error line number and a message, prints it and update the error indicator.
*/
void printError(int errorLine, char *msg)
{
	fprintf(stderr, "Error has been found in line %d: %s\n", errorLine, msg);
	g_error = TRUE; /*indicator for error found*/
}

/*
Search the command in the command struct. if found return the command code, if not return -1
*/
int cmdSearch(char command[])
{
	int i;
	for(i = 0; i <= MAX_CMD_CODES; i++)
	{
		if(strcmp(command, cmds[i].name) == 0)
			return i;
	}
	return -1;
}

