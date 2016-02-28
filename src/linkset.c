#include "link-includes.h"
#include <memory.h>

static LINKSET_SET ss[LINKSET_MAX_SETS];
static wchar_t q_unit_is_used[LINKSET_MAX_SETS];

/* delcarations of non-exported functions */
static void clear_hash_table(const int unit);
static void initialize_unit(const int unit, const int size);
static LINKSET_NODE *linkset_add_internal(const int unit, wchar_t *str);
static int  take_a_unit();
static int  compute_hash(const int unit, const wchar_t *str);
static wchar_t *local_alloc (int nbytes);
 
int linkset_open(const int size)
{
  int unit = take_a_unit();
  initialize_unit(unit, size);
  return unit;
}

void linkset_close(const int unit)
{
  if (!q_unit_is_used[unit]) return;
  linkset_clear(unit);
  free (ss[unit].hash_table);
  q_unit_is_used[unit] = 0;
}

void linkset_clear(const int unit)
{
  int i;
  if (!q_unit_is_used[unit]) return;
  for (i=0; i<ss[unit].hash_table_size; i++)
    {
      LINKSET_NODE *p=ss[unit].hash_table[i];
      while (p!=0)
	{
	  LINKSET_NODE *q = p;
	  p=p->next;
	  if (q->solid) free (q->str);
	  free(q);
	}
    }
  clear_hash_table(unit);
}

int linkset_add(const int unit, wchar_t *str)
{
    /* returns 0 if already there, 1 if new. Stores only the pointer. */
    LINKSET_NODE *sn = linkset_add_internal(unit, str);
    if (sn==NULL) return 0;
    sn->solid = 0;
    return 1;
}

int linkset_add_solid(const int unit, wchar_t *str)
{
    /* returns 0 if already there, 1 if new. Copies string. */
    LINKSET_NODE *sn = linkset_add_internal(unit, str);
    if (sn==NULL) return 0;
    sn->str = (wchar_t *) malloc ((1+wcslen(str))*sizeof(wchar_t));
    if (!sn->str) error(L"linkset: out of memory!");
    wcscpy(sn->str,str);
    sn->solid = 1;
    return 1;
}

int linkset_remove(const int unit, wchar_t *str) 
{
  /* returns 1 if removed, 0 if not found */
  int hashval;
  LINKSET_NODE *p, *last;
  hashval = compute_hash(unit, str);
  last = ss[unit].hash_table[hashval];
  if (!last) return 0;
  if (!wcscmp(last->str,str)) 
    {
	ss[unit].hash_table[hashval] = last->next;
	if (last->solid) free(last->str);
	free(last);
	return 1;
    }
  p = last->next;
  while (p)
    {
	if (!wcscmp(p->str,str)) 
	  {
	      last->next = p->next;
	      if (last->solid) free(last->str);
	      free(p);
	      return 1;
	  }
	p=p->next;
	last = last->next;
    }
  return 0;
}


int linkset_match(const int unit, wchar_t *str) {
    int hashval;
    LINKSET_NODE *p;
    hashval = compute_hash(unit, str);
    p = ss[unit].hash_table[hashval];
    while(p!=0) 
      {
	  if (post_process_match(p->str,str)) return 1;
	  p=p->next;
      }
    return 0;
}

int linkset_match_bw(const int unit, wchar_t *str) {
    int hashval;
    LINKSET_NODE *p;
    hashval = compute_hash(unit, str);
    p = ss[unit].hash_table[hashval];
    while(p!=0)
      {
	  if (post_process_match(str,p->str)) return 1;
	  p=p->next;
      }
    return 0;
}

/***********************************************************************/
static void clear_hash_table(const int unit)
{
  memset(ss[unit].hash_table, 0, 
	 ss[unit].hash_table_size*sizeof(LINKSET_NODE *));
}

static void initialize_unit(const int unit, const int size) {
  if(size<=0) {
     wprintf_s(L"size too small!");
     abort();
  }
  ss[unit].hash_table_size = (int) ((float) size*LINKSET_SPARSENESS);
  ss[unit].hash_table = (LINKSET_NODE**) 
	local_alloc (ss[unit].hash_table_size*sizeof(LINKSET_NODE *));
  clear_hash_table(unit);
}

static LINKSET_NODE *linkset_add_internal(const int unit, wchar_t *str)
{
  LINKSET_NODE *p, *n;
  int hashval;

  /* look for str in set */
  hashval = compute_hash(unit, str);
  for (p=ss[unit].hash_table[hashval]; p!=0; p=p->next)
    if (!wcscmp(p->str,str)) return NULL;  /* already present */
  
  /* create a new node for u; stick it at head of linked list */
  n = (LINKSET_NODE *) local_alloc (sizeof(LINKSET_NODE));      
  n->next = ss[unit].hash_table[hashval];
  n->str = str;
  ss[unit].hash_table[hashval] = n;
  return n;
}

static int compute_hash(const int unit, const wchar_t *str)
 {
   /* hash is computed from capitalized prefix only */
  int i, hashval;
  hashval=LINKSET_DEFAULT_SEED;
  for (i=0; iswupper((wint_t)str[i]); i++)
    hashval = str[i] + 31*hashval;
  hashval = hashval % ss[unit].hash_table_size;
  if (hashval<0) hashval*=-1;
  return hashval;
}

static int take_a_unit() {
  /* hands out free units */
  int i;
  static int q_first = 1;
  if (q_first) {
    memset(q_unit_is_used, 0, LINKSET_MAX_SETS*sizeof(wchar_t));
    q_first = 0;
  }
  for (i=0; i<LINKSET_MAX_SETS; i++)
    if (!q_unit_is_used[i]) break;
  if (i==LINKSET_MAX_SETS) {
    wprintf_s(L"linkset.h: No more free units");   
    abort();
  }
  q_unit_is_used[i] = 1;
  return i;
}

static wchar_t *local_alloc (int nbytes)  {
   wchar_t * p;
   p = (wchar_t *) malloc (nbytes);
   if (!p) { 
        wprintf_s(L"linkset: out of memory");
       abort();
    }
   return p;
}

