
#ifndef _UF_ASXPARSER_H
#define _UF_ASXPARSER_H

#include "uf_playlist.h"

typedef struct asx_parser_t asx_parser_t;

typedef struct
{
    char *buffer;
    int line;
} asx_line_save_t;

struct asx_parser_t
{
    int line; // Curent line
    asx_line_save_t *ret_stack;
    int ret_stack_size;
    char *last_body;
    int deep;
};

asx_parser_t *asx_parser_new(void);

void asx_parser_free(asx_parser_t *parser);

/*
 * Return -1 on error, 0 when nothing is found, 1 on sucess
 */
int asx_get_element(asx_parser_t *parser, char **_buffer,
                    char **_element, char **_body, char *** _attribs);

int asx_parse_attribs(asx_parser_t *parser, char *buffer, char *** _attribs);

/////// Attribs utils

char *asx_get_attrib(const char *attrib, char **attribs);

int asx_attrib_to_enum(const char *val, char **valid_vals);

#define asx_free_attribs(a) asx_list_free(&a,free)

////// List utils

typedef void (*asx_freefunc)(void *arg);

void asx_list_free(void *list_ptr, asx_freefunc free_func);

uf_playlist_t *asx_parser_build_tree(char *asxfile, int len);

#endif /* _UF_ASXPARSER_H */
