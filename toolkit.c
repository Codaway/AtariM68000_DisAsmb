/*****************************************************************/
/*                                                               */
/* Module: Disassembler Toolkit                                  */
/*                                                               */
/*                                                               */
/* Author: Gernot Kunz                                           */
/*                                                               */
/* Date: 1986/11/17                                              */
/*                                                               */
/*                                                               */
/*                                                               */
/*****************************************************************/

#include <toolkit.h>
#include <lpc.h>
#include <opkind.h>
#include <symbol.h>
#include <stdio.h>

/* #define DEBUG */

extern jmp_buf err_buf;
extern int code;


/************/
/* eea()    */
/************/

eea(word, flags, string)
int word;
int flags;
char *string;
/* Extracts the effective address from an opcode and returns the ASCII
assembler string for it. May call "lpc_fetch" or "lpc_lfetch" and the
symbol table routines. Flags may be an OR of:

         DDLIM       data register direct mode
         ADLIM       address register direct mode
         AILIM       address register indirect mode
         PILIM       postincrement mode
         PDLIM       predecrement mode
         IDLIM       indirect with displacement
         IIDLIM      indirect with index and displacement
         ASLIM       absolute short
         ALLIM       absolute long
         PCLIM       program counter relative with displacement
         PCILIM      program counter relative with index and displacement
         IMLIM       immediate

         SRMODE      code for immediate addressing mode is status register
                     operation mode.

         LMODE       long value immediate mode
       
                     This is a group of flags causing an error exit via
                     longjmp, if the addressing mode associated with the
                     specific flag occurs.

         HORDER       use high order mode/register bits 6-11 instead of
                        bits 0-5 (default is 0-5).
*/
{
      int mode;
      int reg;
      char indreg[5];
      int value;
      long dvalue;
      int tag,num;
      char *concat();

#ifdef DEBUG
      printf("eae(0x%04x,0x%04x,string): Entering\n",word,flags);
#endif
     
      if (flags & HORDER)
      {
         reg  = '0' +((word & 0x0e00) >> 9);
         mode = (word & 0x01c0) >> 6;
      }
      else
      {
         reg  = '0' + (word & 0x0007);
         mode = (word & 0x0038) >> 3;
      }

      *string = '\0';

      switch (mode) {
         case (0) :  /* data register direct mode */
                     if (flags & DDLIM)
                        longjmp(err_buf, 1);
                     sprintf(string,"d%c", reg);
                     break;

         case (1) :  /* address register direct mode */
                     if (flags & ADLIM)
                        longjmp(err_buf, 2);
                     sprintf(string,"a%c", reg);
                     break;

         case (2) :  /* address register indirect mode */
                     if (flags & AILIM)
                        longjmp(err_buf, 3);
                     sprintf(string, "(a%c)", reg);
                     break;

         case (3) :  /* postincrement mode */
                     if (flags & PILIM)
                        longjmp(err_buf, 4);
                     sprintf(string, "(a%c)+", reg);
                     break;

         case (4) :  /* predecrement mode */
                     if (flags & PDLIM)
                        longjmp(err_buf, 5);
                     sprintf(string, "-(a%c)", reg);
                     break;

         case (5) :  /* indirect with displacement */
                     if (flags & IDLIM)
                        longjmp(err_buf, 6);
                     lpc_fetch(&value);
                     sprintf(string,"$%x(a%c)", value, reg);
                     break;

         case (6) :  /* index and displacement */
                     if (flags & IIDLIM)
                        longjmp(err_buf, 7);
                     fetch_special(indreg, &value);
                     sprintf(string,"$%x(a%c,%s)", value, reg, indreg);
                     break;

         case (7) :  /* additional modes */
                     switch (reg - '0') {
                       case (0) :   /* absolute short */
                                    if (flags & ASLIM)
                                       longjmp(err_buf, 8);
                                    lpc_fetch(&value);
                                    sprintf(string, "$%x", value);
                                    break;

                       case (1) :   /* absolute long */
                                    if (flags & ALLIM)
                                       longjmp(err_buf, 9);
                                    if (lpc_lfetch(&dvalue)==ABSOLUTE)
                                      sprintf(string, "$%lx", dvalue);
                                    else
                                    {
                                   /* tag = classify(dvalue);
                                      sym_tag(dvalue, tag);   */
                                      sym_retrieve(dvalue,&tag,&num,string);
                                         if (*string=='\0')
                                            cons_symbol(tag,num,string);
                                    }
                                            
                                    break;

                       case (2) :   /* PC relative with displacement */
                                    if (flags & PCLIM)
                                       longjmp(err_buf, 10);
                                    lpc_fetch(&value);

                                    dvalue = lpc_get() + (long)value - 2L;

                                /*  tag = classify(dvalue);
                                    sym_tag(dvalue, tag);  */
                                    sym_retrieve(dvalue,&tag,&num,string);
                                         if (*string=='\0')
                                            cons_symbol(tag,num,string);

                                    concat(string, "(pc)");
                                    break;

                       case (3) :   /* PC relative with index        */
                                    if (flags & PCILIM)
                                       longjmp(err_buf, 11);
                                    fetch_special(indreg, &value);
                                    sprintf(string, "$%x(pc,%s)",value,indreg);
                                    break;

                       case (4) :   /* immediate */
                                    if (flags & SRMODE)
                                       sprintf(string,"sr");
                                    else
                                    {
                                       if (flags & IMLIM)
                                          longjmp(err_buf, 12);

                                       if (flags & LMODE)
                                       {
                                          if (lpc_lfetch(&dvalue)==ABSOLUTE)
                                             sprintf(string, "#$%lx", dvalue);
                                          else
                                          {
                                             *string = '#'; string++;
                                         /*  tag = classify(dvalue);
                                             sym_tag(dvalue, tag);   */
                                             sym_retrieve(dvalue,&tag,&num,string);
                                             if (*string=='\0')
                                                cons_symbol(tag,num,string);
                                          }
                                       }
                                       else
                                       {
                                          lpc_fetch(&value);
                                          sprintf(string,"#$%x", value);
                                       }
                                    }
                                    break;

                       default  :   longjmp(err_buf, 13);
                                    break;
                     }

                     break;

     }
#ifdef DEBUG
   printf("eae(0x%04x,0x%04x,\"%s\"): Leaving\n",word,flags,string);
#endif

}



/*******************/
/* fetch_special() */
/*******************/

fetch_special(indreg, displ)
char *indreg;
int  *displ;
/* fetch an extension word for indirect addressing with index and
displacement. */
{
   int word;
   
#ifdef DEBUG
   printf("fetch_special(indreg, displ): Entering\n");
#endif

   lpc_fetch(&word);

   sprintf(indreg, "%c%c%s", ((word & 0x8000) ? 'a' : 'd'),
                              ('0' + ((word & 0x7000) >> 12)),
                              ((word & 0x0800) ? ".l" : "") );

   *displ = (word & 0x00ff);
   
#ifdef DEBUG
   printf("fetch_special('%c',0x%04x): Leaving\n", indreg,*displ);
#endif
}



/***************/
/* sizespec()  */
/***************/

int sizespec(word, flags, string)
int word;
int flags;
char *string;
/* Extracts the size specification from an opcode and returns
   0 for byte, 1 for word and 2 for long word. Exits via longjmp on error.
   String points to ".b", "", ".l", respectively. Flags may be SHORT,
   NORMAL or HORDER to account for 1bit or 2bit size specs.  */
{
   char *text;
   int  rval;
   
#ifdef DEBUG
   printf("sizespec(0x%04x,0x%04x,string): Entering\n",word,flags);
#endif

   if (flags == SHORT)
      if (word & 0x0100)
      {
         text = ".l";
         rval = 2;
      }
      else
      {
         text = "";
         rval = 1;
      }
   else if (flags == HORDER)
   {
      switch (word & 0x3000) {
         case (0x1000) : text = ".b";
                         rval = 0;
                         break;
         case (0x2000) : text = ".l";
                         rval = 2;
                         break;
         case (0x3000) : text = "";
                         rval = 1;
                         break;
         default :       longjmp(err_buf, 14);
      }
   }
   else
   {
      switch (word & 0x00C0) {
      case (0x0000) : text = ".b";
                      rval = 0;
                      break;
      case (0x0040) : text = "";
                      rval = 1;
                      break;
      case (0x0080) : text = ".l";
                      rval = 2;
                      break;
      default       : longjmp(err_buf, 14);
      }
   }

   strcpy(string, text);
   
#ifdef DEBUG
   printf("%d=sizespec(0x%04x,0x%04x,\"%s\"): Leaving\n",rval,word,flags,string);
#endif

   return(rval);
}



/*******************/
/* reglist()       */
/*******************/

#define TRUE  (1)
#define FALSE (0)

reglist(word, flag, string)
int word;
int flag;
char *string;
/* Returns the assembly code of a "movem" register list. 'flag' specifies
   the order of the register mask. */
{
   int i;     /* shift counter */
   int run;   /* flag signals if bit is part of a run */
   int start; /* start bit of the run */
   int bit;   /* mask of current bit */
   int rc;    /* run count */
   char reg;  /* ascii code of register kind 'd' or 'a' */
   
#ifdef DEBUG
   char *sstr;
   sstr = string;
   printf("reglist(0x%04x,0x%04x,string): Entering\n",word,flag);
#endif

   if (flag) bit = 0x8000;
   else      bit = 0x0001;

   rc = 0;

   for (reg = 'd'; reg != '\0'; reg = ((reg == 'd') ? 'a' : '\0') )
   {
      run = FALSE;

      for (i = 0; i <= 7; i++)
      {
          if (word & bit)       /* bit set */
            if (i == 7)          /* last bit reached */
            {
               if (run)         /* bit is part of a run */
               {
                  *string++ = reg;
                  *string++ = '0' + start;
                  *string++ = '-';
                  *string++ = reg;
                  *string++ = '7';
               }
               else
               {
                  if (rc>0)
                     *string++ = '/';

                  *string++ = reg;
                  *string++ = '7';
               }
            }
            else                /* last bit not yet reached */
            {
               if (!run)        /* if not in a run - start a run */
               {
                  if (rc > 0)   /* all except the first run preceded by '/' */
                     *string++ = '/';

                  start = i;
                  run   = TRUE;
                  rc++;
               }
            }
          else                       /* bit not set */
            if (run)                 /* was there a run opened */
            {
               if (start < (i - 1))  /* consisted the run of >= 1 item */
               {
                  *string++ = reg;
                  *string++ = '0' + start;
                  *string++ = '-';
               }

               *string++ = reg;
               *string++ = '0' + i - 1;
               run = FALSE;
            }

         if (flag) word <<= 1;
         else       word >>= 1;
         }
   }

   *string = '\0';
   
#ifdef DEBUG
   printf("reglist(0x%04x,0x%04x,\"%s\"): Leaving",word,flag,sstr);
#endif

}


/*******************/
/* conditions()    */
/*******************/

conditions(word, text)
int word;
char *text;
/* masks out the condition part of an opcode and returns the mnemonics */
{
   char *lotext;
   
#ifdef DEBUG
   printf("conditions(0x%04x,text): Entering\n",word);
#endif

   switch(word & 0x0f00) {
      case (0x0000) : lotext = "t"; 
                      break;
      case (0x0100) : lotext = "f";
                      break;
      case (0x0200) : lotext = "hi";
                      break;
      case (0x0300) : lotext = "ls";
                      break;
      case (0x0400) : lotext = "cc";
                      break;
      case (0x0500) : lotext = "cs";
                      break;
      case (0x0600) : lotext = "ne";
                      break;
      case (0x0700) : lotext = "eq";
                      break;
      case (0x0800) : lotext = "vc";
                      break;
      case (0x0900) : lotext = "vs";
                      break;
      case (0x0a00) : lotext = "pl";
                      break;
      case (0x0b00) : lotext = "mi";
                      break;
      case (0x0c00) : lotext = "ge";
                      break;
      case (0x0d00) : lotext = "lt";
                      break;
      case (0x0e00) : lotext = "gt";
                      break;
      case (0x0f00) : lotext = "le";
                      break;
   }

   strcpy(text, lotext);
   

}


/*****************/
/* concat()      */
/*****************/

char *concat(left, right)
char *left;
char *right;
/* Concatenates the strings "left" and "right". "left" points to the
   result and is also returned. It must be a pointer to enough memory
   to hold the result. */
{
   char *save;
   
#ifdef DEBUG
   char *sl, *sr;
   sl = left;
   sr = right;
   printf("concat(\"%s\",\"%s\"): Entering\n",left,right);
#endif

   save = left;

   while (*left != '\0')
      left++;

   while (*right != '\0')
      *left++ = *right++;

   *left = '\0';
   
#ifdef DEBUG
   printf("\"%s\"=concat(\"%s\",\"%s\"): Leaving\n",save,sl,sr);
#endif

   return(save);

}


/*****************/
/* find_label()  */
/*****************/

find_label(addr, label)
long addr;
char *label;
/* Tries to retrieve a symbol at a specific address. If the symbol is named,
returns <name>:, else constructs a symbol name from the tag and number. */
{
   int tag,num;

#ifdef DEBUG
   printf("find_label(%lx,label): Entering\n",addr);
#endif

   *label = '\0';

   if (sym_retrieve(addr,&tag,&num,label) == 0)
   {
       if (*label == '\0')
         cons_symbol(tag, num, label);

      concat(label, ":");
   }
#ifdef DEBUG
   printf("find_label(%lx,\"%s\"): Leaving\n",addr,label);
#endif
}



/*****************/
/* cons_symbol() */
/*****************/

cons_symbol(tag, num, symbol)
int tag, num;
char *symbol;
/* Constructs a symbol name from the tag and number. */
{
   char *pf;

#ifdef DEBUG
   printf("cons_symbol(%d,%d,symbol): Entering\n",tag,num);
#endif

   if      (tag & LBL)
      switch (tag) {
         case (SBR) : pf = "sb";
                      break;
         case (LOP) : pf = "lp";
                      break;
         default    : pf = "lb";
                      break;
         }
   else if (tag & VAR)
      if    ((tag & DAT) == DAT)
         pf = "da";
      else
         pf = "bs";
   else
      pf = "ut";

   sprintf(symbol, "%s%04d", pf, num);
   
#ifdef DEBUG
   printf("cons_symbol(%d,%d,\"%s\"): Leaving\n",tag,num,symbol);
#endif
}
