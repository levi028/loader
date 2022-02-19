
#include "uf_xspfparser.h"
#include "uf_asxparser.h"
#include "uf_playlist.h"
#include "uf_util.h"


static char *skip_xml_head(char *file, int len)
{
    char *ret = NULL;
    char *p = NULL;
    int i = 0;
    
    p = file;

    //<?xml .... ?>
    for (i=0; i<len-1; i++)
    {
        if (*p == '\0')
            break;
        if (*p=='?' && *(p+1)=='>')
        {
            ret = p+2;
            break;
        }
        p++;
    }

    return ret;
}

static void get_new_entry(uf_playlist_t *pl, asx_parser_t *parser, char *buffer)
{
    char *element = NULL;
    char *body = NULL;
    char *attribs = NULL;
    char *buf = NULL;
    uf_playlist_entry *item = NULL;
    int r = 0;

    buf = buffer;
    item = uf_playlist_entry_new();
    if (item)
    {
        while (buf && buf[0] != '\0')
        {
            r = asx_get_element(parser, &buf, &element, &body, (char ***)(&attribs));
            if (r>0 && strcasecmp(element, "title") == 0)
            {
                strncpy(item->title, body, UF_TITLE_LEN-1);
            }

            if (r>0 && strcasecmp(element, "location") == 0)
            {
                strncpy(item->link, body, UF_URL_LEN-1);
            }
            if (element)
            {
                free(element);
                element = NULL;
            }
            if (body)
            {
                free(body);
                body = NULL;
            }
            if (attribs)
            {
                free(attribs);
                attribs = NULL;
            }
        }
        uf_playlist_add_entry(pl, item);
    }
}

uf_playlist_t *xspf_parser_build_tree(char *file, int len)
{
    uf_playlist_t *pl = NULL;
    asx_parser_t *parser = NULL;
    char *element = NULL;
    char *body = NULL;
    char *p = NULL;
    char *attribs = NULL;
    char *buf = NULL;
    int r = 0;
    
    if (strlen(file) > len)
    {
        return NULL;
    }

    buf = skip_xml_head(file, len);

    if (buf == NULL)
    {
        return NULL;
    }
    
    parser =  asx_parser_new();
    parser->line = 1;
    parser->deep = 1;// deep;

    r = asx_get_element(parser, &buf, &element, &body, (char ***)(&attribs));

    // playlist
    if (r>0 && strcasecmp(element, "playlist") != 0)
    {
        goto f_exit;
    }
    
    // trackList
    buf = body;
    p = body; // use to free
    body = NULL;
    if (element)
    {
        free(element);
        element = NULL;
    }
    if (attribs)
    {
        free(attribs);
        attribs = NULL;
    }

    r = asx_get_element(parser, &buf, &element, &body, (char ***)(&attribs));
    if (p)
    {
        free(p);
        p = NULL;
    }
    if (r>0 && strcasecmp(element, "trackList") != 0)
    {
        goto f_exit;
    }

    // get all tracks
    pl = uf_playlist_new();
    if (pl == NULL)
    {
        goto f_exit;
    }
    
    buf = body;
    p = body;//use to free
    if (element)
    {
        free(element);
        element = NULL;
    }
    if (attribs)
    {
        free(attribs);
        attribs = NULL;
    }
    
    while (buf && buf[0] != '\0')
    {
        r = asx_get_element(parser, &buf, &element, &body, (char ***)(&attribs));
        if (r>0 && strcasecmp(element, "track") == 0)
        {
            get_new_entry(pl, parser, body);
        }
        else 
        {
            libc_printf("ERROR %s, L:%d\n", __FUNCTION__, __LINE__);
            goto f_exit;
        }

        if(body)
        {
            free(body);
            body = NULL;
        }
        if (element)
        {
            free(element);
            element = NULL;
        }
        if (attribs)
        {
            free(attribs);
            attribs = NULL;
        }

    }

f_exit:
    if (p)
    {
        free(p);
        p = NULL;
    }
    if (element)
        free(element);
    if (body)
        free(body);
    if (attribs)
        free(attribs);
    element = NULL;
    body = NULL;
    attribs = NULL;
    asx_parser_free(parser);
    return pl;
}

