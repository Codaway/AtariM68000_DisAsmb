/* Include file "symbol.h" for module symbol table,
   project ATARI 520 ST disassembler.

   Author: Gernot Kunz

   Date:   1986/10/27
*/

#define UTG 0x0000   /* untagged symbol */

#define LBL 0x1000   /* label category    */
#define VAR 0x2000   /* variable category */

#define BYT 0x2001   /* byte variable category */
#define WRD 0x2002   /* word variable category */
#define LNG 0x2004   /* long variable category */

#define LOP 0x1001   /* loop label category */
#define SBR 0x1002   /* subroutine label category */

#define DAT 0x2010   /* data field variable category */
#define BSS 0x2020   /* bss  field variable category */
