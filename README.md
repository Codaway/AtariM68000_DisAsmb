# AtariM68000_DisAsmb
Disassembler for Motorola 68000 on Atari ST computers

This program, in C, reads "EXE" files for vintage Atari ST (520, 1040) and tries to disassemble them
in an intelligent way.

The challenge is to find out what is code and what is data. 

The disassembler does that by starting with the entry address, then following instructions and branches
to discover the parts of the binary image which are code for sure.

It then disassembles this and creates text output.

The disassembler was working quite well, but is far from perfect.
To be honest: it was mainly used to find places in the code where there was a copy protection, and allow me
to hack it.
