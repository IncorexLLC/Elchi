#ifndef _UTILITIESH_
#define _UTILITIESH_

void safe_strcpy(wchar_t *u, wchar_t * v, int usize);
void safe_strcat(wchar_t *u, wchar_t *v, int usize);
void xfree(void *, int);
void exfree(void *, int);
void free_disjuncts(Disjunct *);
void free_X_nodes(X_node *);
void free_connectors(Connector *);
void init_randtable(void);
int  next_power_of_two_up(int);
int  upper_case_match(wchar_t *, wchar_t *);
void free_Exp(Exp *);
void free_E_list(E_list *);
int  size_of_expression(Exp *);
void left_print_string(FILE* fp, wchar_t *, wchar_t *);

void my_random_initialize(int seed);
void my_random_finalize();
int  my_random(void);

/* routines for copying basic objects */
void *      xalloc(int);
void *      exalloc(int);
Disjunct *  copy_disjunct(Disjunct * );
void        exfree_connectors(Connector *);
Link        excopy_link(Link);
void        exfree_link(Link);
Connector * copy_connectors(Connector *);
Connector * excopy_connectors(Connector * c);
Disjunct *  catenate_disjuncts(Disjunct *, Disjunct *);
X_node *    catenate_X_nodes(X_node *, X_node *);
Exp *       copy_Exp(Exp *);

Connector * init_connector(Connector *c);

/* utility routines moved from parse.c */
int sentence_contains(Sentence sent, wchar_t * s);
void set_is_conjunction(Sentence sent);
int sentence_contains_conjunction(Sentence sent);
int conj_in_range(Sentence sent, int lw, int rw);
int max_link_length(Connector * c);
void string_delete(wchar_t * p);

/* Connector_set routines */
Connector_set * connector_set_create(Exp *e);
void connector_set_delete(Connector_set * conset);
int match_in_connector_set(Connector_set *conset, Connector * c, int d);

#define DICTPATH L"DICTPATH" /* PATH environ variable for dictionary files */
FILE *dictopen(wchar_t *dictname, wchar_t *filename, wchar_t *how);
FILE *ppopen(wchar_t *);

Dict_node * list_whole_dictionary(Dict_node *, Dict_node *);
int word_has_connector(Dict_node *, wchar_t *, int);
int my_match(wchar_t *, wchar_t *);

#endif
