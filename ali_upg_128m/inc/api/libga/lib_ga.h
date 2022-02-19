#ifndef __LIB_GA_H__
#define __LIB_GA_H__


#define DES_TYPE        0
#define AES_TYPE        1

#define VIDEO_DSC       0
#define AUDIO_DSC       1

int gacas_device_init(UINT32 KeyMode);


// Delete the stream in CSA, and set the key pos in CE to idle status
// the resource of CSA and CE were used for the specified pid
int gacas_delete_stream_from_ce(UINT16 pid);


// CAS create stream to Cypto Engine
// Step 1:  find a free key pos to 2 level keyladder from Cypto Engine
// Step 2:  involve PID, KeyPos, stream id together; Create such relation into CSA
//          (So the CSA will know: by using which stream , keyladder and key pos, to decrypt which PID)
// Param:   pid     (Input): the TS data pid which need to be decrypted
//          type    (Input): TS data type, VIDEO_DSC or AUDIO_DSC
int gacas_create_stream_to_ce(UINT16 pid, UINT8 type);


int gacas_generaet_cw_to_ce(char *enc_cw, UINT8 data_type);

int gacas_set_key1(char *key1, unsigned char key1_type);

int gacas_set_key2(char *key2, unsigned char key2_type);

int gacas_set_rootkey_index(unsigned char rk_idx);

int gacas_set_cw_type(unsigned char cw_type);

// Set Odd/Even Key to CE, then start CE to decrypt
int gacas_set_odd_even_key(unsigned char d_type, unsigned char *odd_key, unsigned char *even_key);

int gacas_create_key_samaphore();

int gacas_delete_key_semaphore();


#endif
