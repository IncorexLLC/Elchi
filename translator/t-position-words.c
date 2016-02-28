#include "link-includes.h"
#include "alink.h"
#include "translator.h"

void initialize_positions(int startword) {
  int n;
  for(n=0; n<100; n++) {                /* kind of crude... */
    word_position[n]=-1.0;	  
  }
  word_position[startword] = 0.0;  
}

int position_words(Dictionary dict, Alink * alink, int currentword, int direction, double leftend, double rightend) {
  /* direction: left = 0, right = 1 */
    Disjunct * d, * d0;
    Connector * c;
    Dict_node * dn;
    wchar_t * s;
    wchar_t * ds;
    int numcon, i, n, w, ok;
    int linkage_found = 1;
    Link link;
    wchar_t * ws;
    Alink * al;
    double position;
    double range;
    double newleftend, newrightend;
    /* Right now it goes through and choose the disjunct twice - once for the right and once for the left. This seems
       unnecessary... */

    ds = tword[currentword].gstring; 

    if(localv == 2) wprintf(L"  Tracing word '%s', word %d, direction %d\n", ds, currentword, direction); 

    /* With a conjunction: you could submit a string from one of the andlist element words here as ds, instead of the
       conjunction itself - "currentword" would still be the conjunction, though. (But what about the XL and XR connectors?) */

    dn = dictionary_lookup(dict, ds);

    /* Should we go through the dict_nodes here, or is it okay to just take the first one? */
    d0 = d = build_disjuncts_for_dict_node(dn);
    for(; d!=NULL; d=d->next) { 
      ok = evaluate_disjunct(d, alink, currentword);
      if(ok==1) break;
    }

    if(d == NULL) {
      if(localv == 2) wprintf(L"No disjunct found for word '%s'\n", ds);
      free_disjuncts(d0);
      return 0;
    }

    /* We've found a disjunct to use for the current word. Now we go through all the connectors on the disjunct;
       for each one, we look through the links to find a link of the right type with the current word on one end; 
       then we position the word on the other end and repeat this process recursively */

    range=rightend-leftend;
    if(direction==0) c = d->left;
    if(direction==1) c = d->right;
    numcon = 5; 

    /* A better way:
    numcon = 0;      
    if(direction == 0) {
      for(; c!=NULL; c=c->next) numcon++;
    }
    */
    n=1;
    ok = 1;
    while(c!=NULL) {  /* for(n=1; n<=numcon; n++) { */
      s = c->string;
      if(localv == 2) wprintf(L"    String from disjunct for '%s': %s\n", ds, s);  
      for(al = alink; al!=NULL; al=al->next) {
	if(al->ignore == 1) continue;
	if(direction==0 && al->rightsub == currentword) {  /* Does the link have the current word on the right end? */
	  if (strcmp(s, L"XR")==0) continue;
	  if (easy_match (s, al->connector) == 1 && word_position[al->leftsub]==-1.0) {
	    ws=al->left;
	    position = rightend - (range * ((numcon+1.0 - n) / (numcon+1.0)));
	    word_position[al->leftsub] = position;
	    if(localv == 2) wprintf(L"  Word '%s' has position %6.6f\n", ws, position); 
	    newleftend = ( position + (rightend - (range * ((numcon+1.0 - (n-1.0)) / (numcon+1.0)))) ) / 2.0;
	    newrightend = ( position + (rightend - (range * ((numcon+1.0 - (n+1.0)) / (numcon+1.0)))) ) / 2.0;
	    /* wprintf("Newleftend = %6.6f, newrightend = %6.6f\n", newleftend, newrightend); */
	    linkage_found = position_words(dict, alink, al->leftsub, 0, newleftend, position);
	    if(linkage_found==0) ok = 0;
	    linkage_found = position_words(dict, alink, al->leftsub, 1, position, newrightend);
	    if(linkage_found==0) ok = 0;
	    n++;
	  }
	}
	if(direction==1 && al->leftsub == currentword) {
	  if (strcmp(s, L"XL")==0) continue;
	  if (easy_match (s, al->connector) == 1 && word_position[al->rightsub]==-1.0) {
	    ws = al->right;
	    position = leftend + (range * ((numcon+1.0 - n) / (numcon+1.0)));
	    word_position[al->rightsub] = position;
	    if(localv == 2) wprintf(L"  Word '%s' has position %6.6f\n", ws, position); 
	    newrightend = ( position + (leftend + (range * ((numcon+1.0 - (n-1.0)) / (numcon+1.0)))) ) / 2.0;
	    newleftend = ( position + (leftend + (range * ((numcon+1.0 - (n+1.0)) / (numcon+1.0)))) ) / 2.0;
	    /* wprintf("Newleftend = %6.6f, newrightend = %6.6f\n", newleftend, newrightend); */
 	    linkage_found = position_words(dict, alink, al->rightsub, 0, newleftend, position);
	    if(linkage_found==0) ok = 0;
 	    linkage_found = position_words(dict, alink, al->rightsub, 1, position, newrightend);
	    if(linkage_found==0) ok = 0;
	    n++;
	  }
	}
      }
      c = c->next;
    }
    free_disjuncts(d0);
    if(ok==1) return 1;
    else return 0;
}

int evaluate_disjunct(Disjunct * d, Alink * alink, int currentword) {

    Connector * c;
    int dok, cok, i;
    Alink * al;

    dok = 1;
    for(al = alink; al!=NULL; al=al->next) {     /* Go through the list of links; for each link, go through the
					   connectors on the disjunct. If you ever don't find a match
					   for a link, the disjunct is eliminated. */
      if(al->ignore == 1) continue;
      if(al->rightsub == currentword) {  /* Does the link have the current word on the right end? */      
	cok = 0;
	c = d->left;
	for(; c!=NULL; c=c->next) {
	  if(easy_match (al->connector, c->string) == 1) {
	    cok = 1;
	  }
	}
	if (strcmp(al->connector, L"XR")==0) cok = 1;
	if (cok==0) dok=0;
      }
    }	    
    for(al = alink; al!=NULL; al=al->next) {  /* Now do the same thing on the other side */
      if(al->ignore == 1) continue;
      if(al->leftsub == currentword) {  
	cok=0;
	c = d->right;
	for(; c!=NULL; c=c->next) {
	  if(easy_match (al->connector, c->string) == 1) {
	    cok = 1;
	  }
	}
	if (strcmp(al->connector, L"XL")==0) cok = 1;
	if (cok==0) dok=0;
      }
    }	
	
    if(dok==1) {
      
      if(localv == 2) {
	wprintf(L"  For word %s, disjunct found: ", tword[currentword].gstring); 	  
	wprintf(L"L: ");
	c = d->left;
	for(; c!=NULL; c=c->next) {
	  wprintf(L"%s ", c->string);	
	}
	wprintf(L"; R: ");
	c = d->right;
	for(; c!=NULL; c=c->next) {
	  wprintf(L"%s ", c->string);	
	}
	wprintf(L"\n");
      }
      return 1;	  
    } 

    if(dok==0){
      /* wprintf("No disjunct found for '%s'\n", ds); */
      return 0; 
    }
}

void insert_conjunction_commas(Alinkset * alinkset) {

    /* This function takes andlists with three or more elements, and inserts commas in them. It
       puts a comma halfway between each pair of element words (except the final pair - see 
       below). This should be right on the border between their ranges (because of how the word 
       positioning is done), so it should cleanly separate the phrases of the and list. */

    Andlist * andlist;
    int i, lowest, previous;
    double lowest_position;
    int done[100];

    for(i=0; i<100; i++) done[i]=0;

    for(andlist=alinkset->andlist; andlist!=NULL; andlist=andlist->next) {
      if(andlist->num_elements < 3) continue;
      previous = -1;
      while(1) {
	lowest_position = 100.0;
	for(i=0; i<andlist->num_elements; i++) {
	  if(word_position[andlist->element[i]] < lowest_position && done[andlist->element[i]]==0) {
	    lowest = andlist->element[i];
	    lowest_position = word_position[andlist->element[i]];
	  }
	}
	if(lowest_position == 100.0) break;

	done[lowest]=1;
	if(previous>-1) {
	  tword[numwords].gstring = string_set_add(L",", translator_strings);

	  /* If we've gotten to the last andlist element: then, instead of putting the comma 
	     halfway between this element and the previous one, we put it halfway between
	     the previous element and the conjunction. */

	  if(word_position[lowest] > word_position[andlist->conjunction]) {
	    word_position[numwords] = (word_position[previous] + word_position[andlist->conjunction] ) / 2;
	  }
	  else word_position[numwords] = (word_position[previous] + word_position[lowest] ) / 2;
	  if(localv == 2) wprintf(L"Inserting a comma between %d and %d at position %6.6f\n", previous, lowest, word_position[numwords]);
	  numwords++;
	}
	previous = lowest;
      }
    }
}
	    
int print_words_in_order(Alink * alink) {

    /* This function takes the twords and word positions generated by "position_words", arranges them in order, 
       and prints them. */

    int i, w, lowest, p=0, highestsub=0;
    double lowest_position;
    int done[100];
    int final_order[100];
    Alink * al;
    wchar_t word[100];
    wchar_t * final_word[100];
    int contraction;

    highestsub = 0;
    for(i=0; i<100; i++) {
      if(tword[i].gstring!=NULL) {
	if(i>highestsub) highestsub = i;
      }
    }
    
    for (w=0; w<=highestsub; w++) done[w]=0;

    for (al = alink; al!=NULL; al=al->next) {
      if(al->ignore == 1) continue;
      if(word_position[al->leftsub] == -1.0 || word_position[al->rightsub] == -1.0) return 0;
      /* If this happens, it means some positions of words used in the linkset weren't set; the words aren't all connected */
    }

    /* Now we go through the word positions and assign each word w to a order position in the array final_order:
     final_order[0] is the first word, and so on. */

    while(1) {
      lowest_position = 100.0;
      for(w=0; w<=highestsub; w++) {
	if (word_position[w]==-1.0) continue;              /* This might happen with words that aren't used in the linkage */
	if (word_position[w]<lowest_position && done[w]==0) {
	  lowest_position=word_position[w];
	  lowest=w;
	}
      }
      if(lowest_position == 100.0) break;
      final_order[p]=lowest;
      /* wprintf("Word %d has position %d\n", lowest, p); */
      done[lowest]=1;
      p++;
    }

    numwords = p;       /* Numwords is reset here - now it only counts the number of words used in the German sentence */
    /* wprintf("Numwords in output sentence = %d\n", numwords);  */

    if(localv == 2) wprintf(L"\nFINAL SENTENCE:\n");
    if(localv == 3) print_positioning_blurb();

    contraction=0;

    for(p=0; p<numwords; p++) {
      final_word[p] = NULL;
      if(contraction == 1) {
	contraction = 0;
	continue;
      }

      for(w=0; w<=highestsub; w++) {
	if(w!=final_order[p]) continue;

	if(strcmp(tword[w].gstring, L"LEFT-WALL")==0 || strcmp(tword[w].gstring, L"RIGHT-WALL")==0) break;

	/* If the current word is a comma, and it's the last word (or the next word is a period or the right-wall), 
	   don't print it. */
	if(strcmp(tword[w].gstring, L",")==0) {
	  if(p==numwords-1) break;
	  if(strcmp(tword[final_order[p+1]].gstring, L"RIGHT-WALL")==0 || strcmp(tword[final_order[p+1]].gstring, L".")==0 ||
	     strcmp(tword[final_order[p+1]].gstring, L"?")==0 || strcmp(tword[final_order[p+1]].gstring, L"!")==0) break;
	}

	if(tword[final_order[p-1]].particle==1 & strcmp(tword[final_order[p]].gstring, L"zu")==0) tword[final_order[p]].particle=1;
	contraction = 1;
	if(strcmp(tword[w].gstring, L"zu")==0 && strcmp(tword[final_order[p+1]].gstring, L"dem.d")==0) strcpy(word, L"zum");
	else if(strcmp(tword[w].gstring, L"zu")==0 && strcmp(tword[final_order[p+1]].gstring, L"der.d")==0) strcpy(word, L"zur");
	else if(strcmp(tword[w].gstring, L"in")==0 && strcmp(tword[final_order[p+1]].gstring, L"dem.d")==0) strcpy(word, L"im");
	else if(strcmp(tword[w].gstring, L"an")==0 && strcmp(tword[final_order[p+1]].gstring, L"dem.d")==0) strcpy(word, L"am");
	else if(strcmp(tword[w].gstring, L"ins")==0 && strcmp(tword[final_order[p+1]].gstring, L"das.d")==0) strcpy(word, L"ins");

	else {	

	  if(strcmp(tword[w].gstring, L"NAME")==0) strcpy(word, tword[w].estring);

	  else {
	    /* Copy over the word, but stop at a period (thus omitting the subscript) */
	    for(i=0; tword[w].gstring[i]!=NULL && !(i>0 && tword[w].gstring[i]==L'.'); i++) {
	      word[i] = tword[w].gstring[i];
	    }
	    word[i]=L'\0';
	    if(p==1) word[0] = toupper(word[0]); 
	  }
	  contraction = 0;
	}

	if(word[0]!=L',' && word[0]!=L'.' && word[0]!=L'?' && word[0]!=L'!' && strcmp(word, L"s")!=0 &&
	   tword[final_order[p-1]].particle!=1 && tword[final_order[p]].particle!=2) wprintf(L" "); 

	wprintf(L"%s", word); 

	final_word[p]=string_set_add(word, translator_strings);
	break;      
      }

    }

    wprintf(L"\n");

    if(localv == 3) {
      wprintf(L"!top GERMAN TRANSLATION: ");
      for(p=0; p<numwords; p++) {
        if(final_word[p]==NULL) continue;
	if(strcmp(final_word[p], L",")!=0 && strcmp(final_word[p], L".")!=0 && strcmp(final_word[p], L"?")!=0 && strcmp(final_word[p], L"!")!=0 && strcmp(final_word[p], L"s")!=0 && tword[final_order[p-1]].particle!=1 && tword[final_order[p]].particle!=2) wprintf(L" ");
        wprintf(L"%s", final_word[p]);
      }
      wprintf(L"\n");
    }

    return 1;
}
