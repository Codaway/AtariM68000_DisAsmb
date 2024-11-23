/**********************************************************************/
/*                                                                    */
/*     MODULE symbol table                                            */
/*                                                                    */
/*     Contains a symbol table to provide labels for memory locations */
/*     in text, data and bss sections.                                */
/*                                                                    */
/*     AUTHOR: Gernot Kunz                                            */
/*                                                                    */
/*     DATE:   1986/10/27                                             */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/*
SPECIFICATION:

   #include <symbol.h>


   sym_reset();                     Reset and empty symbol table.


   sym_name(address, printname)     Name an existing symbol or create
   long address;                    a named new untagged one.
   char *printname;


   int sym_tag(address, tag)        Specify the tag of an existing symbol
   long address;                    or create an unnamed new
   int tag;                         one with a specific tag. The number
                                    of the symbol is returned.

   int sym_retrieve(address, tag,   Retrieve symbol data and return zero, or
                number, printname)  don't find it and return nonzero.
   long address;
   int *tag;
   int *number;
   char *printname;


   int sym_lolab(address, end)      Retrieve a symbol with tag LBL in an empty
   long *address;                   block (blk_inblk).
   long *end;                       Returns 0 on success, nonzero otherwise.



POSSIBLE TAGS:

   LBL   label      (tag class, more specifically:)

                 LBL normal label
                 SBR subroutine
                 LOP loop

   VAR   variable   (tag class, more specifically:)

                 DAT data segment variable
                 BSS bss  segment variable

                 BYT byte
                 WRD word          may be 'ored' to tag.
                 LNG long

   UTG   untagged
*/


#include <symbol.h>
#include <daerror.h>

#define NULL (0L)
#define TRUE (1)
#define FALSE (0)

/* #define DEBUG */

char *malloc();



/* The symbol table is organized as a binary tree, sorted by address in
   ascending order. The nodes are structured as follows: */

struct sym_entry {
   long address;
   int  tag;
   int  number;
   char *printname;
   struct sym_entry *left;
   struct sym_entry *right;
   };


/* Labels, which were in an empty block when tagged are kept in a list
   with the following nodes: */

struct lab_node {
   struct sym_entry *symbol;
   struct lab_list *nxlab;
   };


static struct sym_entry *sym_table; /* pointer to the symbol table */

static int running_no;              /* running number to generate unique
                                       symbol numbers. */

static struct lab_node *lab_list;   /* list linking all labels in empty
                                       blocks when tagged */

/***************/
/* sym_reset   */
/***************/

sym_reset()
/* Reset symbol table. */
{
   running_no = 1;

   sym_table = NULL; /* watch out, the structure is simply cut off. */
   lab_list  = NULL; /* and so is lab_list */

#ifdef DEBUG
   printf("sym_reset()\n");
#endif

}




/***************/
/* sym_name    */
/***************/

sym_name(address, printname)
long address;
char *printname;
/* Name an existing symbol or create a named, untagged new one. */
{
   ins_name(&sym_table, address, printname);
}


ins_name(tree, addr, name)
struct sym_entry *(*tree);
long addr;
char *name;
/* Recursive tree-insertion function */
{
   if (*tree == NULL)
   {
      *tree = (struct sym_entry *) malloc(sizeof(struct sym_entry));
      if (*tree == NULL)
         err_print("ins_name", OOM_ERR, ABORT);

      (*tree)->printname = malloc(strlen(name)+1);
      if ((*tree)->printname == NULL)
         err_print("ins_name", OOM_ERR, ABORT);

      (*tree)->address = addr;
      (*tree)->tag     = UTG;
      (*tree)->number  = running_no++;
      strcpy((*tree)->printname, name);
      (*tree)->left    = NULL;
      (*tree)->right   = NULL;
   }
   else if (addr < (*tree)->address)
      ins_name(&((*tree)->left), addr, name);
   else if (addr > (*tree)->address)
      ins_name(&((*tree)->right), addr, name);
   else  /* symbol already exists */
      {
         if ((*tree)->printname != NULL)
            free((*tree)->printname);    /* symbol already has a name */

         (*tree)->printname = malloc(strlen(name)+1);
         if ((*tree)->printname == NULL)
            err_print("ins_name", OOM_ERR, ABORT);

         strcpy((*tree)->printname, name);
      }

}



/***************/
/* sym_tag     */
/***************/


int sym_tag(address, tag)
long address;
int tag;
/* Specify the tag of an existing symbol or create an unnamed, unmarked new
   one with a specific tag. Returns the symbol's number */
{
   return(ins_tag(&sym_table, address, tag));
}


int ins_tag(tree, addr, tag)
struct sym_entry *(*tree);
long addr;
int tag;
/* Recursive tree-insertion function */
{
   if (*tree == NULL)
   {
      *tree = (struct sym_entry *) malloc(sizeof(struct sym_entry));
      if (*tree == NULL)
         err_print("ins_tag", OOM_ERR, ABORT);

      (*tree)->address = addr;
      (*tree)->tag     = tag;
      (*tree)->number  = running_no++;
      (*tree)->printname = NULL;
      (*tree)->left    = NULL;
      (*tree)->right   = NULL;

      if (tag & LBL)                /* Labels are considered for the list */
         consider((*tree), addr);

      return((*tree)->number);
   }
   else if (addr < (*tree)->address)
      return(ins_tag(&((*tree)->left), addr, tag));
   else if (addr > (*tree)->address)
      return(ins_tag(&((*tree)->right), addr, tag));
   else  /* symbol already exists */
   {
      (*tree)->tag = tag;

      if (tag & LBL)                /* Labels are considered for the list */
         consider((*tree), addr);

      return((*tree)->number);
   }

}




/****************/
/* sym_retrieve */
/****************/


int sym_retrieve(address, tag, number, printname)
long address;
int *tag;
int *number;
char *printname;
/* Retrieve old symbol data or return nonzero */
{
   return(ret_sym(&sym_table, address, tag, number, printname));
}


int ret_sym(tree, addr, tag, num, name)
struct sym_entry *(*tree);
long addr;
int *tag;
int *num;
char *name;
/* Recursive tree-retrieval function */
{
   if (*tree == NULL)
   {
      return(-1);
   }
   else if ((*tree)->address == addr)
   {
      *tag = (*tree)->tag;
      *num = (*tree)->number;
      if ((*tree)->printname == NULL)
         *name = '\0';
      else
         strcpy(name, (*tree)->printname);

      return(0);
   }
   else if (addr < (*tree)->address)
      return(ret_sym(&((*tree)->left),addr,tag,num,name));
   else
      return(ret_sym(&((*tree)->right),addr,tag,num,name));

}



int sym_for(func)
int (*func)();
/* Execute func for each symbol in the symbol table.
   as (*func)(adr, tag, num, name) */
{
   ret_for(sym_table, func);
}


ret_for(tree, func)
struct sym_entry *tree;
int (*func)();
{
   if (tree != NULL)
   {
      ret_for(tree->left, func);

      (*func)(tree->address,tree->tag,tree->number,tree->printname);

      ret_for(tree->right, func);
   }
}



/****************/
/* sym_lolab    */
/****************/


int sym_lolab(addr, end)
long *addr;
long *end;
/* Retrieve label in empty block. Returns 0 on
success, -1 otherwise. */
{
   struct lab_node *(*p), *q;
   int found;

   p = &lab_list;
   found = 0;

   while ((*p != NULL) && (!found))
   {
      found = blk_inblk((*addr = ((*p)->symbol)->address), end);
      if (!found)
      {
         q = (*p)->nxlab;
         free(*p);
         *p = q;
      }
      else
         p = &((*p)->nxlab);
   }

   if (found) return(0);
   else       return(-1);
}


consider(sym, adr)
struct sym_entry *sym;
long adr;
/* add a symbol to the label list, if it is in an empty block. */
{
   long dum;
   struct lab_node *p;

   if (blk_inblk(adr, &dum))
   {
      p = lab_list;
      lab_list = (struct lab_node *) malloc(sizeof(struct lab_node));
      if (lab_list == NULL)
         err_print("consider", OOM_ERR, ABORT);

      lab_list->symbol = sym;
      lab_list->nxlab = p;
   }
}


