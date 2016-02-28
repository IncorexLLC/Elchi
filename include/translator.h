#define MAXINPUT 1024

struct {
    wchar_t * estring;
    wchar_t * gstring;
    int tense;
    int xcase;
    int def;
    int particle;
    int conj;
} tword[100];

String_set * translator_strings;
int localv;
Alinkset * build_input_linkset(Linkage);
void initialize_positions(int);
void modify_input_linkset(Alinkset *);
int remove_do_transform(Alink *);
int remove_progressive(Alink *);
int genitive_transform(Alink *);
int modify_relative_clause(Alink *);
int add_that(Alink *);
void set_tenses_of_input_words(Dictionary, Alink *);
Alink * replace_words_dumbly(Dictionary, Dictionary, Alink *);
Alinkset * replace_words(Dictionary, Dictionary, Dictionary, Alinkset *);
void split_separable_words(Dictionary, Alink *, int);
int set_case_values(Alink *, Dictionary, Dictionary);
int choose_correct_wordforms(Dictionary, Dictionary, Alinkset *, Alink *, int);
wchar_t * find_connector_on_word(Dictionary, wchar_t *, wchar_t *, int, int);
void apply_transformations(Alinkset *);
int switch_head(Alinkset *);
void adjust_infinitive_clause(Alink *, Alink *);
void reattach_subordinate_clause(Alink *, Alink *);
void put_participle_at_end(Alink *, Alink *);
int choose_participles(Dictionary, Dictionary, Alink *, Alink *, int);
void s_v_inversion(Alink *, Alink *);
void put_finite_verb_at_end(Alink *, Alink *);
void add_commas(Alink *, Alink *, int);
int generate_fat_linkage(Linkage, Alink *);
Alink * create_new_link(Alink *, wchar_t *, int, wchar_t *, int, wchar_t *);
void replace_one_word(Alink *, int, wchar_t *, int);
void print_links(Alink *);
int is_outside_world_word(int, int, Alinkset *);
void remove_duplicate_links(Alink *);
void free_alinkset(Alinkset *);

int position_words(Dictionary, Alink *, int, int, double, double);
int evaluate_disjunct(Disjunct *, Alink *, int);
void insert_conjunction_commas(Alinkset *);
int print_words_in_order(Alink *);