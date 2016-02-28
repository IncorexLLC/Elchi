 /****************************************************************************/
 /*                                                                          */
 /*  Copyright (C) 1991-2000, all rights reserved                            */
 /*  Daniel Sleator, David Temperley, and John Lafferty                      */
 /*  See file "README" for information about commercial use of this system   */
 /*                                                                          */
 /****************************************************************************/


#include <stdarg.h>
#include "link-includes.h"

static int center[MAX_SENTENCE];
static int N_words_to_print;  /* version of N_words in this file for printing links */
wchar_t * trailer(int mode);
wchar_t * header(int mode);

void set_centers(Linkage linkage, int print_word_0) {
    int i, len, tot;
    tot = 0;
    if (print_word_0) i=0; else i=1;
    for (; i<N_words_to_print; i++) {
	len = wcslen(linkage->word[i]);
	center[i] = tot + (len/2);
	tot += len+1;
    }
}

/* the following are all for generating postscript */
static int link_heights[MAX_LINKS];
  /* tells the height of the links above the sentence */
static int row_starts[MAX_SENTENCE];
  /* the word beginning each row of the display */
static int N_rows;
  /* the number of rows */

void left_append_string(String * string, wchar_t * s, wchar_t * t) {
/* prints s then prints the last |t|-|s| characters of t.
   if s is longer than t, it truncates s.
*/
    int i, j, k;
    j = wcslen(t);
    k = wcslen(s);
    for (i=0; i<j; i++) {
	if (i<k) {
	    append_string(string, L"%c", s[i]);
	} else {
	    append_string(string, L"%c", t[i]);
	}
    }
}



void print_a_link(String * s, Linkage linkage, int link) {
    Sentence sent = linkage_get_sentence(linkage);
    Dictionary dict = sent->dict;
    int l, r;
    wchar_t *label, *llabel, *rlabel;
    
    l      = linkage_get_link_lword(linkage, link);
    r      = linkage_get_link_rword(linkage, link);
    label  = linkage_get_link_label(linkage, link);
    llabel = linkage_get_link_llabel(linkage, link);
    rlabel = linkage_get_link_rlabel(linkage, link);

    if ((l == 0) && dict->left_wall_defined) {
	left_append_string(s, LEFT_WALL_DISPLAY,L"               ");
    } else if ((l == (linkage_get_num_words(linkage)-1)) && dict->right_wall_defined) {
	left_append_string(s, RIGHT_WALL_DISPLAY,L"               ");	
    } else {
	left_append_string(s, linkage_get_word(linkage, l), L"               ");
    }
    left_append_string(s, llabel, L"     ");
    append_string(s, L"   <---"); 
    left_append_string(s, label, L"-----");
    append_string(s, L"->  "); 
    left_append_string(s, rlabel, L"     ");
    append_string(s, L"     %s\n", linkage_get_word(linkage, r));
}


wchar_t * linkage_print_links_and_domains(Linkage linkage) {
    /* To the left of each link, print the sequence of domains it is in. */
    /* Printing a domain means printing its type                         */
    /* Takes info from pp_link_array and pp and chosen_words.            */    
    int link, longest, j;
    int N_links = linkage_get_num_links(linkage);
    String * s = String_create();
    wchar_t * links_string;
    wchar_t ** dname;

    longest = 0;
    for (link=0; link<N_links; link++) {
	if (linkage_get_link_lword(linkage, link) == -1) continue;
	if (linkage_get_link_num_domains(linkage, link) > longest) 
	    longest = linkage_get_link_num_domains(linkage, link);
    }
    for (link=0; link<N_links; link++) {
	if (linkage_get_link_lword(linkage, link) == -1) continue;
	dname = linkage_get_link_domain_names(linkage, link);
	for (j=0; j<linkage_get_link_num_domains(linkage, link); ++j) {
	    append_string(s, L" (%s)", dname[j]);
	}
	for (;j<longest; j++) {
	    append_string(s, L"    ");
	}
	append_string(s, L"   ");
	print_a_link(s, linkage, link);
    }
    append_string(s, L"\n");
    if (linkage_get_violation_name(linkage) != NULL) {
	append_string(s, L"P.P. violations:\n");
	append_string(s, L"        %s\n\n", linkage_get_violation_name(linkage));
    }
    
    links_string = exalloc(sizeof(wchar_t)*(wcslen(s->p)+1));
    wcscpy(links_string, s->p);
    exfree(s->p, sizeof(wchar_t)*s->allocated);
    exfree(s, sizeof(String));

    return links_string; 
}

wchar_t * build_linkage_postscript_string(Linkage linkage) {
    int link, i,j;
    int d;
    int print_word_0 = 0, print_word_N = 0, N_wall_connectors, suppressor_used;
    Sublinkage *sublinkage=&(linkage->sublinkage[linkage->current]);
    int N_links = sublinkage->num_links;
    Link *ppla = sublinkage->link;
    String  * string;
    wchar_t * ps_string;
    Dictionary dict = linkage->sent->dict;
    Parse_Options opts = linkage->opts;

    string = String_create();

    N_wall_connectors = 0;
    if (dict->left_wall_defined) {
	suppressor_used = FALSE;
	if (!opts->display_walls) 
	    for (j=0; j<N_links; j++) {
		if (ppla[j]->l == 0) {
		    if (ppla[j]->r == linkage->num_words-1) continue;
		    N_wall_connectors ++;
		    if (wcscmp(ppla[j]->lc->string, LEFT_WALL_SUPPRESS)==0) {
			suppressor_used = TRUE;
		    }
		}
	    }
	print_word_0 = (((!suppressor_used) && (N_wall_connectors != 0)) 
			|| (N_wall_connectors > 1) || opts->display_walls);
    } else {
	print_word_0 = TRUE;
    }

    N_wall_connectors = 0;
    if (dict->right_wall_defined) {
	suppressor_used = FALSE;
	for (j=0; j<N_links; j++) {
	    if (ppla[j]->r == linkage->num_words-1) {
		N_wall_connectors ++;
		if (wcscmp(ppla[j]->lc->string, RIGHT_WALL_SUPPRESS)==0){
		    suppressor_used = TRUE;
		}
	    }
	}
	print_word_N = (((!suppressor_used) && (N_wall_connectors != 0)) 
			|| (N_wall_connectors > 1) || opts->display_walls);
    } 
    else {
	print_word_N = TRUE;
    }


    if (print_word_0) d=0; else d=1;

    i = 0;
    N_words_to_print = linkage->num_words;
    if (!print_word_N) N_words_to_print--;

    append_string(string, L"[");
    for (j=d; j<N_words_to_print; j++) {
	if ((i%10 == 0) && (i>0)) append_string(string, L"\n");
	i++;
	append_string(string, L"(%s)", linkage->word[j]);
    }
    append_string(string,L"]");
    append_string(string,L"\n");

    append_string(string,L"[");
    j = 0;
    for (link=0; link<N_links; link++) {
	if (!print_word_0 && (ppla[link]->l == 0)) continue;
	if (!print_word_N && (ppla[link]->r == linkage->num_words-1)) continue;
	if (ppla[link]->l == -1) continue;
	if ((j%7 == 0) && (j>0)) append_string(string,L"\n");
	j++;
	append_string(string,L"[%d %d %d",
		ppla[link]->l-d, ppla[link]->r-d, 
		link_heights[link]);
	if (ppla[link]->lc->label < 0) {
	    append_string(string,L" (%s)]", ppla[link]->name);
	} else {
	    append_string(string,L" ()]");
	}
    }
    append_string(string,L"]");
    append_string(string,L"\n");
    append_string(string,L"[");
    for (j=0; j<N_rows; j++ ){
	if (j>0) append_string(string, L" %d", row_starts[j]);
	else append_string(string,L"%d", row_starts[j]);
    }
    append_string(string,L"]\n");

    ps_string = exalloc(sizeof(wchar_t)*(wcslen(string->p)+1));
    wcscpy(ps_string, string->p);
    exfree(string->p, sizeof(wchar_t)*string->allocated);
    exfree(string, sizeof(String));

    return ps_string; 
}

void compute_chosen_words(Sentence sent, Linkage linkage) {
    /*
       This takes the current chosen_disjuncts array and uses it to
       compute the chosen_words array.  "I.xx" suffixes are eliminated.
       */
    int i, l;
    wchar_t * s, *t, *u;
    Parse_info * pi = sent->parse_info;
    wchar_t * chosen_words[MAX_SENTENCE];
    Parse_Options opts = linkage->opts;

    for (i=0; i<sent->length; i++) {   /* get rid of those ugly ".Ixx" */
	chosen_words[i] = sent->word[i].string;
	if (pi->chosen_disjuncts[i] == NULL) {  
	    /* no disjunct is used on this word because of null-links */
	    t = chosen_words[i];
	    l = wcslen(t) + 2;
	    s = (wchar_t *) xalloc(sizeof(wchar_t)*(l+1));
	    swprintf_s(s, l + 1, L"[%s]", t);
	    t = string_set_add(s, sent->string_set);
	    xfree(s, sizeof(wchar_t)*(l+1));
	    chosen_words[i] = t;
	} else if (opts->display_word_subscripts) {
	    t = pi->chosen_disjuncts[i]->string;	
	    if (is_idiom_word(t)) {
		l = wcslen(t);
		s = (wchar_t *) xalloc(sizeof(wchar_t)*(l+1));
		wcscpy(s,t);
		for (u=s; *u != L'.'; u++)
		  ;
		*u = L'\0';
		t = string_set_add(s, sent->string_set);
		xfree(s, sizeof(wchar_t)*(l+1));
		chosen_words[i] = t;
	    } else {
		chosen_words[i] = t;
	    }
	}
    }
    if (sent->dict->left_wall_defined) {
	chosen_words[0] = LEFT_WALL_DISPLAY;
    }
    if (sent->dict->right_wall_defined) {
	chosen_words[sent->length-1] = RIGHT_WALL_DISPLAY;
    }
    for (i=0; i<linkage->num_words; ++i) {
	linkage->word[i] = (wchar_t *) exalloc(sizeof(wchar_t)*(wcslen(chosen_words[i])+1));
	wcscpy(linkage->word[i], chosen_words[i]);
    }
}


#define MAX_HEIGHT 30

static wchar_t picture[MAX_HEIGHT][MAX_LINE];
static wchar_t xpicture[MAX_HEIGHT][MAX_LINE];

/* String allocated with exalloc.  Needs to be freed with exfree */
wchar_t * linkage_print_diagram(Linkage linkage) {
    int i, j, k, cl, cr, row, top_row, width, flag;
    wchar_t *t, *s;
    int print_word_0 = 0, print_word_N = 0, N_wall_connectors, suppressor_used;
    wchar_t connector[MAX_TOKEN_LENGTH];
    int line_len, link_length;
    Sublinkage *sublinkage=&(linkage->sublinkage[linkage->current]);
    int N_links = sublinkage->num_links;
    Link *ppla = sublinkage->link;
    String * string;
    wchar_t * gr_string;
    Dictionary dict = linkage->sent->dict;
    Parse_Options opts = linkage->opts;
    int x_screen_width = parse_options_get_screen_width(opts);

    string = String_create();

    N_wall_connectors = 0;
    if (dict->left_wall_defined) {
	suppressor_used = FALSE;
	if (!opts->display_walls) 
	    for (j=0; j<N_links; j++) {
		if ((ppla[j]->l == 0)) {
		    if (ppla[j]->r == linkage->num_words-1) continue;
		    N_wall_connectors ++;
		    if (wcscmp(ppla[j]->lc->string, LEFT_WALL_SUPPRESS)==0){
			suppressor_used = TRUE;
		    }
		}
	    }
	print_word_0 = (((!suppressor_used) && (N_wall_connectors != 0)) 
			|| (N_wall_connectors > 1) || opts->display_walls);
    } 
    else {
	print_word_0 = TRUE;
    }

    N_wall_connectors = 0;
    if (dict->right_wall_defined) {
	suppressor_used = FALSE;
	for (j=0; j<N_links; j++) {
	    if (ppla[j]->r == linkage->num_words-1) {
		N_wall_connectors ++;
		if (wcscmp(ppla[j]->lc->string, RIGHT_WALL_SUPPRESS)==0){
		    suppressor_used = TRUE;
		}
	    }
	}
	print_word_N = (((!suppressor_used) && (N_wall_connectors != 0)) 
			|| (N_wall_connectors > 1) || opts->display_walls);
    } 
    else {
	print_word_N = TRUE;
    }

    N_words_to_print = linkage->num_words;
    if (!print_word_N) N_words_to_print--;
    
    set_centers(linkage, print_word_0);
    line_len = center[N_words_to_print-1]+1;
    
    for (k=0; k<MAX_HEIGHT; k++) {
	for (j=0; j<line_len; j++) picture[k][j] = L' ';
	picture[k][line_len] = L'\0';
    }
    top_row = 0;
    
    for (link_length = 1; link_length < N_words_to_print; link_length++) {
	for (j=0; j<N_links; j++) {
	    if (ppla[j]->l == -1) continue;
	    if ((ppla[j]->r - ppla[j]->l) != link_length)
	      continue;
	    if (!print_word_0 && (ppla[j]->l == 0)) continue;
	    /* gets rid of the irrelevant link to the left wall */
	    if (!print_word_N && (ppla[j]->r == linkage->num_words-1)) continue;	    
	    /* gets rid of the irrelevant link to the right wall */

	    /* put it into the lowest position */
	    cl = center[ppla[j]->l];
	    cr = center[ppla[j]->r];
	    for (row=0; row < MAX_HEIGHT; row++) {
		for (k=cl+1; k<cr; k++) {
		    if (picture[row][k] != L' ') break;
		}
		if (k == cr) break;
	    }
	    /* we know it fits, so put it in this row */

	    link_heights[j] = row;
	    
	    if (2*row+2 > MAX_HEIGHT-1) {
		append_string(string, L"The diagram is too high.\n");
		gr_string = exalloc(sizeof(wchar_t)*(wcslen(string->p)+1));
		wcscpy(gr_string, string->p);
		exfree(string->p, sizeof(wchar_t)*string->allocated);
		exfree(string, sizeof(String));
		return gr_string; 
	    }
	    if (row > top_row) top_row = row;
	    
	    picture[row][cl] = L'+';
	    picture[row][cr] = L'+';
	    for (k=cl+1; k<cr; k++) {
		picture[row][k] = L'-';
	    }
	    s = ppla[j]->name;
	    
	    if (opts->display_link_subscripts) {
	      if (!iswalpha((wint_t)*s))
		s = L"";
	    } else {
	      if (!iswupper((wint_t)*s)) {
		s = L"";   /* Don't print fat link connector name */
	      }
	    }
	    wcsncpy(connector, s, MAX_TOKEN_LENGTH-1);
	    connector[MAX_TOKEN_LENGTH-1] = L'\0';
	    k=0;
	    if (opts->display_link_subscripts)
	      k = wcslen(connector);
	    else
	      for (t=connector; iswupper((wint_t)*t); t++) k++; /* uppercase len of conn*/
	    if ((cl+cr-k)/2 + 1 <= cl) {
		t = picture[row] + cl + 1;
	    } else {
		t = picture[row] + (cl+cr-k)/2 + 1;
	    }
	    s = connector;
	    if (opts->display_link_subscripts)
	      while((*s != L'\0') && (*t == L'-')) *t++ = *s++; 
	    else
	      while(iswupper((wint_t)*s) && (*t == L'-')) *t++ = *s++; 

	    /* now put in the | below this one, where needed */
	    for (k=0; k<row; k++) {
		if (picture[k][cl] == L' ') {
		    picture[k][cl] = L'|';
		}
		if (picture[k][cr] == L' ') {
		    picture[k][cr] = L'|';
		}
	    }
	}
    }
    
    /* we have the link picture, now put in the words and extra "|"s */
    
    s = xpicture[0];
    if (print_word_0) k = 0; else k = 1;
    for (; k<N_words_to_print; k++) {
	t = linkage->word[k];
	i=0;
	while(*t != L'\0') {
	    *s++ = *t++;
	    i++;
	}
	*s++ = L' ';
    }
    *s = L'\0';
    
    if (opts->display_short) {
	for (k=0; picture[0][k] != L'\0'; k++) {
	    if ((picture[0][k] == L'+') || (picture[0][k] == L'|')) {
		xpicture[1][k] = L'|';
	    } else {
		xpicture[1][k] = L' ';
	    }
	}
	xpicture[1][k] = L'\0';
	for (row=0; row <= top_row; row++) {
	    wcscpy(xpicture[row+2],picture[row]);
	}
	top_row = top_row+2;
    } else {
	for (row=0; row <= top_row; row++) {
	    wcscpy(xpicture[2*row+2],picture[row]);
	    for (k=0; picture[row][k] != L'\0'; k++) {
		if ((picture[row][k] == L'+') || (picture[row][k] == L'|')) {
		    xpicture[2*row+1][k] = L'|';
		} else {
		    xpicture[2*row+1][k] = L' ';
		}
	    }
	    xpicture[2*row+1][k] = L'\0';
	}
	top_row = 2*top_row + 2;
    }
    
    /* we've built the picture, now print it out */
    
    if (print_word_0) i = 0; else i = 1;
    k = 0;
    N_rows = 0;
    row_starts[N_rows] = 0;
    N_rows++;
    while(i < N_words_to_print) {
	append_string(string, L"\n");
	width = 0;
	do {
	    width += wcslen(linkage->word[i])+1;
	    i++;
	} while((i<N_words_to_print) &&
	      (width + ((int)wcslen(linkage->word[i]))+1 < x_screen_width));
	row_starts[N_rows] = i - (!print_word_0);    /* PS junk */
	if (i<N_words_to_print) N_rows++;     /* same */
	for (row = top_row; row >= 0; row--) {
	    flag = TRUE;
	    for (j=k;flag&&(j<k+width)&&(xpicture[row][j]!=L'\0'); j++){
		flag = flag && (xpicture[row][j] == L' ');
	    }
	    if (!flag) {
		for (j=k;(j<k+width)&&(xpicture[row][j]!=L'\0'); j++){
		    append_string(string, L"%c", xpicture[row][j]);
		}
		append_string(string, L"\n");
	    }
	}
	append_string(string, L"\n");
	k += width;
    }
    gr_string = exalloc(sizeof(wchar_t)*(wcslen(string->p)+1));
    wcscpy(gr_string, string->p);
    exfree(string->p, sizeof(wchar_t)*string->allocated);
    exfree(string, sizeof(String));
    return gr_string; 
}

wchar_t * linkage_print_postscript(Linkage linkage, int mode) {
    wchar_t * ps, * qs;
    int size;

    ps = build_linkage_postscript_string(linkage);
    size = wcslen(header(mode)) + wcslen(ps) + wcslen(trailer(mode)) + 1;
    
    qs = exalloc(sizeof(wchar_t)*size);
    swprintf_s(qs, size, L"%s%s%s", header(mode), ps, trailer(mode));
    exfree(ps, sizeof(wchar_t)*(wcslen(ps)+1));
						      
    return qs;
}

void print_disjunct_counts(Sentence sent) {
    int i;
    int c;
    Disjunct *d;
    for (i=0; i<sent->length; i++) {
	c = 0;
	for (d=sent->word[i].d; d != NULL; d = d->next) {
	    c++;
	}
	wprintf_s(L"%s(%d) ",sent->word[i].string, c);
    }
    wprintf_s(L"\n\n");
}

void print_expression_sizes(Sentence sent) {
    X_node * x;
    int w, size;
    for (w=0; w<sent->length; w++) {
	size = 0;
	for (x=sent->word[w].x; x!=NULL; x = x->next) {
	    size += size_of_expression(x->exp);
	}
	wprintf_s(L"%s[%d] ",sent->word[w].string, size);
    }
    wprintf_s(L"\n\n");
}

void print_sentence(FILE *fp, Sentence sent, int w) {
/* this version just prints it on one line.  */
    int i;
    if (sent->dict->left_wall_defined) i=1; else i=0;
    for (; i<sent->length - sent->dict->right_wall_defined; i++) {
	fwprintf_s(fp, L"%s ", sent->word[i].string);
    }
    fwprintf_s(fp, L"\n");
}

wchar_t * trailer(int mode) {
    static wchar_t * trailer_string=
        L"diagram\n"
	L"\n"
	L"%%EndDocument\n"
	;

    if (mode==1) return trailer_string;
    else return L"";
}

wchar_t * header(int mode) {
    static wchar_t * header_string=
        L"%!PS-Adobe-2.0 EPSF-1.2\n"
        L"%%Pages: 1\n"
        L"%%BoundingBox: 0 -20 500 200\n"
        L"%%EndComments\n"
        L"%%BeginDocument: \n"
        L"\n"
        L"% compute size of diagram by adding\n"
        L"% #rows x 8.5\n"
        L"% (#rows -1) x 10\n"
        L"% \\sum maxheight x 10\n"
        L"/nulllink () def                     % The symbol of a null link\n"
        L"/wordfontsize 11 def      % the size of the word font\n"
        L"/labelfontsize 9 def      % the size of the connector label font\n"
        L"/ex 10 def  % the horizontal radius of all the links\n"
        L"/ey 10 def  % the height of the level 0 links\n"
        L"/ed 10 def  % amount to add to this height per level\n"
        L"/radius 10 def % radius for rounded arcs\n"
        L"/row-spacing 10 def % the space between successive rows of the diagram\n"
        L"\n"
        L"/gap wordfontsize .5 mul def  % the gap between words\n"
        L"/top-of-words wordfontsize .85 mul def\n"
        L"             % the delta y above where the text is written where\n"
        L"             % the major axis of the ellipse is located\n"
        L"/label-gap labelfontsize .1 mul def\n"
        L"\n"
        L"/xwordfontsize 10 def      % the size of the word font\n"
        L"/xlabelfontsize 10 def      % the size of the connector label font\n"
        L"/xex 10 def  % the horizontal radius of all the links\n"
        L"/xey 10 def  % the height of the level 0 links\n"
        L"/xed 10 def  % amount to add to this height per level\n"
        L"/xradius 10 def % radius for rounded arcs\n"
        L"/xrow-spacing 10 def % the space between successive rows of the diagram\n"
        L"/xgap wordfontsize .5 mul def  % the gap between words\n"
        L"\n"
        L"/centerpage 6.5 72 mul 2 div def\n"
        L"  % this number of points from the left margin is the center of page\n"
        L"\n"
        L"/rightpage 6.5 72 mul def\n"
        L"  % number of points from the left margin is the the right margin\n"
        L"\n"
        L"/show-string-centered-dict 5 dict def\n"
        L"\n"
        L"/show-string-centered {\n"
        L"  show-string-centered-dict begin\n"
        L"  /string exch def\n"
        L"  /ycenter exch def\n"
        L"  /xcenter exch def\n"
        L"  xcenter string stringwidth pop 2 div sub\n"
        L"  ycenter labelfontsize .3 mul sub\n"
        L"  moveto\n"
        L"  string show\n"
        L"  end\n"
        L"} def\n"
        L"\n"
        L"/clear-word-box {\n"
        L"  show-string-centered-dict begin\n"
        L"  /string exch def\n"
        L"  /ycenter exch def\n"
        L"  /xcenter exch def\n"
        L"  newpath\n"
        L"  /urx string stringwidth pop 2 div def\n"
        L"  /ury labelfontsize .3 mul def\n"
        L"  xcenter urx sub ycenter ury sub moveto\n"
        L"  xcenter urx add ycenter ury sub lineto\n"
        L"  xcenter urx add ycenter ury add lineto\n"
        L"  xcenter urx sub ycenter ury add lineto\n"
        L"  closepath\n"
        L"  1 setgray fill\n"
        L"  0 setgray\n"
        L"  end\n"
        L"} def\n"
        L"\n"
        L"/diagram-sentence-dict 20 dict def\n"
        L"\n"
        L"/diagram-sentence-circle\n"
        L"{diagram-sentence-dict begin  \n"
        L"   /links exch def\n"
        L"   /words exch def\n"
        L"   /n words length def\n"
        L"   /Times-Roman findfont wordfontsize scalefont setfont\n"
        L"   /x 0 def\n"
        L"   /y 0 def\n"
        L"\n"
        L"   /left-ends [x dup words {stringwidth pop add gap add dup}\n"
        L"	                     forall pop pop] def\n"
        L"   /right-ends [x words {stringwidth pop add dup gap add} forall pop] def\n"
        L"   /centers [0 1 n 1 sub {/i exch def\n"
        L"		      left-ends i get\n"
        L"		      right-ends i get\n"
        L"		      add 2 div\n"
        L"		    } for ] def\n"
        L"\n"
        L"   x y moveto\n"
        L"   words {show gap 0 rmoveto} forall\n"
        L"\n"
        L"   .5 setlinewidth \n"
        L"\n"
        L"   links {dup 0 get /leftword exch def\n"
        L"          dup 1 get /rightword exch def\n"
        L"          dup 2 get /level exch def\n"
        L"          3 get /string exch def\n"
        L"          newpath\n"
        L"          string nulllink eq {[2] 1 setdash}{[] 0 setdash} ifelse\n"
        L"%          string nulllink eq {.8 setgray}{0 setgray} ifelse\n"
        L"          centers leftword get\n"
        L"	  y top-of-words add\n"
        L"          moveto\n"
        L"      \n"
        L"          centers rightword get\n"
        L"          centers leftword get\n"
        L"          sub 2  div dup\n"
        L"          radius \n"
        L"          lt {/radiusx exch def}{pop /radiusx radius def} ifelse\n"
        L"  \n"
        L"          \n"
        L" \n"
        L"          centers leftword get\n"
        L"	  y top-of-words add ey ed level mul add add\n"
        L"          centers rightword get\n"
        L"	  y top-of-words add ey ed level mul add add\n"
        L"	  radiusx\n"
        L"          arcto\n"
        L"          4 {pop} repeat\n"
        L"	  centers rightword get\n"
        L"          y top-of-words add ey ed level mul add add\n"
        L"	  centers rightword get\n"
        L"	  y top-of-words add\n"
        L"	  radiusx\n"
        L"	  arcto\n"
        L"          4 {pop} repeat\n"
        L"	  centers rightword get\n"
        L"	  y top-of-words add\n"
        L"	  lineto\n"
        L"\n"
        L"	  stroke\n"
        L"\n"
        L"          /radius-y    ey ed level mul add	  def\n"
        L"\n"
        L"	  /center-arc-x\n"
        L"	     centers leftword get centers rightword get add 2 div\n"
        L"	  def\n"
        L"	  \n"
        L"          /center-arc-y\n"
        L"             y top-of-words radius-y add add\n"
        L"	  def\n"
        L"\n"
        L"          /Courier-Bold findfont labelfontsize scalefont setfont \n"
        L"	  center-arc-x center-arc-y string clear-word-box\n"
        L"	  center-arc-x center-arc-y string show-string-centered\n"
        L"          } forall\n"
        L"	  end\n"
        L"  } def\n"
        L"\n"
        L"/diagramdict 20 dict def\n"
        L"\n"
        L"/diagram\n"
        L"{diagramdict begin\n"
        L"   /break-words exch def\n"
        L"   /links exch def\n"
        L"   /words exch def\n"
        L"   /n words length def\n"
        L"   /n-rows break-words length def\n"
        L"   /Times-Roman findfont wordfontsize scalefont setfont\n"
        L"\n"
        L"   /left-ends [0 dup words {stringwidth pop add gap add dup}\n"
        L"	                     forall pop pop] def\n"
        L"   /right-ends [0 words {stringwidth pop add dup gap add} forall pop] def\n"
        L"\n"
        L"   /lwindows [ break-words {left-ends exch get gap 2 div sub } forall ] def\n"
        L"   /rwindows [1 1 n-rows 1 sub {/i exch def\n"
        L"		      lwindows i get } for\nL"
        L"	              right-ends n 1 sub get gap 2 div add\n"
        L"	      ] def\n"
        L"\n"
        L"\n"
        L"    /max 0 def\n"
        L"    0 1 links length 1 sub {\n"
        L"	/i exch def\n"
        L"	/t links i get 2 get def\n"
        L"	t max gt {/max t def} if\n"
        L"      } for\n"
        L"\n"
        L"    /max-height ed max mul ey add top-of-words add row-spacing add def\n"
        L"    /total-height n-rows max-height mul row-spacing sub def\n"
        L"\n"
        L"    /max-width 0 def            % compute the widest window\n"
        L"    0 1 n-rows 1 sub {\n"
        L"        /i exch def\n"
        L"        /t rwindows i get lwindows i get sub def\n"
        L"        t max-width gt {/max-width t def} if\n"
        L"      } for\n"
        L"\n"
        L"    centerpage max-width 2 div sub 0 translate  % centers it\n"
        L"   % rightpage max-width sub 0 translate      % right justified\n"
        L"                        % Delete both of these to make it left justified\n"
        L"\n"
        L"   n-rows 1 sub -1 0\n"
        L"     {/i exch def\n"
        L"	gsave\n"
        L"	newpath\n"
        L"        %/centering centerpage rwindows i get lwindows i get sub 2 div sub def\n"
        L"               % this line causes each row to be centered\n"
        L"        /centering 0 def\n"
        L"               % set centering to 0 to prevent centering of each row \n"
        L"\n"
        L"	centering -100 moveto  % -100 because some letters go below zero\n"
        L"        centering max-height n-rows mul lineto\n"
        L"        rwindows i get lwindows i get sub centering add\n"
        L"                       max-height n-rows mul lineto\n"
        L"        rwindows i get lwindows i get sub centering add\n"
        L"                       -100 lineto\n"
        L"	closepath\n"
        L"        clip\n"
        L"	lwindows i get neg n-rows i sub 1 sub max-height mul translate\n"
        L"        centerpage centering 0 translate\n"
        L"        words links diagram-sentence-circle\n"
        L"	grestore\n"
        L"     } for\n"
        L"     end\n"
        L"} def \n"
        L"\n"
        L"/diagramx\n"
        L"{diagramdict begin\n"
        L"   /break-words exch def\n"
        L"   /links exch def\n"
        L"   /words exch def\n"
        L"   /n words length def\n"
        L"   /n-rows break-words length def\n"
        L"   /Times-Roman findfont xwordfontsize scalefont setfont\n"
        L"\n"
        L"   /left-ends [0 dup words {stringwidth pop add gap add dup}\n"
        L"	                     forall pop pop] def\n"
        L"   /right-ends [0 words {stringwidth pop add dup gap add} forall pop] def\n"
        L"\n"
        L"   /lwindows [ break-words {left-ends exch get gap 2 div sub } forall ] def\n"
        L"   /rwindows [1 1 n-rows 1 sub {/i exch def\n"
        L"		      lwindows i get } for\n"
        L"	              right-ends n 1 sub get xgap 2 div add\n"
        L"	      ] def\n"
        L"\n"
        L"\n"
        L"    /max 0 def\n"
        L"    0 1 links length 1 sub {\n"
        L"	/i exch def\n"
        L"	/t links i get 2 get def\n"
        L"	t max gt {/max t def} if\n"
        L"      } for\n"
        L"\n"
        L"    /max-height xed max mul xey add top-of-words add xrow-spacing add def\n"
        L"    /total-height n-rows max-height mul xrow-spacing sub def\n"
        L"\n"
        L"    /max-width 0 def            % compute the widest window\n"
        L"    0 1 n-rows 1 sub {\n"
        L"        /i exch def\n"
        L"        /t rwindows i get lwindows i get sub def\n"
        L"        t max-width gt {/max-width t def} if\n"
        L"      } for\n"
        L"\n"
        L"    centerpage max-width 2 div sub 0 translate  % centers it\n"
        L"   % rightpage max-width sub 0 translate      % right justified\n"
        L"                        % Delete both of these to make it left justified\n"
        L"\n"
        L"   n-rows 1 sub -1 0\n"
        L"     {/i exch def\n"
        L"	gsave\n"
        L"	newpath\n"
        L"        %/centering centerpage rwindows i get lwindows i get sub 2 div sub def\n"
        L"               % this line causes each row to be centered\n"
        L"        /centering 0 def\n"
        L"               % set centering to 0 to prevent centering of each row \n"
        L"\n"
        L"	centering -100 moveto  % -100 because some letters go below zero\n"
        L"        centering max-height n-rows mul lineto\n"
        L"        rwindows i get lwindows i get sub centering add\n"
        L"                       max-height n-rows mul lineto\n"
        L"        rwindows i get lwindows i get sub centering add\n"
        L"                       -100 lineto\n"
        L"	closepath\n"
        L"        clip\n"
        L"	lwindows i get neg n-rows i sub 1 sub max-height mul translate\n"
        L"        centerpage centering 0 translate\n"
        L"        words links diagram-sentence-circle\n"
        L"	grestore\n"
        L"     } for\n"
        L"     end\n"
        L"} def \n"
        L"\n"
        L"/ldiagram\n"
        L"{diagramdict begin\n"
        L"   /break-words exch def\n"
        L"   /links exch def\n"
        L"   /words exch def\n"
        L"   /n words length def\n"
        L"   /n-rows break-words length def\n"
        L"   /Times-Roman findfont wordfontsize scalefont setfont\n"
        L"\n"
        L"   /left-ends [0 dup words {stringwidth pop add gap add dup}\n"
        L"	                     forall pop pop] def\n"
        L"   /right-ends [0 words {stringwidth pop add dup gap add} forall pop] def\n"
        L"\n"
        L"   /lwindows [ break-words {left-ends exch get gap 2 div sub } forall ] def\n"
        L"   /rwindows [1 1 n-rows 1 sub {/i exch def\n"
        L"		      lwindows i get } for\n"
        L"	              right-ends n 1 sub get gap 2 div add\n"
        L"	      ] def\n"
        L"\n"
        L"\n"
        L"    /max 0 def\n"
        L"    0 1 links length 1 sub {\n"
        L"	/i exch def\n"
        L"	/t links i get 2 get def\n"
        L"	t max gt {/max t def} if\n"
        L"      } for\n"
        L"\n"
        L"    /max-height ed max mul ey add top-of-words add row-spacing add def\n"
        L"    /total-height n-rows max-height mul row-spacing sub def\n"
        L"\n"
        L"    /max-width 0 def            % compute the widest window\n"
        L"    0 1 n-rows 1 sub {\n"
        L"        /i exch def\n"
        L"        /t rwindows i get lwindows i get sub def\n"
        L"        t max-width gt {/max-width t def} if\n"
        L"      } for\n"
        L"\n"
        L"   % centerpage max-width 2 div sub 0 translate  % centers it\n"
        L"   % rightpage max-width sub 0 translate      % right justified\n"
        L"                        % Delete both of these to make it left justified\n"
        L"\n"
        L"   n-rows 1 sub -1 0\n"
        L"     {/i exch def\n"
        L"	gsave\n"
        L"	newpath\n"
        L"        %/centering centerpage rwindows i get lwindows i get sub 2 div sub def\n"
        L"               % this line causes each row to be centered\n"
        L"        /centering 0 def\n"
        L"               % set centering to 0 to prevent centering of each row \n"
        L"\n"
        L"	centering -100 moveto  % -100 because some letters go below zero\n"
        L"        centering max-height n-rows mul lineto\n"
        L"        rwindows i get lwindows i get sub centering add\n"
        L"                       max-height n-rows mul lineto\n"
        L"        rwindows i get lwindows i get sub centering add\n"
        L"                       -100 lineto\n"
        L"	closepath\n"
        L"        clip\n"
        L"	lwindows i get neg n-rows i sub 1 sub max-height mul translate\n"
        L"        centerpage centering 0 translate\n"
        L"        words links diagram-sentence-circle\n"
        L"	grestore\n"
        L"     } for\n"
        L"     end\n"
        L"} def \n"
	;
    if (mode==1) return header_string;
    else return L"";
}