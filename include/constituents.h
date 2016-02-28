#ifndef _CONSTITUENTSH_
#define _CONSTITUENTSH_

#include "link-includes.h"

/* Invariant: Leaf if child==NULL */
typedef struct CNode_s CNode;
struct CNode_s {
  wchar_t  * label;
  CNode * child;
  CNode * next;
  int   start, end;
};

CNode * linkage_constituent_tree(Linkage linkage);
void    linkage_free_constituent_tree(CNode * n);
wchar_t *  linkage_print_constituent_tree(Linkage linkage, int mode);

#endif
