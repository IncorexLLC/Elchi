typedef struct Alink_s Alink;
struct Alink_s {
    Alink * next;
    wchar_t * left, * right, * connector;
    int leftsub, rightsub, ignore, main_s, ok;
};

typedef struct Alinkset_s Alinkset;
struct Alinkset_s {
    Alinkset * next;
    Alink * link;
    int ok;
    Andlist * andlist;
};

double word_position[100];
wchar_t line[200];
int numlinks, numwords;
int localv;

Disjunct * choose_disjunct(wchar_t *, Alink *, Dictionary, int); 
