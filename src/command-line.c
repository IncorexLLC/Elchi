#include "link-includes.h"

static struct {
    int verbosity;
    int timeout;
    int memory;
    int linkage_limit;
    int null_block;
    int islands_ok;
    int short_length;
    int batch_mode;  
    int panic_mode;  
    int allow_null;
    int echo_on; 
    int screen_width;
    int display_on;
    int display_constituents;
    int max_sentence_length;
    int display_postscript;
    int display_bad;
    int display_links;
    int display_walls;
    int display_union;
} local;

typedef struct {
    wchar_t * string;
    int    isboolean;
    wchar_t * description;
    int  * p;
} Switch;

Switch default_switches[] = {
    {L"verbosity",    0, L"Level of detail in output",        &local.verbosity},
    {L"timeout",      0, L"Abort parsing after this long",    &local.timeout},
    {L"memory",       0, L"Max memory allowed",               &local.memory},
    {L"limit",        0, L"The maximum linkages processed",   &local.linkage_limit},
    {L"null-block",   0, L"Size of blocks with null cost 1",  &local.null_block},
    {L"islands-ok",   1, L"Use of null-linked islands",       &local.islands_ok},
    {L"short",        0, L"Max length of short links",        &local.short_length},    
    {L"batch",        1, L"Batch mode",                       &local.batch_mode},
    {L"panic",        1, L"Use of \"panic mode\"",            &local.panic_mode},
    {L"null",         1, L"Null links",                       &local.allow_null},
    {L"width",        0, L"The width of the display",         &local.screen_width},
    {L"echo",         1, L"Echoing of input sentence",        &local.echo_on},
    {L"graphics",     1, L"Graphical display of linkage",     &local.display_on},
    {L"postscriptL",   1, L"Generate postscript output",       &local.display_postscript},
    {L"constituents", 0, L"Generate constituent output",      &local.display_constituents},
    {L"max-length",   0, L"Maximum sentence length",          &local.max_sentence_length},
    {L"bad",          1, L"Display of bad linkages",          &local.display_bad},
    {L"links",        1, L"Showing of complete link data",    &local.display_links},
    {L"walls",        1, L"Showing of wall words",            &local.display_walls},
    {L"union",        1, L"Showing of 'union' linkage",       &local.display_union},
    {NULL,           1,  NULL,                              NULL}
};

struct {wchar_t * s; wchar_t * str;} user_command[] = {
    {L"variables",    L"List user-settable variables and their functions"},
    {L"help",         L"List the commands and what they do"},
    {NULL,           NULL}
};

void clean_up_string(wchar_t * s) {
    /* gets rid of all the white space in the string s.  Changes s */
    wchar_t * x, * y;
    y = x = s;
    while(*x != L'\0') {
	if (!iswspace((wint_t)*x)) {
	    *y = *x; x++; y++;
	} else {
	    x++;
	}
    }
    *y = L'\0';
}

int is_numerical_rhs(wchar_t *s) {
    /* return TRUE if s points to a number:
     optional + or - followed by 1 or more
     digits.
     */
    if (*s==L'+' || *s == L'-') s++;
    if (*s == L'\0') return FALSE;
    for (; *s != L'\0'; s++) if (!iswdigit((wint_t)*s)) return FALSE;
    return TRUE;
}

void x_issue_special_command(wchar_t * line, Parse_Options opts, Dictionary dict) {
    wchar_t *s, myline[1000], *x, *y;
    int i, count, j, k;
    Switch * as = default_switches;

    wcsncpy(myline, line, sizeof(myline));
    myline[sizeof(myline)-1] = L'\0';
    clean_up_string(myline);

    s = myline;
    j = k = -1;
    count = 0;
    for (i=0; as[i].string != NULL; i++) {
		if (as[i].isboolean && strncasecmp(s, as[i].string, wcslen(s)) == 0) {
			count++;
			j = i;
		}
    }
    for (i=0; user_command[i].s != NULL; i++) {
		if (strncasecmp(s, user_command[i].s, wcslen(s)) == 0) {
			count++;
			k = i;
		}
    }

    if (count > 1) {
		wprintf_s(L"Ambiguous command.  Type \"!help\" or \"!variables\"\n");
		return;
    } 
    else if (count == 1) {
		if (j >= 0) {
			*as[j].p = !(*as[j].p);
			wprintf_s(L"%s turned %s.\n", as[j].description, (*as[j].p)? L"on" : L"off");
			return;
		} 
		else {
			/* replace the abbreviated command by the full one */
			wcscpy(s, user_command[k].s);  
		}
    }



    if (wcscmp(s, L"variables")==0) {
	wprintf_s(L" Variable     Controls                                      Value\n");
	wprintf_s(L" --------     --------                                      -----\n");
	for (i=0; as[i].string != NULL; i++) {
	    wprintf_s(L" ");
	    left_print_string(stdout, as[i].string, L"             ");
	    left_print_string(stdout, as[i].description, L"                                              ");
	    wprintf_s(L"%5d", *as[i].p);
	    if (as[i].isboolean) {
			if (*as[i].p) wprintf_s(L" (On)"); else wprintf_s(L" (Off)");
	    }
	    wprintf_s(L"\n");
	}
	wprintf_s(L"\n");
	wprintf_s(L"Toggle a boolean variable as in \"!batch\"; ");
	wprintf_s(L"set a variable as in \"!width=100\".\n");
	return;
    }
    if (wcscmp(s, L"help")==0) {
	wprintf_s(L"Special commands always begin with \"!\".  Command and variable names\n");
	wprintf_s(L"can be abbreviated.  Here is a list of the commands:\n\n");
	for (i=0; user_command[i].s != NULL; i++) {
	    wprintf_s(L" !");
	    left_print_string(stdout, user_command[i].s, L"                  ");
	    left_print_string(stdout, user_command[i].str, L"                                                    ");
	    wprintf_s(L"\n");
	}
	wprintf_s(L" !!<string>         Print all the dictionary words matching <string>.\n");
	wprintf_s(L"                    Also print the number of disjuncts of each.\n");
	wprintf_s(L"\n");
	wprintf_s(L" !<var>             Toggle the specified boolean variable.\n");
	wprintf_s(L" !<var>=<val>       Assign that value to that variable.\n");
	return;
    }

    if(s[0] == L'!') {
	dict_display_word_info(dict, s+1);
	return;
    }
    
    /* test here for an equation */
    for (x=s; (*x != L'=') && (*x != L'\0') ; x++)
      ;
    if (*x == L'=') {
	*x = L'\0';
	y = x+1;
	x = s;
	/* now x is the first word and y is the rest */

	if (is_numerical_rhs(y)) {
	    for (i=0; as[i].string != NULL; i++) {
		if (wcscmp(x, as[i].string) == 0) break;
	    }
	    if (as[i].string  == NULL) {
		wprintf_s(L"There is no user variable called \"%s\".\n", x);
	    } else {
			*(as[i].p) = _wtoi(y);
			wprintf_s(L"%s set to %d\n", x, _wtoi(y));
	    }
	    return;
	}
    }

    wprintf_s(L"I can't interpret \"%s\" as a command.  Try \"!help\".\n", myline);
}

void put_opts_in_local_vars(Parse_Options opts) {
    local.verbosity = parse_options_get_verbosity(opts);
    local.timeout = parse_options_get_max_parse_time(opts);;
    local.memory = parse_options_get_max_memory(opts);;
    local.linkage_limit = parse_options_get_linkage_limit(opts);
    local.null_block = parse_options_get_null_block(opts);
    local.islands_ok = parse_options_get_islands_ok(opts);
    local.short_length = parse_options_get_short_length(opts);
    local.echo_on = parse_options_get_echo_on(opts);
    local.batch_mode = parse_options_get_batch_mode(opts);
    local.panic_mode = parse_options_get_panic_mode(opts);
    local.screen_width = parse_options_get_screen_width(opts);
    local.allow_null = parse_options_get_allow_null(opts);
    local.screen_width = parse_options_get_screen_width(opts);
    local.display_on = parse_options_get_display_on(opts);
    local.display_postscript = parse_options_get_display_postscript(opts);
    local.display_constituents = parse_options_get_display_constituents(opts);
    local.max_sentence_length = parse_options_get_max_sentence_length(opts);
    local.display_bad = parse_options_get_display_bad(opts);
    local.display_links = parse_options_get_display_links(opts);
    local.display_walls = parse_options_get_display_walls(opts);
    local.display_union = parse_options_get_display_union(opts);
}

void put_local_vars_in_opts(Parse_Options opts) {
    parse_options_set_verbosity(opts, local.verbosity);
    parse_options_set_max_parse_time(opts, local.timeout);
    parse_options_set_max_memory(opts, local.memory);
    parse_options_set_linkage_limit(opts, local.linkage_limit);
    parse_options_set_null_block(opts, local.null_block);
    parse_options_set_islands_ok(opts, local.islands_ok);
    parse_options_set_short_length(opts, local.short_length);
    parse_options_set_echo_on(opts, local.echo_on);
    parse_options_set_batch_mode(opts, local.batch_mode);
    parse_options_set_panic_mode(opts, local.panic_mode);
    parse_options_set_screen_width(opts, local.screen_width);
    parse_options_set_allow_null(opts, local.allow_null);
    parse_options_set_screen_width(opts, local.screen_width);
    parse_options_set_display_on(opts, local.display_on);
    parse_options_set_display_postscript(opts, local.display_postscript);
    parse_options_set_display_constituents(opts, local.display_constituents);
    parse_options_set_max_sentence_length(opts, local.max_sentence_length);
    parse_options_set_display_bad(opts, local.display_bad);
    parse_options_set_display_links(opts, local.display_links);
    parse_options_set_display_walls(opts, local.display_walls);
    parse_options_set_display_union(opts, local.display_union);
}

void issue_special_command(wchar_t * line, Parse_Options opts, Dictionary dict) {
    put_opts_in_local_vars(opts);
    x_issue_special_command(line, opts, dict);
    put_local_vars_in_opts(opts);
}


