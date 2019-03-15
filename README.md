# Assembler
This is an implementation of a two-pass assembler that translates ASM instruction to 32-base digits (called Muzar in the project). The Assembler supports instructions, .data, .string, .entry, .extern. 
\nPass 1: loads the instruction from an input file, addressing each label into the symbol table and check for syntax errors. 
Pass 2: completes or updates all the missing addresses in the symbol table, check for additional errors and translate each instruction to 32 base to output file. 
This project was written as assignment for the university (grade 95).
