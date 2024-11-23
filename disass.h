/********************************************************************/
/*                                                                  */
/* File:   DISASS header file                                       */
/*                                                                  */
/* Author: Gernot Kunz                                              */
/*                                                                  */
/* Date:   1987/11/17                                               */
/*                                                                  */
/*                                                                  */
/* Defines gem header global structure.                             */
/*                                                                  */
/*                                                                  */
/*                                                                  */
/********************************************************************/


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

