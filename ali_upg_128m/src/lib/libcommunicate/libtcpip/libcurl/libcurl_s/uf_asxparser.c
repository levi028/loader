
#include <sys_config.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "uf_asxparser.h"

#define asx_warning_attrib_required(p,e,a) libc_printf("At line %d : element %s don't have the required attribute %s",p->line,e,a)
#define asx_warning_body_parse_error(p,e) libc_printf("At line %d : error while parsing %s body",p->line,e)
#define asx_warning libc_printf

#define LETTER "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define SPACE " \n\t\r"


////// List utils

void asx_list_free(void *list_ptr, asx_freefunc free_func)
{
    void **ptr = *(void ** *)list_ptr;
    if (ptr == NULL) { return; }
    if (free_func != NULL)
    {
        for (; *ptr != NULL ; ptr++)
        {
            free_func(*ptr);
        }
    }
    free(*(void **)list_ptr);
    *(void **)list_ptr = NULL;
}

/////// Attribs utils

char *asx_get_attrib(const char *attrib, char **attribs)
{
    char **ptr;

    if (attrib == NULL || attribs == NULL) { return NULL; }
    for (ptr = attribs; ptr[0] != NULL; ptr += 2)
    {
        if (strcasecmp(ptr[0], attrib) == 0)
        {
            return strdup(ptr[1]);
        }
    }
    return NULL;
}

int asx_attrib_to_enum(const char *val, char **valid_vals)
{
    char **ptr;
    int r = 0;

    if (valid_vals == NULL || val == NULL) { return -2; }
    for (ptr = valid_vals ; ptr[0] != NULL ; ptr++)
    {
        if (strcasecmp(val, ptr[0]) == 0) { return r; }
        r++;
    }

    return -1;
}


asx_parser_t *asx_parser_new(void)
{
    asx_parser_t *parser = calloc(1, sizeof(asx_parser_t));
    return parser;
}

void asx_parser_free(asx_parser_t *parser)
{
    if (!parser) { return; }
    free(parser->ret_stack);
    free(parser);

}

int asx_parse_attribs(asx_parser_t *parser, char *buffer, char *** _attribs)
{
    char *ptr1;
    int n_attrib = 0;
    char **attribs = NULL;

    ptr1 = buffer;
    while (1)
    {
        char *ptr2, *ptr3;
        char *attrib, *val;
        for (; strchr(SPACE, *ptr1) != NULL; ptr1++)  // Skip space
        {
            if (*ptr1 == '\0') { break; }
        }
        ptr3 = strchr(ptr1, '=');
        if (ptr3 == NULL) { break; }
        for (ptr2 = ptr3 - 1; strchr(SPACE, *ptr2) != NULL; ptr2--)
        {
            if (ptr2 == ptr1)
            {
                asx_warning("At line %d : this should never append, back to attribute begin while skipping end space", parser->line);
                break;
            }
        }
        attrib = malloc(ptr2 - ptr1 + 2);
        strncpy(attrib, ptr1, ptr2 - ptr1 + 1);
        attrib[ptr2 - ptr1 + 1] = '\0';

        ptr1 = strchr(ptr3, '"');
        if (ptr1 == NULL || ptr1[1] == '\0') { ptr1 = strchr(ptr3, '\''); }
        if (ptr1 == NULL || ptr1[1] == '\0')
        {
            asx_warning("At line %d : can't find attribute %s value", parser->line, attrib);
            free(attrib);
            break;
        }
        ptr2 = strchr(ptr1 + 1, ptr1[0]);
        if (ptr2 == NULL)
        {
            asx_warning("At line %d : value of attribute %s isn't finished", parser->line, attrib);
            free(attrib);
            break;
        }
        ptr1++;
        val = malloc(ptr2 - ptr1 + 1);
        strncpy(val, ptr1, ptr2 - ptr1);
        val[ptr2 - ptr1] = '\0';
        n_attrib++;

        attribs = realloc(attribs, (2 * n_attrib + 1) * sizeof(char *));
        attribs[n_attrib * 2 - 2] = attrib;
        attribs[n_attrib * 2 - 1] = val;

        ptr1 = ptr2 + 1;
    }

    if (n_attrib > 0)
    {
        attribs[n_attrib * 2] = NULL;
    }

    *_attribs = attribs;

    return n_attrib;
}

/*
 * Return -1 on error, 0 when nothing is found, 1 on sucess
 */
int asx_get_element(asx_parser_t *parser, char **_buffer,
                    char **_element, char **_body, char *** _attribs)
{
    char *ptr1, *ptr2, *ptr3, *ptr4;
    char *attribs = NULL;
    char *element = NULL, *body = NULL, *ret = NULL, *buffer = NULL;
    int n_attrib = 0;
    int body_line = 0, attrib_line, ret_line, in = 0;
    int quotes = 0;

    if (_buffer == NULL || _element == NULL || _body == NULL || _attribs == NULL)
    {
        asx_warning("At line %d : asx_get_element called with invalid value", parser->line);
        return -1;
    }

    *_body = *_element = NULL;
    *_attribs =  NULL;
    buffer = *_buffer;

    if (buffer == NULL) { return 0; }

    if (parser->ret_stack && /*parser->last_body && */buffer != parser->last_body)
    {
        asx_line_save_t *ls = parser->ret_stack;
        int i;
        for (i = 0 ; i < parser->ret_stack_size ; i++)
        {
            if (buffer == ls[i].buffer)
            {
                parser->line = ls[i].line;
                break;
            }

        }
        if (i < parser->ret_stack_size)
        {
            i++;
            if (i < parser->ret_stack_size)
            {
                memmove(parser->ret_stack, parser->ret_stack + i, (parser->ret_stack_size - i)*sizeof(asx_line_save_t));
            }
            parser->ret_stack_size -= i;
            if (parser->ret_stack_size > 0)
            {
                parser->ret_stack = realloc(parser->ret_stack, parser->ret_stack_size * sizeof(asx_line_save_t));
            }
            else
            {
                free(parser->ret_stack);
                parser->ret_stack = NULL;
            }
        }
    }

    ptr1 = buffer;
    while (1)
    {
        for (; ptr1[0] != '<' ; ptr1++)
        {
            if (ptr1[0] == '\0')
            {
                ptr1 = NULL;
                break;
            }
            if (ptr1[0] == '\n') { parser->line++; }
        }
        //ptr1 = strchr(ptr1,'<');
        if (!ptr1 || ptr1[1] == '\0') { return 0; } // Nothing found

        if (strncmp(ptr1, "<!--", 4) == 0) // Comments
        {
            for (; strncmp(ptr1, "-->", 3) != 0 ; ptr1++)
            {
                if (ptr1[0] == '\0')
                {
                    ptr1 = NULL;
                    break;
                }
                if (ptr1[0] == '\n') { parser->line++; }
            }
            //ptr1 = strstr(ptr1,"-->");
            if (!ptr1)
            {
                asx_warning("At line %d : unfinished comment", parser->line);
                return -1;
            }
        }
        else
        {
            break;
        }
    }

    // Is this space skip very useful ??
    for (ptr1++; strchr(SPACE, ptr1[0]) != NULL; ptr1++) // Skip space
    {
        if (ptr1[0] == '\0')
        {
            asx_warning("At line %d : EOB reached while parsing element start", parser->line);
            return -1;
        }
        if (ptr1[0] == '\n') { parser->line++; }
    }

    for (ptr2 = ptr1; strchr(LETTER, *ptr2) != NULL; ptr2++) // Go to end of name
    {
        if (*ptr2 == '\0')
        {
            asx_warning("At line %d : EOB reached while parsing element start", parser->line);
            return -1;
        }
        if (ptr2[0] == '\n') { parser->line++; }
    }

    element = malloc(ptr2 - ptr1 + 1);
    strncpy(element, ptr1, ptr2 - ptr1);
    element[ptr2 - ptr1] = '\0';

    for (; strchr(SPACE, *ptr2) != NULL; ptr2++)  // Skip space
    {
        if (ptr2[0] == '\0')
        {
            asx_warning("At line %d : EOB reached while parsing element start", parser->line);
            free(element);
            return -1;
        }
        if (ptr2[0] == '\n') { parser->line++; }
    }
    attrib_line = parser->line;



    for (ptr3 = ptr2; ptr3[0] != '\0'; ptr3++)  // Go to element end
    {
        if (ptr3[0] == '"') { quotes ^= 1; }
        if (!quotes && (ptr3[0] == '>' || strncmp(ptr3, "/>", 2) == 0))
        {
            break;
        }
        if (ptr3[0] == '\n') { parser->line++; }
    }
    if (ptr3[0] == '\0' || ptr3[1] == '\0')  // End of file
    {
        asx_warning("At line %d : EOB reached while parsing element start", parser->line);
        free(element);
        return -1;
    }

    // Save attribs string
    if (ptr3 - ptr2 > 0)
    {
        attribs = malloc(ptr3 - ptr2 + 1);
        strncpy(attribs, ptr2, ptr3 - ptr2);
        attribs[ptr3 - ptr2] = '\0';
    }
    //bs_line = parser->line;
    if (ptr3[0] != '/')  // Not Self closed element
    {
        ptr3++;
        for (; strchr(SPACE, *ptr3) != NULL; ptr3++)  // Skip space on body begin
        {
            if (*ptr3 == '\0')
            {
                asx_warning("At line %d : EOB reached while parsing %s element body", parser->line, element);
                free(element);
                free(attribs);
                return -1;
            }
            if (ptr3[0] == '\n') { parser->line++; }
        }
        ptr4 = ptr3;
        body_line = parser->line;
        while (1)  // Find closing element
        {
            for (; ptr4[0] != '<' ; ptr4++)
            {
                if (ptr4[0] == '\0')
                {
                    ptr4 = NULL;
                    break;
                }
                if (ptr4[0] == '\n') { parser->line++; }
            }
            if (ptr4 && strncmp(ptr4, "<!--", 4) == 0) // Comments
            {
                for (; strncmp(ptr4, "-->", 3) != 0 ; ptr4++)
                {
                    if (ptr4[0] == '\0')
                    {
                        ptr4 = NULL;
                        break;
                    }
                    if (ptr1[0] == '\n') { parser->line++; }
                }
                continue;
            }
            if (ptr4 == NULL || ptr4[1] == '\0')
            {
                asx_warning("At line %d : EOB reached while parsing %s element body", parser->line, element);
                free(element);
                free(attribs);
                return -1;
            }
            if (ptr4[1] != '/' && strncasecmp(element, ptr4 + 1, strlen(element)) == 0)
            {
                in++;
                ptr4 += 2;
                continue;
            }
            else if (strncasecmp(element, ptr4 + 2, strlen(element)) == 0) // Extract body
            {
                if (in > 0)
                {
                    in--;
                    ptr4 += 2 + strlen(element);
                    continue;
                }
                ret = ptr4 + strlen(element) + 3;
                if (ptr4 != ptr3)
                {
                    ptr4--;
                    for (; ptr4 != ptr3 && strchr(SPACE, *ptr4) != NULL; ptr4--) { ; } // Skip space on body end
                    //        if(ptr4[0] == '\0') parser->line--;
                    //}
                    ptr4++;
                    body = malloc(ptr4 - ptr3 + 1);
                    strncpy(body, ptr3, ptr4 - ptr3);
                    body[ptr4 - ptr3] = '\0';
                }
                break;
            }
            else
            {
                ptr4 += 2;
            }
        }
    }
    else
    {
        ret = ptr3 + 2; // 2 is for />
    }

    for (; ret[0] != '\0' && strchr(SPACE, ret[0]) != NULL; ret++)  // Skip space
    {
        if (ret[0] == '\n') { parser->line++; }
    }

    ret_line = parser->line;

    if (attribs)
    {
        parser->line = attrib_line;
        n_attrib = asx_parse_attribs(parser, attribs, _attribs);
        free(attribs);
        if (n_attrib < 0)
        {
            asx_warning("At line %d : error while parsing element %s attributes", parser->line, element);
            free(element);
            free(body);
            return -1;
        }
    }
    else
    {
        *_attribs = NULL;
    }

    *_element = element;
    *_body = body;

    parser->last_body = body;
    parser->ret_stack_size++;
    parser->ret_stack = realloc(parser->ret_stack, parser->ret_stack_size * sizeof(asx_line_save_t));
    if (parser->ret_stack_size > 1)
    {
        memmove(parser->ret_stack + 1, parser->ret_stack, (parser->ret_stack_size - 1)*sizeof(asx_line_save_t));
    }
    parser->ret_stack[0].buffer = ret;
    parser->ret_stack[0].line = ret_line;
    parser->line = body ? body_line : ret_line;

    *_buffer = ret;
    return 1;

}

//static void
//asx_parse_param(asx_parser_t* parser, char** attribs, playlist_tree_t* pt) {
//  char *name = NULL,*val = NULL;

//  name = asx_get_attrib("NAME",attribs);
//  if(!name) {
//    asx_warning_attrib_required(parser,"PARAM" ,"NAME" );
//    return;
//  }
//  val = asx_get_attrib("VALUE",attribs);
//  //if(m_config_get_option(mconfig,name) == NULL) {
//  //  asx_warning("Found unknown param in asx: %s",name);
//  //  if(val)
//  //    asx_warning("=%s\n",val);
//  //  else
//  //    asx_warning("\n");
//  //  goto err_out;
//  //}
//  asx_warning( "Support for specifying parameters in playlists has been disabled.\n");
////  play_tree_set_param(pt,name,val);
//err_out:
//  free(name);
//  free(val);
//}

static char *asx_parse_ref(asx_parser_t *parser, char **attribs)
{
    char *href;
    href = asx_get_attrib("HREF", attribs);
    if (href == NULL)
    {
        asx_warning_attrib_required(parser, "REF" , "HREF");
        return NULL;
    }

    //play_tree_add_file(pt,href);

    asx_warning("Adding file %s to element entry\n", href);

    //free(href);
    return href;
}
static char *asx_parse_entryref(asx_parser_t *parser, char *buffer, char **_attribs)
{
    char *href;

    //if(parser->deep > 0)
    //    return NULL;

    href = asx_get_attrib("HREF", _attribs);
    if (href == NULL)
    {
        asx_warning_attrib_required(parser, "ENTRYREF" , "HREF");
        return NULL;
    }

    return href;
}

static void asx_parse_entry(asx_parser_t *parser, char *buffer, char **_attribs, uf_playlist_t *pl)
{
    int nref = 0;
    char *title = NULL;
    char *ref = NULL;
    uf_playlist_entry *item = NULL;

    //ref = play_tree_new();

    while (buffer && buffer[0] != '\0')
    {
        char *element = NULL, *body = NULL, **attribs = NULL;
        int r = asx_get_element(parser, &buffer, &element, &body, &attribs);
        if (r < 0)
        {
            asx_warning_body_parse_error(parser, "ENTRY");
            return;
        }
        else if (r == 0)
        {
            // No more element
            break;
        }
        if (strcasecmp(element, "TITLE") == 0)
        {
            title = strdup(body);
        }
        else if (strcasecmp(element, "REF") == 0)
        {
            // ref malloc at asx_parse_ref, we should free it later or save in playlist entry.
            ref = asx_parse_ref(parser, attribs);
            asx_warning("Adding element %s to entry\n", element);
            nref++;
            item = uf_playlist_entry_new();
            if (ref)
            {
                strncpy(item->link, ref, UF_URL_LEN - 1);
            }
            if (title)
            {
                strncpy(item->title, title, UF_TITLE_LEN - 1);
            }
            title = NULL;
            uf_playlist_add_entry(pl, item);
        }
        else
        {
            asx_warning("Ignoring element %s\n", element);
        }

        free(element);
        free(body);
        asx_free_attribs(attribs);
        element = NULL;
        body = NULL;
        attribs = NULL;
    }

    if (title)
    {
        free(title);
        title = NULL;
    }
}


static void asx_parse_repeat(asx_parser_t *parser, char *buffer, char **_attribs, uf_playlist_t *pl)
{
    char *element = NULL, *body = NULL, **attribs = NULL;
    char *count = NULL;
    char *ref = NULL;
    uf_playlist_entry *item = NULL;
    int r;

    count = asx_get_attrib("COUNT", _attribs);
    if (count == NULL)
    {
        asx_warning("Setting element repeat loop to infinit\n");
    }
    else
    {
        free(count);
    }

    while (buffer && buffer[0] != '\0')
    {
        r = asx_get_element(parser, &buffer, &element, &body, &attribs);
        if (r < 0)
        {
            asx_warning_body_parse_error(parser, "REPEAT");
            return; //NULL;
        }
        else if (r == 0)
        {
            // No more element
            break;
        }
        if (strcasecmp(element, "ENTRY") == 0)
        {
            asx_parse_entry(parser, body, attribs, pl);
        }
        else if (strcasecmp(element, "ENTRYREF") == 0)
        {
            ref = asx_parse_entryref(parser, body, attribs);
            if (ref)
            {
                item = uf_playlist_entry_new();
                if (ref)
                {
                    strncpy(item->link, ref, UF_URL_LEN - 1);
                }
                uf_playlist_add_entry(pl, item);
                asx_warning("Adding element %s to asx\n", element);
            }
        }
        else if (strcasecmp(element, "REPEAT") == 0)
        {
            asx_parse_repeat(parser, body, attribs, pl);
            asx_warning("Adding element %s to repeat\n", element);
        }
        else if (strcasecmp(element, "PARAM") == 0)
        {
            //asx_parse_param(parser,attribs,repeat);
        }
        else
        {
            asx_warning("Ignoring element %s\n", element);
        }

        free(element);
        free(body);
        asx_free_attribs(attribs);

        element = NULL;
        body = NULL;
        attribs = NULL;
    }

}



uf_playlist_t *asx_parser_build_tree(char *asxfile, int len)
{
    char *element = NULL, *asx_body = NULL, **asx_attribs = NULL, *body = NULL, **attribs = NULL;
    int r;
    uf_playlist_t *asx = NULL;//*entry = NULL, *list = NULL;
    asx_parser_t *parser = NULL;
    char *title = NULL;
    char *ref = NULL;
    uf_playlist_entry *item = NULL;

    if (strlen(asxfile) > len)
    {
        return NULL;
    }
    parser =  asx_parser_new();
    parser->line = 1;
    parser->deep = 1;// deep;

    r = asx_get_element(parser, &asxfile, &element, &asx_body, &asx_attribs);
    if (r < 0)
    {
        asx_warning("At line %d : Syntax error ???", parser->line);
        asx_parser_free(parser);
        return NULL;
    }
    else if (r == 0)
    {
        // No contents
        asx_warning("empty asx element");
        asx_parser_free(parser);
        return NULL;
    }

    if (strcasecmp(element, "ASX") != 0)
    {
        asx_warning("first element isn't ASX, it's %s\n", element);
        asx_free_attribs(asx_attribs);
        asx_parser_free(parser);
        if (element)
            free(element);
        if (asx_body)
            free(asx_body);
        if (asx_attribs)
            free(asx_attribs);
        return NULL;
    }

    if (!asx_body)
    {
        asx_warning("ASX element is empty");
        asx_free_attribs(asx_attribs);
        asx_parser_free(parser);
        if (element)
            free(element);
        if (asx_body)
            free(asx_body);
        if (asx_attribs)
            free(asx_attribs);
        return NULL;
    }
    
    if (element)
        free(element);

    element = NULL;
    
    asx = uf_playlist_new();

    if (NULL == asx)
    {
        asx_parser_free(parser);
        if (asx_body)
            free(asx_body);
        if (asx_attribs)
            free(asx_attribs);
        return NULL;
    }

    asxfile = asx_body;
    while (asxfile && asxfile[0] != '\0')
    {
        r = asx_get_element(parser, &asxfile, &element, &body, &attribs);
        if (r < 0)
        {
            asx_warning_body_parse_error(parser, "ASX");
            asx_parser_free(parser);
            uf_playlist_free(asx);
            return NULL;
        }
        else if (r == 0)
        {
            // No more element
            break;
        }
        if (strcasecmp(element, "TITLE") == 0)
        {
            // free prev title.
            if (title)
            {
                free(title);
            }
            title = strdup(body);
        }
        else if (strcasecmp(element, "ENTRY") == 0)
        {
            asx_parse_entry(parser, body, attribs, asx);
            // Don't store title for ENTRY, REPEAT
            if (title)
            {
                free(title);
                title = NULL;
            }
        }
        else if (strcasecmp(element, "ENTRYREF") == 0)
        {
            ref = asx_parse_entryref(parser, body, attribs);
            if (ref)
            {
                item = uf_playlist_entry_new();
                if (ref)
                {
                    strncpy(item->link, ref, UF_URL_LEN - 1);
                }
                if (title)
                {
                    strncpy(item->title, title, UF_TITLE_LEN - 1);
                }
                title = NULL;
                uf_playlist_add_entry(asx, item);
                asx_warning("Adding element %s to asx\n", element);
            }
        }
        else if (strcasecmp(element, "REPEAT") == 0)
        {
            asx_parse_repeat(parser, body, attribs, asx);
            // Don't store title for ENTRY, REPEAT
            if (title)
            {
                free(title);
                title = NULL;
            }
        }
        else
        {
            asx_warning("Ignoring element %s\n", element);
            // Don't store title for ENTRY, REPEAT
            if (title)
            {
                free(title);
                title = NULL;
            }
        }

        free(element);
        free(body);
        asx_free_attribs(attribs);
        element = NULL;
        body = NULL;
        attribs = NULL;
    }

    if (title)
    {
        free(title);
        title = NULL;
    }

    free(asx_body);
    asx_free_attribs(asx_attribs);
    asx_parser_free(parser);

    return asx;
}

