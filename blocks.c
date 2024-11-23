/**********************************************************************/
/*                                                                    */
/*     MODULE block list                                              */
/*                                                                    */
/*     Contains a list of blocks already analyzed by the              */
/*     disassembler. Adjoining blocks are automatically compacted to  */
/*     form the smallest possible number of blocks.                   */
/*                                                                    */
/*     AUTHOR: Gernot Kunz                                            */
/*                                                                    */
/*     DATE:   1986/10/27                                             */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

/*
SPECIFICATION:


   blk_reset(start, stop)           Reset and empty block list.
   long start;
   long stop;

   blk_insert(start, stop)          Insert a block into the list and
   long start;                      normalize the structure, if
   long stop;                       necessary.

   int blk_inblk(address, eadr)     Tests, if an address is in an empty
   long address;                    block. Returns also the address of the
   long *eadr;                      next full block (end address)

   int blk_leblk(start, stop)       Returns lowest empty block data and zero
   long *start;                     or nonzero if no empty block is found.
   long *stop;

   blk_debug()                      Prints out the block list for debugging
                                    purposes.

IMPLEMENTATION:
*/

#include <daerror.h>

#define NULL (0L)
#define TRUE (1)
#define FALSE (0)

#define EMPTY (0)
#define FULL  (1)
#define TOGGLE(x) (1 - x)

/* #define DEBUG */

struct block {
   long start;
   struct block *next;
   };

static struct block *blk_list;
static long end;
static int init_status;

char *malloc();


/**************/
/* blk_reset  */
/**************/

blk_reset(strt, stop)
long strt;
long stop;
/* Reset and empty block list */
{
   blk_list = (struct block *) malloc(sizeof(struct block));
   if (blk_list == NULL)
      err_print("blk_reset", OOM_ERR, ABORT);

   blk_list->start = strt;
   blk_list->next = (struct block *) malloc(sizeof(struct block));
   if (blk_list->next == NULL)
      err_print("blk_reset", OOM_ERR, ABORT);

   blk_list->next->start = stop;    /* a kind of sentinel node */
   blk_list->next->next  = NULL;

   init_status = EMPTY;
   end   = stop;

#ifdef DEBUG
   printf("blk_reset(0x%08lx,0x%08lx)\n",strt,stop);
#endif
}



/**************/
/* blk_insert */
/**************/

int blk_insert(strt, stop)
long strt;
long stop;
/* Insert a block into the list and normalize the structure, if
   necessary. Returns zero on success, -1 otherwise. */
{
   struct block *p0, *p1, *p2;
   int status;

   p0 = NULL;
   p1 = blk_list;
   p2 = blk_list->next;
   status = init_status;

   while ((p2 != NULL) && (strt >= p2->start))
   {
      p0 = p1;
      p1 = p2;
      p2 = p2->next;
      status = TOGGLE(status);
   }

/* here  ((strt < p2->start) || (p2 == NULL)) */

   if (status == FULL)
   {
#ifdef DEBUG
      printf("blk_insert: Insert into full block error.\n");
#endif
      return(-1);
   }

   if (p2 == NULL)
   {
#ifdef DEBUG
      printf("blk_insert: Unexpected end of block list.\n");
#endif
      return(-1);
   }

/* here ((strt < p2->start) && (status == EMPTY))  */

   if ((p1->start < strt) && (stop < p2->start))
   {
      /* split empty block */

      p1->next = (struct block *) malloc(sizeof(struct block));
      if (p1->next == NULL)
         err_print("blk_insert", OOM_ERR, ABORT);

      p1->next->start = strt;
      p1->next->next = (struct block *) malloc(sizeof(struct block));
      if (p1->next->next == NULL)
         err_print("blk_insert", OOM_ERR, ABORT);

      p1->next->next->start = stop;
      p1->next->next->next = p2;
   }
   else if ((p1->start == strt) && (stop < p2->start))
   {
      /* append to  full block  (right) */

      if (blk_list == p1)
      {
         blk_list = (struct block *) malloc(sizeof(struct block));
         if (blk_list == NULL)
            err_print("blk_insert", OOM_ERR, ABORT);

         blk_list->start = strt;
         blk_list->next  = p1;
         init_status = FULL;
      }

      p1->start += stop - strt;
   }
   else if ((p1->start < strt) && (stop == p2->start))
   {
      /* append to full block (left) */

      p2->start -= stop - strt;
   }
   else if ((p1->start == strt) && (p2->start == stop))
   {
      if (blk_list == p1)
      {
            p1->next  = p2->next;
            init_status = FULL;
      }
      else
      {
         p0->next = p2->next;
         free(p1);
      }

      free(p2);
   }
   else
      return (-1);

   return(0);

}




/***************/
/* blk_inblk   */
/***************/

int blk_inblk(address, enda)
long address;
long *enda;
/* Test, if an address is in an empty block. Returns the address of the
   next full block, if so. */
{
   struct block *p;
   int status;

   p = blk_list;
   status = TOGGLE(init_status);

   while ((p != NULL) && (address >= p->start))
   {
      status = TOGGLE(status);
      p = p->next;
   }

   if (p != NULL)
       *enda = p->start;
   else
       return(FALSE);

   return (status == EMPTY);
}



/****************/
/* blk_leblk    */
/****************/

int blk_leblk(start, stop)
long *start;
long *stop;
/* Returns lowest empty block data and zero or nonzero, if no empty block
   is found. */
{
   if (init_status == EMPTY)
   {
      *start = blk_list->start;
      *stop = blk_list->next->start;
      return(0);
   }
   else
   {
      if (blk_list->next == NULL)
      {
         return(-1);
      }
      else
      {
         *start = blk_list->next->start;
         *stop = blk_list->next->next->start;
         return(0);
      }
   }
}




blk_debug()
/* Prints out the block list for debugging purposes */
{
   struct block *p;
   int status;

   p = blk_list;
   status = init_status;

   printf("blk_debug:\n");

   while (p != NULL)
   {
      if (status == FULL)
         printf("FULL  ");
      else
         printf("EMPTY ");

      printf("%08lx\n",p->start);

      status = TOGGLE(status);
      p = p->next;
   }
}
