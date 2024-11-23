/* TOOLKIT.H include file

   Gernot Kunz, 1986/11/05
*/


/*      This is a group of flags causing an error return (-1), if
        the addressing mode associated with the specific flag
        occurs.
*/

#define DDLIM  0x0001  /* data register direct mode */
#define ADLIM  0x0002  /* address register direct mode */
#define AILIM  0x0004  /* address register indirect mode */
#define PILIM  0x0008  /* postincrement mode */
#define PDLIM  0x0010  /* predecrement mode */
#define IDLIM  0x0020  /* indirect with displacement */
#define IIDLIM 0x0040  /* indirect with index and displacement */
#define ASLIM  0x0080  /* absolute short */
#define ALLIM  0x0100  /* absolute long */
#define PCLIM  0x0200  /* program counter rel. with displacement */
#define PCILIM 0x0400  /* program counter rel. with index and displacement */
#define IMLIM  0x0800  /* immediate */

#define SRMODE 0x1000  /* enable logical operations on status register */

#define LMODE  0x2000  /* enable long word immediate mode */

#define HORDER 0x4000  /* use high order mode/register bits 6-11 instead of
                          bits 0-5 (default is 0-5). */

#define SHORT  (1)         /* Flag for size specification function */
#define NORMAL (0)       
