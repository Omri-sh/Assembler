#include "project.h"
#include "global.h"

/*
This file manage the command proccessing.
*/

/*This function receive a line and its length, current position in the line, command index, number of line and if it's pass 1 or 2 and divide the proccessing by operands amount which later on reference for another proccessing function*/
void cmdProcess(char line[],int lineLength,int current, int cmdIndex, int linesCounter,int pass)
{
	if(cmds[cmdIndex].oper == 0) /*If there is not operands for the command*/
	{
		wordStruct oper; /*contains a operand in a code line*/
		wordParsing(&oper,line,lineLength,&current);
		if(oper.type != SPACE) /*if there is any chars after command with non operands*/
		{
			printError(linesCounter, "This command doesn't have operands.");
			return;
		}
		g_codeArr[g_IC++] = cmds[cmdIndex].code << OPC_SHIFT; /*Insert op code of the commad*/
	}
	if(cmds[cmdIndex].oper > 0) /*if it's a command with any operands*/
	{
		wordStruct oper, comma, oper2, tmp; /*contains a operands in a code line*/
		bool err = FALSE;
		if(cmds[cmdIndex].oper == 1) /*if the command has 1 operand*/
		{
			int firstAddress = g_IC; /*save the current free address*/
			wordParsing(&oper,line,lineLength,&current); /*get the first operand*/
			wordParsing(&tmp,line,lineLength,&current); /*get word after the operand*/
			if(tmp.type != SPACE) /*if the word isn't SPACE - error cause command has 1 operand*/
			{
				printError(linesCounter, "This command contains only 1 operand.");
				return;
			}
			g_codeArr[g_IC++] = (cmds[cmdIndex].code << OPC_SHIFT) + ABSOLUTE;/*Insert op code of the commad*/
			err = cmdProcess2(&oper,cmdIndex,1,firstAddress, pass, linesCounter); /*calculate and save the next address and update the current address*/
			if(err == FALSE) /*if error was found, the error is about unmatching command and operand*/
				printError(linesCounter, "The operand doesn't match the command.");
		}
		if(cmds[cmdIndex].oper == 2) /*if the command has 2 operands*/
		{
			int firstAddress = g_IC; /*save current address*/
			wordParsing(&oper,line,lineLength,&current); /*get first operand*/
			wordParsing(&comma,line,lineLength,&current); /*get the next word - should be comma*/
			if(comma.type != COMMA) /*if not comma than error*/
			{
				printError(linesCounter, "Comma should saparate between operands.");
				return;
			}
			wordParsing(&oper2,line,lineLength,&current); /*get next operand*/
			wordParsing(&tmp,line,lineLength,&current); /*get next word after operand*/
			if(tmp.type != SPACE) /*if not SPACE - than error*/
			{
				printError(linesCounter, "This command contains only 2 operands.");
				return;
			}
			if(pass == 1)
				g_codeArr[g_IC++] = (cmds[cmdIndex].code << OPC_SHIFT) + ABSOLUTE;/*Insert op code of the commad*/
			else
				g_IC++;
			err =  cmdProcess2(&oper,cmdIndex,1,firstAddress, pass, linesCounter); /*calculate and save the next address and update the current address*/
			if(err == FALSE) /*if error was found, the error is about unmatching command and first operand*/
			{
				printError(linesCounter, "The first operand doesn't match the command.");
				return;
			}
			err =  cmdProcess2(&oper2,cmdIndex,2,firstAddress, pass, linesCounter);
			if(err == FALSE) /*if error was found, the error is about unmatching command and second operand*/
				printError(linesCounter, "The second operand doesn't match the command.");
		}
	}
}


/*This function receives a word which contains the command, command Index, which argument is passed (first or second), the address of the command itself, which pass is it and line number and calculate the bits of a given line. the function add address as needed and in pass 2 the function update labels address*/
int cmdProcess2(wordStruct *arg,int cmdIndex, int argPass, int firstAddress, int pass, int linesCounter)
{
	int dest = 1; /*destinaion or source indicator*/
	if(cmds[cmdIndex].oper==2 && argPass == 1) /*if it's first operand which 2 operands command*/
		dest = 0;
	if(cmds[cmdIndex].permit[dest][0] == 1 && (arg->type == IMMIDIATE || arg->type == WRONG_IMMIDIATE)) /*check if argument is IMMIDIATE and it's valid to be IMMIDIATE*/
	{
		unsigned int num;
		if(arg->type == WRONG_IMMIDIATE) /*if it's a wrong use of '#'*/
		{
			g_IC++;
			printError(linesCounter, "After '#' should be a number.");
			return TRUE;
		}
		if(pass == 2) /*if it's pass 2*/
		{
			g_IC++;
			return TRUE;
		}
		if(arg->num > MAX_VALUE_ALLOWED_IMMIDIATE) /*check if the value is larger than allowed in 8 bits by two's complement*/
		{
			printError(linesCounter, "The value is larger than allowed.");
			g_IC ++;
			return TRUE; /*return true to not print another error in cmdProccess function but the g_err still indicate an error*/
		}
		if(arg->num < MIN_VALUE_ALLOWED_IMMIDIATE) /*check if the value is smaller than allowed in 8 bits by two's complement*/
		{
			printError(linesCounter, "The value is smaller than allowed.");
			g_IC ++;
			return TRUE; /*return true to not print another error in cmdProccess function but the g_err still indicate an error*/
		}
		if(arg->num < 0) /*if the number is negative*/
			num = twosComplement(arg->num,CODE); /*save the two's complement of the number*/
		else /*if positive*/
			num = arg->num;
		if(argPass == 1) /*if it's the first arg*/
			g_codeArr[firstAddress] = g_codeArr[firstAddress] + (IMMIDIATE_ADD << ((cmds[cmdIndex].oper == 1) ? DESTINATION_SHIFT : SOURCE_SHIFT)) + ABSOLUTE; /*update the source or destination in the address, if it's command of 1 operand than the addressing method will be saved in the destination bit's. else - in source bits, and add ABSOLUTE bits*/
		else /*if it's the second arg*/
			g_codeArr[firstAddress] = g_codeArr[firstAddress] + (IMMIDIATE_ADD << DESTINATION_SHIFT); /*update the destination bits*/
		g_codeArr[g_IC++] = (num << DESTINATION_SHIFT) + ABSOLUTE; /*store the number with ABSOLUTE(00)*/
	}
	else if(cmds[cmdIndex].permit[dest][1] == 1 && arg->type == OTHER) /*check if argument is DIRECT and it's valid to be DIRECT*/
	{
		if(pass == 1)
		{
			if(argPass == 1) /*if it's the first arg*/
				g_codeArr[firstAddress] = g_codeArr[firstAddress] + (IMMIDIATE_ADD << ((cmds[cmdIndex].oper == 1) ? DESTINATION_SHIFT : SOURCE_SHIFT)) + ABSOLUTE; /*update the source or destination in the address and add ABSOLUTE bits*/ 
			else /*if it's the second arg*/
				g_codeArr[firstAddress] = g_codeArr[firstAddress] + (IMMIDIATE_ADD << DESTINATION_SHIFT); /*update the destination bits*/
			g_IC++; /*save address for the label address*/
		}
		else
		{
			int x;
			if((x = labelSearch(&arg->str[0])) >= 0) /*check if the label exists*/
			{
					g_codeArr[g_IC++] = (g_labelTable[x].labelAddress << DESTINATION_SHIFT) + RELOCATABLE; /*store address and RECOLATABLE bit's*/
					g_codeArr[firstAddress] = g_codeArr[firstAddress] + (DIRECT_ADD << DESTINATION_SHIFT); /*update the destination bits*/
			}
			else /*if label isn't in label table*/
			{
				if((x = extSearch(&arg->str[0])) >= 0) /*check if the label exists as extern*/
				{
					g_exTable[x].exAddress[g_exTable[x].exAddNum++] = g_IC; /*update address in the exTable*/
					g_codeArr[g_IC++] = EXTERNAL; /*store machine code in the code array*/
					g_codeArr[firstAddress] = g_codeArr[firstAddress] + (DIRECT_ADD << DESTINATION_SHIFT); /*update the destination bits*/
				}
				else /*if label doesn't exists*/
				{
					printError(linesCounter, "The label was not defined.");
					g_IC ++;
				}
			}
		}
	}
	else if(cmds[cmdIndex].permit[dest][2] == 1 && (arg->type == STRUCT_FIRST || arg->type == STRUCT_SECOND || arg->type == INVALID_STRUCT)) /*check if argument is STRUCT and it's valid to be STRUCT*/
	{
		if(pass == 1)
		{
			g_IC++; /*save a place for struct address*/
			if(arg->type == INVALID_STRUCT)
				printError(linesCounter, "Invalid struct name.");
			if(argPass == 1) /*if it's the first arg*/
				g_codeArr[firstAddress] = g_codeArr[firstAddress] + (STRUCT_ADD << ((cmds[cmdIndex].oper == 1) ? DESTINATION_SHIFT : SOURCE_SHIFT)) + ABSOLUTE; /*update the source or destination in the address*/
			if(argPass == 2) /*if it's the second arg*/
				g_codeArr[firstAddress] = g_codeArr[firstAddress] + (STRUCT_ADD << DESTINATION_SHIFT); /*update the destination bits*/
			if(arg->type == STRUCT_FIRST) /*if it's reference to the first field of the struct*/
				g_codeArr[g_IC++] = (1 << DESTINATION_SHIFT) + ABSOLUTE; /*store the field number of the struct and add ABSOLUTE bits*/
			else if(arg->type == STRUCT_SECOND) /*if it's reference to the second field of the struct*/
				g_codeArr[g_IC++] = (2 << DESTINATION_SHIFT) + ABSOLUTE;/*store the field number of the struct and add ABSOLUTE bits*/
		}
		else
		{
			int x,i;
			for(i = 0; arg->str[i] != '.'; i++); /*search the '.' of the struct addressing*/
				arg->str[i] = '\0'; /*change to '\0' from compering with label*/
			if((x = labelSearch(&arg->str[0])) >= 0) /*check if the label exists*/
			{
				g_codeArr[g_IC++] = (g_labelTable[x].labelAddress << DESTINATION_SHIFT) + RELOCATABLE; /*store address and RELOCATABLE bit's*/
				g_IC++;
			}
			else /*if the label doesn't exists*/
			{
				printError(linesCounter, "The struct didn't defined.");
				g_IC += 2;
			}
			arg->str[i] = '.'; /*change back to '.' from compering with label*/	
		}
	}
	else if(cmds[cmdIndex].permit[dest][3] == 1 && arg->type == REGISTER) /*check if argument is REGISTER and it's valid to be REGISTER*/
	{
		if(argPass == 1) /*if it's the first arg*/
		{
			if(pass == 1)
			{
				if(cmds[cmdIndex].oper == 1) /*if command has only 1 operand*/
				{
						g_codeArr[firstAddress] = g_codeArr[firstAddress] + (REG_ADD << DESTINATION_SHIFT) + ABSOLUTE; /*update the destination in the address*/
						g_codeArr[g_IC++] = ((arg->reg) << REG_DESTINATION_SHIFT) + ABSOLUTE; /*save register number in the next address + ABSOLUTE*/
				}
				else /*if command has 2 operands (compute first operand)*/
				{
					g_codeArr[firstAddress] = g_codeArr[firstAddress] + (REG_ADD << SOURCE_SHIFT) + ABSOLUTE; /*update the source in the address*/
					g_codeArr[g_IC++] = ((arg->reg) << REG_SOURCE_SHIFT) + ABSOLUTE; /*save register number in source bits*/
				}
			}
			else
				g_IC++;		
		}	
		else /*if it's the second arg*/
		{
				
				if((g_codeArr[firstAddress] >> SOURCE_SHIFT) == ((cmds[cmdIndex].code << DESTINATION_SHIFT) + REG_ADD))/*if the first operand is REGISTER (11)*/
				{
					if(pass == 1)
					{
						g_codeArr[firstAddress] = g_codeArr[firstAddress] + (REG_ADD << DESTINATION_SHIFT); /*update destination in the address*/
						g_codeArr[g_IC-1] = g_codeArr[g_IC-1] + ((arg->reg) << REG_DESTINATION_SHIFT); /*save the register number is the destination bits with the same address of the source*/		
					}					
					else
						return TRUE;
				}
				else /*if the first operand isn't a REGISTER than the REGISTER will be saved in a new address*/
				{
					if(pass == 2)
						g_IC++;
					else
					{
						g_codeArr[firstAddress] = g_codeArr[firstAddress] + (REG_ADD << DESTINATION_SHIFT); /*update destination in the address*/
						g_codeArr[g_IC++] = ((arg->reg) << REG_DESTINATION_SHIFT) + ABSOLUTE; /*save register in a new address and add ABSOLUTE bits*/
					}
				}
		}
	}
	else /*if none of the above fits than the operands doesn't match the command*/
		return FALSE;
	return TRUE;
	
}

/*
This function returns the two's complement of a number in max address bit's size. The function get a number (assumes negative) and type of sentene (CODE or DATA)
*/
unsigned int twosComplement(int num, int type)
{
	int x = 2;
	x <<= (type == CODE) ? (CODE_WITHOUT_AER_SIZE-1) : (DATA_ADD_SIZE-1); /*power 2 with the amount of bit's in address*/
	return (x+ num);
}
