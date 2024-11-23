/*************************************************************************/
/*    Datei: DISMAIN.C                                                   */
/*************************************************************************/
/*                                                                       */
/*  DDDD      II    SSSSS      AA     SSSSS                              */
/*  DD DD     II   SS         A  A   SS                                  */
/*  DD  DD    II     SS      AAAAAA    SS                                */
/*  DD DD     II       SS    AA  AA      SS                              */
/*  DDDD      II   SSSSS     AA  AA  SSSSS                               */
/*                                                                       */
/*************************************************************************/
/*                                                                       */
/*     Autor: Gernot Kunz                                                */
/*     Datum: 11/1986                                                    */
/*     Version: 1.1                                                      */
/*                                                                       */
/*                                                                       */
/*************************************************************************/     




/*-----------------------*/
/*    includes           */
/*-----------------------*/

#include <obdefs.h>
#include <gembind.h>
#include <gemdefs.h>
#include <define.h>
#include <disas.h>
#include <osbind.h>
#include <lpc.h>
#include <opkind.h>
#include <symbol.h>
#include <daerror.h>
#include <stdio.h>


/*-----------------------*/
/*     defines           */
/*-----------------------*/

#define BUSYBEE 2


/*******************************/
/*******************************/
/**                           **/
/**    Data Structures        **/
/**                           **/
/*******************************/
/*******************************/


/*--------------------------*/
/*     Global               */
/*--------------------------*/

int   contrl[12];   /* control inputs      */
int   intin[128];   /* max string length      */
int   ptsin[256];   /* polygon fill points      */
int   intout[128];   /* open workstation output   */
int   ptsout[128];

int   gl_wchar;      /* character width      */
int   gl_hchar;      /* character height      */
int   gl_wbox;       /* box (cell) width      */
int   gl_hbox;       /* box (cell) height      */
int   gem_handle;    /* GEM vdi handle      */
int   vdi_handle;    /* vdi handle         */
int   work_out[57];  /* open virt. workstation values*/
GRECT   work_area;   /* current window work area   */
GRECT   screen_area; /* screen area         */
int   gl_apid;       /* application id      */
int   gl_rmsg[8];    /* message buffer      */
int   gl_xfull;      /* full window 'x'      */
int   gl_yfull;      /* full window 'y'      */
int   gl_wfull;      /* full window 'w'      */
int   gl_hfull;      /* full window 'h'      */
int   ev_which;      /* event returned value      */
int   type_size;     /* system font cell size   */

int m_out;
int mousex;
int mousekey;
int bstate;
int kstate;
int kreturn;
int bclicks;




/*----------------------*/
/* Local to module      */
/*----------------------*/

OBJECT  *menu;        /* address of menu */
OBJECT  *logo_ad;       /* address of info box */
OBJECT  *analyz_ad;      /* address of analyze box */
OBJECT  *print_ad;       /* address of print box */
OBJECT  *fprint_ad;      /* address of file print box */


char curfil[30];      /* Name of current file */


/*******************************/
/*******************************/
/*                             */
/*     Main Program            */
/*                             */
/*******************************/
/*******************************/

main()
{
   int term_type;      /* zero on OK */

   term_type = disas_init();

   if (term_type == 0 )
      dispatch();

   disas_term(term_type);
}


/*******************************/
/*******************************/
/*                             */
/*     Initialization          */
/*                             */
/*******************************/
/*******************************/

int disas_init()
{
   int work_in[11];
   int i;

   gl_apid = appl_init();
   if (gl_apid == -1)
      return(4);

   graf_mouse(BUSYBEE, 0x0L);

   if (!rsrc_load( "DISAS.RSC" ) ) {
      graf_mouse(ARROW, 0x0L);
      form_alert(1,
   "[3][Panik!!!|Datei DISAS.RSC nicht gefunden.][ ABBRUCH ]");
      return(1);
   }

   /* Open virtual workstation */

   for (i = 0; i < 10; i++) {
      work_in[i] = 1;
   }

   work_in[10] = 2;

   gem_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
   vdi_handle = gem_handle;
   v_opnvwk(work_in, &vdi_handle, work_out);

   if (vdi_handle == 0)
      return(1);

   screen_area.g_w = work_out[0] + 1;
   screen_area.g_h = work_out[1] + 1;
   screen_area.g_x = screen_area.g_y = 0;

   /* Variablen initialisieren */
   var_init();

   /* Display menue */
   mmen_init();
   menu_bar(menu, TRUE);

   graf_mouse(ARROW, 0x0L);
   return(0);
}


/*------------------*/
/*  mmen_ init      */
/*------------------*/

mmen_init()         /* initialize menu item status */
{
   menu_ienable(menu, M_SELECT, 1);
   menu_ienable(menu, M_SAVE, 0);
 
}


/*----------------*/
/*    var_init    */
/*----------------*/


var_init()
{         /* initialize variables */

   *curfil = '\0';

/* Haupt - Objektbaeume holen */
   rsrc_gaddr(R_TREE, MENU, &menu);
   rsrc_gaddr(R_TREE, LOGO, &logo_ad);
   rsrc_gaddr(R_TREE, D_PRINT, &print_ad);
   rsrc_gaddr(R_TREE, D_FPRT, &fprint_ad);
   rsrc_gaddr(R_TREE, D_ANALYZ, &analyz_ad);
}


/*******************************/
/*******************************/
/**                           **/
/**    Termination            **/
/**                           **/
/*******************************/
/*******************************/


/*------------------*/
/*    disas_term    */
/*------------------*/


disas_term(term_type)      /* terminate vokabtr program */
int term_type;
{
   switch (term_type) {

      case(0):   /* normal termination */
      case(3): menu_bar(0x0L, FALSE);
      case(2): v_clsvwk(vdi_handle);
      case(1): appl_exit();
      case(4): break;
   }

}


/*******************************/
/*******************************/
/**                           **/
/**    disas   Event Handler  **/
/**                           **/
/*******************************/
/*******************************/


/*--------------------*/
/*   dispatch         */
/*--------------------*/

dispatch()
{
   int done;

   done = FALSE;

   while ( ! done ) {   /* loop handling user input until done */
      ev_which =
                evnt_multi(MU_BUTTON | MU_MESAG | MU_M1 | MU_KEYBD,
            0x02, 0x01, 0x01,
            m_out,
            work_area.g_x, work_area.g_y,
            work_area.g_w, work_area.g_h,
            0, 0, 0, 0, 0,
            &gl_rmsg, 0, 0,
            &mousex, &mousekey, &bstate, &kstate,
            &kreturn, &bclicks);


      wind_update(BEG_UPDATE);

      if (ev_which & MU_BUTTON)
         done = hndl_button();

      else if (ev_which & MU_M1)
         done = hndl_mouse();

      else if (ev_which & MU_MESAG)
         done = hndl_message();

      else if (ev_which & MU_KEYBD)
         done = hndl_keyboard();

      wind_update(END_UPDATE);

   }
}


/*------------------*/
/*  hndl_button     */
/*------------------*/
int
hndl_button()
{
   return (FALSE);
}


/*------------------*/
/*  hndl_mouse      */
/*------------------*/
int
hndl_mouse()
{
   return (FALSE);
}


/*-------------------*/
/*  hndl_message     */
/*-------------------*/
int
hndl_message()
{
   int done;

   done = FALSE;

   switch (gl_rmsg[0]) {

   case MN_SELECTED:
      done = hndl_mmen(gl_rmsg[3], gl_rmsg[4]);
      break;

   case WM_REDRAW:
      break;

   case WM_TOPPED:
      break;

   case WM_CLOSED:
      break;

   case WM_FULLED:
      break;

   case WM_ARROWED:
      break;

   case WM_HSLID:
      break;

   case WM_VSLID:
      break;

   case WM_SIZED:
      break;

   case WM_MOVED:
      break;

   }

   return (done);
}


/*--------------------*/
/*  hndl_keyboard     */
/*--------------------*/
int
hndl_keyboard()
{
   return (FALSE);
}


/*--------------------*/
/*  hndl_mmen         */
/*--------------------*/
int
hndl_mmen(title, item)
int title, item;
{
   int done;

   done = FALSE;

   switch (title) {

   case M_DESK:
      if (item == M_ABOUT)
         do_about();
      break;

   case M_FILE:
      switch (item) {

      case M_SELECT:
         do_select();
         break;

      case M_SAVE:
         do_save();
         break;

      case M_QUIT:
         done = TRUE;
         break;

      }

   case M_PRINT:
      switch (item) {

      case M_TOPRT:
             menu_icheck(menu, M_TOPRT, 1);
             menu_icheck(menu, M_TOFILE, 0);
         break;

      case M_TOFILE:
         menu_icheck(menu, M_TOFILE, 1);
         menu_icheck(menu, M_TOPRT, 0);
         break;
      }   
      break;

   case M_FORMAT:
      switch (item) {

      case M_SOURCE:
             menu_icheck(menu, M_SOURCE, 1);
             menu_icheck(menu, M_LIST, 0);
         break;

      case M_LIST:
         menu_icheck(menu, M_LIST, 1);
         menu_icheck(menu, M_SOURCE, 0);
         break;
      }   
      break;

   }

   menu_tnormal (menu, title, 1);

   return (done);
}


/*------------------*/
/*  do_about        */
/*------------------*/

do_about()
{
   int x, y;

   objc_offset(menu, M_ABOUT, &x, &y);
   hndl_dial(logo_ad, 0, x, y, 40, 10);
   objc_change(logo_ad, B_ABOUT, 0, 0, 0,
      screen_area.g_w, screen_area.g_h, 0, 0);
}




do_select()
{
   int exit;
   static char path[30];

   strcpy(path, "#\*.*");
   *curfil = '\0';

   fsel_input(path, curfil, &exit);

   if (exit == 0)    /* ABBRUCH */
      return(FALSE);

   analyze();

   return(FALSE);
}

do_save()
{
   return(FALSE);
}

/*--------------------*/
/*  hndl_dial         */
/*--------------------*/
int hndl_dial(tree, def, x, y, w, h)   /* center, display and get input */
long tree;            /* from specified dialog box     */
int def;
int x, y, w, h;
{
   int xdial, ydial, wdial, hdial;
   int exit_obj;

   form_center(tree, &xdial, &ydial, &wdial, &hdial);
   form_dial(0, x, y, w, h, xdial, ydial, wdial, hdial);
   form_dial(1, x, y, w, h, xdial, ydial, wdial, hdial);
   objc_draw(tree, ROOT, MAX_DEPTH, xdial, ydial, wdial, hdial);
   exit_obj = form_do(tree, def);
   form_dial(2, x, y, w, h, xdial, ydial, wdial, hdial);
   form_dial(3, x, y, w, h, xdial, ydial, wdial, hdial);

   return(exit_obj);
}


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
/* analyze    */
/**************/

analyze()
/* Disassembler analyze program. Sets up the
   disassembly environment and disassembles the program file. */
{
   int   infile;     /* file handle of input file */
   long  relsz;      /* size of relocation information */
   long  relpos;     /* file pointer to relocation information */
   long  get_filsz();
   struct gem_header prog_header;


   infile = Fopen(curfil, 0);

   if (infile < 0)
   {
      err_print("analyze", FOP_ERR, CONTINUE);
      return;
   }

   if (get_header(infile, &prog_header) != 0)
   {
      err_print("analyze", NPG_ERR, CONTINUE);
      return;
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

   relsz  = get_filsz(curfil);
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

   endgrph();

/*  printf("Disassembler pass 2...\n");

   Generate code for text segment

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

   printf("Done.\n");             */

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



static int frect[4];
static int counter;
static int xdial, ydial, wdial, hdial;
static int offx, bwdth;
static long aus;


dgrph(start, stop)
long start;
long stop;
{
   frect[0] = (int) (offx + (bwdth * start) / aus);
   frect[2] = (int) (offx + (bwdth * stop) / aus);
   vr_recfl(vdi_handle, frect);
}


initgrph(start, stop)
long start;
long stop;
{
   form_center(analyz_ad, &xdial, &ydial, &wdial, &hdial);
   form_dial(0, 0, 0, 40, 10, xdial, ydial, wdial, hdial);
   form_dial(1, 0, 0, 40, 10, xdial, ydial, wdial, hdial);
   objc_draw(analyz_ad, ROOT, MAX_DEPTH, xdial, ydial, wdial, hdial);

   objc_offset(analyz_ad, ANABOX, &offx, &frect[1]);
   bwdth = ((OBJECT *)(analyz_ad+ANABOX))->ob_width;
   frect[3] = frect[1] + ((OBJECT *)(analyz_ad+ANABOX))->ob_height - 1;

   vsf_interior(vdi_handle,2);
   vsf_style(vdi_handle,2);
   aus = stop;

}


endgrph()
{
   form_dial(2, 0, 0, 40, 10, xdial, ydial, wdial, hdial);
   form_dial(3, 0, 0, 40, 10, xdial, ydial, wdial, hdial);
}



err_print(module, number, action)
char *module;
int number;
int action;
{
   char *text;
   static char message[30];

   switch (number) {
      case (OOM_ERR) :  text = "Out of memory.";
            break;
      case (FSK_ERR) :  text = "File seek error.";
            break;
      case (RLC_ERR) :  text = "Reloc table too big.";
            break;
      case (FRD_ERR) :  text = "File read error.";
            break;
      case (FOP_ERR) :  text = "Can't open file.";
            break;
      case (NPG_ERR) :  text = "File is not executable.";
            break;
      default: text = "Error #   .";
               sprintf((text+7),"%03d", number);
               break;
      }


      sprintf(message,"[3][%s: %s][  OK  ]",module,text);

      form_alert(1,message);

      if (action == ABORT)
      exit(1);
}


/*
int   gl_xfull;      /* full window 'x'      */
int   gl_yfull;      /* full window 'y'      */
int   gl_wfull;      /* full window 'w'      */
int   gl_hfull;      /* full window 'h'      */
int   wk_xfull;      /* full work area 'x'   */
int   wk_yfull;      /* full work area 'y'   */
int   wk_wfull;      /* full work area 'w'   */
int   wk_hfull;      /* full work area 'h'   */
int   w_handle;      /* window handle */
int   gl_xact;       /* actual window 'x'      */
int   gl_yact;       /* actual window 'y'      */
int   gl_wact;       /* actual window 'w'      */
int   gl_hact;       /* actual window 'h'      */
int   wk_xact;       /* actual work area 'x'   */
int   wk_yact;       /* actual work area 'y'   */
int   wk_wact;       /* actual work area 'w'   */
int   wk_hact;       /* actual work area 'h'   */
int   w_handle;      /* window handle */

   wind_get(0, 4, &gl_xfull, &gl_yfull, &gl_wfull, &gl_hfull);
   wind_calc(1,
   (NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE),
             gl_xfull, gl_yfull, gl_wfull, gl_hfull,
             &wk_xfull, &wk_yfull, &wk_wfull, &wk_hfull);
   w_handle = wind_create(
   (NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE),
               gl_xfull, gl_yfull, gl_wfull, gl_hfull);

   gl_xact = gl_xfull + gl_wfull / 10;
   gl_yact = gl_yfull + gl_hfull / 10;
   gl_wact = (gl_wfull * 4) / 5;
   gl_hact = (gl_hfull * 4) / 5;

   wind_calc(1,
   (NAME|CLOSE|FULL|MOVE|SIZE|UPARROW|DNARROW|VSLIDE|LFARROW|RTARROW|HSLIDE),
             gl_xact, gl_yact, gl_wact, gl_hact,
             &wk_xact, &wk_yact, &wk_wact, &wk_hact);

   wind_open(w_handle, gl_xact, gl_yact, gl_wact, gl_hact);
*/
