 /****************************************************************************/
 /*                                                                          */
 /*  Copyright (C) 1991-2000, all rights reserved                            */
 /*  Daniel Sleator, David Temperley, and John Lafferty                      */
 /*  See file "README" for information about commercial use of this system   */
 /*                                                                          */
 /****************************************************************************/

#include "link-includes.h"

#define DEFAULTPATH L".:./data:"  /* Appended to the end of the 
				    environment dictpath */

/* This file contains certain general utilities. */

int   verbosity;

void safe_strcpy(wchar_t *u, wchar_t * v, int usize) {
/* Copies as much of v into u as it can assuming u is of size usize */
/* guaranteed to terminate u with a '\0'.                           */    
    wcsncpy(u, v, usize-1);
    u[usize-1] = L'\0';
}

void safe_strcat(wchar_t *u, wchar_t *v, int usize) {
/* catenates as much of v onto u as it can assuming u is of size usize           */
/* guaranteed to terminate u with a '\0'.  Assumes u and v are null terminated.  */
    wcsncat(u, v, usize-wcslen(u)-1);
    u[usize-1] = L'\0';
}    

void free_connectors(Connector *e) {
/* free the list of connectors pointed to by e
   (does not free any strings) 
*/
    Connector * n;
    for(;e != NULL; e = n) {
	n = e->next;
	xfree((wchar_t *)e, sizeof(Connector));
    }
}

void free_disjuncts(Disjunct *c) {
/* free the list of disjuncts pointed to by c
   (does not free any strings) 
*/
    Disjunct *c1;
    for (;c != NULL; c = c1) {
		c1 = c->next;
		free_connectors(c->left);
		free_connectors(c->right);
		xfree((wchar_t *)c, sizeof(Disjunct));
    }
}

Connector * init_connector(Connector *c) {
    c->length_limit = UNLIMITED_LEN;
    /*    c->my_word = NO_WORD;  */       /* mark it unset, to make sure it gets set later */
    return c;
}

void free_X_nodes(X_node * x) {
/* frees the list of X_nodes pointed to by x, and all of the expressions */
    X_node * y;
    for (; x!= NULL; x = y) {
	y = x->next;
	free_Exp(x->exp);
	xfree((wchar_t *)x, sizeof(X_node));
    }
}

void free_E_list(E_list *);
void free_Exp(Exp * e) {
    if (e->type != CONNECTOR_type) {
	free_E_list(e->u.l);
    }
    xfree((wchar_t *)e, sizeof(Exp));
}

void free_E_list(E_list * l) {
    if (l == NULL) return;
    free_E_list(l->next);
    free_Exp(l->e);
    xfree((wchar_t *)l, sizeof(E_list));
}

int size_of_expression(Exp * e) {
/* Returns the number of connectors in the expression e */
    int size;
    E_list * l;
    if (e->type == CONNECTOR_type) return 1;
    size = 0;
    for (l=e->u.l; l!=NULL; l=l->next) {
	size += size_of_expression(l->e);
    }
    return size;
}

unsigned int randtable[RTSIZE];

/* There is a legitimate question of whether having the hash function    */
/* depend on a large array is a good idea.  It might not be fastest on   */
/* a machine that depends on caching for its efficiency.  On the other   */
/* hand, Phong Vo's hash (and probably other linear-congruential) is     */
/* pretty bad.  So, mine is a "competitive" hash function -- you can't   */
/* make it perform horribly.                                             */

void init_randtable(void) {
    int i;
    srand(10);
    for (i=0; i<RTSIZE; i++) {
	randtable[i] = rand();
    }
}

/* Build a copy of the given expression (don't copy strings, of course) */
E_list * copy_E_list(E_list * l);
Exp * copy_Exp(Exp * e) {
    Exp * n;
    if (e == NULL) return NULL;
    n = (Exp *) xalloc(sizeof(Exp));
    *n = *e;
    if (e->type != CONNECTOR_type) {
	n->u.l = copy_E_list(e->u.l);
    }
    return n;
}

E_list * copy_E_list(E_list * l) {
    E_list * nl;
    if (l == NULL) return NULL;
    nl = (E_list *) xalloc(sizeof(E_list));
    *nl = *l;    /* not necessary -- both fields will be built below */
    nl->next = copy_E_list(l->next);
    nl->e = copy_Exp(l->e);
    return nl;
}

Connector * copy_connectors(Connector * c) {
/* This builds a new copy of the connector list pointed to by c.
   Strings, as usual, are not copied.
*/
    Connector *c1;
    if (c == NULL) return NULL;
    c1 = init_connector((Connector *) xalloc(sizeof(Connector)));
    *c1 = *c;
    c1->next = copy_connectors(c->next);
    return c1;
}

Disjunct * copy_disjunct(Disjunct * d) {
/* This builds a new copy of the disjunct pointed to by d (except for the
   next field which is set to NULL).  Strings, as usual,
   are not copied.
*/
    Disjunct * d1;
    if (d == NULL) return NULL;
    d1 = (Disjunct *) xalloc(sizeof(Disjunct));
    *d1 = *d;
    d1->next = NULL;
    d1->left = copy_connectors(d->left);
    d1->right = copy_connectors(d->right);
    return d1;
}

int max_space_in_use;
int space_in_use;
int max_external_space_in_use;
int external_space_in_use;

void * xalloc(int size) {
/* To allow printing of a nice error message, and keep track of the
   space allocated.
*/   
    wchar_t * p = (wchar_t *) malloc(size);
    space_in_use += size;
    if (space_in_use > max_space_in_use) max_space_in_use = space_in_use;
    if ((p == NULL) && (size != 0)){
        wprintf_s(L"Ran out of space.\n");
	abort();
        exit(1);
    }
    return (void *) p;
}

void xfree(void * p, int size) {
    space_in_use -= size;
    free(p);
}

void * exalloc(int size) {

    wchar_t * p = (wchar_t *) malloc(size);
    external_space_in_use += size;
    if (external_space_in_use > max_external_space_in_use) {
		max_external_space_in_use = external_space_in_use;
    }
    if ((p == NULL) && (size != 0)){
        wprintf_s(L"Ran out of space.\n");
		abort();
        exit(1);
    }
    return (void *) p;
}

void exfree(void * p, int size) {
    external_space_in_use -= size;
    free(p);
}

/* This is provided as part of the API */
void string_delete(wchar_t * p) {
    exfree(p, sizeof(wchar_t)*(wcslen(p)+1));
}

void exfree_connectors(Connector *e) {
    Connector * n;
    for(;e != NULL; e = n) {
		n = e->next;
		exfree(e->string, sizeof(wchar_t)*(wcslen(e->string)+1));
		exfree(e, sizeof(Connector));
    }
}

Connector * excopy_connectors(Connector * c) {
    Connector *c1;

    if (c == NULL) return NULL;

    c1 = init_connector((Connector *) exalloc(sizeof(Connector)));
    *c1 = *c;
    c1->string = (wchar_t *) exalloc(sizeof(wchar_t)*(wcslen(c->string)+1));
    wcscpy(c1->string, c->string);
    c1->next = excopy_connectors(c->next);

    return c1;
}


Link excopy_link(Link l) {
     Link newl;

     if (l == NULL) return NULL;

     newl = (Link) exalloc(sizeof(struct Link_s));
     newl->name = (wchar_t *) exalloc(sizeof(wchar_t)*(wcslen(l->name)+1));
     wcscpy(newl->name, l->name);
     newl->l = l->l;
     newl->r = l->r;
     newl->lc = excopy_connectors(l->lc);
     newl->rc = excopy_connectors(l->rc);

     return newl;
}

void exfree_link(Link l) {
     exfree_connectors(l->rc);
     exfree_connectors(l->lc);
     exfree(l->name, sizeof(wchar_t)*(wcslen(l->name)+1));
     exfree(l, sizeof(struct Link_s));
}


Disjunct * catenate_disjuncts(Disjunct *d1, Disjunct *d2) {
/* Destructively catenates the two disjunct lists d1 followed by d2. */
/* Doesn't change the contents of the disjuncts */
/* Traverses the first list, but not the second */    
    Disjunct * dis = d1;

    if (d1 == NULL) return d2;
    if (d2 == NULL) return d1;
    while (dis->next != NULL) dis = dis->next;
    dis->next = d2;
    return d1;
}

X_node * catenate_X_nodes(X_node *d1, X_node *d2) {
/* Destructively catenates the two disjunct lists d1 followed by d2. */
/* Doesn't change the contents of the disjuncts */
/* Traverses the first list, but not the second */    
    X_node * dis = d1;

    if (d1 == NULL) return d2;
    if (d2 == NULL) return d1;
    while (dis->next != NULL) dis = dis->next;
    dis->next = d2;
    return d1;
}

int next_power_of_two_up(int i) {
/* Returns the smallest power of two that is at least i and at least 1 */
    int j=1;
    while(j<i) j = j<<1;
    return j;
}

int upper_case_match(wchar_t *s, wchar_t *t) {
/* returns TRUE if the initial upper case letters of s and t match */
    while(iswupper((wint_t)*s) || iswupper((wint_t)*t)) {
	if (*s != *t) return FALSE;
	s++;
	t++;
    }
    return (!iswupper((wint_t)*s) && !iswupper((wint_t)*t));
}

void left_print_string(FILE * fp, wchar_t * s, wchar_t * t) {
/* prints s then prints the last |t|-|s| characters of t.
   if s is longer than t, it truncates s.
*/
    int i, j, k;
    j = wcslen(t);
    k = wcslen(s);
    for (i=0; i<j; i++) {
	if (i<k) {
	    fwprintf_s(fp, L"%c", s[i]);
	} else {
	    fwprintf_s(fp, L"%c", t[i]);
	}
    }
}

int sentence_contains(Sentence sent, wchar_t * s) {
/* Returns TRUE if one of the words in the sentence is s */
    int w;
    for (w=0; w<sent->length; w++) {
	if (wcscmp(sent->word[w].string, s) == 0) return TRUE;
    }
    return FALSE;
}

void set_is_conjunction(Sentence sent) {
    int w;
    wchar_t * s;
    for (w=0; w<sent->length; w++) {
	s = sent->word[w].string;
	sent->is_conjunction[w] = ((wcscmp(s, L"and")==0) || (wcscmp(s, L"or" )==0) ||
				   (wcscmp(s, L"but")==0) || (wcscmp(s, L"nor")==0));
    }
}

int sentence_contains_conjunction(Sentence sent) {
/* Return true if the sentence contains a conjunction.  Assumes
   is_conjunction[] has been initialized.
*/
    int w;
    for (w=0; w<sent->length; w++) {
	if (sent->is_conjunction[w]) return TRUE;
    }
    return FALSE;
}

int conj_in_range(Sentence sent, int lw, int rw) {
/* Returns true if the range lw...rw inclusive contains a conjunction     */
    for (;lw <= rw; lw++) {
	if (sent->is_conjunction[lw]) return TRUE;
    }
    return FALSE;
}

FILE *old_dictopen(wchar_t *dictname, wchar_t *filename, wchar_t *how) {
    /* This is the old version.  I think it's buggy and inelegant.  I've
       rewritten it below   *DS*  */

    /* This function is used to open a dictionary file or a word file.
       It tries the following sequence of places to look for the file.
       The first one that is successful is used.  (1) Open the given
       file.  (2) if DICTPATH is defined, try the paths defined by that.
       (3) Use the path name of the given dictionary.  */

    wchar_t completename[MAX_PATH_NAME+1], fullpath[MAX_PATH_NAME+1], *dictpath;
    wchar_t dummy[MAX_PATH_NAME+1];
    wchar_t *pos, *oldpos;
    int filenamelen, len;
    FILE *fp;

    swprintf_s(fullpath, MAX_PATH_NAME+1, L"%s%s", getenv(DICTPATH), DEFAULTPATH);
    dictpath = fullpath;

    if (dictpath == NULL && dictname != NULL) {
	/* look in the dictname part */
	//safe_strcpy(dummy, dictname, sizeof(dummy));
	safe_strcpy(dummy, dictname, MAX_PATH_NAME+1);
	pos = wcsrchr(dummy, L'/');
	if (pos != NULL) {
	    *pos = L'\0';
	    dictpath = dummy;
	}
    }

    dictpath = fullpath;

    if (dictpath == NULL && dictname != NULL) {
	/* look in the dictname part */
	//safe_strcpy(dummy, dictname, sizeof(dummy));
	safe_strcpy(dummy, dictname, MAX_PATH_NAME+1);
	pos = wcsrchr(dummy, L'/');
	if (pos != NULL) {
	    *pos = L'\0';
	    dictpath = dummy;
	}
    }


    if (dictpath == NULL) {
	wprintf_s(L"   Opening %s\n", filename); 
	return (_wfopen(filename, how));
    } else if ((fp = _wfopen(filename, how)) != NULL) {
	wprintf_s(L"   Opening %s\n", filename);
	return fp;
    }
    
    filenamelen = wcslen(filename);
    len = wcslen(dictpath)+ filenamelen + 1 + 1;
    oldpos = dictpath;
    fp = NULL;
    while ((pos = wcschr(oldpos, L':')) != NULL) {
	wcsncpy(completename, oldpos, (pos-oldpos));
	*(completename+(pos-oldpos)) = L'/';
	wcscpy(completename+(pos-oldpos)+1,filename);
	if ((fp = _wfopen(completename, how)) != NULL) {
	    wprintf_s(L"   Opening %s\n", completename); 
	    return fp;
	}
	oldpos = pos+1;
    }
    pos = oldpos+wcslen(oldpos);
    wcscpy(completename, oldpos);
    *(completename+(pos-oldpos)) = L'/';
    wcscpy(completename+(pos-oldpos)+1,filename);
    fp = _wfopen(completename,how);
    wprintf_s(L"   Opening %s\n", completename); 
    return fp;
}




FILE *dictopen(wchar_t *dictname, wchar_t *filename, wchar_t *how) {

    /* This function is used to open a dictionary file or a word file,
       or any associated data file (like a post process knowledge file).

       It works as follows.  If the file name begins with a "/", then
       it's assumed to be an absolute file name and it tries to open
       that exact file.

       If the filename does not begin with a "/", then it uses the
       dictpath mechanism to find the right file to open.  This looks
       for the file in a sequence of directories until it finds it.  The
       sequence of directories is specified in a dictpath string, in
       which each directory is followed by a ":".

       The dictpath that it uses is constructed as follows.  If the
       dictname is non-null, and is an absolute path name (beginning
       with a "/", then the part after the last "/" is removed and this
       is the first directory on the dictpath.  After this comes the
       DICTPATH environment variable, followed by the DEFAULTPATH
    */

    wchar_t completename[MAX_PATH_NAME+1];
    wchar_t fulldictpath[MAX_PATH_NAME+1];
    wchar_t dummy1[MAX_PATH_NAME+1];
    wchar_t *dp1, *dp2;
    wchar_t *pos, *oldpos;
    int filenamelen, len;
    FILE *fp;

    if (filename[0] == L'/') {
	return _wfopen(filename, how);  /* If the file does not exist NULL is returned */
    }

    dp1 = L"";
    /*    if (dictname != NULL && dictname[0] == '/') { */
    if (dictname != NULL) {
	//safe_strcpy(dummy1, dictname, sizeof(dummy1));
	safe_strcpy(dummy1, dictname, MAX_PATH_NAME+1);
	pos = wcsrchr(dummy1, L'/');
	if (pos != NULL) {
	    *pos = L':';
	    *(pos+1) = L'\0';
	    dp1 = dummy1;
	}
    }
    /* dp1 now points to the part of the dictpath due to dictname */
    
    dp2 = L"";
    /*  We're no longer using the dictpath in the environment
    if (wcslen(getenv(DICTPATH)) > 0) {
	sprintf(dummy2, "%s:", getenv(DICTPATH));
	dp2 = dummy2;
    }
    */
    /* dp2 now points to the part of the dictpath due to the environment var */

    swprintf_s(fulldictpath, MAX_PATH_NAME+1, L"%s%s%s", dp1, dp2, DEFAULTPATH);
    /* now fulldictpath is our dictpath, where each entry is followed by a ":"
       including the last one */
    
    filenamelen = wcslen(filename);
    len = wcslen(fulldictpath)+ filenamelen + 1 + 1;
    oldpos = fulldictpath;
    while ((pos = wcschr(oldpos, L':')) != NULL) {
		wcsncpy(completename, oldpos, (pos-oldpos));
		*(completename+(pos-oldpos)) = L'/';
		wcscpy(completename+(pos-oldpos)+1,filename);
	
		if ((fp = _wfopen(completename, how)) != NULL) {
			wprintf_s(L"   Opening %s\n", completename); 
			return fp;
		}
		oldpos = pos+1;
    }
    return NULL;
}

static int random_state[2] = {0,0};
static int random_count = 0;
static int random_inited = FALSE;

int step_generator(int d) {
    /* no overflow should occur, so this is machine independent */
    random_state[0] = ((random_state[0] * 3) + d + 104729) % 179424673;
    random_state[1] = ((random_state[1] * 7) + d + 48611) % 86028121;
    return random_state[0] + random_state[1];;
}

void my_random_initialize(int seed) {
    assert(!random_inited, L"Random number generator not finalized.");
    
    seed = (seed < 0) ? -seed : seed;
    seed = seed % (1 << 30);

    random_state[0] = seed % 3;
    random_state[1] = seed % 5;
    random_count = seed;
    random_inited = TRUE;
}

void my_random_finalize() {
    assert(random_inited, L"Random number generator not initialized.");
    random_inited = FALSE;
}

int my_random(void) {
    random_count++;
    return (step_generator(random_count));
}


/* Here's the connector set data structure */

#if 0
/* this is defined in parser_api.h */
typedef struct {
    Connector ** hash_table;
    int          table_size;
    int          is_defined;  /* if 0 then there is no such set */
} Connector_set;
#endif

int connector_set_hash(Connector_set *conset, wchar_t * s, int d) {
/* This hash function only looks at the leading upper case letters of
   the string, and the direction, '+' or '-'.
*/
    int i;
    for(i=d; iswupper((wint_t)*s); s++) i = i + (i<<1) + randtable[(*s + i) & (RTSIZE-1)];
    return (i & (conset->table_size-1));
}

void build_connector_set_from_expression(Connector_set * conset, Exp * e) {
    E_list * l;
    Connector * c;
    int h;
    if (e->type == CONNECTOR_type) {
	c = init_connector((Connector *) xalloc(sizeof(Connector)));
	c->string = e->u.string;
	c->label = NORMAL_LABEL;        /* so we can use match() */
	c->priority = THIN_priority;
	c->word = e->dir;     /* just use the word field to give the dir */
	h = connector_set_hash(conset, c->string, c->word);
	c->next = conset->hash_table[h];
	conset->hash_table[h] = c;
    } else {
	for (l=e->u.l; l!=NULL; l=l->next) {
	    build_connector_set_from_expression(conset, l->e);
	}
    }
}	

Connector_set * connector_set_create(Exp *e) {
    int i;
    Connector_set *conset;

    conset = (Connector_set *) xalloc(sizeof(Connector_set));
    conset->table_size = next_power_of_two_up(size_of_expression(e));
    conset->hash_table =
      (Connector **) xalloc(conset->table_size * sizeof(Connector *));
    for (i=0; i<conset->table_size; i++) conset->hash_table[i] = NULL;
    build_connector_set_from_expression(conset, e);
    return conset;
}

void connector_set_delete(Connector_set * conset) {
    int i;
    if (conset == NULL) return;
    for (i=0; i<conset->table_size; i++) free_connectors(conset->hash_table[i]);
    xfree(conset->hash_table, conset->table_size * sizeof(Connector *));
    xfree(conset, sizeof(Connector_set));
}

int match_in_connector_set(Connector_set *conset, Connector * c, int d) {
/* Returns TRUE the given connector is in this conset.  FALSE otherwise.
   d='+' means this connector is on the right side of the disjunct.
   d='-' means this connector is on the left side of the disjunct.
*/
    int h;
    Connector * c1;
    if (conset == NULL) return FALSE;
    h = connector_set_hash(conset, c->string, d);
    for (c1=conset->hash_table[h]; c1!=NULL; c1 = c1->next) {
	if (x_match(c1, c) && (d == c1->word)) return TRUE;
    }
    return FALSE;
}

Dict_node * list_whole_dictionary(Dict_node *root, Dict_node *dn) {
    Dict_node *c, *d;
    if (root == NULL) return dn;
    c = (Dict_node *) xalloc(sizeof(Dict_node));
    *c = *root;
    d = list_whole_dictionary(root->left, dn);
    c->right = list_whole_dictionary(root->right, d);
    return c;
}

void free_listed_dictionary(Dict_node *dn) {
    Dict_node * dn2;
    while(dn!=NULL) {
      dn2=dn->right;
      xfree(dn, sizeof(Dict_node));
      dn=dn2;
    }
}

int easy_match(wchar_t * s, wchar_t * t) {

    /* This is like the basic "match" function in count.c - the basic connector-matching 
       function used in parsing - except it ignores "priority" (used to handle fat links) */

    while(iswupper((wint_t)*s) || iswupper((wint_t)*t)) {
	if (*s != *t) return FALSE;
	s++;
	t++;
    }

    while ((*s!=L'\0') && (*t!=L'\0')) {
      if ((*s == L'*') || (*t == L'*') ||
	  ((*s == *t) && (*s != L'^'))) {
	s++;
	t++;
      } else return FALSE;
    }
    return TRUE;

}

int word_has_connector(Dict_node * dn, wchar_t * cs, int direction) {

  /* This function takes a dict_node (corresponding to an entry in a given dictionary), a
     string (representing a connector), and a direction (0 = right-pointing, 1 = left-pointing);
     it returns 1 if the dictionary expression for the word includes the connector, 0 otherwise.
     This can be used to see if a word is in a certain category (checking for a category 
     connector in a table), or to see if a word has a connector in a normal dictionary. The
     connector check uses a "smart-match", the same kind used by the parser. */

    Connector * c2 = NULL;
    Disjunct * d, *d0;
    if(dn == NULL)
		return -1;
    
	d0 = d = build_disjuncts_for_dict_node(dn);
    if(d == NULL)
		return 0;
    
	for(; d != NULL; d = d->next) { 
		if(direction == 0)
			c2 = d->right;
		else if(direction == 1)
			c2 = d->left;
		
		for(; c2 != NULL; c2 = c2->next) {
			if(easy_match(c2->string, cs) == 1) {
				free_disjuncts(d0);
				return 1;
			}
		}
    }
    
	free_disjuncts(d0);
    return 0;
}
