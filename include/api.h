 /****************************************************************************/
 /*                                                                          */
 /*  Copyright (C) 1991-2000, all rights reserved                            */
 /*  Daniel Sleator, David Temperley, and John Lafferty                      */
 /*  See file "README" for information about commercial use of this system   */
 /*                                                                          */
 /****************************************************************************/

/*****************************************************************************
*
* Functions to manipulate Dictionaries
*
*****************************************************************************/


Dictionary dictionary_create(wchar_t * dict_name, wchar_t * pp_name, wchar_t * cons_name, wchar_t * affix_name);
int           dictionary_delete(Dictionary dict);
int           dictionary_get_max_cost(Dictionary dict);
/*  obsolete *DS*
void          dictionary_open_affix_file(Dictionary dict, wchar_t * affix_file);
void          dictionary_open_constituent_knowledge(Dictionary dict, wchar_t * cons_file);
*/

/*****************************************************************************
*
* Functions to manipulate Parse Options 
*
*****************************************************************************/

Parse_Options  parse_options_create();
int            parse_options_delete(Parse_Options opts);
void           parse_options_set_verbosity(Parse_Options opts, int verbosity);
int            parse_options_get_verbosity(Parse_Options opts);
void           parse_options_set_linkage_limit(Parse_Options opts, int linkage_limit);
int            parse_options_get_linkage_limit(Parse_Options opts);
void           parse_options_set_disjunct_cost(Parse_Options opts, int disjunct_cost);
int            parse_options_get_disjunct_cost(Parse_Options opts);
void           parse_options_set_min_null_count(Parse_Options opts, int null_count);
int            parse_options_get_min_null_count(Parse_Options opts);
void           parse_options_set_max_null_count(Parse_Options opts, int null_count);
int            parse_options_get_max_null_count(Parse_Options opts);
void           parse_options_set_null_block(Parse_Options opts, int null_block);
int            parse_options_get_null_block(Parse_Options opts);
void           parse_options_set_islands_ok(Parse_Options opts, int islands_ok);
int            parse_options_get_islands_ok(Parse_Options opts);
void           parse_options_set_short_length(Parse_Options opts, int short_length);
int            parse_options_get_short_length(Parse_Options opts);
void           parse_options_set_max_memory(Parse_Options  opts, int mem);
int            parse_options_get_max_memory(Parse_Options opts);
void           parse_options_set_max_sentence_length(Parse_Options  opts, int len);
int            parse_options_get_max_sentence_length(Parse_Options opts);
void           parse_options_set_max_parse_time(Parse_Options  opts, int secs);
int            parse_options_get_max_parse_time(Parse_Options opts);
void           parse_options_set_cost_model_type(Parse_Options opts, int cm);
int            parse_options_get_cost_model_type(Parse_Options opts);
int            parse_options_timer_expired(Parse_Options opts);
int            parse_options_memory_exhausted(Parse_Options opts);
int            parse_options_resources_exhausted(Parse_Options opts);
void           parse_options_set_screen_width(Parse_Options opts, int val);
int            parse_options_get_screen_width(Parse_Options opts);
void           parse_options_set_allow_null(Parse_Options opts, int val);
int            parse_options_get_allow_null(Parse_Options opts);
void           parse_options_set_display_walls(Parse_Options opts, int val);
int            parse_options_get_display_walls(Parse_Options opts);
void           parse_options_set_all_short_connectors(Parse_Options opts, int val);
int            parse_options_get_all_short_connectors(Parse_Options opts);
void           parse_options_reset_resources(Parse_Options opts);


/*****************************************************************************
*
* The following Parse_Options functions do not directly affect the
* operation of the parser, but they can be useful for organizing the
* search, or displaying the results.  They were included as switches for
* convenience in implementing the "standard" version of the link parser
* using the API.
*
*****************************************************************************/

void           parse_options_set_batch_mode(Parse_Options opts, int val);
int            parse_options_get_batch_mode(Parse_Options opts);
void           parse_options_set_panic_mode(Parse_Options opts, int val);
int            parse_options_get_panic_mode(Parse_Options opts);
void           parse_options_set_display_on(Parse_Options opts, int val);
int            parse_options_get_display_on(Parse_Options opts);
void           parse_options_set_display_postscript(Parse_Options opts, int val);
int            parse_options_get_display_postscript(Parse_Options opts);
void           parse_options_set_display_constituents(Parse_Options opts, int val);
int            parse_options_get_display_constituents(Parse_Options opts);
void           parse_options_set_display_bad(Parse_Options opts, int val);
int            parse_options_get_display_bad(Parse_Options opts);
void           parse_options_set_display_links(Parse_Options opts, int val);
int            parse_options_get_display_links(Parse_Options opts);
void           parse_options_set_display_union(Parse_Options opts, int val);
int            parse_options_get_display_union(Parse_Options opts);
void           parse_options_set_echo_on(Parse_Options opts, int val);
int            parse_options_get_echo_on(Parse_Options opts);

/*****************************************************************************
*
* Functions to manipulate Sentences
*
*****************************************************************************/


Sentence     sentence_create(wchar_t *input_string, Dictionary dict);
void         sentence_delete(Sentence sent);
int          sentence_parse(Sentence sent, Parse_Options opts);
int          sentence_length(Sentence sent);
wchar_t *       sentence_get_word(Sentence sent, int wordnum);
int          sentence_null_count(Sentence sent);
int          sentence_num_linkages_found(Sentence sent);
int          sentence_num_valid_linkages(Sentence sent);
int          sentence_num_linkages_post_processed(Sentence sent);
int          sentence_num_violations(Sentence sent, int i);
int          sentence_disjunct_cost(Sentence sent, int i);


/*****************************************************************************
*
* Functions that create and manipulate Linkages.
* When a Linkage is requested, the user is given a
* copy of all of the necessary information, and is responsible
* for freeing up the storage when he/she is finished, using
* the routines provided below.
*
*****************************************************************************/


Linkage      linkage_create(int index, Sentence sent, Parse_Options opts);
int          linkage_set_current_sublinkage(Linkage linkage, int index);
void         linkage_delete(Linkage linkage);
Sentence     linkage_get_sentence(Linkage linkage);
int          linkage_get_num_sublinkages(Linkage linkage);
int          linkage_get_num_words(Linkage linkage);
int          linkage_get_num_links(Linkage linkage);
int          linkage_get_link_lword(Linkage linkage, int index);
int          linkage_get_link_rword(Linkage linkage, int index);
int          linkage_get_link_length(Linkage linkage, int index);
wchar_t *       linkage_get_link_label(Linkage linkage, int index);
wchar_t *       linkage_get_link_llabel(Linkage linkage, int index);
wchar_t *       linkage_get_link_rlabel(Linkage linkage, int index);
int          linkage_get_link_num_domains(Linkage linkage, int index);
wchar_t **      linkage_get_link_domain_names(Linkage linkage, int index);
wchar_t **      linkage_get_words(Linkage linkage);
wchar_t *       linkage_get_word(Linkage linkage, int w);
wchar_t *       linkage_print_links_and_domains(Linkage linkage);
wchar_t *       linkage_print_postscript(Linkage linkage, int mode);
wchar_t *       linkage_print_diagram(Linkage linkage);
wchar_t *       linkage_print_constituents(Linkage linkage);
int          linkage_compute_union(Linkage linkage);
int          linkage_unused_word_cost(Linkage linkage);
int          linkage_disjunct_cost(Linkage linkage);
int          linkage_and_cost(Linkage linkage);
int          linkage_link_cost(Linkage linkage);
int          linkage_is_canonical(Linkage linkage);
int          linkage_is_improper(Linkage linkage);
int          linkage_has_inconsistent_domains(Linkage linkage);
wchar_t *       linkage_get_violation_name(Linkage linkage);


/*****************************************************************************
* 
* Functions that allow special-purpose post-processing of linkages 
*
*****************************************************************************/

Postprocessor * post_process_open(wchar_t *dictname, wchar_t *path);
void            post_process_close(PostProcessor postprocessor);
void            linkage_post_process(Linkage linkage, PostProcessor postprocessor);

