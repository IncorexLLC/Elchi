 /****************************************************************************/
 /*                                                                          */
 /*  Copyright (C) 1991-2000, all rights reserved                            */
 /*  Daniel Sleator, David Temperley, and John Lafferty                      */
 /*  See file "README" for information about commercial use of this system   */
 /*                                                                          */
 /****************************************************************************/

#include "link-includes.h"

wchar_t * get_a_word(Dictionary dict, FILE * fp) {
/* Reads in one word from the file, allocates space for it,
   and returns it.
*/
    wchar_t word[MAX_WORD+1];
    wchar_t * s;
    int j;
	wint_t c;

    do {
		c = getwc(fp);
    } while ((c != WEOF) && iswspace(c));
    
	if (c == WEOF) return NULL;

    for (j=0; (j <= MAX_WORD-1) && (!iswspace(c)) && (c != WEOF); j++) {
		word[j] = c;
		c = getwc(fp);
    }

    if (j == MAX_WORD) {
		error(L"The dictionary contains a word that is too long.");
    }
    word[j] = L'\0';
    s = string_set_add(word, dict->string_set);
    return s;
}

Dict_node * read_word_file(Dictionary dict, Dict_node * dn, wchar_t * filename) {
/*

   (1) opens the word file and adds it to the word file list
   (2) reads in the words
   (3) puts each word in a Dict_node
   (4) links these together by their left pointers at the
       front of the list pointed to by dn
   (5) returns a pointer to the first of this list

*/
    Dict_node * dn_new;
    Word_file * wf;
    FILE * fp;
    wchar_t * s;
    wchar_t file_name_copy[MAX_PATH_NAME+1];

    safe_strcpy(file_name_copy, filename+1, MAX_PATH_NAME+1); /* get rid of leading '/' */

    if ((fp = dictopen(dict->name, file_name_copy, L"r")) == NULL) {
		lperror(WORDFILE, L"%s\n", file_name_copy);
		return NULL;
    }

    /*fwprintf("   Reading \"%s\"\n", file_name_copy);*/
    /*fwprintf("*"); fflush(stdout);*/

    wf = (Word_file *) xalloc(sizeof (Word_file));
    safe_strcpy(wf->file, file_name_copy, MAX_PATH_NAME+1);
    wf->changed = FALSE;
    wf->next = dict->word_file_header;
    dict->word_file_header = wf;

    while ((s = get_a_word(dict, fp)) != NULL) {
		dn_new = (Dict_node *) xalloc(sizeof(Dict_node));
		dn_new->left = dn;
		dn = dn_new;
		dn->string = s;
		dn->file = wf;
    }
    fclose(fp);
    return dn;
}

#define LINE_LIMIT 70
int xwhere_in_line;

void routput_dictionary(Dict_node * dn, FILE *fp, Word_file *wf) {
/* scan the entire dictionary rooted at dn and print into the file
   pf all of the words whose origin is the file wf.
*/
    if (dn == NULL) return;
    routput_dictionary(dn->left, fp, wf);
    if (dn->file == wf) {
	if (xwhere_in_line+wcslen(dn->string) > LINE_LIMIT) {
	    xwhere_in_line = 0;
	    fwprintf_s(fp,L"\n");
	}
	xwhere_in_line += wcslen(dn->string) + 1;
	fwprintf_s(fp,L"%s ", dn->string);
    }
    routput_dictionary(dn->right, fp, wf);
}

void output_dictionary(Dict_node * dn, FILE *fp, Word_file *wf) {
    xwhere_in_line = 0;
    routput_dictionary(dn, fp, wf);
    fwprintf_s(fp,L"\n");
}

void save_files(Dictionary dict) {
    Word_file *wf;
    FILE *fp;
    for (wf = dict->word_file_header; wf != NULL; wf = wf->next) {
	if (wf->changed) {
	    if ((fp = _wfopen(wf->file, L"w")) == NULL) {
	     wprintf_s(L"\nCannot open %s. Gee, this shouldn't happen.\n", wf->file);
             wprintf_s(L"file not saved\n");
	     return;
	    }
	    wprintf_s(L"   saving file \"%s\"\n", wf->file);
	    /*output_dictionary(dict_root, fp, wf);*/
	    fclose(fp);
	    wf->changed = FALSE;
	}
    }
}

int files_need_saving(Dictionary dict) {
    Word_file *wf;
    for (wf = dict->word_file_header; wf != NULL; wf = wf->next) {
	if (wf->changed) return TRUE;
    }
    return FALSE;
}