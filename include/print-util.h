#ifndef _PRINTUTILH_
#define _PRINTUTILH_

typedef struct String_s String;
struct String_s {
    unsigned int allocated;  /* Unsigned so VC++ doesn't complain about comparisons */
    wchar_t * p;
    wchar_t * eos;
};

String * String_create();
int append_string(String * string, wchar_t *fmt, ...);

#endif


