#include "link-includes.h"
#include "command-line.h"
#include "alink.h"
#include "translator.h"
/*********************************************************/
/*                   Main function                       */
/*                                                       */
/*********************************************************/
int main() 
{
    Dictionary    e_dict;
    Dictionary    g_dict;
    Dictionary    e_table;
    Dictionary    g_table;
    Parse_Options opts;
    Sentence      sent;
    wchar_t          input_string[MAXINPUT+1];
    wchar_t *        linkname;
    Linkage       linkage;
    int           i, num_linkages;
    int           startword;
    int           linkage_found, ok, sentence_ok, transform_performed;
    int z, n, w, l, r;
    wchar_t *        c;
    Link          link;
    Alink * front_alink, *nalink, * out_alink, * al, * al2;
    Alinkset * out_alinkset, * in_alinkset;    
    int question_type;

    localv = 1;

    translator_strings = string_set_create();

    opts  = parse_options_create();

    e_dict  = dictionary_create(L"4.0.dict", L"4.0.knowledge", L"4.0.constituent-knowledge", L"4.0.affix"); 
    if (e_dict == NULL) {
	fprintf(stderr, L"%s\n", lperrmsg);
	exit(-1);
    }
    g_dict  = dictionary_create(L"german.dict", NULL, NULL, L"german.affix"); 
    if (g_dict == NULL) {
	fprintf(stderr, L"%s\n", lperrmsg);
	exit(-1);
    }
    e_table  = dictionary_create(L"e_table", NULL, NULL, NULL); 
    if (e_table == NULL) {
	fprintf(stderr, L"%s\n", lperrmsg);
	exit(-1);
    }
    g_table  = dictionary_create(L"g_table", NULL, NULL, NULL); 
    if (g_table == NULL) {
	fprintf(stderr, L"%s\n", lperrmsg);
	exit(-1);
    }

    if(localv < 3) wprintf(L"\nType in an English sentence at the prompt.\n");

    while (my_fget_input_string(input_string, stdin, stdout, opts)) {

      for(i=0; i<100; i++) {
	tword[i].estring=NULL;
	tword[i].gstring=NULL;
	tword[i].tense=0;
	tword[i].xcase=0;
	tword[i].def=0;
	tword[i].particle=0;
	tword[i].conj=-1;
      }

      if (strcmp(input_string, L"quit\n")==0) break;
      if (strcmp(input_string, L"\n")==0) continue;       

      if(strncmp(input_string, L"!mark", 5)==0) {
        if(localv == 3) wprintf(L"mark: %s\n", &input_string[6]); 
	fflush(stdout);
        continue;
      }

      sent = sentence_create(input_string, e_dict);
      if (sent == NULL) {
	fprintf(stderr, L"%s\n", lperrmsg);
	if (lperrno != NOTINDICT) exit(-1);
	else continue;
      }
      
      /* parse with cost 0 or 1 and no null links */
      parse_options_set_disjunct_cost(opts, 2);
      parse_options_set_min_null_count(opts, 0);
      parse_options_set_max_null_count(opts, 0);
      parse_options_reset_resources(opts);
      
      num_linkages = sentence_parse(sent, opts);
      
      if (num_linkages == 0) {
	wprintf(L"!top ERROR: No linkage found for input sentence\n");
	if (localv == 3) print_noinlinkage_error();
	sentence_delete(sent);
	continue;
      }

      linkage = linkage_create(0, sent, opts);
      linkage_compute_union(linkage);
      linkage->current = linkage->num_sublinkages-1;

      numwords = linkage->num_words;
      /* wprintf("Number of words = %d\n", numwords);  */

      in_alinkset = build_input_linkset(linkage);
      if(localv == 3) print_input_blurb(linkage, in_alinkset);

      front_alink = in_alinkset->link;

      set_tenses_of_input_words(e_table, front_alink); 

      modify_input_linkset(in_alinkset);

      out_alinkset = replace_words(e_table, g_dict, g_table, in_alinkset);

      if(out_alinkset == NULL) {
	free_alinkset(in_alinkset);
	linkage_delete(linkage);
	sentence_delete(sent);
	continue;
      }

      out_alink = out_alinkset->link;

      if(localv == 2) wprintf(L"After choosing correct wordforms:\n");
      if(localv == 2) print_links(out_alink);

      if(localv == 2) wprintf(L"\nModifying linkset...\n");

      apply_transformations(out_alinkset);

      if(linkage->info.fat==TRUE) {
	ok = generate_fat_linkage(linkage, out_alink); 
	if(ok == 0) continue;
	if(localv == 3) print_conjunction_blurb(out_alinkset);
      }

      if(localv == 2) wprintf(L"Transformations completed\n\nGenerating new sentence...\n"); 

      for (al=out_alink; al!=NULL; al=al->next) {
        if(strcmp(al->left, L"LEFT-WALL")==0) {
          startword=al->leftsub;
          break;
        }
      }

      initialize_positions(startword);
      linkage_found = position_words(g_dict, out_alink, startword, 1, 0.0, 1.0);
      if(linkage_found == 0) {
        wprintf(L"!top ERROR: No translation found. (The translator cannot handle some of the word usages \n!top in your sentence.)\n");
	if(localv == 3) print_nooutlinkage_error();
	free_alinkset(in_alinkset);
	free_alinkset(out_alinkset); 
	linkage_delete(linkage);
	sentence_delete(sent);
        continue;
      }
      
      if(linkage->info.fat==TRUE) insert_conjunction_commas(out_alinkset);
      
      linkage_found = print_words_in_order(out_alink);

      if(linkage_found == 0) wprintf(L"!top ERROR: No linkage found for output linkset. (Words in\n output linkset are not fully connected.)\n");      

      free_alinkset(in_alinkset);
      free_alinkset(out_alinkset); 
      /* This next line causes it to crash with conjunction sentences */
      /* linkage_delete(linkage);   */
      sentence_delete(sent); 

    }

    dictionary_delete(e_table);
    dictionary_delete(g_table);  
    dictionary_delete(e_dict);
    dictionary_delete(g_dict);  
    parse_options_delete(opts);
    string_set_delete(translator_strings); 

    /*
    if (space_in_use != 0) {
        fprintf(stderr, "Warning: %d bytes of space leaked.\n", 
		space_in_use);
    }
    if (external_space_in_use != 0) {
        fprintf(stderr, "Warning: %d bytes of external space leaked.\n", 
		external_space_in_use);
    } */
    return 0;
}

int my_fget_input_string(wchar_t *input_string, FILE *in, FILE *out, Parse_Options opts) {

    if(localv < 3) fprintf(out, L"translator> "); 
    /* For web version, both list of words and prompt should be commented out */
    /* fflush(out); */
    /* input_pending = FALSE; */
    if (fgets(input_string, MAXINPUT, in)) return 1;
    else return 0;
}

/*********************************************************/
/*          Functions for building and manipulating      */
/*                  the input linkset                    */
/*********************************************************/

Alinkset * build_input_linkset(Linkage linkage) {

    int i, l, r, j;
    Alink * front_alink, * nalink;
    Alinkset * alinkset;
    Andlist * andlist;
    wchar_t * string;

    front_alink = NULL;

    if(localv == 2) {
      string = linkage_print_diagram(linkage);
      fprintf(stdout, L"%s", string);
    }

    if(localv == 2 && linkage->info.fat == TRUE) {
      for(andlist = linkage->info.andlist; andlist!=NULL; andlist=andlist->next) {
	wprintf(L"And-list, c=%d: ", andlist->conjunction);
	for(i = 0; i < andlist->num_elements; i++) {
	  wprintf(L"%d ", andlist->element[i]);
	}
	wprintf(L"\n");
      }
    }

    for(i=0; i<linkage->num_words; i++) {
      tword[i].estring = linkage->word[i];
      tword[i].conj = -1;
      for(andlist = linkage->info.andlist; andlist!=NULL; andlist=andlist->next) {
	for(j=0; j<andlist->num_elements; j++) {
	  if(andlist->element[j] == i) tword[i].conj = andlist->conjunction;
	}
      }
      if(tword[i].conj == -1) tword[i].conj = i;
      /* wprintf("word %d = %s, conj = %d\n", i, tword[i].estring, tword[i].conj);  */
    }

    if(localv == 2) wprintf(L"INPUT LINKS:\n");

    for(i=0; i<linkage_get_num_links(linkage); i++) {
      l = linkage_get_link_lword(linkage, i);
      r = linkage_get_link_rword(linkage, i);
      nalink = (Alink *) xalloc (sizeof(Alink));
      nalink->left = string_set_add(linkage->word[l], translator_strings);
      nalink->connector = string_set_add(linkage_get_link_label(linkage, i), translator_strings);
      nalink->right = string_set_add(linkage->word[r], translator_strings);
      nalink->leftsub = l;
      nalink->rightsub = r;
      nalink->ignore = 0;
      nalink->next = front_alink;
      front_alink = nalink;
      if(localv == 2) wprintf(L"  Link %d: %s (%d) %s %s (%d)\n", i, nalink->left, nalink->leftsub, nalink->connector, nalink->right,
	     nalink->rightsub);   		
    }

    alinkset = (Alinkset *) xalloc (sizeof(Alinkset));
    alinkset->andlist = linkage->info.andlist;
    alinkset->link = front_alink;
    return alinkset;
}

void modify_input_linkset(Alinkset * alinkset) {

    Alink * alink, * al, * al2;
    int trans1, trans2, trans3, trans4, trans5;
    wchar_t * newword;

    alink = alinkset->link;

    /* Replace any Q links with W */
    for(al = alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"Q") == 0) {
	al->connector = string_set_add(L"Wd", translator_strings);
      }
    }

    /* Replace any SX or SF with S (same with inverted forms of these links) */
    for(al = alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"SX") == 0 || uppercompare(al->connector, L"SF") == 0) {
	al->connector = string_set_add(L"S", translator_strings);
      }
    }
    for(al = alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"SXI") == 0 || uppercompare(al->connector, L"SFI") == 0) {
	al->connector = string_set_add(L"SI", translator_strings);
      }
    }

    /* If there are any verbs taking two objects (in which case the direct object link will
       be an O#n), replace the INDIRECT object link with a J*d. */
    for(al = alink; al!=NULL; al=al->next) {
      if(post_process_match(L"O#n", al->connector) == 1) {
	for(al2 = alink; al2!=NULL; al2=al2->next) {
	  if(al2->left == al->left && post_process_match(L"O", al2->connector)==1 &&
	     post_process_match(L"O#n", al2->connector)==0) { /* al2->connector is an O, but not an O#n */
	    al2->connector = string_set_add(L"J*d", translator_strings);
	  }
	}
      }
    }

    /* Replace any Pv's with PV; replace the form of "be" with "will" or "would" */
    for(al = alink; al!=NULL; al=al->next) {
      if(easy_match(al->connector, L"Pv") == 1) {
	if(tword[al->leftsub].tense == 1) newword = string_set_add(L"will", translator_strings); 
	if(tword[al->leftsub].tense == 2) newword = string_set_add(L"would", translator_strings); 
	replace_one_word(alink, al->leftsub, newword, al->leftsub);
	tword[al->leftsub].estring = newword;
	al->connector = string_set_add(L"PV", translator_strings);
      }
    }

    trans1 = modify_relative_clause(alink);

    if(trans1 == 1) {
      if(localv == 2) wprintf(L"After modifying relative clause:\n");
      if(localv == 2) print_links(alink);
    }

    trans2 = remove_do_transform(alink);

    trans3 = remove_progressive(alink);

    if(trans2 == 1 || trans3 == 1) {
      if(localv == 2) wprintf(L"After removing auxiliary:\n");
      if(localv == 2) print_links(alink);
    }

    trans4 = genitive_transform(alink);

    if(trans4 == 1) {
      if(localv == 2) wprintf(L"After genitive transform:\n");
      if(localv == 2) print_links(alink);
    }

    trans5 = add_that(alink);
    if(trans5 == 1) {
      if(localv == 2) wprintf(L"After adding 'that' before embedded clause:\n");
      if(localv == 2) print_links(alink);
    }

    if(localv == 3 && (trans1 == 1 || trans2 == 1 || trans3 == 1 || trans4 == 1 || trans5 == 1)) print_input_transf_blurb(alinkset);
}

int remove_do_transform(Alink * alink) {

  /* This function takes the input (English) linkset as input. It looks to see if there's
     an SI link with a form of "do" on the left end. If so, it finds the I link with the
     do-word on the left end (there has to be one), takes the word on the right end, and
     replaces the SI-left word with it, anywhere the SI-left word occurs. (It does the
     same thing if there's an S with "do" on the right end.) It may not choose the correct 
     verb form for the English sentence, but that doesn't matter. */

    Alink * al, * al2, * al3;
    int ok, i;

    ok=0;
    for(al = alink; al!=NULL; al=al->next) {
      if((uppercompare(al->connector, L"SI") == 0 && (strcmp(al->left, L"do.v")==0 || strcmp(al->left, L"does.v")==0 || strcmp(al->left, L"did.v")==0)) ||
       (uppercompare(al->connector, L"S") == 0 && (strcmp(al->right, L"do.v")==0 || strcmp(al->right, L"does.v")==0 || strcmp(al->right, L"did.v")==0))) {
	ok=1;
	for(al2 = alink; al2!=NULL; al2=al2->next) {                  /* Now al is the SI link */

	  if(uppercompare(al2->connector, L"I") == 0 && (al2->leftsub == al->leftsub || al2->leftsub == al->rightsub)) {
	    al2->ignore = 1;
	    tword[al2->rightsub].tense = tword[al2->leftsub].tense;
	    replace_one_word(alink, al2->leftsub, al2->right, al2->rightsub);
	  }
	}
      }
    }
    if(ok==0) return 0;
    return 1;
}

int remove_progressive(Alink * alink) {
    Alink * al;
    int ok = 0;
    for(al = alink; al!=NULL; al=al->next) {
      if(easy_match(al->connector, L"Pg")==1) {  
	replace_one_word(alink, al->leftsub, al->right, al->rightsub);
	al->ignore = 1;
	ok = 1;
      }
    }
    if(ok == 1) return 1;
    return 0;
}

int genitive_transform(Alink * alink) {
    Alink * al, * al2, * newlink;
    wchar_t ds1[100];
    wchar_t * connector;
    int ok = 0, j_found;

    for(al = alink; al!=NULL; al=al->next) {
      if(strcmp(al->left, L"of")==0) {  
	for(al2 = alink; al2!=NULL; al2=al2->next) {
	  if(al2->rightsub==al->leftsub) {
	    al->ignore = 1;
	    al2->ignore = 1;
	    connector = string_set_add(L"GE", translator_strings);
	    newlink = create_new_link(alink, al2->left, al2->leftsub, al->right, al->rightsub, connector);
	    ok = 1;
	  }
	}
      }
      remove_duplicate_links(alink);

      if(strcmp(al->connector, L"YS")==0 || strcmp(al->connector, L"YP")==0) {  
	strcpy(ds1, al->left);
	if(isupper(ds1[0])) {
	  wprintf(L"%s has cap first letter\n", al->left);
	  continue;
	}
	for(al2 = alink; al2!=NULL; al2=al2->next) {
	  if(al2->leftsub==al->rightsub) {
	    /* We've found the relevant D link; change the left end to "the" */
	    al2->left = string_set_add(L"the", translator_strings);
	    tword[al2->leftsub].estring = al2->left;
	    replace_one_word(alink, al2->leftsub, al2->left, al2->leftsub); /* unnecessary */
	    /* Now copy the left end of the YS to the right end; copy the right end of the D to the left end of the YS;
	       and change the YS to a GE */
	    al->right = al->left;
	    al->rightsub = al->leftsub;
	    al->left = al2->right;
	    al->leftsub = al2->rightsub;
	    al->connector = string_set_add(L"GE", translator_strings);
	    ok = 1;
	  }
	}
      }
    }
    if(ok == 1) return 1;
    return 0;
}

int modify_relative_clause(Alink * alink) {

    Alink * al, * al2, * newlink;
    int trans_performed=0, ok;
    wchar_t * leftword, * connector;

    for(al = alink; al!=NULL; al=al->next) {
      if(strcmp(al->connector, L"Bs")==0 || strcmp(al->connector, L"Bp")==0) {
	al->ignore=1;
	trans_performed=1;
      }
    }

    /* If there's an Rn, that means it's a object-type relative clause without a relative
       pronoun; insert one */
    for(al = alink; al!=NULL; al=al->next) {
      if(strcmp(al->connector, L"Rn")==0) {
	leftword = string_set_add(L"that.r", translator_strings);
	connector = string_set_add(L"Cr", translator_strings);
	newlink = create_new_link(alink, leftword, numwords, al->right, al->rightsub, connector);
	numwords++;	
	tword[newlink->leftsub].estring = newlink->left;
	tword[newlink->leftsub].tense = 0;

	al->right = newlink->left;
	al->rightsub = newlink->leftsub;
	trans_performed=1;
      }
    }

    /* If there's a Cr, it's an object-type relative clause; find the S, transfer the right
       end of the Cr to the right end of the S, change the Cr to an RO */

    for(al = alink; al!=NULL; al=al->next) {
      if(strcmp(al->connector, L"Cr")==0) {
	ok=0;
	for(al2 = alink; al2!=NULL; al2=al2->next) {
	  if(al2->leftsub == al->rightsub && uppercompare(al2->connector, L"S")==0) {
	    al->rightsub = al2->rightsub;
	    al->right = al2->right;
	    al->connector = string_set_add(L"RO", translator_strings);
	    ok=1;
	    trans_performed=1;
	  }
	}
	if(localv == 2) if(ok==0) wprintf(L"Warning: no S link found in object-type relative clause");
      }
    }

    /* Now we set the case values of the relative pronouns to 1 or 2 */
    for(al = alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"RS")==0) tword[al->leftsub].xcase=1;
      if(uppercompare(al->connector, L"RO")==0) tword[al->leftsub].xcase=2;
    }

    return trans_performed;
}

int add_that(Alink * alink) {

    Alink * al;
    wchar_t * that;
    wchar_t * connector;
    int trans_performed = 0;

    for(al=alink; al!=NULL; al=al->next) {
      if(post_process_match(al->connector, L"Ce*")==1) {
	that = string_set_add(L"that.c", translator_strings);
	connector = string_set_add(L"TH", translator_strings);
	create_new_link(alink, al->left, al->leftsub, that, numwords, connector);
	al->left = that;
	al->leftsub = numwords;
	tword[numwords].estring = that;
	numwords++;
	trans_performed = 1;
      }
    }
    return trans_performed;
}


void set_tenses_of_input_words(Dictionary e_table, Alink * alink) {

  /* This sets the tenses of all the words in the input linkset. It looks them up in
     the English table. If a word is marked "PRESENT", tense = 1; if "PAST", tense = 2;
     if neither, tense = 0. */

    Alink * al;
    int side;
    Dict_node * dn1;
    Disjunct * d1;
    Connector * c1;

    for(al=alink; al!=NULL; al=al->next) {
      for(side=0; side<2; side++) {
	if(side==0) tword[al->leftsub].tense = 0;
	if(side==1) tword[al->rightsub].tense = 0;
	if(side==0) dn1 = dictionary_lookup(e_table, al->left);
	if(side==1) dn1 = dictionary_lookup(e_table, al->right);
	if(dn1==NULL) continue;
	d1 = build_disjuncts_for_dict_node(dn1);
	if(d1->left==NULL) {
	  free_disjuncts(d1);
	  continue;
	}
	c1 = d1->left;
	if(strcmp(c1->string, L"PRESENT")==0) {
	  if(side==0) tword[al->leftsub].tense = 1;
	  if(side==1) tword[al->rightsub].tense = 1;
	}	  
	else if(strcmp(c1->string, L"PAST")==0) {
	  if(side==0) tword[al->leftsub].tense = 2;
	  if(side==1) tword[al->rightsub].tense = 2;
	}	  
	free_disjuncts(d1);
      }
    }

    /*
    wprintf("Twords with tenses:\n");
    for(al=alink; al!=NULL; al=al->next) {
      wprintf("%s %d %s %d\n", tword[al->leftsub].estring, tword[al->leftsub].tense, tword[al->rightsub].estring, 
	     tword[al->rightsub].tense);
    }  */

}

/*********************************************************/
/*          Functions for replacing English words        */
/*                 with German ones                      */
/*********************************************************/

Alink * replace_words_dumbly(Dictionary e_table, Dictionary g_table, Alink * alink) {
    Alink * al, * al2;
    wchar_t ds1[100];
    Dict_node * dn1;
    Disjunct * d1;
    Connector * c1;
    Dict_node * dn2, * dn3;
    int x, i;
    wchar_t * ws, * cs1;
    wchar_t s[100];
    wchar_t * capword = {L"CAPITALIZED-WORDS"};
    int side;
    
    Alink * front_alink, * nalink;
    
    front_alink = NULL;

    for(i=0; i<numwords; i++) {

      if(tword[i].estring == NULL) {
	if(localv == 2) wprintf(L"Error: an estring that is supposed to be filled is null\n");
	return NULL;
      }
      strcpy(ds1, tword[i].estring);

      /* wprintf("Looking up %s in e_table\n", ds1); */
      
      dn1 = dictionary_lookup(e_table, ds1);
      if(dn1==NULL) {
	if(isupper(ds1[0])) {
	  dn1 = dictionary_lookup(e_table, capword);
	}
	else continue;
	/* If there's no matching word in the English table, just go on to the next word */
      }

      d1 = build_disjuncts_for_dict_node(dn1);
      cs1 = d1->right->string;                                   /* We've looked up a word and found the table entry and the
							   connector */
      /* wprintf("Found category connector for %s: %s\n", ds1, d1->right->string); */
      
      dn2 = list_whole_dictionary(g_table->root, NULL);
      dn3 = dn2;
      for(; dn2!=NULL; dn2=dn2->right) {                  
	x = word_has_connector(dn2, cs1, 0);            /* Look in the g_table, find another word in the same category.
								  (This routine just picks the first matching word that it 
								  finds in the g_table. Later we worry about choosing the correct
								  word form) */
	if(x==1) {
	  ws = dn2->string;
	  tword[i].gstring = string_set_add(ws, translator_strings);
	  break;
	}
      }

      if(localv == 2) if (x==1) wprintf(L"  Replaced '%s' with '%s'\n", ds1, ws); 
      if(localv == 2) if (x==0) wprintf(L"  Not replacing '%s'\n", ds1);
      free_listed_dictionary(dn3);
      free_disjuncts(d1);
    }


    for(al=alink; al!=NULL; al=al->next) {
      if(al->ignore==1) continue;
      for(side=0; side<=1; side++) {
	if(side==0) {
	  if(tword[al->leftsub].gstring==NULL) {
	    wprintf(L"!top ERROR: No match found for word '%s'; sentence cannot be translated\n", tword[al->leftsub].estring);
	    if(localv == 3) print_nomatch_error(tword[al->leftsub].estring);
	    return NULL;
	  }
	  nalink = (Alink *) xalloc (sizeof(Alink));
	  nalink->left = tword[al->leftsub].gstring;
	  nalink->leftsub = al->leftsub;
	  nalink->connector = string_set_add(al->connector, translator_strings);
	} 
	if(side==1) {
	  if(tword[al->rightsub].gstring==NULL) {
	    wprintf(L"!top ERROR: No match found for word '%s'; sentence cannot be translated\n", tword[al->rightsub].estring);
	    if(localv == 3) print_nomatch_error(tword[al->rightsub].estring);
	    return NULL;
	  }
	  nalink->right = tword[al->rightsub].gstring;
	  nalink->rightsub = al->rightsub;
	  nalink->ignore = 0;
	  nalink->next = front_alink;
	  front_alink = nalink;
	}
      }
    }    
    if(localv==2) wprintf(L"After dumb word replacement:\n");
    if(localv == 2) print_links(front_alink);
    return front_alink;
}


Alinkset * replace_words(Dictionary e_table, Dictionary g_dict, Dictionary g_table, Alinkset * in_alinkset) {

    Alink * out_alink, * al;
    Alinkset * out_alinkset;
    int ok, sentence_ok, w, ending_added=0;
    wchar_t * c;

    if(localv == 2) wprintf(L"\nReplacing words...\n");
    out_alink = replace_words_dumbly(e_table, g_table, in_alinkset->link);
    if (out_alink == NULL) return NULL;

    out_alinkset = (Alinkset *) xalloc (sizeof(Alinkset));
    out_alinkset->link = out_alink;
    out_alinkset->andlist = in_alinkset->andlist;

    if(localv == 3) print_dumb_replacement_blurb(out_alinkset);

    sentence_ok = set_case_values(out_alink, g_table, g_dict);
    if(sentence_ok == 0) {
      if(localv==2) wprintf(L"Error: problem with setting case values\n");
      return NULL;
    }
    if(localv == 2) wprintf(L"\nChoosing correct word forms...\n");

    /* put in participle forms where needed, infinitive or past */
    for(al=out_alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"I")==0) {
	ok = choose_participles(g_dict, g_table, out_alink, al, 1);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"P")==0) {
	ok = choose_participles(g_dict, g_table, out_alink, al, 3);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"PP")==0) {
	ok = choose_participles(g_dict, g_table, out_alink, al, 2);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"PV")==0) {
	ok = choose_participles(g_dict, g_table, out_alink, al, 4);
	if(ok==0) sentence_ok=0;
      }
    }
    if(localv == 2) wprintf(L"After choosing participles:\n");
    if(localv == 2) print_links(out_alink);
    
    /* Choose subject nouns of the right case. (This selects nominative pronouns, and also selects normal as
     opposed to genitive noun forms.) */

    for(al=out_alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"S")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 0);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"SI")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"MX")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
    }

    /* Choose correct relative pronoun */
    for(al=out_alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"R")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
    }
    
    /* Choose verb forms that agree with their subjects */
    for(al=out_alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"S")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"RS")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"SI")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 0);
	if(ok==0) sentence_ok=0;
      }
    }

    for(al=out_alink; al!=NULL; al=al->next) {

      /* Choose accusative and nominative pronouns of the right case; also choose normal / genitive noun forms */
      if(uppercompare(al->connector, L"O")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"J")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"GE")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 1);
	if(ok==0) sentence_ok=0;
      }
    }

    for(al=out_alink; al!=NULL; al=al->next) {
      
      /* Choose determiners and adjectives of the right case and gender */
      if(uppercompare(al->connector, L"D")==0) {
	ok = choose_correct_wordforms(g_dict, g_table, out_alinkset, al, 0);
	if(ok==0) sentence_ok=0;
      }
      if(uppercompare(al->connector, L"A")==0) {
	ok = choose_correct_linktype(g_dict, out_alinkset, al, 0);
	if(ok==0) sentence_ok=0;
	ending_added=1;
	add_adjective_ending(g_dict, g_table, out_alinkset, al);
      }
    }

      
    /* Choose correct form of "when" */
    for(al=out_alink; al!=NULL; al=al->next) {
      if(tword[al->leftsub].estring==NULL) continue;
      if(strcmp(tword[al->leftsub].estring, L"when")==0) {
	if(uppercompare(al->connector, L"W")==0) {
	  al->left = string_set_add(L"wann", translator_strings);
	}
	else al->left = string_set_add(L"als", translator_strings);
	replace_one_word(out_alink, al->leftsub, al->left, al->leftsub);
	tword[al->leftsub].gstring = al->left;
      }
    }

    /* Split separable words */
    for(w=0; w<numwords; w++) {
      if(tword[w].gstring==NULL) continue;
      for(c = tword[w].gstring; *c!=L'\0'; c++) {
	if(*c==L'+') {	
	  split_separable_words(g_dict, out_alink, w);
	}
      }
    }


    if(sentence_ok = 0) {
      wprintf(L"!top ERROR: Sentence cannot be translated; valid wordforms could not be found\n");
      if(localv == 3) print_nooutlinkage_error();
      return NULL;
    }      

    if(localv == 3) print_smart_replacement_blurb(out_alinkset);
    if(localv == 3 && ending_added==1) print_ending_blurb();
    return out_alinkset;
}

void split_separable_words(Dictionary g_dict, Alink * alink, int w) {
    Alink * al, * newlink;
    wchar_t * c, * c2;
    wchar_t firstword[100];
    wchar_t secondword[100];
    wchar_t * bigword, * connector, * leftword, * rightword;

    bigword = tword[w].gstring;
    for(c = bigword; *c!=L'\0'; c++) {
      if(*c==L'+') {
	if(localv == 2) wprintf(L"separable word found: '%s'\n", tword[w].gstring);
	strncpy(firstword, bigword, c-bigword);
	firstword[c-bigword]=L'\0';
	for(c2 = c; *c2!=L'\0'; c2++);
	strncpy(secondword, c+1, c2-(c+1));

	if(localv == 2) wprintf(L"split it into two words: '%s' and '%s'\n", firstword, secondword);
      }
    }
    
    replace_one_word(alink, w, secondword, w);
    tword[w].gstring = string_set_add(secondword, translator_strings);

    connector = string_set_add(L"K", translator_strings);
    leftword = string_set_add(secondword, translator_strings);
    rightword = string_set_add(firstword, translator_strings);
    newlink = create_new_link(alink, leftword, w, rightword, numwords, connector);
    tword[newlink->rightsub].gstring = newlink->right;
    tword[newlink->rightsub].tense = 0;
    numwords++;	

    if(localv == 2) wprintf(L"After separable-word transformation:\n");
    if(localv == 2) print_links(alink);
}
      

int set_case_values(Alink * out_alink, Dictionary g_table, Dictionary g_dict) {

    wchar_t * cs;
    Alink * al;
    Dict_node * dn;

    /* Set case values (nom=1, acc=2, dat=3, gen=4) and def values (1 if a noun has a definite determiner, 2 if indefinite) */
    for(al=out_alink; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"S")==0) tword[al->leftsub].xcase=1;
      if(uppercompare(al->connector, L"SI")==0) tword[al->rightsub].xcase=1;
      if(uppercompare(al->connector, L"MX")==0) tword[al->rightsub].xcase=1;
      if(uppercompare(al->connector, L"O")==0) tword[al->rightsub].xcase=2;
      if(uppercompare(al->connector, L"J")==0) {
	cs = find_connector_on_word(g_dict, al->left, L"J", 1, al->leftsub);
	if(cs == NULL) return 0;
	if(post_process_match(L"J#d", cs)==1) tword[al->rightsub].xcase=3;
	if(post_process_match(L"J#a", cs)==1) tword[al->rightsub].xcase=2;
      }    
      if(uppercompare(al->connector, L"GE")==0) tword[al->rightsub].xcase=4;

      if(uppercompare(al->connector, L"D")==0) {
	dn = dictionary_lookup(g_table, al->left);
	if(word_has_connector(dn, L"DEF", 0)==1) tword[al->rightsub].def=1;
	else if(word_has_connector(dn, L"INDEF", 0)==1) tword[al->rightsub].def=2;
	else return 0;
      }
    }
    return 1;
}

int choose_participles(Dictionary g_dict, Dictionary g_table, Alink * alink, Alink * al, int type) {

  /* This function takes a link as input; the link is either an I link (type = 1), a PP (type = 2), a
     P (type = 3), or a PV (type = 4). The
     function looks at the word on the right end, and searches for a word in the same category which has an
     I-, a PP-, or a P-, as appropriate; it then replaces the old right-end word with the new one. (In the
     case of P-, this ensures that the predicative form of adjectives is used.) */

    Alink * al2;
    Dict_node * dn1, * dn2, * dn3, * dn4;
    Connector * c1;
    wchar_t * cs;
    Disjunct * d1;
    int x, ok, sein=0, found=0;
    wchar_t * ws;

    dn1 = dictionary_lookup(g_table, al->right);
    d1 = build_disjuncts_for_dict_node(dn1);
    cs = d1->right->string;
    free_disjuncts(d1);

    dn2 = list_whole_dictionary(g_table->root, NULL);
    dn4 = dn2;
    for(; dn2!=NULL; dn2=dn2->right) {
      x = word_has_connector(dn2, cs, 0); 
      if(x==1) {
	ws = dn2->string;
	dn3 = dictionary_lookup(g_dict, ws);

	if(type == 1) ok = word_has_connector(dn3, L"I", 1);         /* This will do a smart match */

	if(type == 2) {
	  ok = word_has_connector(dn3, L"PP", 1);          
	  if (ok != 1) {
	    ok = word_has_connector(dn3, L"P", 1);         
	    if (ok==1) sein=1;
	  }
	}

	if(type == 3) ok = word_has_connector(dn3, L"P", 1);         

	if(type == 4) ok = word_has_connector(dn3, L"PV", 1);         

	if(localv == 2) if(ok == -1) wprintf(L" Warning: The word '%s' is not in the dictionary\n", ws);

	if(ok == 1) {
	  found = 1;
	  al->right = string_set_add(ws, translator_strings);
	  tword[al->rightsub].gstring = al->right;
	  for(al2=alink; al2!=NULL; al2=al2->next) {
	    if(al2->leftsub == al->rightsub) al2->left = al->right;
	    if(al2->rightsub == al->rightsub) al2->right = al->right;
	  }
	  break;
	}
      }
    }
    free_listed_dictionary(dn4);

    if(type==2 && sein==1) {
      al->connector = string_set_add(L"P", translator_strings);
      al->left = string_set_add(L"ist.v", translator_strings);
      tword[al->leftsub].gstring = al->left;
      for(al2=alink; al2!=NULL; al2=al2->next) {
	if(al2->leftsub == al->leftsub) al2->left = al->left;
	if(al2->rightsub == al->leftsub) al2->right = al->left;
      }
    }
    return found;
}


wchar_t * find_connector_on_word(Dictionary dict, wchar_t * w, wchar_t * cs, int direction, int wordsub) {  

    /* This function takes a word (w) and a linkname (cs) as input, and checks to see what exact form of 
       the link is present on the word; it returns this linkname as a string. If the 
       connector is a D or A, then we have to check for case (finding the form of the connector that's 
       conjoined with S, O, or J, as appropriate). With D or A, we also check that the form of D found is 
       definite or indefinite, as appropriate (or in the case of A, that the A found is conjoined
       with the right kind of D). NOTE: In this function, direction = 1 implies a *right-pointing*
       connector, unlike elsewhere. */

    Dict_node *dn;
    Disjunct * d, * d0; 
    Connector * c;
    int ok;
    dn = dictionary_lookup(dict, w);
    /* wprintf("Checking for %s\n", cs); */
    for (;dn != NULL; dn = dn->right) {
      /* wprintf("Trying node %s\n", dn->string); */
      d0 = d = build_disjuncts_for_dict_node(dn);

      for(; d!=NULL; d=d->next) {

	/* First check the disjunct for case, if necessary. If case == 1 (nominative), there has to be an S-
	   on the disjunct from which the D connector is being chosen; if case == 2, there has to be an O+.  */

	if(tword[wordsub].xcase != 0 && (uppercompare(cs, L"D")==0 || uppercompare(cs, L"A")==0)) {
	  ok = 0;
	  for (c = d->right; c!=NULL; c=c->next) {
	    if(tword[wordsub].xcase==1 && uppercompare(c->string, L"S")==0) ok = 1;
	  }
	  for (c = d->left; c!=NULL; c=c->next) {
	    if(tword[wordsub].xcase==2 && (uppercompare(c->string, L"O")==0 || easy_match(c->string, L"J*a")==1)) ok = 1;
	  }
	  for (c = d->left; c!=NULL; c=c->next) {
	    if(tword[wordsub].xcase==3 && easy_match(c->string, L"J*d")==1) ok = 1;
	  }
	  for (c = d->left; c!=NULL; c=c->next) {
	    if(tword[wordsub].xcase==4 && easy_match(c->string, L"GE")==1) ok = 1;
	  }
	  if(ok == 0) continue;
	}

	ok=0;

	/* If it's a D or A connector, you have to find the D- on the noun that matches the article:
	   definite (def=1) or indefinite (def=2) */

	if(uppercompare(cs, L"D")==0 || uppercompare(cs, L"A")==0) {
	  ok = 1;
	  /* wprintf("for fixed word '%s', def = %d\n", w, tword[wordsub].def); */
	  for (c = d->left; c!=NULL; c=c->next) {
	    /* If the noun has a definite determiner (def=1), and we're looking at a disjunct on the noun 
	       with a D- that post-process-matches D##i, that means it can't be used to choose the determiner.
	       (This means that either D##d or D##* will be accepted.) */

	    if(tword[wordsub].def==1 && post_process_match(L"D##i", c->string)==1) ok = 0;
	    if(tword[wordsub].def==2 && post_process_match(L"D##d", c->string)==1) ok = 0;
	  }
	  if(ok==0) continue;
	} 

	if(direction==0) c = d->left;
	if(direction==1) c = d->right;
	for(; c!=NULL; c=c->next) {
	  if(easy_match(c->string, cs)==1) {
	    free_disjuncts(d0);
	    return c->string;         /* This does a smart match */
	  }
	}
      }
    }
    free_disjuncts(d0);
    return NULL;
}

int choose_correct_wordforms(Dictionary g_dict, Dictionary g_table, Alinkset * alinkset, Alink * al, int direction) {

    /* This function takes a link as input. It looks at the link type, call it X. If direction == 1, 
       it looks at the left word (call this the fixed word), and finds the exact type of X connector that word has, 
       according to the dictionary. It then looks at the right word (call this the target word), and looks that 
       word up in the g_table to find all words of the same category. It goes through these words, and chooses one 
       which has an X connector smart-matching that of the fixed word. It then replaces the old target word with 
       this new word. If direction == 0, it performs the same process except with right and left reversed. */

    Connector * c1;
    int ok, x, i, tw, fw;
    Alink * al2;
    wchar_t * cs1; /* ? */
    wchar_t * ws;
    Dict_node * dn1, * dn2, * dn3, * dn4;
    wchar_t ds1[100];
    wchar_t cs2[100];
    Disjunct * d1;
    int tense;
    wchar_t * present = {L"PRESENT"};
    wchar_t * past = {L"PAST"};
    wchar_t * cs3, * cs4;

    if(direction == 0) fw = al->rightsub;
    if(direction == 1) fw = al->leftsub;
    if(direction == 0) tw = al->leftsub;
    if(direction == 1) tw = al->rightsub;

    /* Replace the connector in the link with unsubscripted form */
    safe_strcpy(cs2, al->connector, sizeof(cs2));
    for(i=0; cs2[i] != L'\0'; i++) {
	if (!isupper(cs2[i])) cs2[i] = L'\0'; /* terminate at first non capital */
    }
    al->connector = string_set_add(cs2, translator_strings);    

    /* Step 1: Find the form of the connector on the fixed word; call it cs1 */

    if((direction == 1 && (uppercompare(cs2, L"SI")==0 || uppercompare(cs2, L"O")==0)) ||
       (direction == 0 && uppercompare(cs2, L"S")==0)) {   /* why doesn't strcmp work here? */
      /* If it's an S or SI or O, and you're going to choose the noun, use an unsubscripted link. (With J, you need the
	 subscript to determine case.) */
      if(localv == 2) wprintf(L"Replacing '%s': Find a noun of the right case\n", tword[tw].gstring);
      cs1 = cs2;
    }
    else if (direction == 1 && uppercompare(cs2, L"S")==0 && is_outside_world_word(al->leftsub, tword[al->rightsub].conj, alinkset)) {
      /* If you're choosing the right end of an S, and it's an outside world word for the left end, use Sp */
      cs1 = string_set_add(L"Sp", translator_strings);
    }
    else {
      cs1 = find_connector_on_word(g_dict, tword[fw].gstring, cs2, direction, fw);
      if(localv == 2) wprintf(L"Replacing '%s': '%s' has a %s; find a form with a matching connector\n", tword[tw].gstring, tword[fw].gstring, cs1);
    }

    if (cs1==NULL) {
      if(localv == 2) wprintf(L"No matching connector found\n");
      return 0;
    }

    /* Step 2: Look up the target word, find its category connector (c1) */

    dn1 = dictionary_lookup(g_table, tword[tw].gstring);
    d1 = build_disjuncts_for_dict_node(dn1);
    cs4 = d1->right->string;                              
    free_disjuncts(d1);

    /* Step 3: go through all the words in the table with a category connector c1; for each one, look it up in the
       dictionary, see if it has a cs1 connector */

    dn2 = list_whole_dictionary(g_table->root, NULL);
    dn4 = dn2;
    for(; dn2!=NULL; dn2=dn2->right) {                  
      x = word_has_connector(dn2, cs4, 0);
      if(x==1) {
	ws = dn2->string;
	dn3 = dictionary_lookup(g_dict, ws);
	ok = word_has_connector(dn3, cs1, direction);
	if(localv == 2) if(ok == -1) wprintf(L"  Warning: The word '%s' isn't in the dictionary\n", ws);

	if(ok==1 && uppercompare(cs1, L"R")==0) {
	  if(localv == 2) wprintf(L"The word '%s' matches the antecedent\n", ws);
	  if(tword[al->rightsub].xcase==1) ok = word_has_connector(dn3, L"RS", 0);
	  if(tword[al->rightsub].xcase==2) ok = word_has_connector(dn3, L"RO", 0);
	  
	  if(localv == 2) if(ok==0) wprintf(L" ...but it's the wrong case for the relative clause\n");
	  if(localv == 2) if(ok==1) wprintf(L" ...and it's the right case for the relative clause\n");
	}

	if(ok==1) {
	  if(localv == 2) wprintf(L"  The word '%s' has a match\n", ws);

	  if(tword[tw].tense==1) cs3 = present;
	  if(tword[tw].tense==2) cs3 = past;
	  if(tword[tw].tense!=0) {
	    ok = word_has_connector(dn2, cs3, 1);
	    if(localv == 2) if(ok == 0) wprintf(L"  ...but it doesn't have the right tense\n");
	    if(localv == 2) if(ok == 1) wprintf(L"  ...and it has the right tense\n", ws);
	  }
	} 

	if(ok==1) {

	  tword[tw].gstring = string_set_add(ws, translator_strings);
	  if(direction == 1) al->right = tword[tw].gstring;
	  if(direction == 0) al->left = tword[tw].gstring;

	  if(uppercompare(al->connector, L"D")==0) al->connector = string_set_add(cs1, translator_strings);
	  for(al2=alinkset->link; al2!=NULL; al2=al2->next) {
	    if(al2->rightsub == tw) al2->right = tword[tw].gstring;
	    if(al2->leftsub == tw) al2->left = tword[tw].gstring;
	  }
	  free_listed_dictionary(dn4);
	  return 1;
	}
	if(localv == 2) if(ok==0 && tword[tw].tense==0) wprintf(L"  Word '%s' doesn't have match\n", ws);
      }
    }
    free_listed_dictionary(dn4);
    return 0;
}


int choose_correct_linktype(Dictionary g_dict, Alinkset * alinkset, Alink * al, int direction) {

    Connector * c1;
    int ok, x, i, tw, fw;
    Alink * al2;
    wchar_t * cs1; /* ? */
    wchar_t * ws;
    Dict_node * dn1, * dn2, * dn3, * dn4;
    wchar_t ds1[100];
    wchar_t cs2[100];
    Disjunct * d1;
    int tense;
    wchar_t * present = {L"PRESENT"};
    wchar_t * past = {L"PAST"};
    wchar_t * cs3, * cs4;

    if(direction == 0) fw = al->rightsub;
    if(direction == 1) fw = al->leftsub;
    if(direction == 0) tw = al->leftsub;
    if(direction == 1) tw = al->rightsub;

    /* Replace the connector in the link with unsubscripted form */
    safe_strcpy(cs2, al->connector, sizeof(cs2));
    for(i=0; cs2[i] != L'\0'; i++) {
	if (!isupper(cs2[i])) cs2[i] = L'\0'; /* terminate at first non capital */
    }
    al->connector = string_set_add(cs2, translator_strings);    

    /* Step 1: Find the form of the connector on the fixed word; call it cs1 */

    cs1 = find_connector_on_word(g_dict, tword[fw].gstring, cs2, direction, fw);
    if(localv == 2) wprintf(L"Replacing '%s' link: using %s\n", tword[tw].gstring, cs1);

    al->connector = cs1;
    return 1;
}

#if 0
    }

    if (cs1==NULL) {
      if(localv == 2) wprintf(L"No matching connector found\n");
      return 0;
    }

    /* Step 2: Look up the target word, find its category connector (c1) */

    dn1 = dictionary_lookup(g_table, tword[tw].gstring);
    d1 = build_disjuncts_for_dict_node(dn1);
    cs4 = d1->right->string;                              
    free_disjuncts(d1);

    /* Step 3: go through all the words in the table with a category connector c1; for each one, look it up in the
       dictionary, see if it has a cs1 connector */

    dn2 = list_whole_dictionary(g_table->root, NULL);
    dn4 = dn2;
    for(; dn2!=NULL; dn2=dn2->right) {                  
      x = word_has_connector(dn2, cs4, 0);
      if(x==1) {
	ws = dn2->string;
	dn3 = dictionary_lookup(g_dict, ws);
	ok = word_has_connector(dn3, cs1, direction);
	if(localv == 2) if(ok == -1) wprintf(L"  Warning: The word '%s' isn't in the dictionary\n", ws);

	if(ok==1 && uppercompare(cs1, L"R")==0) {
	  if(localv == 2) wprintf(L"The word '%s' matches the antecedent\n", ws);
	  if(tword[al->rightsub].xcase==1) ok = word_has_connector(dn3, L"RS", 0);
	  if(tword[al->rightsub].xcase==2) ok = word_has_connector(dn3, L"RO", 0);
	  
	  if(localv == 2) if(ok==0) wprintf(L" ...but it's the wrong case for the relative clause\n");
	  if(localv == 2) if(ok==1) wprintf(L" ...and it's the right case for the relative clause\n");
	}

	if(ok==1) {
	  if(localv == 2) wprintf(L"  The word '%s' has a match\n", ws);

	  if(tword[tw].tense==1) cs3 = present;
	  if(tword[tw].tense==2) cs3 = past;
	  if(tword[tw].tense!=0) {
	    ok = word_has_connector(dn2, cs3, 1);
	    if(localv == 2) if(ok == 0) wprintf(L"  ...but it doesn't have the right tense\n");
	    if(localv == 2) if(ok == 1) wprintf(L"  ...and it has the right tense\n", ws);
	  }
	} 

	if(ok==1) {

	  tword[tw].gstring = string_set_add(ws, translator_strings);
	  if(direction == 1) al->right = tword[tw].gstring;
	  if(direction == 0) al->left = tword[tw].gstring;

	  if(uppercompare(al->connector, L"D")==0) al->connector = string_set_add(cs1, translator_strings);
	  for(al2=alinkset->link; al2!=NULL; al2=al2->next) {
	    if(al2->rightsub == tw) al2->right = tword[tw].gstring;
	    if(al2->leftsub == tw) al2->left = tword[tw].gstring;
	  }
	  free_listed_dictionary(dn4);
	  return 1;
	}
	if(localv == 2) if(ok==0 && tword[tw].tense==0) wprintf(L"  Word '%s' doesn't have match\n", ws);
      }
    }
    free_listed_dictionary(dn4);
    return 0;
}

#endif

add_adjective_ending(Dictionary g_dict, Dictionary g_table, Alinkset * alinkset, Alink * al) {

    wchar_t * ending, * connector;
    Alink * newlink;

    if(easy_match(al->connector, L"Ae")==1) ending = string_set_add(L"e", translator_strings);
    if(easy_match(al->connector, L"An")==1) ending = string_set_add(L"en", translator_strings);
    if(easy_match(al->connector, L"Ar")==1) ending = string_set_add(L"er.f", translator_strings);
    if(easy_match(al->connector, L"As")==1) ending = string_set_add(L"es.f", translator_strings);
    if(easy_match(al->connector, L"Am")==1) ending = string_set_add(L"em", translator_strings);
    connector = string_set_add(L"FS", translator_strings);
    newlink = create_new_link(alinkset->link, al->left, al->leftsub, ending, numwords, connector);
    tword[newlink->rightsub].gstring = newlink->right;
    tword[newlink->rightsub].tense = 0;
    tword[newlink->rightsub].particle = 2;
    numwords++;    
}

/*********************************************************/
/*   Functions for transforming German linkset           */
/*                                                       */
/*********************************************************/

void apply_transformations(Alinkset * out_alinkset) {

    int transform_performed;
    Alink * al, * al2;

    /* Make the finite verb the head (connecting to the outside), rather than the main noun */
    transform_performed = switch_head(out_alinkset);
    if(transform_performed == 1) {
      if(localv == 2) wprintf(L"After switching head:\n");
      if(localv == 2) print_links(out_alinkset->link);
      if(localv == 3) print_switchhead_blurb(out_alinkset);
    }

    transform_performed = 0;
    /* Adjust infinitives */
    for(al=out_alinkset->link; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"TO")==0 || strcmp(al->connector, L"MVi")==0) {
	adjust_infinitive_clause(out_alinkset->link, al);
      }
    }
    
    /* Attach any subordinate clauses to the finite verb rather than the main verb */
    for(al=out_alinkset->link; al!=NULL; al=al->next) {
      if(strcmp(al->connector, L"MVs")==0 || strcmp(al->connector, L"TH")==0) {
	reattach_subordinate_clause(out_alinkset->link, al);
      }
    }

    /* Put participles at the end (substituting MVI for MV, etc.) */
    for(al=out_alinkset->link; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"I")==0 || uppercompare(al->connector, L"PP")==0 ||
	 uppercompare(al->connector, L"P")==0 || uppercompare(al->connector, L"PV")==0 || strcmp(al->connector, L"SCz")==0) {
	put_participle_at_end(out_alinkset->link, al);
	transform_performed=1;
      }
    }

    /* If there's an opener, invert the subject and verb */
    for(al=out_alinkset->link; al!=NULL; al=al->next) {
      if(uppercompare(al->connector, L"CO")==0) {
	for(al2=out_alinkset->link; al2!=NULL; al2=al2->next) {	  
	  if(al2->rightsub == al->rightsub && uppercompare(al2->connector, L"S")==0) {
	    s_v_inversion(out_alinkset->link, al2);
	    transform_performed=1;
	  }
	}
      }
    }

    /* If it's a subordinate clause, put the finite verb at the end */
    for(al=out_alinkset->link; al!=NULL; al=al->next) {
      if(al->ignore==1) continue;
      if(uppercompare(al->connector, L"C")==0 || uppercompare(al->connector, L"RO")==0 ||
	 uppercompare(al->connector, L"RS")==0) {
	put_finite_verb_at_end(out_alinkset->link, al);
	transform_performed = 1;
      }
    }
    if(transform_performed == 1 && localv == 3) print_transformation_blurb(out_alinkset);
}

int switch_head(Alinkset * alinkset) {

    /* This function finds the main subject word; it takes the word on the right end of this link (the finite
       verb), and replaces the main subject word with the finite verb on the W connector. In the case
       of questions, this transform doesn't get performed. */

    Alink * al, * al2, * al3, * newlink;
    int co_found, co_leftsub, transform_performed=0;
    int s_found, w_found, c_found, s_rightsub, i, j;
    Andlist * andlist;
    wchar_t * connector;

    for(al=alinkset->link; al!=NULL; al=al->next) {

      if(easy_match(al->connector, L"Wd")==1 || uppercompare(al->connector, L"C")==0 || uppercompare(al->connector, L"CO")==0) {       /* Find a W link, call it al */
	s_found = 0;

	for(al2=alinkset->link; al2!=NULL; al2=al2->next) {
	  if(al2->leftsub == al->rightsub && uppercompare(al2->connector, L"S")==0) { 
	                                             /* Find an S link that has al->right on the left end; call it al2 */
	    s_found++;
	    al->ignore = 1;
	    s_rightsub = al2->rightsub;
	    newlink = create_new_link(alinkset->link, al->left, al->leftsub, al2->right, al2->rightsub, al->connector);

	    /* Now, if the finite verb is not an outside world word, that means it's two clauses anded together;
	       that means we have to replace the subject nouns with the finite verbs as and-list elements */
	    if(tword[al2->leftsub].conj != al2->leftsub) {	
	      if(tword[al2->leftsub].conj != al2->leftsub && is_outside_world_word(al2->leftsub, tword[al2->rightsub].conj, alinkset)==0) {
		for(andlist = alinkset->andlist; andlist!=NULL; andlist=andlist->next) {
		  if(andlist->conjunction == tword[al2->leftsub].conj) {
		    for(i=0; i<andlist->num_elements; i++) {
		      if(andlist->element[i] == al2->leftsub) {
			andlist->element[i]=al2->rightsub; 
			tword[al2->rightsub].conj = tword[al2->leftsub].conj;
		      }
		    }
		  }
		}
	      }
	    }
	  }
	}
	transform_performed = 1;
      }
    }
    remove_duplicate_links(alinkset->link);

    return transform_performed;
}

void adjust_infinitive_clause(Alink * alink, Alink * al) {

    /* This changes the "I" connector to a "ZU", and reattaches the clause to the finite verb of the main clause
       instead of the main verb */

    Alink * al2;

    al->connector = string_set_add(L"SCz", translator_strings);
    for(al2=alink; al2!=NULL; al2=al2->next) {
      if(al2->leftsub == al->rightsub && uppercompare(al2->connector, L"I")==0) {
	al2->connector = string_set_add(L"ZU", translator_strings);
	al->right = al2->right;
	al->rightsub = al2->rightsub;
	
	add_commas(alink, al, 1);
	
      }
    }
    if(localv == 2) wprintf(L"After modifying infinitive:\n");
    if(localv == 2) print_links(alink);
}

void reattach_subordinate_clause(Alink * alink, Alink * al) {

  /* This function reattaches a subordinate clause from a participle to the finite verb of the main clause. 
     More specifically, it takes an MVs or TH as input. It looks for the PP/P/I link whose right end is the left end 
     of the MVs/TH. It copies the word on the left of the PP/P/I on to the left of the MVs/TH, and changes it to an SC
     (SCc in the case of TH). */

    Alink * al2;

    if(strcmp(al->connector, L"MVs")==0) al->connector = string_set_add(L"SC", translator_strings);  
    if(strcmp(al->connector, L"TH")==0) {
      add_commas(alink, al, 1);
      al->connector = string_set_add(L"SCc", translator_strings);  
    }

    for(al2=alink; al2!=NULL; al2=al2->next) {
      if(al2->rightsub == al->leftsub && (uppercompare(al2->connector, L"I")==0 || uppercompare(al2->connector, L"PP")==0 ||
	  uppercompare(al2->connector, L"P")==0 || uppercompare(al2->connector, L"PV")==0)) {
                                                            /* Find the link whose right end is the left end of the MVs */
	al->left = al2->left;
	al->leftsub = al2->leftsub;
      }
    }
    /* A more robust version: see if there's an S or SI with al2->left on the right end. If not, repeat the
       for loop. Now it will look for a PP/I/P link with the new left-end of al on the right end; if it finds one
       it will put its left end on the left end of al. This will handle cases where there's more than one 
       P/PP/I link, like "Ich habe sagen wollen" */

    if(localv == 2) wprintf(L"After reattaching subordinate clause:\n");
    if(localv == 2) print_links(alink);
}

void put_participle_at_end(Alink * alink, Alink * al) {

    /* This function takes an I, PP, PV, P, or SCz link as input. It looks at the word on the right end; it looks to see
     if this word is on the left end of any MV or O (or other complement) links. If so, it changes the O to a B and the
     MV to an MVI (inverting the words also). */

    Alink * al2;

    for(al2=alink; al2!=NULL; al2=al2->next) {
      if(al2->leftsub == al->rightsub) {
	if(strcmp(al2->connector, L"Xc")==0) continue;
	if(uppercompare(al2->connector, L"O")==0) {
	  al2->connector = string_set_add(L"B", translator_strings);
	}
	if(uppercompare(al2->connector, L"J")==0) {
	  al2->connector = string_set_add(L"JB", translator_strings);
	}
	if(uppercompare(al2->connector, L"MV")==0) {
	  al2->connector = string_set_add(L"MVI", translator_strings);
	}
	if(uppercompare(al2->connector, L"P")==0) {
	  put_participle_at_end(alink, al2);
	  al2->connector = string_set_add(L"PI", translator_strings);
	}
	if(uppercompare(al2->connector, L"PV")==0) {
	  put_participle_at_end(alink, al2);
	  al2->connector = string_set_add(L"PVI", translator_strings);
	}
	if(uppercompare(al2->connector, L"K")==0) {
	  al2->connector = string_set_add(L"KI", translator_strings);
	  tword[al2->leftsub].particle = 1;
	}
	al2->left = al2->right;
	al2->leftsub = al2->rightsub;
	al2->right = al->right;
	al2->rightsub = al->rightsub;
      }
    }
    if(localv == 2) wprintf(L"After putting participle at end:\n");
    if(localv == 2) print_links(alink);
}


void s_v_inversion(Alink * alink, Alink * al) {

  /* This function is only called if there's a CO link in the input sentence. The S linking
     into the right end of the CO is the input to the function. This S is changed into an SI
     and inverted. (Head-switching has already been performed here; the CO attaches to the
     right end of the S.) */

    wchar_t * tempword;
    int tempnum;

    tempword = al->right;
    /* strcpy(tempword, al->right); */
    al->right = al->left; 
    al->left = string_set_add(tempword, translator_strings);
    tempnum = al->rightsub;
    al->rightsub = al->leftsub;
    al->leftsub = tempnum;
    al->connector = string_set_add(L"SI", translator_strings);
    if(localv == 2) wprintf(L"After subject-verb inversion:\n");
    if(localv == 2) print_links(alink);
}

void put_finite_verb_at_end(Alink * alink, Alink * al) {

  /* This takes a C link as input (al). The right end of the C has already been switched to the finite verb. 
     The function searches for any links with the finite verb to the left, such as O, MV, PP, or I, and inverts them. */

    Alink * al2, * al3;

    /* Now find the S link of the subordinate clause (if the input link was an RS, use that) */
    if(uppercompare(al->connector, L"C")==0 || uppercompare(al->connector, L"RO")==0) {
      for(al2=alink; al2!=NULL; al2=al2->next) {
	if(uppercompare(al2->connector, L"S")==0 && al2->rightsub == al->rightsub) break;
      }
    }
    else if(uppercompare(al->connector, L"RS")==0) al2 = al;
    if(al2==NULL) {
      wprintf(L"Error! No S was found at the right end of a C / RO connector\n");
      return;
    }

    for(al3=alink; al3!=NULL; al3=al3->next) {
      if(al3->leftsub == al2->rightsub) {
	if(uppercompare(al3->connector, L"SC")==0) continue;
	al3->left=al3->right;
	al3->leftsub=al3->rightsub;
	al3->right = al2->right;
	al3->rightsub = al2->rightsub;
	if(uppercompare(al3->connector, L"O")==0) {
	  al3->connector = string_set_add(L"B", translator_strings);
	}
	if(uppercompare(al3->connector, L"J")==0) {
	  al3->connector = string_set_add(L"JB", translator_strings);
	}
	if(uppercompare(al3->connector, L"MV")==0) {
	  al3->connector = string_set_add(L"MVI", translator_strings);
	}
	if(uppercompare(al3->connector, L"PP")==0) {
	  al3->connector = string_set_add(L"PPI", translator_strings);
	}
	if(uppercompare(al3->connector, L"I")==0) {
	  al3->connector = string_set_add(L"II", translator_strings);
	}
	if(uppercompare(al3->connector, L"P")==0) {
	  al3->connector = string_set_add(L"PI", translator_strings);
	}
	if(uppercompare(al3->connector, L"PV")==0) {
	  al3->connector = string_set_add(L"PVI", translator_strings);
	}
	if(uppercompare(al3->connector, L"K")==0) {
	  al3->connector = string_set_add(L"KI", translator_strings);
	  tword[al3->leftsub].particle = 1;
	}
      }
    }

    if(uppercompare(al->connector, L"RS")==0 || uppercompare(al->connector, L"RO")==0) {
      add_commas(alink, al, 0);
    }

    if(localv == 2) wprintf(L"After putting the finite verb on the end of the subordinate clause:\n");
    if(localv == 2) print_links(alink);
}

void add_commas(Alink * alink, Alink * al, int side) {

    /* This function takes a linkset (alink is the front Alink) and a link. It looks at the word on the side "side" of the
       link, and adds links coming out of that word - an Xd linking to a comma on the left, an Xc to a comma on the right. */

    Alink * al2, * newlink;
    int wsub;
    wchar_t * word, * commaword, * connector;
    if(side == 0) {
      wsub = al->leftsub;
      word = al->left;
    }
    if(side == 1) {
      wsub = al->rightsub;
      word = al->right;
    }

    commaword = string_set_add(L",", translator_strings);
    connector = string_set_add(L"Xd", translator_strings);
    newlink = create_new_link(alink, commaword, numwords, word, wsub, connector);
    tword[newlink->leftsub].gstring = newlink->left;
    tword[newlink->leftsub].tense = 0;
    numwords++;

    connector = string_set_add(L"Xc", translator_strings);
    newlink = create_new_link(alink, word, wsub, commaword, numwords, connector);
    tword[newlink->rightsub].gstring = newlink->right;
    tword[newlink->rightsub].tense = 0;
    numwords++;
}

int generate_fat_linkage(Linkage linkage, Alink * alink) {
    Andlist * andlist;
    int i, j, outside_link_generated, a;
    Alink * newlink, * al, * al2;
    wchar_t * connector;

    for(andlist = linkage->info.andlist; andlist!=NULL; andlist=andlist->next) {
      if(tword[andlist->conjunction].gstring == NULL) {
        wprintf(L"!top ERROR: The conjunction '%s' is not understood by the translator.\n", tword[andlist->conjunction].estring);
	return 0;
      }

      /* For each andlist: First, update the outside world list */
      andlist->num_outside_words=0;
      if(localv==2) wprintf(L"Rebuilding outside-world list for andlist (conj=%d): ", andlist->conjunction);
      for(al=alink; al!=NULL; al=al->next) {
	if(al->ignore==1) continue;
	for(al2=al->next; al2!=NULL; al2=al2->next) {
	  if(al2->ignore==1) continue;
	  if(al2->rightsub == al->rightsub && tword[al2->leftsub].conj==andlist->conjunction &&
	     tword[al->leftsub].conj==andlist->conjunction) {
	    /*	    for(i=0; i<andlist->num_outside_words; i++) if(al2->rightsub==andlist->outside_word[i]) continue; */
	    andlist->outside_word[andlist->num_outside_words]=tword[al->rightsub].conj;
	    if(localv==2) wprintf(L"%d ", tword[al->rightsub].conj);
	    andlist->num_outside_words++;
	  }
	  if(al2->leftsub == al->leftsub && tword[al2->rightsub].conj==andlist->conjunction &&
	     tword[al->rightsub].conj==andlist->conjunction) {
	    andlist->outside_word[andlist->num_outside_words]=tword[al->leftsub].conj;
	    if(localv==2) wprintf(L"%d ", tword[al->leftsub].conj);
	    andlist->num_outside_words++;
	  }
	}
      }
      if(localv==2) wprintf(L"\n");

      /* Now generate new links from each and-list element head-word to the conjunction */
      for(i=0; i<andlist->num_elements; i++) {
	newlink = (Alink *) xalloc (sizeof(Alink));	
	if(andlist->element[i]<andlist->conjunction) {
	  connector = string_set_add(L"XL", translator_strings);
	  newlink = create_new_link(alink, tword[andlist->element[i]].gstring, andlist->element[i], tword[andlist->conjunction].gstring, andlist->conjunction, connector);
	}
	else {
	  connector = string_set_add(L"XR", translator_strings);
	  newlink = create_new_link(alink, tword[andlist->conjunction].gstring, andlist->conjunction, tword[andlist->element[i]].gstring, andlist->element[i], connector);
	}
      }
      if(localv == 2) wprintf(L"After adding fat links:\n");
      if(localv == 2) print_links(alink);

      /* Now, for each outside world word, generate a new link. Go through the union linkset: the first
       link from an and-element word to the outside world gets turned into the "and" link; the others get
       "ignored". (In some cases, a single word may be listed as an outside word more than once in a single
       and-list. This doesn't seem to cause problems though. The first time through, all the outside links to the 
       element words are shifted to the conjunction; after that, no links are found and no action is taken.) */
      for(i=0; i<andlist->num_outside_words; i++) {
	outside_link_generated = 0;
	for(j=0; j<andlist->num_elements; j++) {
	  for(al=alink; al!=NULL; al=al->next) {
	    if(al->ignore == 1) continue;
	    if(al->leftsub == andlist->element[j] && tword[al->rightsub].conj == andlist->outside_word[i]) {
	      if(outside_link_generated == 0) {
		al->left = tword[andlist->conjunction].gstring;
		al->leftsub = andlist->conjunction;
		/* These next two lines are only needed if the outside word is itself a conjunction. Then we make the
		   outside link to that, rather than to one of the element words */
		al->right = tword[andlist->outside_word[i]].gstring;
		al->rightsub = andlist->outside_word[i];
		outside_link_generated = 1;
		if(localv == 2) wprintf(L"Outside link generated to %d\n", andlist->outside_word[i]); 
	      }
	      else al->ignore = 1;
	    }
	    if(al->rightsub == andlist->element[j] && tword[al->leftsub].conj == andlist->outside_word[i]) {
	      if(outside_link_generated == 0) {
		al->right = tword[andlist->conjunction].gstring;
		al->rightsub = andlist->conjunction;
		al->left = tword[andlist->outside_word[i]].gstring;
		al->leftsub = andlist->outside_word[i];
		outside_link_generated = 1;
		if(localv == 2) wprintf(L"Outside link generated to %d\n", andlist->outside_word[i]);
	      }
	      else al->ignore = 1;
	    }
	  }
	}
      }
      if(localv == 2) wprintf(L"After adding links to outside world:\n");
      if(localv == 2) print_links(alink);
    }
    return 1;
}	

int is_outside_world_word(int eword, int oword, Alinkset * alinkset) {

    /* Returns 1 if oword is an outside world word of the andlist of which eword is an element*/

    Andlist * andlist;
    int i, j;

    for(andlist = alinkset->andlist; andlist!=NULL; andlist=andlist->next) {
      for(i = 0; i< andlist->num_elements; i++) {
	if(andlist->element[i] == eword) {
	  for(j=0; j< andlist->num_outside_words; j++) {
	    if(andlist->outside_word[j] == oword) return 1;
	  }
	}
      }
    }
    
    return 0;
}

/*********************************************************/
/*          Some general-purpose functions               */
/*                                                       */
/*********************************************************/

Alink * create_new_link(Alink * alink, wchar_t * left, int leftsub, wchar_t * right, int rightsub, wchar_t * connector) {

    Alink * newlink, * al;

    newlink = (Alink *) xalloc (sizeof(Alink));
    newlink->left = left;
    newlink->leftsub = leftsub;
    newlink->right = right;
    newlink->rightsub = rightsub;
    newlink->connector = connector;
    newlink->ignore=0;
    newlink->next=NULL;
    for(al=alink; al->next!=NULL; al=al->next);
    al->next = newlink;
    return newlink;
}

void replace_one_word(Alink * alink, int i, wchar_t * word, int j) {

  /* Takes as input a word subscript i, and a word and its subscript j. Everywhere i occurs,
     it is replaced with the word "word" and its subscript. */

    Alink * al;

    for(al = alink; al!=NULL; al=al->next) {
      if (al->leftsub == i) {
	al->left = string_set_add(word, translator_strings);
	al->leftsub = j;
      }
      if (al->rightsub == i) {
	al->right = string_set_add(word, translator_strings);
	al->rightsub = j;
      }
    }
}

void print_links(Alink * alink) {
    Alink * al;
    for(al=alink; al!=NULL; al=al->next) {
      if(al->ignore == 1) continue;
      wprintf(L"  %12s (%2d) %4s  (%2d) %s \n", al->left, al->leftsub, al->connector, 
	     al->rightsub, al->right);   		
    }
}

int uppercompare(wchar_t * s, wchar_t * t) {
  while(isupper(*s) || isupper(*t)) {
    if (*s != *t) return 1;
    s++;
    t++;
  }
  return 0;
}  

void remove_duplicate_links(Alink * alink) {
    Alink * al, * al2;
    for(al = alink; al!=NULL; al=al->next) {
      for(al2 = al; al2!=NULL; al2=al2->next) {
	if(al2->left == al->left && al2->right == al->right) al2->ignore == 1;
      }
    }
}

void free_alinkset(Alinkset * alinkset) {
    Alink * al, * al2;
    al = alinkset->link;
    while(al!=NULL) {
      al2=al->next;
      xfree(al, sizeof(Alink));
      al=al2;
    }
    xfree(alinkset, sizeof(Alinkset));
}