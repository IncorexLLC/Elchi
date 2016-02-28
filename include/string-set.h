String_set * string_set_create(void);
wchar_t *       string_set_add(wchar_t * source_string, String_set * ss);
wchar_t *       string_set_lookup(wchar_t * source_string, String_set * ss);
void         string_set_delete(String_set *ss);
