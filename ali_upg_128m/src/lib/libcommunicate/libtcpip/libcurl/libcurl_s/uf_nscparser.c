#include "uf_nscparser.h"
#include "uf_playlist.h"
#include "uf_util.h"


/**
 * \brief "converts" utf16 to ascii by just discarding every second byte
 * \param buf buffer to convert
 * \param len lenght of buffer, must be > 0
 */
static void utf16_to_ascii(char *buf, int len) 
{
    int i;
    if (len <= 0) return;
    for (i = 0; i < len / 2; i++)
        buf[i] = buf[i * 2];
    buf[i] = 0; // just in case
}


/**
 * \brief decode the base64 used in nsc files
 * \param in input string, 0-terminated
 * \param buf output buffer, must point to memory suitable for realloc,
 *            will be NULL on failure.
 * \return decoded length in bytes
 */
static int decode_nsc_base64(char *in, char **buf) 
{
    int i, j, n;
    if (in[0] != '0' || in[1] != '2')
    {
        goto err_out;
    }
    in += 2; // skip prefix
    if (strlen(in) < 16) // error out if nothing to decode
    {
        goto err_out;
    }
    in += 12; // skip encoded string length
    n = strlen(in) / 4;
    *buf = realloc(*buf, n * 3);
    for (i = 0; i < n; i++) 
    {
        uint8_t c[4];
        for (j = 0; j < 4; j++) 
        {
            c[j] = in[4 * i + j];
            if (c[j] >= '0' && c[j] <= '9') c[j] += 0 - '0';
            else if (c[j] >= 'A' && c[j] <= 'Z') c[j] += 10 - 'A';
            else if (c[j] >= 'a' && c[j] <= 'z') c[j] += 36 - 'a';
            else if (c[j] == '{') c[j] = 62;
            else if (c[j] == '}') c[j] = 63;
            else // Invalid character
                goto err_out;
        }
        (*buf)[3 * i] = (c[0] << 2) | (c[1] >> 4);
        (*buf)[3 * i + 1] = (c[1] << 4) | (c[2] >> 2);
        (*buf)[3 * i + 2] = (c[2] << 6) | c[3];
    }
    
    return 3 * n;
    
err_out:
    free(*buf);
    *buf = NULL;
    return 0;
}

uf_playlist_t *nsc_parser_build_tree(char *file, int len)
{
    char *line = NULL, *addr = NULL, *url = NULL, *unicast_url = NULL;
    char *buf = NULL;
    int port = 0;
    uf_playlist_t *pl = NULL;
    uf_playlist_entry *item = NULL;


    while((buf = uf_playlist_get_line(buf, &line)) != NULL) 
    {
        uf_strstrip(line);
        if(!line[0]) // Ignore empties
        {
            free(line);
            line = NULL;
            continue;
        }
        if (strncasecmp(line,"[Address]", 9) == 0)
        {
            break; // nsc header found
        }
        else
        {
            free(line);
            line = NULL;
            return NULL;
        }
        
        free(line);
        line = NULL;
    }
    // Detected nsc playlist format

    while ((buf = uf_playlist_get_line(buf, &line)) != NULL) 
    {
        uf_strstrip(line);
        if (!line[0])
        {
            free(line);
            line = NULL;
            continue;
        }
        if (strncasecmp(line, "Unicast URL=", 12) == 0) 
        {
            int len = decode_nsc_base64(&line[12], &unicast_url);
            if (len <= 0)
            {
                // [nsc] Unsupported Unicast URL encoding
            }
            else
            {
                utf16_to_ascii(unicast_url, len);
            }
        }
        else if (strncasecmp(line, "IP Address=", 11) == 0) 
        {
            int len = decode_nsc_base64(&line[11], &addr);
            if (len <= 0)
            {
                // [nsc] Unsupported IP Address encoding
            }
            else
            {
                utf16_to_ascii(addr, len);
            }
        } 
        else if (strncasecmp(line, "IP Port=", 8) == 0) 
        {
            port = strtol(&line[8], NULL, 0);
        }
        free(line);
        line = NULL;
    }

    if (unicast_url)
    {
        url = unicast_url;
        unicast_url = NULL;
    }
    else if (addr && port) 
    {
        url = malloc(strlen(addr) + 7 + 20 + 1);
        sprintf(url, "http://%s:%i", addr, port);
    }
    else
    {
        goto out;
    }

    pl = uf_playlist_new();
    if (pl == NULL)
    {
        goto out;
    }
    item = uf_playlist_entry_new();
    if (item && url)
    {
        strncpy(item->link, url, UF_URL_LEN-1);
        uf_playlist_add_entry(pl, item);
    }
    
out:
    if (addr)
    {
        free(addr);
    }
    if (unicast_url)
    {
        free(unicast_url);
    }
    if (url)
    {
        free(url);
    }
    
    return pl;
}

