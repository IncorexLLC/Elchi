#include "link-includes.h"
#include "alink.h"
#include "translator.h"

print_input_blurb(Linkage linkage, Alinkset * alinkset) {

    wchar_t * string;

    wprintf_s(L"    -------------------------------------------------------------");
/*
    TUTORIAL. The translation of a sentence involves four basic
    stages: parsing, word replacement, transformation, and word 
    positioning.

    I. PARSING. The translator begins by parsing the English sentence
    using the normal link grammar procedure. If more than one linkage
    is produced for the sentence, it chooses the 'lowest-cost' linkage
    (the one that would be outputted first by the normal English parser); 
    this is the one that will get translated. In this case the linkage 
    is as follows:\n");

    string = linkage_print_diagram(linkage);
    fwprintf(stdout, "%s", string);
    string_delete(string);

    wprintf_s("    Note that a linkage consists of a set of LINKS--a link being a
    pair of words with a link type in between. Rather than arranging
    the words of a linkage in a particular order, we can also think of
    a linkage as an unordered set of links. Such a structure, called a
    LINKSET, is the basic representation of a sentence used by the
    translator. In this case, the linkset looks like this:\n\n");
*/
    print_links(alinkset->link);
/*
    wprintf_s("\n    (The numbers beside the words are necessary to show whether or not
    two instances of the same word actually refer to the same word in
    the sentence.)\n\n");
*/
}

print_input_transf_blurb(Alinkset * alinkset) {
/*
    wprintf_s("    Before proceeding, it is necessary to manipulate the input linkset
    a little bit in order to prepare it for translation. For example,
    progressive verb forms ('-ing' words) are removed, as German does
    not have these; relative pronouns are added where necessary
    (sometimes these are omitted in English); and the auxiliary 'do' 
    is removed (often used in English questions to allow subject-verb
    inversion, but not needed in German).  In the current case, these
    initial transformations (plus a few others) produce the following
    linkset:\n\n");
*/
    print_links(alinkset->link);
    wprintf_s(L"\n");
}

print_dumb_replacement_blurb(Alinkset * alinkset) {
/*
    wprintf_s("    II. WORD REPLACEMENT. The next step is to replace all the words
    of the English linkset. To do this, the translator uses 'tables'
    which indicate the correspondences between English and German
    words. The tables use the same format as link grammar
    dictionaries; they consist of lists of words, followed by a
    'connector' which in this case simply indicates the semantic
    category of the word.

       ENGLISH TABLE               GERMAN TABLE
                                    
       dog.n: DOG+;                Hund.n: DOG+;
       he him: HE+;                er ihn ihm: HE+;
       the: THE+;                  der die das den dem des: THE+;
       run.v runs.v ran.v          laufe.v laufst.v lauft.v 
          running.v: RUN+;            (etc.): RUN+;

    To replace the English words, the translator looks up each English
    word in the English table, finds its 'category connector', and
    searches for a word in the German table with the same category
    connector.  Note that all the forms of a given verb are contained
    in a single category; the same with different definite articles
    and different forms of an adjective (though this arises only in
    German), and pronouns of different cases. How does the translator
    know which form to choose? To start with, the translator just
    chooses the first form that it finds; this is called 'dumb
    replacement'. In this case, the linkset produced (after all
    English words have been replaced) is as follows:\n\n");
*/
    print_links(alinkset->link);
    wprintf_s(L"\n");
}

print_smart_replacement_blurb(Alinkset * alinkset) {
/*
    wprintf_s("    You will probably see that the translator did not initially choose
    the right forms for all of the words that it replaced. Articles
    may be the wrong case or gender, verbs may be the wrong form, and
    so on. Thus the translator must choose the correct forms. This is
    done using a German dictionary, similar to the standard link 
    grammar dictionary for English. You may recall that, in link
    grammar, things like noun-verb agreement are specified by
    subscripts on connectors: singular nouns and verbs have an Ss
    connector, plural forms have an Sp. Agreement is specified in a
    similar way in the German dictionary; and this subscript
    information is what the translator uses to choose the correct
    forms. For example, in translating the link 'the D dog', the
    translator sees that the word 'Hund' in the German dictionary has
    a D*m- connector (m for masculine); thus, from among the words in
    the German table in the 'THE' category, it must look up each one
    in the German dictionary and find one with a D*m+ connector. In
    this case, 'smart word replacement' produces the following
    linkset:\n\n");
*/
    print_links(alinkset->link);
    wprintf_s(L"\n");
}

print_ending_blurb() {
/*
    wprintf_s("    Your sentence contained a 'attributive adjective' - an adjective
    before a noun. Such adjectives require endings, depending on the case
    and gender of the following noun. The translator treats adjective
    endings as separate words; this works in the following manner. Link
    logic ensures that the 'A' link connecting the noun to the adjective
    will be subscripted according to the case and gender of the
    noun. Based on the subscript of the A link, an appropriate adjective
    ending is then chosen, and an 'FS' link is added connecting the
    adjective to its ending. (When the words are printed out, the space
    between the adjective and the ending is omitted, thus combining them
    into a single word.)\n\n");
*/
}

print_switchhead_blurb(Alinkset * alinkset) {
/*
    wprintf_s("    III. TRANSFORMATION. While some simple sentences can be translated
    by simple word-to-word replacement, this is usually not
    sufficient. In many cases, words need to be added or deleted, or
    word order must be changed. The translator accomplishes this
    through 'transformations': manipulations of the links in the
    linkset. One transformation in particular is required by nearly
    every sentence. In standard English link grammar, the 'head' of a
    clause--the word attaching to the rest of the sentence, or to the
    'left-wall' in the case of the main clause--is the subject noun;
    in German, however, it proves to be much more convenient to treat
    the finite verb as the head word. This can be accomplished simply
    by replacing the subject noun with the finite verb as the right
    end of the W link (or the C link in the case of dependent
    clauses), thus in effect transferring the W link from the subject
    noun to the finite verb.  In the current sentence, this
    produces:\n\n");
*/
    print_links(alinkset->link);
    wprintf_s(L"\n");
}

print_transformation_blurb(Alinkset * alinkset) {
/*
    wprintf_s("    In some cases, other transformations are necessary. If the
    sentence contains a participle, and a prepositional phrase or
    something else following it, the participle must be put at the end
    of the clause. This can be done simply by reversing the order of
    the participle and the preposition in the link that connects
    them. If the sentence contains any dependent clauses, their finite
    verbs must be put at the end. And If the sentence contains an
    'opener' phrase, the noun and the finite verb of the main clause
    must be inverted. Each of these transformations (and a few others
    that I will not discuss here) can be accomplished simply by
    inverting one or more of the links in the sentence. (In most
    cases, the link type is also changed.) In the current case, all of
    these transformations produce the following:\n\n");
*/
    print_links(alinkset->link);
    wprintf_s(L"\n");
}

print_conjunction_blurb(Alinkset * alinkset) {
/*
    wprintf_s("    The sentence you typed in contains the word 'and'. Sentences with
    conjunctions require special handling by link grammar, since they
    often involve crossing links. Conjunction sentences are first
    analyzed as a 'fat linkage', a kind of linkage in which the set of
    words joined by the conjunction (the 'andlist') attach to the
    conjunction itself which then attaches to the 'outside world'.
    This is then transformed into a 'union linkage', in which the
    words of the andlist attach directly to outside words, sometimes
    leading to crossing links. The initial linkset for your sentence
    reflects the 'union linkage'; this is what is subjected to the
    word replacement and transformation procedures described
    above. After these procedures, however, the conjunction(s) must be
    put back into the sentence somehow, in a way that avoids crossing
    links (this will be important for what follows). Essentially, we
    reconstruct a 'fat linkset', in which all the words of the andlist
    connect only to the conjunction. In the current case, this
    produces the following linkset:\n\n");
*/
    print_links(alinkset->link);
    wprintf_s(L"\n");
}

print_positioning_blurb() {
/*
    wprintf_s("    IV. WORD-POSITIONING. Once the transformations of the linkset are
    complete, it must be turned back into a sentence. It can be seen
    that the position of words in the linkset constrains their
    relative order: if the linkset contains the link 'dog S run', this
    means that 'dog' must occur before 'run'. However, the linkset
    still allows many possible orderings of the words; how does the
    translator know which one to choose? Here again, the German
    dictionary comes in useful. Recall that the ordering of connectors
    in a link grammar dictionary expression dictates the relative
    closeness of the words being linked to. The fact that the
    expression for 'dog' contains '{@A-} & D- & (O- or ...)' means
    that the word making an A link to 'dog', if any, must be closest,
    then the word making a D link, then the word making an O
    link. Thus the translator proceeds as follows. For each word in
    the linkset, it looks at all the links that contain that word; it
    then consults the German dictionary to find the correct relative
    closeness of each of these links. (In most cases, there is only
    one legal ordering for any set of possible links to a word.) If a
    noun has an O, an A, and a D link coming out of it, the word on
    the end of the A link must be closest, and so on. (More
    specifically, the translator begins with the 'left-wall' at the
    left end of the sentence, looks at all the words connected to it
    and assigns them positions on a real number line, and then repeats
    this process recursively, assigning positions to the remaining
    words in the gaps between those already positioned. The words can
    then be printed in order of their positions on the real number
    line.) In this case, this leads to the following final ordering of
    the words:\n\n");
*/
}

print_nomatch_error(wchar_t * word) {
/*
    wprintf_s("    At this point, the translator would normally replace all the
    English words in the sentence with German words, using translation
    'tables'. However, at least one of the words in your
    sentence--'%s'--was not present in the English table. Try another
    sentence, using only the words listed at the top of the page.\n",
    word);
*/
}

print_noinlinkage_error() {
/*
    wprintf_s("    The translator would normally begin by generating a linkage for
    the input sentence. In this case, however, it was unable to find a
    linkage ('null links' are not allowed). See if you can construct a
    grammatical English sentence using the words listed above.\n");
*/
}

print_nooutlinkage_error(wchar_t * word) {
/*
    wprintf_s("    At this point, the translator would normally turn the German
    linkset into an ordered sequence of words, using the constraints
    on ordering specified by the linkset as well as other information
    in the German dictionary. However, it was unable to do that in
    this case. Probably you used a word in a way that the translator
    cannot understand. For example, the translator cannot handle the
    verb 'run' used transitively; it always translates 'run' as the
    intransitive 'laufen'. If 'run' is used transitively (with an 'O'
    link to the right), the translator will still replace it with a
    form of 'laufen'; but the German dictionary does not provide any
    usage of 'laufen' with an O to the right, so the words cannot be
    positioned and no sentence can be generated.\n\n");
*/
}