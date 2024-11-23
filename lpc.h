/* Include file "lpc.h" for module logical program counter,
   project ATARI 520 ST disassembler.

   Author: Gernot Kunz

   Date:   1986/10/28
*/

#define NORMAL   (0)
#define RELATIVE (1)
#define ABSOLUTE (2)
#define OVERRUN  (-1)

long lpc_get();
