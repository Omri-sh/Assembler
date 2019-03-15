#include "project.h"
#include "global.h"

/*
This file manage the output of the file to ob ent and ext files
*/

/*This function write the ob file by receiving a FILE pointer*/
void writeObject(FILE *fp)
{
	int i; /*for index the arrays*/
	char str[MUZAR_ARRAY_SIZE+1]; /*char array for converation to 32base-muzar and place for '\0'*/
	str[MUZAR_ARRAY_SIZE] = '\0'; /*end with null terminator*/
	fprintf (fp, "\t");
	dtom(g_IC-START_ADD,str); /*convert the g_IC to 32base-muzar minus the starting address*/
	for(i = 0; str[i] == muzarBase[0] && i < MUZAR_ARRAY_SIZE-1; i++); /*skipping leading zeros (IC prints without leading zeros in the guide)*/
	for( ; i < MUZAR_ARRAY_SIZE; i++) /*print the rest of the string*/
			fprintf(fp, "%c", str[i]);
	fprintf (fp, " ");
	dtom(g_DC,str);/*convert the g_DC to 32base-muzar*/
	for(i = 0; str[i] == muzarBase[0] && i < MUZAR_ARRAY_SIZE-1; i++); /*skipping leading zeros (DC prints without leading zeros in the guide)*/
	for( ; i < MUZAR_ARRAY_SIZE; i++)/*print the rest of the string*/
			fprintf(fp, "%c", str[i]);
	fprintf(fp, "\n");
	for(i = START_ADD; i < g_IC; i++) /*the loop runs on the codeArray*/
	{
		dtom(i,str); /*convert code address to 32Base-Muzar*/
		fprintf (fp, "%s\t", str);
		dtom(g_codeArr[i], str); /*convert the code to 32 base-Muzar*/
		fprintf (fp, "%s\n", str);
	}
	for( ; i < g_DC+g_IC; i++) /*the loop runs on the dataArray*/
	{
		dtom(i,str); /*convert data address to 32Base-Muzar*/
		fprintf (fp, "%s\t", str);
		dtom(g_dataArr[i-g_IC], str); /*convert the data to 32 base-Muzar*/
		fprintf (fp, "%s\n", str);
		
	}
}

/*This function write the ext file by receiving a FILE pointer*/
void writeExtern(FILE *fp)
{
	int i; /*for index the array*/
	char str[MUZAR_ARRAY_SIZE+1]; /*char array for converation to 32base-muzar*/
	str[MUZAR_ARRAY_SIZE] = '\0'; /*end with null terminator*/
	for(i = 0; i < g_exTableIndex; i++) /*the loop runs on the extern table*/
	{
		int j;
		for(j = 0; j < g_exTable[i].exAddNum; j++) /*the loop runs on the address of each extern label*/
		{
			dtom(g_exTable[i].exAddress[j], str); /*convert extern address to 32base-muzar*/
			fprintf(fp,"%s\t%s\n", g_exTable[i].label.str, str);
		}
	}
}

/*This function write the ent file by receiving a FILE pointer*/
void writeEntry(FILE *fp)
{
	int i; /*for index the array*/
	char str[MUZAR_ARRAY_SIZE+1]; /*char array for converation to 32base-muzar*/
	str[MUZAR_ARRAY_SIZE] = '\0'; /*end with null terminator*/
	for(i = 0; i< g_labelIndex; i++) /*the loop runs on the label table*/
	{
		if(g_labelTable[i].isEntry == TRUE) /*if the label is entry*/
		{
			dtom(g_labelTable[i].labelAddress, str); /*convert the label address to 32base-muzar*/
			fprintf(fp,"%s\t%s\n", g_labelTable[i].label.str, str);
		}
	}
}

/*This function convert to base 32 - Muzar. The function receives integer to convert and a string to store the converation.*/
void dtom(unsigned int num, char str[])
{
	int i = MUZAR_ARRAY_SIZE-1; /*index*/
	while(num >= 0 && i >= 0) /*the loop runs until the number is fully divided by 32*/
	{
		str[i] = muzarBase[num % MUZAR_BASE_SIZE];
		num /= MUZAR_BASE_SIZE;
		i--;
	}
	while(i >= 0) /*runs to add leading zeros*/
	{
		str[i] = muzarBase[0];
		i--;
	}
}
