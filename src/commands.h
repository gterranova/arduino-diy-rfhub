#ifndef Commands_h
#define Commands_h

#define MAX_ARGS 3

extern "C" {
    typedef struct cmdlist_t {
    const char *commandname;
    void (*commandfunc)(char*[MAX_ARGS]);
    } cmdlist_t;
}

#endif
