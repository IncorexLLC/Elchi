 /****************************************************************************/
 /*                                                                          */
 /*  Copyright (C) 1991-2000, all rights reserved                            */
 /*  Daniel Sleator, David Temperley, and John Lafferty                      */
 /*  See file "README" for information about commercial use of this system   */
 /*                                                                          */
 /****************************************************************************/

#include "link-includes.h"
     
int contains_underbar(wchar_t * s) {
    /* Returns TRUE if the string contains an underbar character.
     */
    while(*s != L'\0') {
	if (*s == L'_') return TRUE;
	s++;
    }
    return FALSE;
}

int is_idiom_string(wchar_t * s) {
    /* Returns FALSE if it is not a correctly formed idiom string.
       correct such string:
       () contains no "."
       () non-empty strings separated by _
       */
    wchar_t * t;
    for (t=s; *t != L'\0'; t++) {
	if (*t == L'.') {
	    return FALSE;
	}
    }
    if ((s[0] == L'_') || (s[wcslen(s)-1] == L'_')) {
	return FALSE;
    }
    for (t=s; *t != L'\0'; t++) {
	if ((*t == L'_') && (*(t+1) == L'_')) {
	    return FALSE;
	}
    }
    return TRUE;
}    

int is_number(wchar_t *s) {
    /* return TRUE if the string s is a sequence of digits. */
    while(*s != L'\0') {
	if (!iswdigit((wint_t)*s)) return FALSE;
	s++;
    }
    return TRUE;
}

int numberfy(wchar_t * s) {
    /* if the string contains a single ".", and ends in ".Ix" where
       x is a number, return x.  Return -1 if not of this form.
       */
    for (; (*s != L'\0') && (*s != L'.'); s++)
      ;
    if (*s++ != L'.') return -1;
    if (*s++ != L'I') return -1;
    if (!is_number(s)) return -1;
    return _wtoi(s);
}

int max_postfix_found(Dict_node * d) {
    /* Look for words that end in ".Ix" where x is a number.
       Return the largest x found.
       */
    int i, j;
    i = 0;
    while(d != NULL) {
	j = numberfy(d->string);
	if (j > i) i = j;
	d = d->right;
    }
    return i;
}

wchar_t * build_idiom_word_name(Dictionary dict, wchar_t * s) {
    /* Allocates string space and returns a pointer to it.
       In this string is placed the idiomized name of the given string s.
       This is the same as s, but with a postfix of ".Ix", where x is an
       appropriate number.  x is the minimum number that distinguishes
       this word from others in the dictionary.
       */
    wchar_t * new_s, * x, *id;
    int count, sz;

    count = max_postfix_found(dictionary_lookup(dict, s))+1;

    sz = wcslen(s)+10;
    new_s = x = (wchar_t *) xalloc(sizeof(wchar_t)*sz); /* fails if > 10**10 idioms */
    while((*s != L'\0') && (*s != L'.')) {
	*x = *s;
	x++;
	s++;
    }
    swprintf_s(x, sz - (x - new_s), L".I%d", count);

    id = string_set_add(new_s, dict->string_set);
    xfree(new_s, sizeof(wchar_t)*sz);
    return id;
}

Dict_node * make_idiom_Dict_nodes(Dictionary dict, wchar_t * string) {
    /* Tear the idiom string apart.
       Destroys the string s, but does not free it.
       Put the parts into a list of Dict_nodes (connected by their right pointers)
       Sets the string fields of these Dict_nodes pointing to the
       fragments of the string s.  Later these will be replaced by
       correct names (with .Ix suffixes).
       The list is reversed from the way they occur in the string.
       A pointer to this list is returned.
       */
    Dict_node * dn, * dn_new;
    wchar_t * t, *s, *p;
    int more, sz;
    dn = NULL;

    sz = wcslen(string)+1;
    p = s = xalloc(sizeof(wchar_t)*sz);
    wcscpy(s, string);

    while (*s != L'\0') {
	t = s;
    	while((*s != L'\0') && (*s != L'_')) s++;
	if (*s == L'_') {
	    more = TRUE;
	    *s = L'\0';
	} else {
	    more = FALSE;
	}
	dn_new = (Dict_node *) xalloc(sizeof (Dict_node));
	dn_new->right = dn;
	dn = dn_new;
	dn->string = string_set_add(t, dict->string_set);
	dn->file = NULL;
	if (more) s++;
    }

    xfree(p, sizeof(wchar_t)*sz);// added sizeof(wchar_t)*
    return dn;
}

static wchar_t current_name[] = L"AAAAAAAA";
#define CN_size wcslen(current_name)

void increment_current_name(void) {
    int i, carry;
    i = CN_size-1;
    carry = 1;
    while (carry == 1) {
	current_name[i]++;
	if (current_name[i] == L'Z'+1) {
	    current_name[i] = L'A';
	    carry = 1;
	} else {
	    carry = 0;
	}
	i--;
    }
}

wchar_t * generate_id_connector(Dictionary dict) {
    /* generate a new connector name
       obtained from the current_name
       allocate string space for it.
       return a pointer to it.
     */
    int i, sz;
    wchar_t * t, * s, *id;
    
    for (i=0; current_name[i] == L'A'; i++)
      ;
    /* i is now the number of characters of current_name to skip */
    sz = CN_size - i + 2 + 1 + 1;
    s = t = (wchar_t *) xalloc(sizeof(wchar_t)*sz);
    *t++ = L'I';
    *t++ = L'D';
    for (; i < CN_size; i++ ) {
	*t++ = current_name[i] ;
    }
    *t++ = L'\0';
    id = string_set_add(s, dict->string_set);
    xfree(s, sizeof(wchar_t)*sz);
    return id;
}

void insert_idiom(Dictionary dict, Dict_node * dn) {
    /* Takes as input a pointer to a Dict_node.
       The string of this Dict_node is an idiom string.
       This string is torn apart, and its components are inserted into the
       dictionary as special idiom words (ending in .I*, where * is a number).
       The expression of this Dict_node (its node field) has already been
       read and constructed.  This will be used to construct the special idiom
       expressions.
       The given dict node is freed.  The string is also freed.
       */
    Exp * nc, * no, * n1;
    E_list *ell, *elr;
    wchar_t * s;
    int s_length;
    Dict_node * dn_list, * xdn, * start_dn_list;
    
    no = dn->exp;
    s = dn->string;
    s_length = wcslen(s);
    
    if (!is_idiom_string(s)) {
	wprintf_s(L"*** Word \"%s\" on line %d is not",s, dict->line_number);
	wprintf_s(L" a correctly formed idiom string.\n");
	wprintf_s(L"    This word will be ignored\n");
	/* xfree((wchar_t *)s, s_length+1);  strings are handled now by string_set */
	xfree((wchar_t *)dn, sizeof (Dict_node));
	return;
    }
    
    dn_list = start_dn_list = make_idiom_Dict_nodes(dict, s);
    xfree((wchar_t *)dn, sizeof (Dict_node));
    
    if (dn_list->right == NULL) {
      error(L"Idiom string with only one connector -- should have been caught");
    }
    
    /* first make the nodes for the base word of the idiom (last word) */
    /* note that the last word of the idiom is first in our list */
    
    /* ----- this code just sets up the node fields of the dn_list ----*/
    nc = Exp_create(dict);
    nc->u.string = generate_id_connector(dict);
    nc->dir = L'-';
    nc->multi = FALSE;
    nc->type = CONNECTOR_type;
    nc->cost = 0;
    
    n1 = Exp_create(dict);
    n1->u.l = ell = (E_list *) xalloc(sizeof(E_list));
    ell->next = elr = (E_list *) xalloc(sizeof(E_list));
    elr->next = NULL;
    ell->e = nc;
    elr->e = no;
    n1->type = AND_type;
    n1->cost = 0;

    dn_list->exp = n1;
    
    dn_list = dn_list->right;
    
    while(dn_list->right != NULL) {
	/* generate the expression for a middle idiom word */
	
	n1 = Exp_create(dict);
	n1->u.string = NULL;
	n1->type = AND_type;
	n1->cost = 0;
	n1->u.l = ell = (E_list *) xalloc(sizeof(E_list));
	ell->next = elr = (E_list *) xalloc(sizeof(E_list));
	elr->next = NULL;
	
	nc = Exp_create(dict);
	nc->u.string = generate_id_connector(dict);
	nc->dir = L'+';
	nc->multi = FALSE;
	nc->type = CONNECTOR_type;
	nc->cost = 0;
	elr->e = nc;
	
	increment_current_name();
	
	nc = Exp_create(dict);
	nc->u.string = generate_id_connector(dict);
	nc->dir = L'-';
	nc->multi = FALSE;
	nc->type = CONNECTOR_type;
	nc->cost = 0;

	ell->e = nc;
	
	dn_list->exp = n1;
	
	dn_list = dn_list->right;
    }
    /* now generate the last one */
    
    nc = Exp_create(dict);
    nc->u.string = generate_id_connector(dict);
    nc->dir = L'+';
    nc->multi = FALSE;
    nc->type = CONNECTOR_type; 
    nc->cost = 0;
    
    dn_list->exp = nc;
    
    increment_current_name();
    
    /* ---- end of the code alluded to above ---- */
    
    /* now its time to insert them into the dictionary */
    
    dn_list = start_dn_list;
    
    while (dn_list != NULL) {
	xdn = dn_list->right;
	dn_list->left = dn_list->right = NULL;
        dn_list->string = build_idiom_word_name(dict, dn_list->string);
	dict->root = insert_dict(dict, dict->root, dn_list);
	dict->num_entries++;
	dn_list = xdn;
    }
    /* xfree((wchar_t *)s, s_length+1); strings are handled by string_set */
}

int is_idiom_word(wchar_t * s) {
    /* returns TRUE if this is a word ending in ".Ix", where x is a number. */
    return (numberfy(s) != -1) ;
}

/*
  int only_idiom_words(Dict_node * dn) {
  returns TRUE if the list of words contains only words that are
  idiom words.  This is useful, because under this condition you want
   to be able to insert the word anyway, as long as it doesn't match
   exactly.

    while(dn != NULL) {
	if (!is_idiom_word(dn->string)) return FALSE;
	dn = dn->right;
    }
    return TRUE;
}
*/