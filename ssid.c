/***************************************************************************/
/*                                                                         */
/*  FILE:    SSID.C                                                        */
/*                                                                         */
/*  AUTHOR:  DI Gernot Kunz                                                */
/*                                                                         */
/*  DATE:    22. 01. 1988                                                  */
/*                                                                         */
/*                                                                         */
/*           Super Symbolic Instruction Debugger                           */
/*                                                                         */
/*                                                                         */
/*                                                                         */
/***************************************************************************/


#include <osbind.h>
#include <stdio.h>
#include <ctype.h>

/* Screen resolutions */
#define LOW_RES  0
#define MED_RES  1
#define HIGH_RES 2

/* Terminal emulator codes */
#define CURS_UP    "\033A"
#define CURS_DOWN  "\033B"
#define CURS_RIGHT "\033C"
#define CURS_LEFT  "\033D"
#define CLEAR_HOME "\033E"
#define CURS_HOME  "\033H"

/* maximal command length */
#define MAX_CL 80

main()
{
   char command[MAX_CL], *cp;  /* command line string and pointer */


   start_logo();

   do {
        /* prompt for next command */
        printf("SSID> ");
        fflush(stdout);
        fgets(command, MAX_CL, stdin);

        /* ignore leading blanks */
        for (cp=command; *cp==' '; cp++);

        switch (toupper(*cp))
        {
         case 'E' : do_exec();
                    break;

         case 'V' : do_dparms();
                    break;

         case 'R' : do_read();
                    break;

         case 'W' : do_write(); 
                    break;

         case 'D' : do_display();
                    break;

         case 'L' : do_list();
                    break;

         case 'X' : do_registers();
                    break;

         case 'G' : do_go();
                    break;

         case 'T' : do_trace();
                    break;

         case 'U' : do_untrace();
                    break;

         case 'Q' : printf("Really quit (y/n) ? ");
                    fflush(stdout);
                    if (toupper(getchar()&0xff)=='Y') goto quit;
                    printf("\n");
                    break;

         case '\0' :break;

         default  : do_help();
                    break;
        }

      } while (1);

   quit: ;
}
 



start_logo()
{
   printf("%s  ******************\n", CLEAR_HOME);
   printf(  "  * SUPER SID V1.0 *\n");
   printf(  "  *       BY       *\n");
   printf(  "  * DI GERNOT KUNZ *\n");
   printf(  "  ******************\n\n");
}


do_exec()
{
}

do_dparms()
{
}

do_read()
{
}

do_write()
{
}

do_display()
{
}

do_list()
{
}

do_registers()
{
}

do_go()
{
}

do_trace()
{
}

do_untrace()
{
}

do_help()
{
   printf("\n\
SSID commands:\n\
 E<name>            Load program to execute\n\
 V                  Display program parameters\n\
 R<name>            Read file into memory\n\
 W<name>,<s>,<f>    Write file from address s to f\n");

   printf("\
 D<s>               Display memory starting from s (bytes)\n\
 D<s>,<f>           Display memory from s to f (bytes)\n\
 DW<s>              Display memory starting from s (words)\n\
 DW<s>,<f>          Display memory from s to f (words)\n");

   printf("\
 DL<s>              Display memory starting from s (long words)\n\
 DL<s>,<f>          Display memory form s to f (long words)\n\
 L<s>               Disassemble starting from s\n\
 L<s>,<f>           Disassemble from s to f\n");

   printf("\
 X                  Display 68000 registers\n\
 X<r>               Alter register contents\n\
 G                  Start execution at current location\n\
 G<s>               Start execution at location s\n");

   printf("\
 G<s>,b1..bn        Start execution at s with breakpoints b1..bn\n\
 G,b1..bn           Start execution at current location with breakpoints\n\
 T                  Trace program (1 step)\n\
 T<n>               Trace program (n steps)\n");

   printf("\
 U                  Execute 1 instruction without trace\n\
 U<n>               Execute n instructions without trace\n\
 Q                  Quit program\n");
}

