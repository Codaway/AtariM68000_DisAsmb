/* SYMBOL MODULE TEST PROGRAM */

#include "osbind.h"
#include "symbol.h"

main()
{
      char text[40];
      long lword;
      int code;
      int ret;
      long inlword();
      int mark, tag;
      int number;


      sym_reset();

in:   while (1) {

      Cconws("Symbol Namen eingeben: ");

      text[0] = 40;
      Cconrs(text);

      if (text[2] == '~')
         goto out;

      text[2+text[1]] = '\0';

      Cconws("Adresse (hex) eingeben: ");
      lword = inlword();

      sym_name(lword, &text[2]);

      }

out:  while (1) {

      Cconws("Adresse (hex) eingeben: ");
      lword = inlword();

      if (lword == 0L) goto in;

      ret = sym_retrieve(lword, &tag, &number, text, &mark);

      printf("Text: >%s<\nNummer: %3d\nret = %d\n",text, number, ret);

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

