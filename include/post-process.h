/********************************************************************** 
  Calling paradigm:
   . call post_process_open() with the name of a knowledge file. This
     returns a handle, used for all subsequent calls to post-process.
   . Do for each sentence:
       - Do for each generated linkage of sentence:
             + call post_process_scan_linkage()
       - Do for each generated linkage of sentence:
             + call post_process()
       - Call post_process_close_sentence() 
***********************************************************************/

#ifndef _POSTPROCESSH_
#define _POSTPROCESSH_

#define PP_FIRST_PASS  1
#define PP_SECOND_PASS 2

/* Postprocessor * post_process_open(wchar_t *dictname, wchar_t *path);  this is in api-prototypes.h */

void     post_process_free_data(PP_data * ppd);
void     post_process_close_sentence(Postprocessor *);
void     post_process_scan_linkage(Postprocessor * pp, Parse_Options opts,
				   Sentence sent , Sublinkage * sublinkage);
PP_node *post_process(Postprocessor * pp, Parse_Options opts, 
		      Sentence sent, Sublinkage *, int cleanup);
int      post_process_match(wchar_t *s, wchar_t *t);  /* utility function */

void          free_d_type(D_type_list * dtl);
D_type_list * copy_d_type(D_type_list * dtl);


#endif
