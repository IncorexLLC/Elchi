#ifndef _PHRASE_PARSEH_
#define _PHRASE_PARSEH_

#include "link-includes.h"

#define MAXINPUT 1024
#define DISPLAY_MAX 100
#define COMMENT_CHAR L'%'  /* input lines beginning with this are ignored */

#define DICTIONARY L"/afs/cs/project/link-8/link-3.1/data/3.1.dict"
#define PP_KNOWLEDGE L"/afs/cs/project/link-8/link-3.1/data/3.1.knowledge"
#define CONSTITUENT_KNOWLEDGE L"/afs/cs/project/link-8/link-3.1/data/constituent.knowledge"

static int display_all=FALSE;
static int max_sentence_length;
static int min_short_sent_len=20;
static PostProcessor pp;

String_set * phrase_ss;

struct {
  int left;
  int right;
  wchar_t * type;
  wchar_t domain_type;
  wchar_t * start_link;
  int start_num;
  int subl; 
  int canon;
  int valid;
  int aux;      /* 0: it's an ordinary VP (or other type); 1: it's an AUX, don't print it; 2:
		   it's an AUX, and print it */
} constituent[1000];

int myword[256];
int word_used[16][256];
struct {
  int num;
  int e[10];
  int valid;
} andlist[1000];

int templist[10];

Linkage get_postscript_input();

wchar_t ps_line[1000];
wchar_t word[100][100];
wchar_t linkexp[100];
wchar_t linknameexp[10];

struct {
  int lword;
  int rword;
  int mysnum;
  wchar_t type[100];
} xlink[100];

String_set * postscript_ss;

int fget_input_string(wchar_t *, FILE *, FILE *, Parse_Options);
int uppercompare(wchar_t *, wchar_t *);
void generate_misc_word_info(Linkage);
void count_words_used(Linkage);
int read_constituents_from_domains(Linkage, int, int);
void adjust_for_left_comma(Linkage, int);
void adjust_for_right_comma(Linkage, int);
int generate_complement_constituent(Linkage, int, int, wchar_t *, wchar_t *, wchar_t *, int);
void adjust_subordinate_clauses(Linkage, int, int);
void print_constituent(Linkage, int);
int merge_constituents(Linkage, int);
int find_next_element(Linkage, int, int, int, int);
int last_minute_fixes(Linkage, int);
int handle_islands(Linkage, int, int);
void print_constituent_structure(Linkage, int);
void derive_constituents(Linkage, PostProcessor, int);
void process_linkage(Linkage, Parse_Options);
void print_parse_statistics(Sentence, Parse_Options);
void bogus_linkage_delete(Linkage);
void setup_parse_options(Parse_Options);
void setup_panic_parse_options(Parse_Options);
Linkage get_postscript_input();

#endif
