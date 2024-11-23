/* BLOCKS MODULE TEST PROGRAM */

#include <osbind.h>

main()
{
      char text[40];
      long lword;
      int code;
      int ret;
      long inlword();
      int mark, tag;
      int number;
      long strt, end;

      Cconws("Start:");
      strt = inlword();

      Cconws("End  :");
      end = inlword();

      blk_reset(strt, end);
      blk_debug();


      while (1) {

      Cconws("New block start: ");
      strt = inlword();
      Cconws("New block end  : ");
      end  = inlword();

      blk_insert(strt, end);
      blk_debug();

      Cconws("Test address   : ");
      strt = inlword();

      if (blk_inblk(strt, &end))
         printf("\nin empty block, end = %08lx\n",end);
      else
         printf("\nnot in empty block\n");

      }

}


long inlword()
{
   static char text[30];
   char *tp;
   long word;

   text[0] = 30;

   Cconrs(text);

   tp = &text[2];
   tp[text[1]] = '\0';

   sscanf(tp, "%lx", &word);
   return(word);
}

