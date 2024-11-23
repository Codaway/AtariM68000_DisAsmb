/**********************************************************************/
/*                                                                    */
/*     MODULE logical program counter                                 */
/*                                                                    */
/*     Simulates a logical program counter to retrieve opcodes and    */
/*     effective addresses. Hides relocation.                         */
/*                                                                    */
/*     AUTHOR: Gernot Kunz                                            */
/*                                                                    */
/*     DATE:   1986/10/28                                             */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/*
SPECIFICATION:

   #include <lpc.h>


   lpc_init(handle, relsz)          Initializes the LPC module
   int handle;
   long relsz;

   lpc_set(value)                   Set LPC to a specific value
   long value;

   long lpc_get()                   Get current LPC value

   lpc_fetch(word)                  Fetch a word from LPC address and increment
   int *word;                       LPC. longjmps on overrun error.

   int lpc_lfetch(lword)            Fetch a long word from LPC address and
   long *lword;                     increment LPC. Returns ABSOLUTE or RELATIVE
                                    longjmps on overrun error.

   lpc_rset(addr, nxt)              Set relocation info address to addr. Return
   long addr;                       address of next relocated longword at or
   long *nxt;                       after addr.

   lpc_rnxt(nxt)                    Get address of next longword to be
   long *nxt;                       relocated after current reloc info address
                                    Set reloc info address to this new value.

DEFINES:

   ABSOLUTE                         Signifies absolute long word (lpc_lfetch)
   RELATIVE                         Signifies relative long word (lpc_lfetch)

IMPLEMENTATION:
*/

#include <lpc.h>
#include <stdio.h>
#include <osbind.h>
#include <daerror.h>

/* #define DEBUG */


/* EXTERNAL VARIABLES: */

extern jmp_buf err_buf;       /* Error buffer - used by the fetch routines! */
extern char hex[];            /* Filled by the fetch routines. */



#define NULL (0L)
#define TRUE (1)
#define FALSE (0)


static long lpc;         /* logical program counter */
static int  prg_file;    /* handle of program file */

static unsigned char *reloc_buf;  
                   /* relocation buffer */

static long reloc_first; /* first relocated long word address */
static long reloc_next;  /* next relocated long word address */
static unsigned char *reloc_ptr;  
                   /* next reloc_buf byte to be processed */

static unsigned char *rinfo_ptr;
                   /*  relocation info pointer */
static long rinfo_next;
                   /* next lword address to be returned */


/******************/
/* lpc_init       */
/******************/

lpc_init(handle, relsz)
int handle;                   /* program file handle */
long relsz;                   /* size of relocating information in bytes */
/* Initializes the LPC module */
{
   char *malloc();
   long status;

   prg_file = handle;

   if (relsz > 0x0000ffff)
      err_print("lpc_init", RLC_ERR, ABORT);

   reloc_buf = (unsigned char *) malloc((unsigned int)relsz);
   if (reloc_buf == NULL)
      err_print("lpc_init", OOM_ERR, ABORT);

   status = Fseek((0L - relsz), prg_file, 2);

   if (status < 0L)
      err_print("lpc_init", FSK_ERR, ABORT);

   status = Fread(prg_file, relsz, reloc_buf);
   if (status < relsz)
      err_print("lpc_init", FRD_ERR, ABORT);

   reloc_next = reloc_first = (*reloc_buf++<<24)|(*reloc_buf++<<16)|
                              (*reloc_buf++<<8)|(*reloc_buf++);

   rinfo_next = reloc_first;

   if (reloc_first == 0L)
      reloc_next = reloc_first = 0x7fffffff;

   rinfo_ptr = reloc_ptr = reloc_buf;

   lpc = 0L;

   status = Fseek(28L, prg_file, 0);   /* Start of text segment */
   if (status < 28L)
      err_print("lpc_init", FSK_ERR, ABORT);

}



/****************/
/* lpc_set      */
/****************/

lpc_set(value)
long value;
/* Set LPC to a specific value */
{
   long status;

   status = Fseek((value + 28L), prg_file, 0);
   if (status < (value + 28L))
      err_print("lpc_set", FSK_ERR, ABORT);

   lpc = value;

   reloc_ptr = reloc_buf;
   reloc_next = reloc_first;

   while (reloc_next < lpc)
   {
      while (*reloc_ptr == 1)
      {
         reloc_next += 254L;
         reloc_ptr++;
      }

      if (*reloc_ptr == 0)
         reloc_next = 0x7fffffff;
      else
      {
         reloc_next += *reloc_ptr;
         reloc_ptr++;
      }

   }

#ifdef DEBUG
   printf("lpc_set(0x%08lx)\n", value);
   printf("       reloc_next = 0x%08lx\n", reloc_next);
#endif
   
}




/*****************/
/* lpc_get       */
/*****************/

long lpc_get()
/* Get current LPC value */
{
   return(lpc);
}




/*****************/
/* lpc_fetch     */
/*****************/

lpc_fetch(word)
int *word;
/* Fetch a word frow LPC location. Does a longjmp, if
   a reloc overrun occurred. */
{
   long status;
   char wch[5];

   status = Fread(prg_file, 2L, word);
   if (status < 2L)
      err_print("lpc_fetch", FRD_ERR, ABORT);

   lpc += 2L;

   if ((lpc - 2L) >= reloc_next)
   {
      while (*reloc_ptr == 1)
      {
         reloc_next += 254;
         reloc_ptr++;
      }

      if (*reloc_ptr == 0)
         reloc_next = 0x7fffffff;
      else
      {
         reloc_next += *reloc_ptr;
         reloc_ptr++;
      }

      longjmp(err_buf, 15);  /* relocation overrun */
   }

   sprintf(wch,"%04x",*word);
   concat(hex,wch);

#ifdef DEBUG
printf("lpc_fetch(0x%04x)\n", *word);
#endif
}



/*****************/
/* lpc_lfetch    */
/*****************/

int lpc_lfetch(lword)
long *lword;
/* Fetch a long word frow LPC location. Returns the kind of long word
   encountered, RELATIVE or ABSOLUTE. longjmps on relocation OVERRUN */
{
   long status;
   long savnxt;
   int rval;
   char lwch[9];

   status = Fread(prg_file, 4L, lword);
   if (status < 4L)
      err_print("lpc_lfetch", FRD_ERR, ABORT);

   sprintf(lwch,"%08lx",*lword);
   concat(hex,lwch);

   lpc += 4;

   if ((lpc - 4) >= reloc_next)
   {
      savnxt = reloc_next;

      while (*reloc_ptr == 1)
      {
          savnxt += 254;
          reloc_ptr++;
      }

      if (*reloc_ptr == 0)
         savnxt = 0x7fffffff;
      else
      {
         savnxt += *reloc_ptr;
         reloc_ptr++;
      }

      if ((lpc - 4) == reloc_next)
      {
         reloc_next = savnxt;

#ifdef DEBUG
         printf("lpc_lfetch(0x%08lx) => RELATIVE\n", *lword);
         printf("          reloc_next = 0x%08lx\n", reloc_next);
#endif

         return(RELATIVE);
      }
      else
      {
          reloc_next = savnxt;
         longjmp(err_buf, 15);  /* relocation overrun error */
      }
   }

#ifdef DEBUG
         printf("lpc_lfetch(0x%08lx) => ABSOLUTE\n", *lword);
#endif

   return (ABSOLUTE);
}




/*****************/
/* lpc_rset      */
/*****************/

lpc_rset(addr, nxt)
long addr;
long *nxt;
/* set the relocation info pointer to the next relocated longword
   at or after addr. */
{

   rinfo_ptr = reloc_buf;
   rinfo_next = reloc_first;

   while (rinfo_next < addr)
      lpc_rnxt(&rinfo_next);

   *nxt = rinfo_next;

}



/*****************/
/* lpc_rnxt      */
/*****************/

lpc_rnxt(nxt)
long *nxt;
/* set the relocation info pointer to the next longword to be relocated. */
{
   while (*rinfo_ptr == 1)
   {
      rinfo_next += 254L;
      rinfo_ptr++;
   }

   if (*rinfo_ptr ==
