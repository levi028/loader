#include "uf_plsparser.h"
#include "uf_playlist.h"
#include "uf_util.h"

static char* pls_entry_get_value(char* line) 
{
    char* i;

    i = strchr(line,'=');
    
    if(!i || i[1] == '\0')
    {
        return NULL;
    }
    else
    {
        return i+1;
    }

    return NULL;
}

uf_playlist_t *pls_parser_build_tree(char *file, int len)
{
    char *line = NULL;
    char *buf = NULL;    
    char *v = NULL;
    __MAYBE_UNUSED__ int  n_entries = 0;
    uf_playlist_t *pl = NULL;
    uf_playlist_entry *item = NULL;
    __MAYBE_UNUSED__ char *title = NULL;
    __MAYBE_UNUSED__ char *link = NULL;
    
    if (strlen(file) > len)
    {
        return NULL;
    }

    pl = uf_playlist_new();

    if (pl == NULL)
    {
        return NULL;
    }

    buf = file;
    while((buf = uf_playlist_get_line(buf, &line))) 
    {
        uf_strstrip(line);
        if(strlen(line))
        {
            break;
        }
    }
    if (!line)
    {
        return NULL;
    }
    if(strcasecmp(line,"[playlist]"))
    {
        return NULL;
    }

    free(line);
    line = NULL;
    // check NumberOfEntries
    buf = uf_playlist_get_line(buf, &line);
    if(!line)
    {
        return NULL;
    }
    uf_strstrip(line);
    
    if(strncasecmp(line,"NumberOfEntries",15) == 0) {
        v = pls_entry_get_value(line);
        n_entries = atoi(v);
        
        free(line);
        line = NULL;
        buf = uf_playlist_get_line(buf, &line);
    }

    while(line) 
    {
        uf_strstrip(line);
        if(line[0] == '\0') 
        {
            free(line);
            line = NULL;
            buf = uf_playlist_get_line(buf, &line);
            continue;
        }

        if(strncasecmp(line,"File",4) == 0) 
        {
            if (item)
            {
                uf_playlist_add_entry(pl, item);
                item = NULL;
            }
            item = uf_playlist_entry_new();
            if (item)
            {
                v = pls_entry_get_value(line);
                strncpy(item->link, v, UF_URL_LEN-1);
            }
        } 
        else if(strncasecmp(line,"Title",5) == 0) 
        {
            if (item)
            {
                v = pls_entry_get_value(line);
                strncpy(item->title, v, UF_TITLE_LEN-1);
            }
        } 
        else if(strncasecmp(line,"Length",6) == 0) 
        {
            // Do Nothing
        }

        free(line);
        line = NULL;
        
        buf = uf_playlist_get_line(buf, &line);
    }

    if (item)
    {
        uf_playlist_add_entry(pl, item);
        item = NULL;
    }

    return pl;
}

