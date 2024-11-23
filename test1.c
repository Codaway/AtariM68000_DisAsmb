/* OPCODE FINDER TEST PROGRAM */

#include "osbind.h"
#include "opcode.h"

main()
{
      char text[40];
      unsigned int word;
      int code;
      int ret;

      while (1) {

      Cconws("Wort (hex) eingeben: ");
      word = inword();

      op_l(word, &code, text);

      printf("%04x: %10s  %3d\n", word, text, code);

      ret = sizespec(word, NORMAL, text);
      printf("Normal size spec: >%s<, ret = %d\n", text, ret);

      ret = sizespec(word, SHORT, text);
      printf("Short size spec: >%s<, ret = %d\n", text, ret);

      ret = eea(word, NORMAL, text);
      printf("Normal Effective address: >%s<, ret = %d\n", text, ret);

      ret = eea(word, HORDER, text);
      printf("Horder Effective address: >%s<, ret = %d\n", text, ret);

      reglist(word, NORMAL, text);
      printf("Reg. list: >%s<\n", text);

      } 
}


int inword()
{
   static char text[30];
   char *tp;
   int word;

   text[0] = 30;

   Cconrs(text);

   tp = &text[2];
   tp[text[1]] = '\0';

   sscanf(tp, "%x", &word);
   return(word);
}

