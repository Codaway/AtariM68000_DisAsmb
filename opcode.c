/**********************************************************************/
/*                                                                    */
/*     MODULE opcode finder                                           */
/*                                                                    */
/*     Classifies a word to find the M68000 opcode associated with it */
/*                                                                    */
/*     AUTHOR: Gernot Kunz                                            */
/*                                                                    */
/*     DATE:   1986/10/29                                             */
/*                                                                    */
/*                                                                    */
/**********************************************************************/


#include <toolkit.h>
#include <lpc.h>
#include <opkind.h>
#include <symbol.h>
#include <stdio.h>

/* #define DEBUG */

#define HREG (((word&0x0e00)>>9)+'0')  /* high position register */
#define LREG ((word&0x0007)+'0')       /* low position  register */

/* EXTERNALS */

extern jmp_buf err_buf;   /* global error return (TOOLKIT, LPC) */

extern char adr[];
extern char hex[];
extern char lbl[];
extern char opc[];
extern char prm[];
extern char cmt[];

/* GLOBALS: */

int code;                 /* opcode index (export to toolkit) */


/* FILE SPECIFIC VARIABLES: */

static unsigned int word; /* opcode word */

static char size[3];      /* size mnemonics string */
static char eaddr[40];    /* effective address string */
static int  val;          /* additional value */
static long lval;         /* additional long value */


/*********************/
/*  disassemble      */
/*********************/

int disassemble()
/* disassemble the instruction at the current logical program counter
   and return the text. Returns the opcode kind on OK , else a negative
   error code is returned. */
{
   char costr[3];
   int tag, num;
   long sav_lpc;

#ifdef DEBUG
   printf("disassemble() Entering\n");
#endif

   *opc = *prm = *cmt = *lbl = *hex = '\0';

   if (val = setjmp(err_buf))
   {
         lpc_set(sav_lpc);
         strcpy(opc, ".dc.w");
         sprintf(prm, "$%04x", word);
         sprintf(hex, "%04x", word);

         return((-val));
   }

   sprintf(adr, "%08lx", lpc_get());

   find_label(lpc_get(),lbl);

   lpc_fetch(&word);
   sav_lpc = lpc_get();

   switch (word & 0xf000) {
   case (0x0000) : op_l0();
                   break;
   case (0x1000) : op_l1();
                   break;
   case (0x2000) : op_l1();
                   break;
   case (0x3000) : op_l1();
                   break;
   case (0x4000) : op_l4();
                   break;
   case (0x5000) : op_l5();
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
                     strcpy(opc, "bra");
                     code = BRA;
                     tag  = LBL;
                   }
                   else if (*costr=='f')
                   {
                     strcpy(opc, "bsr");
                     code = BSR;
                     tag  = SBR;
                   }
                   else
                   {
                     sprintf(opc, "b%s", costr);
                     tag = LBL;
                   }

                /* sym_tag(lval, tag);   */
                   sym_retrieve(lval,&tag,&num,prm);
                      if (*prm == '\0')
                         cons_symbol(tag,num,prm);
                   break;
   case (0x7000) : op_l7();
                   break;
   case (0x8000) : op_l8();
                   break;
   case (0x9000) : op_l9();
                   break;
   case (0xa000) : code = LINEA;  /* Line A Opcode */
                   sprintf(opc, ".dc.w");
                   sprintf(prm, "$%x", word);
                   strcpy(cmt, "* Line A Opcode");
                   break;
   case (0xb000) : op_lb();
                   break;
   case (0xc000) : op_lc();
                   break;
   case (0xd000) : op_ld();
                   break;
   case (0xe000) : op_le();
                   break;
   case (0xf000) : code = LINEF;  /* Line F Opcode */
                   sprintf(opc, ".dc.w");
                   sprintf(prm, "$%x", word);
                   strcpy(cmt, "* Line F Opcode");
                   break;
   }


   return(code);
}





op_l0()
{
   char *text;
   int  srflag;

   srflag = 0;             /* flag to enable logical operations on status
                              register */

   switch (word & 0xff00) {
   case (0x0000) : code = ORI;
                   text = "ori";
                   srflag = SRMODE;
                   break;
   case (0x0200) : code = ANDI;
                   text = "andi";
                   srflag = SRMODE;
                   break;
   case (0x0400) : code = SUBI;
                   text = "subi";
                   break;
   case (0x0600) : code = ADDI;
                   text = "addi";
                   break;
   case (0x0800) : op_l08();
                   return;
   case (0x0a00) : code = EORI;
                   text = "eori";
                   srflag = SRMODE;
                   break;
   case (0x0c00) : code = CMPI;
                   text = "cmpi";
                   break;
   default       : op_l01();
                   return;
   }


   if (sizespec(word, NORMAL, size) == 2)
   {
      lpc_lfetch(&lval);
      eea(word,(ADLIM|PCLIM|PCILIM|IMLIM|srflag),eaddr);
      sprintf(prm, "#$%lx,%s", lval, eaddr);
   }
   else
   {
      lpc_fetch(&val);
      eea(word,(ADLIM|PCLIM|PCILIM|IMLIM|srflag),eaddr);
      sprintf(prm, "#$%x,%s", val, eaddr);
   }

   sprintf(opc, "%s%s", text, size);

}



op_l08()
{
   char *text;

   switch (word & 0xffc0) {
   case (0x0800) : code = BTSTi;
                   text = "btst";
                   break;
   case (0x0840) : code = BCHGi;
                   text = "bchg";
                   break;
   case (0x0880) : code = BCLRi;
                   text = "bclr";
                   break;
   case (0x08c0) : code = BSETi;
                   text = "bset";
                   break;
   default       : op_l1();
                   return;
   }

   lpc_fetch(&val);

   strcpy(opc, text);

   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), eaddr);

   sprintf(prm, "#%d,%s", val,eaddr);

}



op_l03()
{
   char *text;

   switch (word & 0xf1c0) {
   case (0x0100) : code = BTSTr;
                   text = "btst";
                   break;
   case (0x0140) : code = BCHGr;
                   text = "bchg";
                   break;
   case (0x0180) : code = BCLRr;
                   text = "bclr";
                   break;
   case (0x01c0) : code = BSETr;
                   text = "bset";
                   break;
   default       : op_l1();
                   return;
   }

   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), eaddr);

   strcpy(opc, text);
   sprintf(prm, "d%c,%s", HREG, eaddr);

}



op_l01()
{
   int addr;
   int data;

   switch (word & 0xf138) {
   case (0x0108) : code = MOVEP;
                   addr = LREG;
                   data = HREG;
                   lpc_fetch(&val);
                   strcpy(opc, "movep");
                   if (word & 0x0040)
                     concat(opc, ".l");

                   if (word & 0x0080)
                     sprintf(prm, "d%c,$%x(a%c)",data,val,addr);
                   else
                     sprintf(prm, "$%x(a%c),d%c",val,addr,data);
                   break;
   default       : op_l03();
                   return;
   }

}



op_l1()
{
   int addr;
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
 
                   addr = HREG;
                   eea(word, siz, eaddr);

                   sprintf(opc, "movea%s", size);
                   sprintf(prm, "%s,a%c", eaddr, addr);
                   break;

   default       : op_l2();
   }

}



op_l2()
{
   int siz;

   switch (word & 0xc000) {
   case (0x0000) : code = MOVE;
                   if(sizespec(word, HORDER, size) == 2)
                      siz = LMODE;
                   else
                      siz = 0;
                   eea(word, (PCLIM|PCILIM|siz), eaddr);
                   sprintf(opc, "move%s", size);
                   sprintf(prm, "%s,", eaddr);
                   eea(word, (HORDER|ADLIM|PCLIM|PCILIM|IMLIM), eaddr);
                   concat(prm, eaddr);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



op_l4()
{
   switch (word & 0xff00) {
   case (0x4000) : op_l40();
                   break;
   case (0x4200) : code = CLR;
                   sizespec(word, NORMAL, size);
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM),prm);
                   sprintf(opc, "clr%s", size);
                   break;
   case (0x4400) : op_l44();
                   break;
   case (0x4600) : op_l46();
                   break;
   case (0x4800) : op_l48();
                   break;
   case (0x4a00) : op_l4a();
                   break;
   case (0x4c00) : op_l4c();
                   break;
   case (0x4e00) : op_l4e();
                   break;
   default       : op_l41();
                   break;
   }

}



op_l40()
{
   switch (word & 0xffc0) {
   case (0x40c0) : code = MOVEfs;
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), eaddr);
                   strcpy(opc, "move");
                   sprintf(prm, "sr,%s", eaddr);
                   break;

   default       : code = NEGX;
                   sizespec(word, NORMAL, size);

                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);

                   sprintf(opc, "negx%s", size);
                   break;
   }

}



op_l44()
{
   switch (word & 0xffc0) {
   case (0x44c0) : code = MOVEc;
                   eea(word, ADLIM , eaddr);

                   strcpy(opc, "move");
                   sprintf(prm, "%s,ccr", eaddr);
                   break;

   default       : code = NEG;
                   sizespec(word, NORMAL, size);
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);
                   sprintf(opc, "neg%s", size);
                   break;
   }

}



op_l46()
{
   switch (word & 0xffc0) {
   case (0x46c0) : code = MOVEts;
                   eea(word, ADLIM, eaddr);
                   strcpy(opc, "move");
                   sprintf(prm, "%s,sr", eaddr);
                   break;
   default       : code = NOT;
                   sizespec(word, NORMAL, size);
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);
                   sprintf(opc, "not%s", size);
                   break;
   }

}



op_l48()
{
   switch (word & 0xfff8) {
   case (0x4840) : code = SWAP;
                   strcpy(opc, "swap");
                   sprintf(prm, "d%c", LREG);
                   break;
   default       : op_l49();
                   break;
   }

}



op_l49()
{
   switch (word & 0xff38) {
   case (0x4800) : code = EXT;
                   switch (word&0x00c0) {
                     case (0x0080) : strcpy(opc, "ext");
                                     break;
                     case (0x00c0) : strcpy(opc, "ext.l");
                                     break;
                     default :       longjmp(err_buf, 14);
                   }
                   sprintf(prm, "d%c", LREG);
                   break;
   default       : op_l510();
                   break;
   }

}



op_l510()
{
   switch (word & 0xffc0) {
   case (0x4800) : code = NBCD;
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);
                   strcpy(opc, "nbcd");
                   break;
   case (0x4840) : code = PEA;
                   eea(word, (DDLIM|ADLIM|PILIM|PDLIM|IMLIM), prm);
                   strcpy(opc, "pea");
                   break;
   default       : op_l53();
                   break;
   }

}



op_l53()
{
   int mask;

   switch (word & 0xff80) {
   case (0x4880) : code = MOVEMt;
                   lpc_fetch(&mask);
                   eea(word, (DDLIM|ADLIM|PILIM|PCLIM|PCILIM|IMLIM), eaddr);
                   if ((word & 0x0038) == 0x0020) /* predecrement mode */
                     reglist(mask, 1, prm);
                   else
                     reglist(mask, 0, prm);

                   strcpy(opc, "movem");
                   concat(prm, ",");
                   concat(prm, eaddr);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



op_l4a()
{
   switch (word & 0xffc0) {
   case (0x4ac0) : code = TAS;
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);
                   strcpy(opc, "tas");
                   break;
   default       : code = TST;
                   sizespec(word, NORMAL, size);
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);
                   sprintf(opc, "tst%s", size);
                   break;
   }

}



op_l4c()
{
   int mask;

   switch (word & 0xff80) {
   case (0x4c80) : code = MOVEMf;
                   lpc_fetch(&mask);
                   eea(word, (DDLIM|ADLIM|PDLIM|IMLIM), prm);

                   reglist(mask, 0, eaddr);

                   strcpy(opc, "movem");
                   concat(prm, ",");
                   concat(prm, eaddr);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



op_l4e()
{
   int num;

   switch (word & 0xfff0) {
   case (0x4e70) : op_l4e7();
                   break;
   case (0x4e50) : op_l4e5();
                   break;
   case (0x4e60) : code = MOVEu;
                   strcpy(opc, "move");

                   num = LREG;

                   if (word & 0x0008)
                     sprintf(prm, "usp,a%c", num);
                   else
                     sprintf(prm, "a%c,usp", num);
                   break;
   case (0x4e40) : code = TRAP;
                   strcpy(opc, "trap");
                   sprintf(prm,"#$%x", (word & 0x000f) );
                   break;
   default       : op_l4e8();
                   break;
   }

}



op_l4e7()
{
   char *text;

   switch (word & 0xffff) {
   case (0x4e70) : code = RESET;
                   text = "reset";
                   break;
   case (0x4e71) : code = NOP;
                   text = "nop";
                   break;
   case (0x4e72) : code = STOP;
                   text = "stop";
                   break;
   case (0x4e73) : code = RTE;
                   text = "rte";
                   break;
   case (0x4e75) : code = RTS;
                   text = "rts";
                   break;
   case (0x4e76) : code = TRAPV;
                   text = "trapv";
                   break;
   case (0x4e77) : code = RTR;
                   text = "rtr";
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

   strcpy(opc, text);

}



op_l4e5()
{
   switch (word & 0xfff8) {
   case (0x4e50) : code = LINK;
                   strcpy(opc, "link");
                   lpc_fetch(&val);
                   sprintf(prm, "a%c,#$%x", LREG, val);
                   break;
   case (0x4e58) : code = UNLK;
                   strcpy(opc, "unlk");
                   sprintf(prm, "a%c", LREG);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }


}



op_l4e8()
{
   char *text;

   switch (word & 0xffc0) {
   case (0x4ec0) : code = JMP;
                   text = "jmp";
                   break;
   case (0x4e80) : code = JSR;
                   text = "jsr";
                   break;
   default       : code = 200;
                   longjmp(err_buf,200);
   }

   eea(word, (DDLIM|ADLIM|PILIM|PDLIM|IMLIM), prm);

   strcpy(opc, text);

}



op_l41()
{
   switch (word & 0xf1c0) {
   case (0x4180) : code = CHK;
                   eea(word, ADLIM, eaddr);
                   strcpy(opc, "chk");
                   sprintf(prm, "%s,d%c", eaddr, HREG);
                   break;
   case (0x41c0) : code = LEA;
                   eea(word, (DDLIM|ADLIM|PILIM|PDLIM|IMLIM), eaddr);
                   strcpy(opc, "lea");
                   sprintf(prm, "%s,a%c", eaddr, HREG);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



op_l5()
{
   int tag,num;

   switch (word & 0xf0f8) {
   case (0x50c8) : code = DBcc;
                   conditions(word, eaddr);
                   sprintf(opc, "db%s", eaddr);
                   lpc_fetch(&val);
                   lval = lpc_get() + (long)val - 2L;

                   /* sym_tag(lval, LOP); */
                   sym_retrieve(lval,&tag,&num,eaddr);

                      if (*eaddr == '\0')
                         cons_symbol(tag,num,eaddr);

                   sprintf(prm,"d%c,%s",LREG,eaddr);

                   break;

   default       : op_l51();
                   break;
   }

}


op_l51()
{
   switch (word & 0xf0c0) {
   case (0x50c0) : code = Scc;
                   conditions(word, eaddr);
                   sprintf(opc, "s%s", eaddr);
                   eea(word, (ADLIM|PCLIM|PCILIM|IMLIM), prm);
                   break;
   default       : op_l52();
                   break;
   }

}



op_l52()
{
   char *text;
   int data;

   switch (word & 0xf100) {
   case (0x5000) : code = ADDQ;
                   text = "addq";
                   break;
   case (0x5100) : code = SUBQ;
                   text = "subq";
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

   sizespec(word, NORMAL, size);
   sprintf(opc, "%s%s", text, size);

   eea(word, (PCLIM|PCILIM|IMLIM), eaddr);

   data = ((word&0x0e00)>>9);
   if (data == 0) data = 8;

   sprintf(prm, "#%d,%s", data, eaddr);

}



op_l7()
{
   switch (word & 0xf100) {
   case (0x7000) : code = MOVEQ;
                   strcpy(opc, "moveq");
                   sprintf(prm, "#$%x,d%c",(word & 0x00ff),HREG);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}



op_l8()
{
   int src, des;

   src =  LREG;
   des =  HREG;


   switch (word & 0xf1f8) {
   case (0x8100) : code = SBCDr;
                   sprintf(prm, "d%c,d%c", src, des);
                   break;
   case (0x8108) : code = SBCDm;
                   sprintf(prm, "(a%c),(a%c)", src, des);
                   break;
   default       : op_l81();
                   return;
   }

   strcpy(opc, "sbcd");

}



op_l81()
{
   char *text;
   int siz;
   int hreg;

   hreg = HREG;

   switch (word & 0xf1c0) {
   case (0x81c0) : code = DIVS;
                   text = "divs";
                   break;
   case (0x80c0) : code = DIVU;
                   text = "divu";
                   break;
   default       : code = OR;
                   if (sizespec(word,NORMAL,size)==2)
                     siz = LMODE;
                   else
                     siz = 0;
                   sprintf(opc,"or%s",size);

                   if (word & 0x0100)
                   {
                     eea(word,(ADLIM|PCLIM|PCILIM|IMLIM),eaddr);
                     sprintf(prm,"d%c,%s",hreg,eaddr);
                   }
                   else
                   {
                     eea(word, siz, eaddr);
                     sprintf(prm,"%s,d%c",eaddr,hreg);
                   }
                   return;
   }

   strcpy(opc, text);
   eea(word, ADLIM, eaddr);
   sprintf(prm, "%s,d%c", eaddr, hreg);

}



op_l9()
{
   int src, des;

   src =  LREG;
   des =  HREG;


   switch (word & 0xf138) {
   case (0x9100) : code = SUBXr;
                   sprintf(prm, "d%c,d%c", src, des);
                   break;
   case (0x9108) : code = SUBXm;
                   sprintf(prm, "(a%c),(a%c)", src, des);
                   break;
   default       : op_l91();
                   return;
   }

   strcpy(opc, "subx");

}



op_l91()
{
   int siz;

   switch (word & 0xf0c0) {
   case (0x90c0) : code = SUBA;
                   if(sizespec(word,SHORT,size)==2)
                      siz = LMODE;
                   else
                    siz = 0;
                   sprintf(opc,"suba%s",size);
                   eea(word,siz,eaddr);
                   sprintf(prm,"%s,a%c", eaddr, HREG);
                   break;
   default       : code = SUB;
                   if (sizespec(word,NORMAL,size)==2)
                      siz =LMODE;
                   else
                      siz = 0;
                   sprintf(opc,"sub%s",size);

                   if (word & 0x0100)
                   {
                     eea(word,(ADLIM|PCLIM|PCILIM|IMLIM),eaddr);
                     sprintf(prm,"d%c,%s",LREG,eaddr);
                   }
                   else
                   {
                     eea(word, siz, eaddr);
                     sprintf(prm,"%s,d%c",eaddr,LREG);
                   }
                   break;
   }

}



op_lb()
{
   int siz;

   switch (word & 0xf0c0) {
   case (0xb0c0) : code = CMPA;
                   if (sizespec(word,SHORT,size)==2)
                      siz = LMODE;
                   else
                      siz = 0;
                   sprintf(opc,"cmpa%s",size);
                   eea(word,siz,eaddr);
                   sprintf(prm,"%s,a%c", eaddr, HREG);
                   break;
   default       : op_lb1();
                   break;
   }

}



op_lb1()
{
   switch (word & 0xf138) {
   case (0xb108) : code = CMPM;
                   sizespec(word,NORMAL,size);
                   sprintf(opc,"cmpm%s", size);
                   sprintf(prm,"(a%c)+,(a%c)+", LREG,HREG);
                   break;
   default       : op_lb2();
                   break;
   }

}



op_lb2()
{
   int siz;
   
   switch (word & 0xf100) {
   case (0xb000) : code = CMP;
                   if(sizespec(word,NORMAL,size)== 2)
                      siz = LMODE;
                   else
                      siz = 0;
                   sprintf(opc,"cmp%s",size);
                   eea(word,siz,eaddr);
                   sprintf(prm,"%s,d%c",eaddr,HREG);
                   break;
   case (0xb100) : code = EOR;
                   sizespec(word,NORMAL,size);
                   sprintf(opc,"eor%s",size);
                   eea(word,(ADLIM|PCLIM|PCILIM|IMLIM),eaddr);
                   sprintf(prm,"d%c,%s",HREG, eaddr);
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

}




op_lc1()
{
   int src, des, siz;

   code = EXG;
   strcpy(opc, "exg");
   src =  LREG;
   des =  HREG;

   switch(word & 0x01f8) {
     case (0x0140) : sprintf(prm,"d%c,d%c",src,des);
                     break;
     case (0x0148) : sprintf(prm,"a%c,a%c",src,des);
                     break;
     case (0x0188) : sprintf(prm,"d%c,a%c",src,des);
                     break;
     default:        code = AND;

                     if (sizespec(word,NORMAL,size)== 2)
                        siz = LMODE;
                     else
                        siz = 0;
                     sprintf(opc,"and%s",size);

                     if (word & 0x0100)
                     {
                        eea(word,(ADLIM|PCLIM|PCILIM|IMLIM),eaddr);
                        sprintf(prm,"d%c,%s",des,eaddr);
                     }
                     else
                     {
                        eea(word, siz, eaddr);
                        sprintf(prm,"%s,d%c",eaddr,des);
                     }
                     return;
     }

}




op_lc0()
{
   char *text;
   int siz;

   switch (word & 0xf1c0) {
   case (0xc0c0) : code = MULU;
                   text = "mulu";
                   break;
   case (0xc1c0) : code = MULS;
                   text = "muls";
                   break;
   default       : op_lc1();
                   return;
   }

   strcpy(opc, text);
   eea(word, ADLIM, eaddr);
   sprintf(prm, "%s,d%c", eaddr, HREG);

}




op_lc()
{
   int src, des;

   src =  LREG;
   des =  HREG;


   switch (word & 0xf1f8) {
   case (0xc100) : code = ABCDr;
                   sprintf(prm, "d%c,d%c", src, des);
                   break;
   case (0xc108) : code = ABCDm;
                   sprintf(prm, "(a%c),(a%c)", src, des);
                   break;
   default       : op_lc0();
                   return;
   }

   strcpy(opc, "abcd");

}


op_ld()
{
   int siz;
   
   switch (word & 0xf0c0) {
   case (0xd0c0) : code = ADDA;
                   if (sizespec(word,SHORT,size)== 2)
                      siz = LMODE;
                   else
                      siz = 0;
                   sprintf(opc,"adda%s",size);
                   eea(word,siz,eaddr);
                   sprintf(prm,"%s,a%c", eaddr, HREG);
                   break;
   default       : op_ld1();
                   break;
   }

}




op_ld1()
{
   int src, des, siz;

   src =  LREG;
   des =  HREG;


   switch (word & 0xf138) {
   case (0xd100) : code = ADDXr;
                   sprintf(prm, "d%c,d%c", src, des);
                   break;
   case (0xd108) : code = ADDXm;
                   sprintf(prm, "(a%c),(a%c)", src, des);
                   break;
   default       : code = ADD;
                   if (sizespec(word,NORMAL,size)==2)
                        siz=LMODE;
                   else
                        siz = 0;
                   sprintf(opc,"add%s",size);

                   if (word & 0x0100)
                   {
                     eea(word,(ADLIM|PCLIM|PCILIM|IMLIM),eaddr);
                     sprintf(prm,"d%c,%s",des,eaddr);
                   }
                   else
                   {
                     eea(word, siz, eaddr);
                     sprintf(prm,"%s,d%c",eaddr,des);
                   }
                   return;
   }

   strcpy(opc, "addx");

}



op_le()
{
   char *text;

   switch (word & 0xffc0) {
   case (0xe0c0) : code = ASRm;
                   text = "asr";
                   break;
   case (0xe1c0) : code = ASLm;
                   text = "asl";
                   break;
   case (0xe2c0) : code = LSRm;
                   text = "lsr";
                   break;
   case (0xe3c0) : code = LSLm;
                   text = "lsl";
                   break;
   case (0xe4c0) : code = ROXRm;
                   text = "roxr";
                   break;
   case (0xe5c0) : code = ROXLm;
                   text = "roxl";
                   break;
   case (0xe6c0) : code = RORm;
                   text = "ror";
                   break;
   case (0xe7c0) : code = ROLm;
                   text = "rol";
                   break;
   default       : op_le2();
                   return;
   }

   eea(word, (DDLIM|ADLIM|PCLIM|PCILIM|IMLIM), prm);

   strcpy(opc, text);

}




op_le2()
{
   char *text;
   int scnt, data;

   switch (word & 0xf118) {
   case (0xe100) : code = ASLr;
                   text = "asl";
                   break;
   case (0xe000) : code = ASRr;
                   text = "asr";
                   break;
   case (0xe108) : code = LSLr;
                   text = "lsl";
                   break;
   case (0xe008) : code = LSRr;
                   text = "lsr";
                   break;
   case (0xe118) : code = ROLr;
                   text = "rol";
                   break;
   case (0xe018) : code = RORr;
                   text = "ror";
                   break;
   case (0xe110) : code = ROXLr;
                   text = "roxl";
                   break;
   case (0xe010) : code = ROXRr;
                   text = "roxr";
                   break;
   default       : code = 200;
                   longjmp(err_buf, 200);
   }

   sizespec(word, NORMAL, size);

   sprintf(opc, "%s%s", text, size);

   data = LREG;
   scnt = HREG;

   if (word & 0x0020)
      sprintf(prm, "d%c,d%c", scnt, data);
   else
   {
      if (scnt == '0')
         scnt = '8';

      sprintf(prm, "#%c,d%c", scnt, data);
   }
}
