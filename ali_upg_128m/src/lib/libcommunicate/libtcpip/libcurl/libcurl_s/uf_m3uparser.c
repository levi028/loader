#include "uf_m3uparser.h"
#include "uf_playlist.h"
#include "uf_util.h"



uf_playlist_t *m3u_parser_build_tree(char *file, int len)
{
    char *title = NULL;
    char *line = NULL;
    char *buf = file;
    char *p = NULL;
    int   linelen = 0;
    int   i = 0;
    uf_playlist_t *pl = NULL;
    uf_playlist_entry *item = NULL;
    
    if (strlen(file) > len)
    {
        return NULL;
    }

    pl = uf_playlist_new();

    if (pl == NULL)
    {
        return NULL;
    }
    while((buf = uf_playlist_get_line(buf, &line)) != NULL)
    {
        uf_strstrip(line);
        if(line[0] == '\0')
        {
            free(line);
            line = NULL;
            continue;
        }
        /* EXTM3U files contain such lines:
         * #EXTINF:<seconds>, <title>
         * followed by a line with the filename
         * for now we have no place to put that
         * so we just skip that extra-info ::atmos
         */
        if(line[0] == '#') 
        {
            linelen = strlen(line);
            if (strncmp(line, "#EXTINF", 7) == 0)
            {
                p = line;
                for (i=0; i<linelen; i++)
                {
                    if (*p == ',')
                    {
                        break;
                    }
                    p++;
                }
                if (*p == ',')
                {
                    p++;
                    title = strdup(p);
                }
            }

            free(line);
            line = NULL;
            continue;
        }

        item = uf_playlist_entry_new();
        if (item)
        {
            strncpy(item->link, line, UF_URL_LEN - 1);
            if (title)
            {
                strncpy(item->title, title, UF_TITLE_LEN - 1);
            }
            uf_playlist_add_entry(pl, item);
        }
        if (title)
        {
            free(title);
            title = NULL;
        }
        if (line)
        {
            free(line);
            line = NULL;
        }
    }

    return pl;
}

