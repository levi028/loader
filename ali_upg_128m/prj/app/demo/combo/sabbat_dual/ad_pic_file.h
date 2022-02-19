#ifndef   __AD_PIC_FILE__
#define   __AD_PIC_FILE__

#ifdef PIP_PNG_GIF

extern int ad_pic_file_read(file_h fh, BYTE *buf, UINT32 size);
extern int ad_pic_file_seek(file_h fh, long offset, UINT32 origin);
extern int ad_pic_file_tell(file_h fh);
extern int ad_pic_file_init(file_h *fh, const char* filename);
extern int ad_pic_file_exit(file_h fh);
extern int ad_pic_bufffile_init(file_h *fh, const char* bufffile, UINT32 bufffile_len);
extern int ad_pic_bufffile_exit(file_h fh);

extern int ad_pic_bufmgr_init_logo();
extern int ad_pic_bufmgr_init();
extern int ad_pic_bufmgr_get_decodebuffer(UINT8** decode_buf,  INT32* deccode_buflen);
extern int ad_pic_bufmgr_get_picturebuffer(UINT8** picture_buf, INT32* picture_buflen);
extern int ad_pic_bufmgr_get_fileoperbuffer(UINT8** fileoper_buf, INT32* fileoper_buflen);

#endif

#endif

