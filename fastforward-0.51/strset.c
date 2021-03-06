#include "strset.h"
#include "str.h"
#include "byte.h"

uint32 strset_hash(s)
char *s;
{
 unsigned char ch;
 uint32 h;
 h = 5381;
 while (ch = *s)
  {
   h = ((h << 5) + h) ^ ch;
   ++s;
  }
 return h;
}

int strset_init(set)
strset *set;
{
 int h;
 set->mask = 15;
 set->n = 0;
 set->a = 10;

 set->first = (int *) alloc(sizeof(int) * (set->mask + 1));
 if (!set->first) return 0;
 set->p = (strset_list *) alloc(sizeof(strset_list) * set->a);
 if (!set->p) { alloc_free(set->first); return 0; }
 set->x = (char **) alloc(sizeof(char *) * set->a);
 if (!set->x) { alloc_free(set->p); alloc_free(set->first); return 0; }

 for (h = 0;h <= set->mask;++h) set->first[h] = -1;

 return 1;
}

char *strset_in(set,s)
strset *set;
char *s;
{
 uint32 h;
 strset_list *sl;
 int i;
 char *xi;

 h = strset_hash(s);
 i = set->first[h & set->mask];
 while (i >= 0)
  {
   sl = set->p + i;
   if (sl->h == h)
    {
     xi = set->x[i];
     if (!str_diff(xi,s)) return xi;
    }
   i = sl->next;
  }
 return 0;
}

int strset_add(set,s)
strset *set;
char *s;
{
 uint32 h;
 int n;
 strset_list *sl;

 n = set->n;

 if (n == set->a)
  {
   int newa;
   strset_list *newp;
   char **newx;

   newa = n + 10 + (n >> 3);
   newp = (strset_list *) alloc(sizeof(strset_list) * newa);
   if (!newp) return 0;
   newx = (char **) alloc(sizeof(char *) * newa);
   if (!newx) { alloc_free(newp); return 0; }

   byte_copy(newp,sizeof(strset_list) * n,set->p);
   byte_copy(newx,sizeof(char *) * n,set->x);
   alloc_free(set->p);
   alloc_free(set->x);
   set->p = newp;
   set->x = newx;
   set->a = newa;

   if (n + n + n > set->mask)
    {
     int newmask;
     int *newfirst;
     int i;
     uint32 h;

     newmask = set->mask + set->mask + 1;
     newfirst = (int *) alloc(sizeof(int) * (newmask + 1));
     if (!newfirst) return 0;

     for (h = 0;h <= newmask;++h) newfirst[h] = -1;

     for (i = 0;i < n;++i)
      {
       sl = set->p + i;
       h = sl->h & newmask;
       sl->next = newfirst[h];
       newfirst[h] = i;
      }

     alloc_free(set->first);
     set->first = newfirst;
     set->mask = newmask;
    }
  }

 h = strset_hash(s);

 sl = set->p + n;
 sl->h = h;
 h &= set->mask;
 sl->next = set->first[h];
 set->first[h] = n;
 set->x[n] = s;
 set->n = n + 1;
 return 1;
}
