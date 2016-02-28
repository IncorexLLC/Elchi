#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

#include "error.h"

static wchar_t   buf[1024];
#define CRLF  wprintf_s(L"\n")

int    lperrno;
wchar_t   lperrmsg[1024];

const wchar_t * msg_of_lperror(int lperr) {
    switch(lperr) {
    case NODICT:
	return L"Could not open dictionary ";
	break;
    case DICTPARSE:
	return L"Error parsing dictionary ";
	break;
    case WORDFILE:
	return L"Error opening word file ";
	break;
    case SEPARATE:
	return L"Error separating sentence ";
	break;
    case NOTINDICT:
	return L"Sentence not in dictionary ";
	break;
    case BUILDEXPR:
	return L"Could not build sentence expressions ";
	break;
    case INTERNALERROR:
	return L"Internal error.  Send mail to link@juno.com ";
	break;
    default:
	return L"";
	break;
    }
    return NULL;
}


void lperror(int lperr, wchar_t *fmt, ...) {
    wchar_t temp[1024];
    va_list args;

    va_start(args, fmt);
    swprintf_s(lperrmsg, 1024, L"PARSER-API: %s", msg_of_lperror(lperr));
    vswprintf_s(temp, 1024, fmt, args); 
    wcscat_s(lperrmsg, 1024, temp);
    va_end(args);
    lperrno = lperr;
    fflush(stderr);
}

void error(wchar_t *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args); CRLF;
    va_end(args);
    fwprintf_s(stderr, L"\n");
    if (errno > 0) {
	perror(buf);
	fwprintf_s(stderr, L"errno=%d\n", errno);
	fwprintf_s(stderr, buf);
	fwprintf_s(stderr, L"\n");
    }
    fflush(stderr);
    fflush(stdout);
    fwprintf_s(stderr, L"Parser quitting.\n");
    exit(1); /* Always fail and print out this file name */
}