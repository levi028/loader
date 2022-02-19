#include "uf_refparser.h"

static char *get_ref_url(char *filestr, char **out_url)
{
    char *ptr1 = NULL, *ptr2 = NULL;
    char *link = NULL;
    int n = 0;

    ptr1 = strchr(filestr, ':');

    if (ptr1 == NULL)
    {
        *out_url = NULL;
        return NULL;
    }

    ptr2 = strchr(filestr, '\n');

    // Maybe last line no '\n'
    if (ptr2 == NULL)
    {
        ptr2 = ptr1 + strlen(ptr1);
    }

    n = (ptr2 - ptr1);
    if (n > 0 && *(ptr2 - 1) == '\r')
    {
        n--; //
    }

    link = (char *)MALLOC(3/*mms*/ + n + 1);

    if (NULL == link)
    {
        *out_url = NULL;
        return filestr;
    }

    MEMSET(link, 0, n + 4);
    strncpy(link, "mms", 3);
    strncpy(link + 3, ptr1, n);

    *out_url = link;

    return (ptr2 + 1);
}

uf_playlist_t *ref_parser_build_tree(char *file_buf, int len)
{
    uf_playlist_t *pl = NULL;
    uf_playlist_entry *item = NULL;
    //char *title = NULL;
    char *link = NULL;
    char *cur_str = NULL;

    if (strlen(file_buf) > len)
    {
        return NULL;
    }

    pl = uf_playlist_new();

    if (NULL == pl)
    {
        return NULL;
    }

    // skip header [Reference]
    cur_str = strchr(file_buf, '\n');
    if (NULL == cur_str)
    {
        return NULL;
    }
    cur_str++;

    while (cur_str && cur_str[0] != '\0')
    {
        cur_str = get_ref_url(cur_str, &link);
        if (cur_str)
        {
            item = uf_playlist_entry_new();
            strcpy(item->link, link);
            uf_playlist_add_entry(pl, item);
        }
    }

    return pl;
}

