#include "uf_util.h"


#define WHITES " \n\r\t"

char *uf_strstr(char *s1, int len, char *s2)
{
    char *tmp = s1;
    while (tmp < s1 + len)
    {
        if (*tmp == *s2 && MEMCMP(tmp, s2, STRLEN(s2)) == 0)
        {
            return tmp;
        }
        tmp++;
    }
    return NULL;
}

void uf_strstrip(char* str) 
{
    char* i;

    if (str==NULL)
    {
        return;
    }
    for(i = str ; i[0] != '\0' && strchr(WHITES,i[0]) != NULL; i++)
    {
        /* NOTHING */;
    }
    if(i[0] != '\0') 
    {
        memmove(str,i,strlen(i) + 1);
        for(i = str + strlen(str) - 1 ; strchr(WHITES,i[0]) != NULL; i--)
        {
            /* NOTHING */;
        }
        i[1] = '\0';
    } 
    else
    {
        str[0] = '\0';
    }
}


char* uf_playlist_get_line(char *buf, char **line)
{
    char *p = buf;
    int  len = 0;
    int  i = 0;
    int  buf_len = 0;

    if ((buf == NULL)||buf[0] == '\0')
    {
        return NULL;
    }
    
    buf_len = strlen(buf);
    
    for (i=0; i<buf_len; i++)
    {
        if (*p == '\n')
        {
            break;
        }
        p++;
    }
    len = i+1;
    
    *line = malloc(len+1);
    memset(*line, 0, len+1);

    p = buf;
    for (i=0; i<len; i++)
    {
        (*line)[i] = *p;
        
        if (*p == '\0')
        {
            break;
        }
        p++;
    }

    return p;
}

