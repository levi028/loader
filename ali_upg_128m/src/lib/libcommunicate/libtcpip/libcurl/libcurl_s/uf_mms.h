#ifndef _UF_MMS_H__
#define _UF_MMS_H__
typedef struct
{
    char *buff;
    int   size;
    int   len;
    
    char  url[1024];
    int   idx;
} uf_mms_ref;

typedef struct
{
    double   dtimeoffset;
    double   dtimelength;
    int      ulseekinfo;
    uf_mms_ref  ref;
} uf_mmspriv;

#endif

