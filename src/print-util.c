#include <stdarg.h>
#include "link-includes.h"

/* This is a "safe" append function, used here to build up a link diagram
   incrementally.  Because the diagram is built up a few characters at
   a time, we keep around a pointer to the end of string to prevent
   the algorithm from being quadratic. */

String * String_create() {
    String * string;
    string = (String *) exalloc(sizeof(String));
    string->allocated = 1;
    string->p = (wchar_t *) exalloc(sizeof(wchar_t));
    string->p[0] = L'\0';
    string->eos = string->p;
    return string;
}

int append_string(String * string, wchar_t *fmt, ...) {
    wchar_t temp_string[1024];
    wchar_t * p;
    int new_size;
    va_list args;

    va_start(args, fmt);
    vswprintf_s(temp_string, 1024, fmt, args); 
    va_end(args);

    if (string->allocated <= wcslen(string->p)+wcslen(temp_string)) {
	new_size = 2*string->allocated+wcslen(temp_string)+1;
	p = exalloc(sizeof(wchar_t)*new_size);
	wcscpy(p, string->p);
	wcscat(p, temp_string);
	exfree(string->p, sizeof(wchar_t)*string->allocated);
	string->p = p;
	string->eos = wcschr(p,L'\0');
	string->allocated = new_size;
    }
    else {
	wcscat(string->eos, temp_string);
	string->eos += wcslen(temp_string);
    }

    return 0;
}