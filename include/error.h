#ifndef _ERRORH_
#define _ERRORH_

typedef enum {
    NODICT=1,
    DICTPARSE,
    WORDFILE,
    SEPARATE,
    NOTINDICT,
    BUILDEXPR,
    INTERNALERROR,
}   LP_error_type;

void lperror(int lperr, wchar_t *fmt, ...);
void error(wchar_t *fmt, ...);

#endif

