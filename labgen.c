/**********************************************************************/
/*                                                                    */
/*     MODULE label generator                                         */
/*                                                                    */
/*     Disassembles program code without generating text for          */
/*     generating labels.                                             */
/*                                                                    */
/*     AUTHOR: Gernot Kunz                                            */
/*                                                                    */
/*     DATE:   1986/11/25                                             */
/*                                                                    */
/*                                                                    */
/**********************************************************************/


#include <toolkit.h>
#include <lpc.h>
#include <opkind.h>
#include <symbol.h>
#include <stdio.h>

/* #define DEBUG */

/* EXTERNALS */

extern jmp_buf err_buf;   /* global error return (TOOLKIT, LPC) */
extern long dat_start;
extern long bss_start;
extern long bss_end;
extern int code;


/* FILE SPECIFIC VARIABLES: */

overlay "labgen"

static unsigned int word; /* opcode word */

static int  val;          /* additional value */
static long lval;         /* additional long value */
static char size[3];      /* size string */


/*********************/
/*  labgen           */
/*********************/

int labgen()
/* disassemble the instruction at the current logical program counter
   without generating text. Returns the opcode kind on OK , else a negative
   error code is returned. */
{
   char costr[3];
   int tag, num;
   long sav_lpc;

#ifdef DEBUG
   printf("labgen() Entering\n");
#endif

   if (val = setjmp(err_buf))
   {
         lpc_set(sav_lpc);
         return((-val));
   }

   lpc_fetch(&word);
   sav_lpc = lpc_get();

   switch (word & 0xf000) {
   case (0x0000) : lg_l0();
                   break;
   case (0x1000) : lg_l1();
                   break;
   case (0x2000) : lg_l1();
                   break;
   case (0x3000) : lg_l1();
                   break;
   case (0x4000) : lg_l4();
                   break;
   case (0x5000) : lg_l5();
                   break;
   case (0x6000) : code = Bcc;
                   conditions(word, costr);
                   val = (word & 0x00ff);
                   lval = lpc_get();

                   if (val == 0)
                     lpc_fetch(&val);
                   else
                       if (val & 0x0080)
                          val |= 0xff00;

                   lval += (long)val;
                   
                   if (*costr=='t')
                   {
                     code = BRA;
                     tag  = LBL;
                   }
                   else if (*costr=='f')
                   {
                     code = BSR;
                     tag  = SBR;
                   }
                   else
                   {
                     tag = LBL;
                   }

                   sym_tag(lval, tag);
                   break;
   case (0x7000) : lg_l7();
                   break;
   case (0x8000) : lg_l8();
                   break;
   case (0x9000) : lg_l9();
                   break;
   case (0xa000) : code = LINEA;  /* Line A Opcode */
                   break;
   case (0xb000) : lg_lb();
                   break;
   case (0xc000) : lg_lc();
                   break;
   case (0xd000) : lg_ld();
                   break;
   case (0xe000) : lg_le();
                   break;
   case (0xf000) : code = LINEF;  /* Line F Opcode */
                   break;
   }

   return(code);
}





lg_l0()
{
   int  srflag;

   srflag = 0;             /* flag to enable logical operations on status
                              register */

   switch (word & 0xff00) {
   case (0x0000) : code = ORI;
                   srflag = SRMODE;
                   break;
   case (0x0200) : code = ANDI;
                   srflag = SRMODE;
                   break;
   case (0x0400) : code = SUBI;
                   break;
   case (0x0600) : code = ADDI;
                   break;
   case (0x0800) : lg_l08();
                   return;
   case (0x0a00) : code = EORI;
                   srflag = SRMODE;
                   break;
   case (0x0c00) : code = CMPI;
                   break;
   default       : lg_l01();
                   return;
   }


   if (sizespec(word, NORMAL, size) == 2)
      lpc_lfetch(&lval);
   else
      lpc_fetch(&val);

   effad(word,(ADLIM|PCLIM|PCILIM|IMLIM|srflag));

}



lg_l08()
{

   switch (word & 0xffc0) {
   case (0x0800) : code = BTSTi;
                   break;
   case (0x0840) : code = BCHGi;
                   break;
   case (0x0880) : code = BCLRi;
                   break;
   case (0x08c0) : code = BSETi;
                   break;
   default       : lg_l1();
                   return;
   }

   lpc_fetch(&val);

   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));

}



lg_l03()
{

   switch (word & 0xf1c0) {
   case (0x0100) : code = BTSTr;
                   break;
   case (0x0140) : code = BCHGr;
                   break;
   case (0x0180) : code = BCLRr;
                   break;
   case (0x01c0) : code = BSETr;
                   break;
   default       : lg_l1();
                   return;
   }

   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));

}



lg_l01()
{
   switch (word & 0xf138) {
   case (0x0108) : code = MOVEP;
                   lpc_fetch(&val);
                   break;
   default       : lg_l03();
                   return;
   }

}



lg_l1()
{
   int siz;
   
   switch (word & 0xc1c0) {
   case (0x0040) : code = MOVEA;
                  switch(sizespec(word,HORDER,size))
                  {
                     case (0) : longjmp(err_buf,80);
                     case (1) : siz = 0;
                                break;
                     case (2) : siz = LMODE;
                                break;
                  }
 
                   effad(word, siz);
                   break;

   default       : lg_l2();
   }

}



lg_l2()
{
   int siz;

   switch (word & 0xc000) {
   case (0x0000) : code = MOVE;
                   if(sizespec(word, HORDER, size) == 2)
                      siz = LMODE;
                   else
                      siz = 0;
                   effad(word, (PCLIM|PCILIM|siz));
                   effad(word, (HORDER|ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



lg_l4()
{
   switch (word & 0xff00) {
   case (0x4000) : lg_l40();
                   break;
   case (0x4200) : code = CLR;
                   sizespec(word, NORMAL, size);
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   case (0x4400) : lg_l44();
                   break;
   case (0x4600) : lg_l46();
                   break;
   case (0x4800) : lg_l48();
                   break;
   case (0x4a00) : lg_l4a();
                   break;
   case (0x4c00) : lg_l4c();
                   break;
   case (0x4e00) : lg_l4e();
                   break;
   default       : lg_l41();
                   break;
   }

}



lg_l40()
{
   switch (word & 0xffc0) {
   case (0x40c0) : code = MOVEfs;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;

   default       : code = NEGX;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   }

}



lg_l44()
{
   switch (word & 0xffc0) {
   case (0x44c0) : code = MOVEc;
                   effad(word, ADLIM);
                   break;

   default       : code = NEG;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   }

}



lg_l46()
{
   switch (word & 0xffc0) {
   case (0x46c0) : code = MOVEts;
                   effad(word, ADLIM);
                   break;
   default       : code = NOT;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   }

}



lg_l48()
{
   switch (word & 0xfff8) {
   case (0x4840) : code = SWAP;
                   break;
   default       : lg_l49();
                   break;
   }

}



lg_l49()
{
   switch (word & 0xff38) {
   case (0x4800) : code = EXT;
                   switch (word&0x00c0) {
                     case (0x0080) : break;
                     case (0x00c0) : break;
                     default :       longjmp(err_buf, 14);
                   }
                   break;
   default       : lg_l510();
                   break;
   }

}



lg_l510()
{
   switch (word & 0xffc0) {
   case (0x4800) : code = NBCD;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   case (0x4840) : code = PEA;
                   effad(word, (DDLIM|ADLIM|PILIM|PDLIM|IMLIM));
                   break;
   default       : lg_l53();
                   break;
   }

}



lg_l53()
{
   int mask;

   switch (word & 0xff80) {
   case (0x4880) : code = MOVEMt;
                   lpc_fetch(&mask);
                   effad(word, (DDLIM|ADLIM|PILIM|PCLIM|PCILIM|IMLIM));
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



lg_l4a()
{
   switch (word & 0xffc0) {
   case (0x4ac0) : code = TAS;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   default       : code = TST;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   }

}



lg_l4c()
{
   int mask;

   switch (word & 0xff80) {
   case (0x4c80) : code = MOVEMf;
                   lpc_fetch(&mask);
                   effad(word, (DDLIM|ADLIM|PDLIM|IMLIM));
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



lg_l4e()
{

   switch (word & 0xfff0) {
   case (0x4e70) : lg_l4e7();
                   break;
   case (0x4e50) : lg_l4e5();
                   break;
   case (0x4e60) : code = MOVEu;
                   break;
   case (0x4e40) : code = TRAP;
                   lpc_fetch(&val);
                   break;
   default       : lg_l4e8();
                   break;
   }

}



lg_l4e7()
{

   switch (word & 0xffff) {
   case (0x4e70) : code = RESET;
                   break;
   case (0x4e71) : code = NOP;
                   break;
   case (0x4e72) : code = STOP;
                   break;
   case (0x4e73) : code = RTE;
                   break;
   case (0x4e75) : code = RTS;
                   break;
   case (0x4e76) : code = TRAPV;
                   break;
   case (0x4e77) : code = RTR;
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



lg_l4e5()
{
   switch (word & 0xfff8) {
   case (0x4e50) : code = LINK;
                   lpc_fetch(&val);
                   break;
   case (0x4e58) : code = UNLK;
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }


}



lg_l4e8()
{

   switch (word & 0xffc0) {
   case (0x4ec0) : code = JMP;
                   break;
   case (0x4e80) : code = JSR;
                   break;
   default       : code = 200;
                   longjmp(err_buf,200);
   }

   effad(word, (DDLIM|ADLIM|PILIM|PDLIM|IMLIM));

}



lg_l41()
{
   switch (word & 0xf1c0) {
   case (0x4180) : code = CHK;
                   effad(word, ADLIM);
                   break;
   case (0x41c0) : code = LEA;
                   effad(word, (DDLIM|ADLIM|PILIM|PDLIM|IMLIM));
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



lg_l5()
{
   int tag,num;

   switch (word & 0xf0f8) {
   case (0x50c8) : code = DBcc;
                   lpc_fetch(&val);
                   lval = lpc_get() + (long)val - 2L;

                   sym_tag(lval, LOP);
                   break;

   default       : lg_l51();
                   break;
   }

}


lg_l51()
{
   switch (word & 0xf0c0) {
   case (0x50c0) : code = Scc;
                   effad(word, (ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   default       : lg_l52();
                   break;
   }

}



lg_l52()
{

   switch (word & 0xf100) {
   case (0x5000) : code = ADDQ;
                   break;
   case (0x5100) : code = SUBQ;
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

   effad(word, (PCLIM|PCILIM|IMLIM));
}



lg_l7()
{
   switch (word & 0xf100) {
   case (0x7000) : code = MOVEQ;
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



lg_l8()
{
   switch (word & 0xf1f8) {
   case (0x8100) : code = SBCDr;
                   break;
   case (0x8108) : code = SBCDm;
                   break;
   default       : lg_l81();
                   return;
   }

}



lg_l81()
{
   int siz;


   switch (word & 0xf1c0) {
   case (0x81c0) : code = DIVS;
                   break;
   case (0x80c0) : code = DIVU;
                   break;
   default       : code = OR;
                   if (sizespec(word,NORMAL,size)==2)
                     siz = LMODE;
                   else
                     siz = 0;

                   if (word & 0x0100)
                     effad(word,(ADLIM|PCLIM|PCILIM|IMLIM));
                   else
                     effad(word, siz);
                   return;
   }

   effad(word, ADLIM);

}



lg_l9()
{

   switch (word & 0xf138) {
   case (0x9100) : code = SUBXr;
                   break;
   case (0x9108) : code = SUBXm;
                   break;
   default       : lg_l91();
                   return;
   }

}



lg_l91()
{
   int siz;

   switch (word & 0xf0c0) {
   case (0x90c0) : code = SUBA;
                   if(sizespec(word,SHORT,size)==2)
                      siz = LMODE;
                   else
                    siz = 0;
                   effad(word,siz);
                   break;
   default       : code = SUB;
                   if (sizespec(word,NORMAL,size)==2)
                      siz =LMODE;
                   else
                      siz = 0;

                   if (word & 0x0100)
                     effad(word,(ADLIM|PCLIM|PCILIM|IMLIM));
                   else
                     effad(word, siz);
                   break;
   }

}



lg_lb()
{
   int siz;

   switch (word & 0xf0c0) {
   case (0xb0c0) : code = CMPA;
                   if (sizespec(word,SHORT,size)==2)
                      siz = LMODE;
                   else
                      siz = 0;
                   effad(word,siz);
                   break;
   default       : lg_lb1();
                   break;
   }

}



lg_lb1()
{
   switch (word & 0xf138) {
   case (0xb108) : code = CMPM;
                   break;
   default       : lg_lb2();
                   break;
   }

}



lg_lb2()
{
   int siz;
   
   switch (word & 0xf100) {
   case (0xb000) : code = CMP;
                   if(sizespec(word,NORMAL,size)== 2)
                      siz = LMODE;
                   else
                      siz = 0;
                   effad(word,siz);
                   break;
   case (0xb100) : code = EOR;
                   effad(word,(ADLIM|PCLIM|PCILIM|IMLIM));
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}




lg_lc1()
{
   int siz;

   code = EXG;

   switch(word & 0x01f8) {
     case (0x0140) : break;
     case (0x0148) : break;
     case (0x0188) : break;
     default:        code = AND;

                     if (sizespec(word,NORMAL,size)== 2)
                        siz = LMODE;
                     else
                        siz = 0;

                     if (word & 0x0100)
                        effad(word,(ADLIM|PCLIM|PCILIM|IMLIM));
                     else
                        effad(word, siz);
                     return;
     }

}




lg_lc0()
{
   int siz;

   switch (word & 0xf1c0) {
   case (0xc0c0) : code = MULU;
                   break;
   case (0xc1c0) : code = MULS;
                   break;
   default       : lg_lc1();
                   return;
   }

   effad(word, ADLIM);
}




lg_lc()
{

   switch (word & 0xf1f8) {
   case (0xc100) : code = ABCDr;
                   break;
   case (0xc108) : code = ABCDm;
                   break;
   default       : lg_lc0();
                   return;
   }

}


lg_ld()
{
   int siz;
   
   switch (word & 0xf0c0) {
   case (0xd0c0) : code = ADDA;
                   if (sizespec(word,SHORT,size)== 2)
                      siz = LMODE;
                   else
                      siz = 0;
                   effad(word,siz);
                   break;
   default       : lg_ld1();
                   break;
   }

}




lg_ld1()
{
   int siz;

   switch (word & 0xf138) {
   case (0xd100) : code = ADDXr;
                   break;
   case (0xd108) : code = ADDXm;
                   break;
   default       : code = ADD;
                   if (sizespec(word,NORMAL,size)==2)
                        siz=LMODE;
                   else
                        siz = 0;

                   if (word & 0x0100)
                     effad(word,(ADLIM|PCLIM|PCILIM|IMLIM));
                   else
                     effad(word, siz);
                   return;
   }

}



lg_le()
{

   switch (word & 0xffc0) {
   case (0xe0c0) : code = ASRm;
                   break;
   case (0xe1c0) : code = ASLm;
                   break;
   case (0xe2c0) : code = LSRm;
                   break;
   case (0xe3c0) : code = LSLm;
                   break;
   case (0xe4c0) : code = ROXRm;
                   break;
   case (0xe5c0) : code = ROXLm;
                   break;
   case (0xe6c0) : code = RORm;
                   break;
   case (0xe7c0) : code = ROLm;
                   break;
   default       : lg_le2();
                   return;
   }

   effad(word, (DDLIM|ADLIM|PCLIM|PCILIM|IMLIM));
}




lg_le2()
{
   switch (word & 0xf118) {
   case (0xe100) : code = ASLr;
                   break;
   case (0xe000) : code = ASRr;
                   break;
   case (0xe108) : code = LSLr;
                   break;
   case (0xe008) : code = LSRr;
                   break;
   case (0xe118) : code = ROLr;
                   break;
   case (0xe018) : code = RORr;
                   break;
   case (0xe110) : code = ROXLr;
                   break;
   case (0xe010) : code = ROXRr;
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



/************/
/* effad    */
/************/

effad(word, flags)
int word;
int flags;
/* Extracts the effective address from an opcode without generating text.
May call "lpc_fetch" or "lpc_lfetch" and the
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
      printf("effad(0x%04x,0x%04x): Entering\n",word,flags);
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

      switch (mode) {
         case (0) :  /* data register direct mode */
                     if (flags & DDLIM)
                        longjmp(err_buf, 1);
                     break;

         case (1) :  /* address register direct mode */
                     if (flags & ADLIM)
                        longjmp(err_buf, 2);
                     break;

         case (2) :  /* address register indirect mode */
                     if (flags & AILIM)
                        longjmp(err_buf, 3);
                     break;

         case (3) :  /* postincrement mode */
                     if (flags & PILIM)
                        longjmp(err_buf, 4);
                     break;

         case (4) :  /* predecrement mode */
                     if (flags & PDLIM)
                        longjmp(err_buf, 5);
                     break;

         case (5) :  /* indirect with displacement */
                     if (flags & IDLIM)
                        longjmp(err_buf, 6);
                     lpc_fetch(&value);
                     break;

         case (6) :  /* index and displacement */
                     if (flags & IIDLIM)
                        longjmp(err_buf, 7);
                     lpc_fetch(&value);
                     break;

         case (7) :  /* additional modes */
                     switch (reg - '0') {
                       case (0) :   /* absolute short */
                                    if (flags & ASLIM)
                                       longjmp(err_buf, 8);
                                    lpc_fetch(&value);
                                    break;

                       case (1) :   /* absolute long */
                                    if (flags & ALLIM)
                                       longjmp(err_buf, 9);
                                    if (lpc_lfetch(&dvalue)!=ABSOLUTE)
                                      sym_tag(dvalue, classify(dvalue));
                                            
                                    break;

                       case (2) :   /* PC relative with displacement */
                                    if (flags & PCLIM)
                                       longjmp(err_buf, 10);
                                    lpc_fetch(&value);

                                    dvalue = lpc_get() + (long)value - 2L;

                                    sym_tag(dvalue, classify(dvalue));
                                    break;

                       case (3) :   /* PC relative with index        */
                                    if (flags & PCILIM)
                                       longjmp(err_buf, 11);
                                    lpc_fetch(&value);
                                    break;

                       case (4) :   /* immediate */
                                    if (!(flags & SRMODE))
                                    {
                                       if (flags & IMLIM)
                                          longjmp(err_buf, 12);

                                       if (flags & LMODE)
                                       {
                                          if (lpc_lfetch(&dvalue)!=ABSOLUTE)
                                             sym_tag(dvalue, classify(dvalue));
                                       }
                                       else
                                          lpc_fetch(&value);
                                    }
                                    break;

                       default  :   longjmp(err_buf, 13);
                                    break;
                     }

                     break;

     }
#ifdef DEBUG
   printf("effad(0x%04x,0x%04x): Leaving\n",word,flags);
#endif

}



/*****************/
/* classify()    */
/*****************/

int classify(dvalue)
long dvalue;
/* determine, what kind of symbol is at hand and return the appropriate tag */
{
   int tag;
   extern int code;

   if (dvalue < bss_start)
      if (dvalue < dat_start)
         if (code == JSR)
            return(SBR);
         else
            return(LBL);
      else
         return(DAT);
   else
      return(BSS);
}



