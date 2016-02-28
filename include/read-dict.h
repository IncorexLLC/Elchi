int open_dictionary(wchar_t * dict_path_name, Dictionary dict);
int  read_dictionary(Dictionary dict);
void dict_display_word_info(Dictionary dict, wchar_t * s);
void print_dictionary_data(Dictionary dict);
void print_dictionary_words(Dictionary dict);
void print_expression(Exp *);
int  boolean_dictionary_lookup(Dictionary dict, wchar_t *);
int  boolean_abridged_lookup(Dictionary dict, wchar_t *);
int  delete_dictionary_words(Dictionary dict, wchar_t *);
void free_lookup_list(void); 
            /* really doesn't need to be called outside 
               of the dictionary lookup code.  */
Dict_node * dictionary_lookup(Dictionary dict, wchar_t *);    
            /* remember, this returns a list  */
Dict_node * abridged_lookup(Dictionary dict, wchar_t *);
Dict_node * insert_dict(Dictionary dict, Dict_node * n, Dict_node * newnode);
void        free_dictionary(Dictionary dict);
Exp *       Exp_create(Dictionary dict);
