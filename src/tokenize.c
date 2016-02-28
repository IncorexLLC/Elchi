 /****************************************************************************/
 /*                                                                          */
 /*  Copyright (C) 1991-2000, all rights reserved                            */
 /*  Daniel Sleator, David Temperley, and John Lafferty                      */
 /*  See file "README" for information about commercial use of this system   */
 /*                                                                          */
 /****************************************************************************/

#include "link-includes.h"

#define MAX_STRIP 10

int post_quote[MAX_SENTENCE];
/*static wchar_t * strip_left[] = {"(", "$", "``", NULL}; */
/*static wchar_t * strip_right[] = {")", "%", ",", ".", ":", ";", "?", "!", "''", "'", "'s", NULL};*/

static int is_initials_word(wchar_t * word) {
    /* This is rather esoteric and not terribly important.
       It returns TRUE if the word matches the pattern /[A-Z]\.]+/
       */
    int i;
    for (i=0; word[i] != L'\0'; i += 2) {
	if (!iswupper((wint_t)word[i])) return FALSE;
	if (word[i+1] != L'.') return FALSE;
    }
    return TRUE;
}

static int is_number(wchar_t * s) {
    if (!iswdigit((wint_t)*s)) return FALSE;
    for (; *s != L'\0'; ++s) {
	if (iswdigit((wint_t)*s) || (*s == L'.') || (*s == L',') || (*s == L':')) continue;
	/* The ":" is included here so we allow "10:30" to be a number. */
	return FALSE;
    }
    return TRUE;
}

static int ishyphenated(wchar_t * s) {
/* returns TRUE iff it's an appropriately formed hyphenated word.
   This means all letters, numbers, or hyphens, not beginning and
   ending with a hyphen.
*/
    int hyp, nonalpha;
    hyp = nonalpha = 0;
    if (*s == L'-') return FALSE;
    
	while (*s != L'\0') {
		if (!iswalpha((wint_t)*s) && !iswdigit((wint_t)*s) && (*s != L'.') && (*s != L',') && (*s != L'-')) return FALSE;
		if (*s == L'-') ++hyp;
		++s;
    }
    return ((*(s-1)!=L'-') && (hyp>0));
}

static int is_ing_word(wchar_t * s) {

    int i=0;
    for (; *s != L'\0'; ++s) ++i;
    
	if(i < 5) return FALSE;
    if(wcsncmp(L"ing", s - 3, 3) == 0) return TRUE; 
    return FALSE;
}

static int is_s_word(wchar_t * s) {

    for (; *s != L'\0'; ++s);
    s--;
    if(*s != L's') return FALSE;
    s--;
    if(*s == L'i' || *s == L'u' || *s == L'o' || *s == L'y' || *s == L's') return FALSE;
    return TRUE;
}

static int is_ed_word(wchar_t * s) {

    int i=0;
    for (; *s != L'\0'; ++s) ++i;
    if(i<4) return FALSE;
    if(wcsncmp(L"ed", s-2, 2)==0) return TRUE; 
    return FALSE;
}

static int is_ly_word(wchar_t * s) {

    int i=0;
    for (; *s != L'\0'; ++s) ++i;
    if(i<4) return FALSE;
    if(wcsncmp(L"ly", s-2, 2)==0) return TRUE; 
    return FALSE;
}

static int issue_sentence_word(Sentence sent, wchar_t * s) {
    /* the string s is the next word of the sentence do not issue the empty
     * string.  return false if too many words or the word is too long.  */
    if (*s == L'\0') return TRUE;
    if (wcslen(s) > MAX_WORD) {
	lperror(SEPARATE, 
		L". The word \"%s\" is too long.\n"  
		L"A word can have a maximum of %d characters.\n", s, MAX_WORD);
	return FALSE;
    }

    if (sent->length == MAX_SENTENCE) {
	lperror(SEPARATE, L". The sentence has too many words.\n");
	return FALSE;
    }
    wcscpy(sent->word[sent->length].string, s);
    /* Now we record whether the first character of the word is upper-case. 
       (The first character may be made lower-case
       later, but we may want to get at the original version) */
    if(iswupper((wint_t)s[0])) sent->word[sent->length].firstupper=1;
    else sent->word[sent->length].firstupper = 0; 
    ++sent->length;
    return TRUE;
}

/*
    Here's a summary of how subscripts are handled:

    Reading the dictionary:

      If the last "." in a string is followed by a non-digit character,
      then the "." and everything after it is considered to be the subscript
      of the word.

      The dictionary reader does not allow you to have two words that
      match according to the criterion below.  (so you can't have
      "dog.n" and "dog")

      Quote marks are used to allow you to define words in the dictionary
      which would otherwise be considered part of the dictionary, as in

       ";": {@Xca-} & Xx- & (W+ or Qd+) & {Xx+};
       "%" : (ND- & {DD-} & <noun-sub-x> & 
	   (<noun-main-x> or B*x+)) or (ND- & (OD- or AN+));

    Rules for chopping words from the input sentence:

       First the prefix chars are stripped off of the word.  These
       characters are "(" and "$" (and now "``")

       Now, repeat the following as long as necessary:

	   Look up the word in the dictionary.
	   If it's there, the process terminates.

	   If it's not there and it ends in one of the right strippable
	   strings (see "right_strip") then remove the strippable string
	   and make it into a separate word.

	   If there is no strippable string, then the process terminates. 

    Rule for defining subscripts in input words:

       The subscript rule is followed just as when reading the dictionary.

    When does a word in the sentence match a word in the dictionary?

       Matching is done as follows: Two words with subscripts must match
       exactly.  If neither has a subscript they must match exactly.  If one
       does and one doesn't then they must match when the subscript is
       removed.  Notice that this is symmetric.

    So, under this system, the dictonary could have the words "Ill" and
    also the word "Ill."  It could also have the word "i.e.", which could be
    used in a sentence.
*/



static int separate_word(Sentence sent, wchar_t *w, wchar_t *wend, int is_first_word, int quote_found) {
    /* w points to a string, wend points to the wchar_t one after the end.  The
     * "word" w contains no blanks.  This function splits up the word if
     * necessary, and calls "issue_sentence_word()" on each of the resulting
     * parts.  The process is described above.  returns TRUE of OK, FALSE if
     * too many punctuation marks */
    int i, j, k, l, len;
    int r_strippable=0, l_strippable=0, s_strippable=0, p_strippable=0, n_r_stripped, s_stripped,
      word_is_in_dict, s_ok;
    int r_stripped[MAX_STRIP];  /* these were stripped from the right */
    wchar_t ** strip_left=NULL;
    wchar_t ** strip_right=NULL;
    wchar_t ** prefix=NULL;
    wchar_t ** suffix=NULL;
    wchar_t word[MAX_WORD+1];
    wchar_t newword[MAX_WORD+1];
    Dict_node * dn, * dn2, * start_dn;
    wchar_t * rpunc_con = L"RPUNC";
    wchar_t * lpunc_con = L"LPUNC";
    wchar_t * suf_con = L"SUF";
    wchar_t * pre_con = L"PRE";

    if(sent->dict->affix_table!=NULL) {

      start_dn = list_whole_dictionary(sent->dict->affix_table->root, NULL);
      for(dn=start_dn; dn!=NULL; dn=dn->right) {
	if(word_has_connector(dn, rpunc_con, 0)) ++r_strippable;
	if(word_has_connector(dn, lpunc_con, 0)) ++l_strippable;
	if(word_has_connector(dn, suf_con, 0)) ++s_strippable;
	if(word_has_connector(dn, pre_con, 0)) ++p_strippable;
      }
      strip_right = (wchar_t **) xalloc(r_strippable * sizeof(wchar_t *));
      strip_left = (wchar_t **) xalloc(l_strippable * sizeof(wchar_t *));
      suffix = (wchar_t **) xalloc(s_strippable * sizeof(wchar_t *));
      prefix = (wchar_t **) xalloc(p_strippable * sizeof(wchar_t *));
      
      i=0;
      j=0;
      k=0;
      l=0;
      dn=start_dn;
      while(dn!=NULL) {
	if(word_has_connector(dn, rpunc_con, 0)) {
	  strip_right[i] = dn->string;
	  ++i;
	}
	if(word_has_connector(dn, lpunc_con, 0)) {
	  strip_left[j] = dn->string;
	  ++j;
	}
	if(word_has_connector(dn, suf_con, 0)) {
	  suffix[k] = dn->string;
	  ++k;
	}
	if(word_has_connector(dn, pre_con, 0)) {
	  prefix[l] = dn->string;
	  ++l;
	}
	dn2=dn->right;
	xfree(dn, sizeof(Dict_node));
	dn=dn2;
      }
    }

    for (;;) {
	for (i=0; i<l_strippable; ++i) {
	    if (wcsncmp(w, strip_left[i], wcslen(strip_left[i])) == 0) {
		if (!issue_sentence_word(sent, strip_left[i])) return FALSE;
		w += wcslen(strip_left[i]);
		break;
	    }
	}
	if (i==l_strippable) break;
    }

    /* Now w points to the string starting just to the right of any left-stripped characters. */
    /* stripped[] is an array of numbers, indicating the index numbers (in the strip_right array) of any
       strings stripped off; stripped[0] is the number of the first string stripped off, etc. When it
       breaks out of this loop, n_stripped will be the number of strings stripped off. */

    for (n_r_stripped = 0; n_r_stripped < MAX_STRIP; ++n_r_stripped) {

	wcsncpy(word, w, wend-w);
	word[wend-w] = L'\0';
	if (wend == w) break;  /* it will work without this */
	
	if (boolean_dictionary_lookup(sent->dict, word) || is_initials_word(word)) break;
	if (is_first_word && iswupper((wint_t)word[0])) {
	  /* This should happen if it's a word after a colon, also! */
	    word[0] = tolower(word[0]);
	    if (boolean_dictionary_lookup(sent->dict, word)) {
		word[0] = toupper(word[0]);  /* restore word to what it was */
		break;
	    }
	    word[0] = toupper(word[0]);
	}
	for (i=0; i < r_strippable; ++i) {
	    len = wcslen(strip_right[i]);
	    if ((wend-w) < len) continue;  /* the remaining w is too short for a possible match */
	    if (wcsncmp(wend-len, strip_right[i], len) == 0) {
		r_stripped[n_r_stripped] = i;
		wend -= len;
		break;
	    }
	}
	if(i==r_strippable) break;
    }

    /* Now we strip off suffixes...w points to the remaining word, "wend" to the end of the word. */

    s_stripped = -1;
    wcsncpy(word, w, wend-w);
    word[wend-w] = L'\0';
    word_is_in_dict=0;

    if (boolean_dictionary_lookup(sent->dict, word) || is_initials_word(word)) word_is_in_dict=1;
    if (is_first_word && iswupper((wint_t)word[0])) {
      word[0] = tolower(word[0]);
      if (boolean_dictionary_lookup(sent->dict, word)) word_is_in_dict=1;
      word[0] = toupper(word[0]);
    }
    if(word_is_in_dict==0) {
      j=0;
      for (i=0; i <= s_strippable; ++i) {
	s_ok = 0;
	/* Go through once for each suffix; then go through one final time for the no-suffix case */
	if(i < s_strippable) {
	  len = wcslen(suffix[i]);
	  if ((wend-w) < len) continue;  /* the remaining w is too short for a possible match */
	  if (wcsncmp(wend-len, suffix[i], len) == 0) s_ok=1;
	  	}
	else len=0;

	if(s_ok==1 || i==s_strippable) {
	  
	  wcsncpy(newword, w, (wend-len)-w);
	  newword[(wend-len)-w] = L'\0';

	  /* Check if the remainder is in the dictionary; for the no-suffix case, it won't be */	  
	  if (boolean_dictionary_lookup(sent->dict, newword)) {
	    if(verbosity>1) if(i< s_strippable) wprintf_s(L"Splitting word into two: %s-%s\n", newword, suffix[i]); 
	    s_stripped = i;
	    wend -= len;
	    wcsncpy(word, w, wend-w);
	    word[wend-w] = L'\0';
	    break;
	  }

	  /* If the remainder isn't in the dictionary, try stripping off prefixes */
	  else {
	    for (j=0; j<p_strippable; ++j) {
	      if (wcsncmp(w, prefix[j], wcslen(prefix[j])) == 0) {
		wcsncpy(newword, w+wcslen(prefix[j]), (wend-len)-(w+wcslen(prefix[j])));
		newword[(wend-len)-(w+wcslen(prefix[j]))]=L'\0';
		if(boolean_dictionary_lookup(sent->dict, newword)) {
		  if(verbosity>1) if(i < s_strippable) wprintf_s(L"Splitting word into three: %s-%s-%s\n", prefix[j], newword, suffix[i]); 
		  if (!issue_sentence_word(sent, prefix[j])) return FALSE;
		  if(i < s_strippable) s_stripped = i;
		  wend -= len;
		  w += wcslen(prefix[j]);
		  wcsncpy(word, w, wend-w);
		word[wend-w] = L'\0';
		break;
		}
	      }
	    }
	  }
	  if(j!=p_strippable) break;
	}
      }
    }
    
    /* word is now what remains after all the stripping has been done */

    /*    
    if (n_stripped == MAX_STRIP) {
	lperror(SEPARATE, 
		".\n\"%s\" is followed by too many punctuation marks.\n", word);
	return FALSE;
    } */

    if (quote_found==1) post_quote[sent->length]=1;

    if (!issue_sentence_word(sent, word)) return FALSE;

    if(s_stripped != -1) {
      if (!issue_sentence_word(sent, suffix[s_stripped])) return FALSE;
    }

    for (i=n_r_stripped-1; i>=0; i--) {
	if (!issue_sentence_word(sent, strip_right[r_stripped[i]])) return FALSE;
    }

    if(sent->dict->affix_table!=NULL) {
      xfree(strip_right, r_strippable * sizeof(wchar_t *));
      xfree(strip_left, l_strippable * sizeof(wchar_t *));
      xfree(suffix, s_strippable * sizeof(wchar_t *));
      xfree(prefix, p_strippable * sizeof(wchar_t *));
    }
    return TRUE;
}

int separate_sentence(wchar_t * s, Sentence sent) {
    /* The string s has just been read in from standard input.
       This function breaks it up into words and stores these words in
       the sent->word[] array.  Returns TRUE if all is well, FALSE otherwise.
       Quote marks are treated just like blanks.
       */
    wchar_t *t;
    int i, is_first, quote_found;
    Dictionary dict = sent->dict;

    for(i=0; i<MAX_SENTENCE; ++i) post_quote[i]=0;
    sent->length = 0;
    
    if (dict->left_wall_defined) 
	if (!issue_sentence_word(sent, LEFT_WALL_WORD)) return FALSE;
    
    is_first = TRUE;
    for(;;) {
        quote_found = FALSE;
	while(iswspace((wint_t)*s) || (*s == L'\"')) {
	  ++s;
	  if(*s == L'\"') quote_found=TRUE;
	}
	if (*s == L'\0') break;
	for (t=s; !((iswspace((wint_t)*t) || (*t == L'\"')) || *t==L'\0'); ++t);
	if (!separate_word(sent, s, t, is_first, quote_found)) return FALSE;
	is_first = FALSE;
	s = t;
	if (*s == L'\0') break;
    }
    
    if (dict->right_wall_defined) 
	if (!issue_sentence_word(sent, RIGHT_WALL_WORD)) return FALSE;

    return (sent->length > dict->left_wall_defined + dict->right_wall_defined);
}

static int special_string(Sentence sent, int i, wchar_t * s) {
    X_node * e;
    if (boolean_dictionary_lookup(sent->dict, s)) {
	sent->word[i].x = build_word_expressions(sent, s);
	for (e = sent->word[i].x; e != NULL; e = e->next) {
	    e->string = sent->word[i].string;
	}
	return TRUE;
    } else {
	lperror(BUILDEXPR, L".\n To process this sentence your dictionary "
		L"needs the word \"%s\".\n", s);
	return FALSE;
    }
}

static int guessed_string(Sentence sent, int i, wchar_t * s, wchar_t * type) {
    X_node * e;
    wchar_t *t, *u;
    wchar_t str[MAX_WORD+1];
    if (boolean_dictionary_lookup(sent->dict, type)) {
      sent->word[i].x = build_word_expressions(sent, type);
      e = sent->word[i].x;
	  if(is_s_word(s)) {

	    for (; e != NULL; e = e->next) {
	      t = wcschr(e->string, L'.');
	      if (t != NULL) {
		swprintf_s(str, 61, L"%.50s[!].%.5s", s, t+1);
	      } else {
		swprintf_s(str, 61, L"%.50s[!]", s);
	      }
	      t = (wchar_t *) xalloc(sizeof(wchar_t)*(wcslen(str)+1));
	      wcscpy(t,str);
	      u = string_set_add(t, sent->string_set);
	      xfree(t, sizeof(wchar_t)*(wcslen(str)+1));
	      e->string = u;
	    }
	  }

	  else {
	    if(is_ed_word(s)) {
	      swprintf_s(str, 61, L"%.50s[!].v", s);
	    }
	    else if(is_ing_word(s)) {
	      swprintf_s(str, 61, L"%.50s[!].g", s);
	    }
	    else if(is_ly_word(s)) {
	      swprintf_s(str, 61, L"%.50s[!].e", s);
	    }
	    else swprintf_s(str, 61, L"%.50s[!]", s);

	    t = (wchar_t *) xalloc(sizeof(wchar_t)*(wcslen(str)+1));
	    wcscpy(t,str);
	    u = string_set_add(t, sent->string_set);
	    xfree(t, sizeof(wchar_t)*(wcslen(str)+1));
	    e->string = u;
	  }
	  return TRUE;

    } else {
	lperror(BUILDEXPR, L".\n To process this sentence your dictionary "
		L"needs the word \"%s\".\n", type);
	return FALSE;
    }
}

static void handle_unknown_word(Sentence sent, int i, wchar_t * s) {
  /* puts into word[i].x the expression for the unknown word */
  /* the parameter s is the word that was not in the dictionary */    
  /* it massages the names to have the corresponding subscripts */
  /* to those of the unknown words */
  /* so "grok" becomes "grok[?].v"  */
    wchar_t *t,*u;
    X_node *d;
    wchar_t str[MAX_WORD+1];
    
    sent->word[i].x = build_word_expressions(sent, UNKNOWN_WORD);
    if (sent->word[i].x == NULL) 
	assert(FALSE, L"UNKNOWN_WORD should have been there");
    
    for (d = sent->word[i].x; d != NULL; d = d->next) {
	t = wcschr(d->string, L'.');
	if (t != NULL) {
	    swprintf_s(str, 61, L"%.50s[?].%.5s", s, t+1);
	} else {
	    swprintf_s(str, 61, L"%.50s[?]", s);
	}
	t = (wchar_t *) xalloc(sizeof(wchar_t)*(wcslen(str)+1));
	wcscpy(t,str);
	u = string_set_add(t, sent->string_set);
	xfree(t, sizeof(wchar_t)*(wcslen(str)+1));
	d->string = u;
    }
}

int build_sentence_expressions(Sentence sent) {
    /* Corrects case of first word, fills in other proper nouns, and
       builds the expression lists for the resulting words.
       
       Algorithm:  
       Apply the following step to all words w:
       if w is in the dictionary, use it.
       else if w is upper case use PROPER_WORD disjuncts for w.
       else if it's hyphenated, use HYPHENATED_WORD
       else if it's a number, use NUMBER_WORD.
       
       Now, we correct the first word, w.
       if w is upper case, let w' be the lower case version of w.
       if both w and w' are in the dict, concatenate these disjncts.
       else if w' is in dict, use disjuncts of w'
       else leave the disjuncts alone
       */
    int i, first_word;  /* the index of the first word after the wall */
    wchar_t *s, *u, temp_word[MAX_WORD+1];
    X_node * e;
    Dictionary dict = sent->dict;


    if (dict->left_wall_defined) {
	first_word = 1;
    } else {
	first_word = 0;
    }
    
    /* the following loop treats all words the same 
       (nothing special for 1st word) */
    for (i=0; i<sent->length; ++i) {
	s = sent->word[i].string;
	if (boolean_dictionary_lookup(sent->dict, s)) {
	    sent->word[i].x = build_word_expressions(sent, s);
	} else if (iswupper((wint_t)s[0]) && is_s_word(s) && dict->pl_capitalized_word_defined) {
	    if (!special_string(sent, i, PL_PROPER_WORD)) return FALSE;
	} else if (iswupper((wint_t)s[0]) && dict->capitalized_word_defined) {
	    if (!special_string(sent, i, PROPER_WORD)) return FALSE;
	} else if (is_number(s) && dict->number_word_defined) {
	    /* we know it's a plural number, or 1 */
	    /* if the string is 1, we'll only be here if 1's not in the dictionary */
	    if (!special_string(sent, i, NUMBER_WORD)) return FALSE;
	} else if (ishyphenated(s) && dict->hyphenated_word_defined) {
	    /* singular hyphenated */
	    if (!special_string(sent, i, HYPHENATED_WORD)) return FALSE;
	} else if (is_ing_word(s) && dict->ing_word_defined) {
	    if (!guessed_string(sent, i, s, ING_WORD)) return FALSE;
	} else if (is_s_word(s) && dict->s_word_defined) {
	    if (!guessed_string(sent, i, s, S_WORD)) return FALSE;
	} else if (is_ed_word(s) && dict->ed_word_defined) {
	    if (!guessed_string(sent, i, s, ED_WORD)) return FALSE;
	} else if (is_ly_word(s) && dict->ly_word_defined) {
	    if (!guessed_string(sent, i, s, LY_WORD)) return FALSE;
	} else if (dict->unknown_word_defined && dict->use_unknown_word) {
	    handle_unknown_word(sent, i, s);
	} else {
	    /* the reason I can assert this is that the word
	       should have been looked up already if we get here. */
	    assert(FALSE, L"I should have found that word.");
	}
    }

    /* Under certain cases--if it's the first word of the sentence, or if it follows a colon
       or a quotation mark--a word that's capitalized has to be looked up as an uncapitalized 
       word (as well as a capitalized word). */


    for (i=0; i<sent->length; ++i) {
	if (! (i==first_word || (i>0 && wcscmp(L":", sent->word[i-1].string)==0) || post_quote[i]==1) ) continue;
	s = sent->word[i].string;
	if (iswupper((wint_t)s[0])) {
	    safe_strcpy(temp_word, s, MAX_WORD + 1);
	    temp_word[0] = tolower(temp_word[0]);
	    u = string_set_add(temp_word, sent->string_set);
	    /* If the lower-case version is in the dictionary... */
	    if (boolean_dictionary_lookup(sent->dict, u)) {
	        /* Then check if the upper-case version is there. If it is, the disjuncts for
		   the upper-case version have been put there already. So add on the disjuncts
		   for the lower-case version. */
		if (boolean_dictionary_lookup(sent->dict, s)) {
		    e = build_word_expressions(sent, u);
		    sent->word[i].x = 
			catenate_X_nodes(sent->word[i].x, e);
		} else {
		  /* If the upper-case version isn't there, replace the u.c. disjuncts with l.c. ones */
		    s[0] = tolower(s[0]);
		    e = build_word_expressions(sent, s);
		    free_X_nodes(sent->word[i].x);
		    sent->word[i].x = e;
		}
	    }
	}
    }
    
    return TRUE;
}


/* This just looks up all the words in the sentence, and builds
   up an appropriate error message in case some are not there.
   It has no side effect on the sentence.  Returns TRUE if all
   went well.
 */
int sentence_in_dictionary(Sentence sent){
    int w, ok_so_far;
    wchar_t * s;
    Dictionary dict = sent->dict;
    wchar_t temp[1024];

    ok_so_far = TRUE;
    for (w=0; w<sent->length; ++w) {
	s = sent->word[w].string;
	if (!boolean_dictionary_lookup(dict, s) &&
	    !(iswupper((wint_t)s[0])   && dict->capitalized_word_defined) &&
	    !(iswupper((wint_t)s[0]) && is_s_word(s) && dict->pl_capitalized_word_defined) &&
	    !(ishyphenated(s) && dict->hyphenated_word_defined)  &&
	    !(is_number(s)    && dict->number_word_defined) &&
	    !(is_ing_word(s)  && dict->ing_word_defined)  &&
	    !(is_s_word(s)    && dict->s_word_defined)  &&
	    !(is_ed_word(s)   && dict->ed_word_defined)  &&
	    !(is_ly_word(s)   && dict->ly_word_defined)) {
	    if (ok_so_far) {
		safe_strcpy(temp, L"The following words are not in the dictionary:", 1024);
		ok_so_far = FALSE;
	    }
	    safe_strcat(temp, L" \"", 1024);
	    safe_strcat(temp, sent->word[w].string, 1024);
	    safe_strcat(temp, L"\"", 1024);
	}
    }
    if (!ok_so_far) {
	lperror(NOTINDICT, L"\n%s\n", temp);
    }
    return ok_so_far;
}
