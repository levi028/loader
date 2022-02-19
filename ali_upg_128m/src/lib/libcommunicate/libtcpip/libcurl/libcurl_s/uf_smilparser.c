#include "uf_smilparser.h"
#include "uf_playlist.h"
#include "uf_util.h"

uf_playlist_t *smil_parser_build_tree(char *file, int len)
{
    uf_playlist_t *pl = NULL;
    uf_playlist_entry *item = NULL;
    char *buf = NULL;
    char *line = NULL;
    char *src_line = NULL;
    char *s_start = NULL;
    char *s_end = NULL;
    char *pos = NULL;
    char source[512] = {0};
    BOOL is_rmsmil = FALSE;
    BOOL entrymode = FALSE;
    unsigned int npkt = 0, ttlpkt = 0;
    
    buf = file;
    
    // Check if smil
    while((buf = uf_playlist_get_line(buf, &line)) != NULL) 
    {
        uf_strstrip(line);
        if ((line[0] == '\0') 
        || (strncasecmp(line,"<?xml",5)==0)
        || (strncasecmp(line,"<!DOCTYPE smil",13)==0))
        {
            free(line);
            line = NULL;
            continue;
        }
        if ((strncasecmp(line,"<smil",5)==0)
        || (strncasecmp(line,"<?wpl",5)==0)
        || (strncasecmp(line,"(smil-document",14)==0))
        {
            break; // smil header found
        }
        else
        {
            free(line);
            line = NULL;
            return NULL; //line not smil exit
        }

        free(line);
        line = NULL;
    }

    if (line == NULL)
    {
        return NULL;
    }

    if (strncasecmp(line,"(smil-document",14)==0) 
    {
        // Special smil-over-realrtsp playlist header
        is_rmsmil = TRUE;
        if (sscanf(line, "(smil-document (ver 1.0)(npkt %u)(ttlpkt %u", &npkt, &ttlpkt) != 2) 
        {
            // smil-over-realrtsp: header parsing failure, assuming single packet.
            npkt = ttlpkt = 1;
        }
        if (ttlpkt == 0 || npkt > ttlpkt) 
        {
            // smil-over-realrtsp: bad packet counters (npkk = %u, ttlpkt = %u), assuming single packet.
            npkt = ttlpkt = 1;
        }
    }

    src_line = line;
    line = NULL;

    pl = uf_playlist_new();

    if (pl == NULL)
    {
        return NULL;
    }

    do 
    {
        uf_strstrip(src_line);
        if (line)
        {
            free(line);
            line = NULL;
        }
        /* If we're parsing smil over realrtsp and this is not the last packet and
         * this is the last line in the packet (terminating with ") ) we must get
         * the next line, strip the header, and concatenate it to the current line.
         */
        if (is_rmsmil && npkt != ttlpkt && strstr(src_line,"\")"))
        {
            char *payload;

            line = strdup(src_line);
            free(src_line);
            src_line = NULL;
            if(!(buf = uf_playlist_get_line(buf, &src_line)))
            {
                break;
            }
            uf_strstrip(src_line);
            // Skip header, packet starts after "
            if(!(payload = strchr(src_line,'\"'))) 
            {
                //smil-over-realrtsp: can't find start of packet, using complete line.
                payload = src_line;
            } 
            else
            {
                payload++;
            }
            // Skip ") at the end of the last line from the current packet
            line[strlen(line)-2] = 0;
            line = realloc(line, strlen(line)+strlen(payload)+1);
            strcat (line, payload);
            npkt++;
        }
        else
        {
            line = strdup(src_line);
        }

        free(src_line);
        src_line = NULL;
        
        /* Unescape \" to " for smil-over-rtsp */
        if (is_rmsmil && line[0] != '\0') 
        {
            int i, j;

            for (i = 0; i < strlen(line); i++)
            {
                if (line[i] == '\\' && line[i+1] == '"')
                {
                    for (j = i; line[j]; j++)
                        line[j] = line[j+1];
                }
            }
        }
        
        pos = line;
        while (pos) 
        {
            if (!entrymode)  // all entries filled so far
            {
                while ((pos=strchr(pos, '<'))) 
                {
                    if ((strncasecmp(pos,"<video",6)==0)
                    || (strncasecmp(pos,"<audio",6)==0)
                    || (strncasecmp(pos,"<media",6)==0))
                    {
                        entrymode = TRUE;
                        break; // Got a valid tag, exit '<' search loop
                    }
                    pos++;
                }
            }
            if (entrymode)  //Entry found but not yet filled
            {
                pos = strstr(pos,"src=");   // Is source present on this line
                if (pos != NULL) 
                {
                    entrymode=0;
                    if (pos[4] != '"' && pos[4] != '\'') 
                    {
                        // Unknown delimiter
                        break;
                    }
                    s_start=pos+5;
                    s_end=strchr(s_start,pos[4]);
                    if (s_end == NULL) 
                    {
                        // Error parsing this source line 
                        break;
                    }
                    if (s_end-s_start> 511) 
                    {
                        // Cannot store such a large source
                        break;
                    }
                    strncpy(source,s_start,s_end-s_start);
                    source[(s_end-s_start)]='\0'; // Null terminate

                    item = uf_playlist_entry_new();
                    if (item)
                    {
                        strncpy(item->link, source, 511);
                        uf_playlist_add_entry(pl, item);
                    }
                    pos = s_end;
                }
            }
        }
    } while((buf = uf_playlist_get_line(buf, &src_line)) != NULL);

    if (src_line)
    {
        free(src_line);
        src_line = NULL;
    }
    if (line)
    {
        free(line);
        line = NULL;
    }
    return pl;
}

