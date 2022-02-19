#ifndef __RBUFFER_H__
#define __RBUFFER_H__

typedef struct
{
    char    *buff;
    int      size;
    int      wr;     //next write ptr
    int      rd;     //next read ptr
    int      wpb;    //write pointer buffer after read pointer
    char     allocb; //buffer is alloced
    char     res[3];
	UINT8     rdma_id;
    UINT8     wdma_id;
} rbuffer;

int rbuffer_init(rbuffer *rb, char *buffer, int size);
void rbuffer_destroy(rbuffer *rb);
void rbuffer_set_wpb(rbuffer *rb, int wpb);
int rbuffer_dlen(rbuffer *rb);
int rbuffer_flen(rbuffer *rb);
void rbuffer_clean(rbuffer *rb);
int rbuffer_write(rbuffer *rb, char *data, int len);
int rbuffer_read(rbuffer *rb, char *out, int len);
int rbuffer_seek(rbuffer *rb, int offset);
#endif
