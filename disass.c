/********************************************************************/
/*                                                                  */
/* Module: DISASS main                                              */
/*                                                                  */
/* Author: Gernot Kunz                                              */
/*                                                                  */
/* Date:   1987/11/10                                               */
/*                                                                  */
/*                                                                  */
/* Disassembler project main module.                                */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/********************************************************************/

/* SPECIFICATION:

   main(argc, argv)                 Main program. Initializes modules and
   int argc;                        invokes the disassembly routines.
   char *argv[];


   long get_filsz(name)             Get the file size in bytes of a given
   char *name;                      file. The file must be known to exist.


   int get_header(infile, prog_header)    Read the GEMDOS program header from
   int infile;                            an executable file. Returns zero
   struct gem_header *prog_header;        on success.


   get_symtab(infile, sym_len)            Reads the symbol table (if any) into
   int infile;                            the SYMBOL data structure.
   long sym_len;


   dis_algo()                             Disassembly algorithm.

   IMPLEMENTATION:
*/

#include <osbind.h>
#include <lpc.h>
#include <opkind.h>
#include <symbol.h>
#include <daerror.h>
#include <stdio.h>

/* #define DEBUG */

struct gem_header {
   int  prg_id;
   long text_len;
   long data_len;
   long bss_len;
   long sym_len;
   long filler1;
   long filler2;
   int  filler3;
} ;


/* GLOBAL VARIABLES */

overlay "init!"

jmp_buf err_buf;     /* Error buffer LPC, OPCODE, TOOLKIT */

long dat_start;      /* LPC data segment start */
long bss_start;      /* LPC bss segment start */
long bss_end;        /* LPC bss segment end */

char adr[9];         /* address string */
char hex[21];        /* instruction hex code */
char lbl[10];        /* location label */
char opc[10];        /* opcode string */
char prm[30];        /* parameter string */
char cmt[30];        /* comment string */


/* FILE SPECIFIC VARIABLES */

overlay "main"

static FILE *ofile;



/**************/
/* main()     */
/**************/

main(argc, argv)
int argc;
char *argv[];
/* Disassembler main program. Takes a filename parameter, sets up the
   disassembly environment and disassembles the program file. */
{
   int   infile;     /* file handle of input file */
   long  relsz;      /* size of relocation information */
   long  relpos;     /* file pointer to relocation information */
   long  get_filsz();
   struct gem_header prog_header;

   if (argc < 2)
   {
      printf("Usage: disas <prg-file>\n");
      exit(1);
   }

   infile = Fopen(argv[1], 0);

   if (infile < 0)
   {
      printf("Cannot open file %s\n", argv[1]);
      exit(1);
   }

   if (get_header(infile, &prog_header) != 0)
   {
      printf("File %s is not a program file\n", argv[1]);
      exit(1);
   }

#ifdef DEBUG
      printf("text size = %ld\n",prog_header.text_len);
      printf("data size = %ld\n",prog_header.data_len);
      printf("symb size = %ld\n",prog_header.sym_len );
      printf("bss  size = %ld\n",prog_header.bss_len );
#endif

/* logical program counter addresses: */

   dat_start = prog_header.text_len;
   bss_start = dat_start + prog_header.data_len;
   bss_end   = bss_start + prog_header.bss_len;


#ifdef DEBUG
   printf("dat_start = 0x%08lx\n",dat_start);
   printf("bss_start = 0x%08lx\n",bss_start);
   printf("bss_end   = 0x%08lx\n",bss_end);
#endif

   relsz  = get_filsz(argv[1]);
   relpos = ( bss_start          /* symbol start in file */
             +prog_header.sym_len
             +sizeof(struct gem_header) );
   relsz -= relpos;

#ifdef DEBUG
      printf("relpos = %ld\nrelsz = %ld\n",relpos,relsz);
#endif

   get_symtab(infile, prog_header.sym_len);
   
   lpc_init(infile, relsz);
   
   dis_algo();
      
   Fclose(infile);
   
}



/***************/
/* get_filsz() */
/***************/

long get_filsz(name)
char *name;
/* Retrieves the file size in bytes of the file the name of which is
   passed in 'name'. The file must exist. */
{
   struct dta_buf {
      char reserved[22];
      int  time;
      int  date;
      long size;
      char nam[14];
      } dir_entry;
      

   Fsetdta(&dir_entry);
   Fsfirst(name, 0);


   return(dir_entry.size);
}   
      


/****************/
/* get_header() */
/****************/

int get_header(infile, prog_header)
int infile;
struct gem_header *prog_header;
/* Reads in the header of a program file with handle 'infile'. Returns zero
on success, nonzero otherwise. */
{
   long status;
   
   status = Fread(infile, (long)sizeof(struct gem_header), 
               prog_header);

   if ((status < (long)sizeof(struct gem_header)) ||
      (prog_header->prg_id != 0x601A))
   {
      return(-1);
   }

   return(0);
}



/*****************/
/* get_symtab()  */
/*****************/

#define BSSREL 0x0100
#define TXTREL 0x0200
#define DATREL 0x0400
#define EXTREF 0x0800
#define EQDREG 0x1000
#define GLOBAL 0x2000
#define EQUTED 0x4000
#define DEFNED 0x8000

get_symtab(infile, sym_len)
int infile;
long sym_len;
/* Read the symbol table from the program file 'infile' with known header
   'prog_header' into the internal data structure provided by module
   SYMBOL.
*/
{
   struct sym_entry {
      char name[8];
      int  type;
      long value;
      } entry_buf;
      
   char nm_buf[9];
   long status;
   int  i;
   long counter;
   
   status = Fseek( ( bss_start   /* in the file, symbol table start */
                    +(long)sizeof(struct gem_header) ), infile, 0);
               
   if (status < 0)
      err_print("get_symtab", FSK_ERR, ABORT);
   
   for(counter = 0L;
       counter < sym_len;
       counter += (long)sizeof(struct sym_entry) )
   {
      status = Fread(infile, 
                  (long) sizeof(struct sym_entry), 
                  &entry_buf);
                  
      if (status < (long)sizeof(struct sym_entry))
      {
         printf("get_symtab: File read error.\n");
         exit(1);
      }
      
      for (i = 0; (i < 8) && (entry_buf.name[i] != '\0'); i++)
         nm_buf[i] = entry_buf.name[i];
         
      nm_buf[i] = '\0';
      
      if (entry_buf.type & BSSREL)
      {
         sym_name( ( bss_start
                    +entry_buf.value      ), nm_buf);
      }
      else if (entry_buf.type & DATREL)
      {
         sym_name( ( dat_start
                    +entry_buf.value      ), nm_buf);
      }          
      else if (entry_buf.type & TXTREL)
      {
         sym_name(entry_buf.value, nm_buf);
      }
         
   }
   
}



/******************/
/* dis_algo()     */
/******************/

#define min(a, b)  (a<b ? a : b)

dis_algo()
{
   long lval, end, nxt;
   int ret, val;
   long ctr;
   int tag, num;
   int do_bss();
   extern char adr[],hex[],lbl[],opc[],prm[],cmt[];

   printf("Disassembler pass 1...\n");

   if (ret = setjmp(err_buf))
   {
      printf("dis_algo: Longjump error %d\n",ret);
      exit(1);
   }

/* Analyze text segment and keep labels. */

   blk_reset(0L,dat_start);

   lval = 0L; end = dat_start;

   initgrph(0L,dat_start);

   do {
     do {
          lpc_set(lval);


          do {
               *hex = '\0';
               ret = labgen();
             } while ( (lpc_get() < end) && (!STPINST(ret)) );


          blk_insert(lval, min(end, lpc_get()) );
          dgrph(lval, min(end, lpc_get()));

        }   while (!sym_lolab(&lval, &end));
      }   while (!blk_leblk(&lval, &end));

   printf("\n");

/* Analyze data segment and keep labels */

   lpc_set(dat_start);
   lpc_rset(dat_start, &nxt);

   while (lpc_get() < bss_start)
   {
      *hex = '\0';

      if (lpc_get() == nxt)
      {
         lpc_lfetch(&lval);
         sym_tag(lval, classify(lval));
         lpc_rnxt(&nxt);
      }
      else
         lpc_fetch(&val);
   }

   printf("Disassembler pass 2...\n");

/* Generate code for text segment */

   lpc_set(0L);
   ofile = fopen("DISAS.OUT", "w");

   fprintf(ofile, "%-40s.text\n", " ");

   while (lpc_get() < dat_start)
   {
      ret = disassemble();
      fprintf(ofile,"%-9s%-21s%-10s%-9s%s\n",adr,hex,lbl,opc,prm);
   }

   fprintf(ofile, "%-40s.data\n", " ");
   lpc_set(dat_start);
   lpc_rset(dat_start, &nxt);

   while (lpc_get() < bss_start)
   {
      find_label(lpc_get(), lbl);
      sprintf(adr, "%08lx", lpc_get());
      *hex = '\0';

      if (lpc_get() == nxt)
      {
         lpc_lfetch(&lval);
         sym_retrieve(lval,&tag,&num,prm);
         if (*prm == '\0')
            cons_symbol(tag,num,prm);
         strcpy(opc, ".dc.l");
         lpc_rnxt(&nxt);
      }
      else
      {
         lpc_fetch(&val);
         strcpy(opc, ".dc.w");
         sprintf(prm, "$%s", hex);
      }
      fprintf(ofile, "%-9s%-21s%-10s%-9s%s\n",adr,hex,lbl,opc,prm);
   }

   fprintf(ofile, "%-40s.bss\n", " ");

#ifdef DEBUG
   printf("Starting bss segment disassembly\n");
#endif

   init_bss();
   sym_for(do_bss);
   exit_bss();

   fclose(ofile);

   printf("Done.\n");
}



static char savlab[10];
static long savadr;

init_bss()
{
   *savlab = '\0';
   savadr = bss_start;
#ifdef DEBUG
   printf("init_bss() Leaving.\n");
#endif
}


do_bss(addr, tag, num, name)
long addr;
int tag;
int num;
char *name;
{
#ifdef DEBUG
   printf("do_bss(0x%08lx,0x%04x,%d,0x%08x) Entering\n",addr,tag,num,name);
#endif

   if (addr < bss_start)
   return;

   sprintf(adr, "%08lx", savadr);

   switch (addr - savadr) {
      case (0L) : break;
      case (2L) : fprintf(ofile,"%-9s%-21s%-10s.ds.w    1\n",adr," ",savlab);
                  break;
      case (4L) : fprintf(ofile,"%-9s%-21s%-10s.ds.l    1\n",adr," ",savlab);
                  break;
      default  :  fprintf(ofile,"%-9s%-21s%-10s.ds.b    %ld\n",adr," ",savlab,addr - savadr);
                  break;
      }

      savadr = addr;
      if (name == NULL)
        cons_symbol(tag, num, savlab);
      else
        strcpy(savlab, name);

      concat(savlab, ":");
#ifdef DEBUG
   printf("do_bss: Leaving. savlab = %s, savadr = 0x%08lx\n",savlab,savadr);
#endif
}


exit_bss()
{
#ifdef DEBUG
   printf("exit_bss() Entering.\n");
#endif
   sprintf(adr, "%08lx", savadr);

   switch (bss_end - savadr) {
      case (0L) : break;
      case (2L) : fprintf(ofile,"%-9s%-21s%-10s.ds.w    1\n",adr," ",savlab);
                  break;
      case (4L) : fprintf(ofile,"%-9s%-21s%-10s.ds.l    1\n",adr," ",savlab);
                  break;
      default  :  fprintf(ofile,"%-9s%-21s%-10s.ds.b    %ld\n",adr," ",savlab,bss_end - savadr);
                  break;
      }
#ifdef DEBUG
   printf("exit_bss() Leaving.\n");
#endif
}


static char display[72];
static long aus;

initgrph(start, stop)
long start;
long stop;
{
   int i;

   for (i = 0; i < 70; i++)
      display[i] = '_';

   display[70] = 13;
   display[71] = '\0';
   aus = stop;
}


dgrph(start, stop)
long start;
long stop;
{
   int i;

   for (i = ((70 * start) / aus); i < ((70 * stop) / aus); i++)
      display[i] = '@';

   Cconws(display);
}


err_print(module, number, action)
char *module;
int number;
int action;
{
   char *text;

   switch (number) {
      case (30) :  text = "Out of memory.";
            break;
      case (31) :  text = "File seek error.";
            break;
      case (32) :  text = "Reloc table too big.";
            break;
      case (33) :  text = "File read error.";
            break;
      default: text = "Error #   .";
               sprintf((text+7),"%03d", number);
               break;
      }

   printf("%s: %s\n", module, text);

   if (action == ABORT)
      exit(1);
}
