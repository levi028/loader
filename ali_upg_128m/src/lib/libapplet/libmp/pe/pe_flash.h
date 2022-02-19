
#ifndef _PE_FLASH_DATA_H_
#define _PE_FLASH_DATA_H_

extern int is_flashdata(char *path);
//path format: flshchk://[sto dev id]:[chunk id]:[data len]
extern UINT32 fd_open(char *path, char *mode);
extern void fd_close(void *ptr);
extern size_t fd_read(void *ptr, size_t size, size_t nmemb, void *file);
extern size_t fd_write(void *ptr, size_t size, size_t nmemb, void *file);
extern int fd_seek(void *ptr, off_t offset, int whence);
extern off_t fd_tell(void *ptr);
extern int fd_eof(void *ptr);

#endif
