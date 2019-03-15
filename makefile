assembler: cmdProcess.o fileParsing.o outputFiles.o wordParsing.o main.o
	gcc -Wall -ansi -pedantic cmdProcess.o fileParsing.o outputFiles.o wordParsing.o main.o -o assembler
 
cmdProcess.o: cmdProcess.c global.h project.h
	gcc -Wall -ansi -pedantic cmdProcess.c -c

fileParsing.o: fileParsing.c global.h project.h
	gcc -Wall -ansi -pedantic fileParsing.c -c 

outputFiles.o: outputFiles.c global.h project.h
	gcc -Wall -ansi -pedantic outputFiles.c -c 

wordParsing.o: wordParsing.c global.h project.h
	gcc -Wall -ansi -pedantic wordParsing.c -c 

main.o: main.c global.h project.h
	gcc -Wall -ansi -pedantic main.c -c 
