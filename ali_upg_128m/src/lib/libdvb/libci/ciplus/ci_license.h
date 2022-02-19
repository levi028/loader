/*
 * CI+ license functions such as f-CC & f-SAC
 *
 */


#define DES_56_ECB        0
#define AES_128_CBC        1

//kp: IN, Kp data
//kp_len: IN, shall be 32
//clk: IN, CLK data
//clk_len: IN, shall be 16
//cck, clk: OUT, CCK, CLK buffer
//len: IN, cck, clk buffer length.
//          scrambler: DES_56_ECB - shall be 7
//                       AES_128_CBC - shall be 16
//scrambler: IN, DES_56_ECB/AES_128_CBC
//return value: 0 - success
//                other - failed
int ci_f_cc(unsigned char *kp, unsigned long ci_kp_len,
            unsigned char *clk, unsigned long clk_len,
            unsigned char *cck, unsigned char *civ, unsigned long ci_len,
            int scrambler);


//ks: IN, Ks data
//ks_len: IN, shall be 32
//slk: IN, SLK data
//slk_len: IN, shall be 16
//sek, sak: OUT, SEK, SAK buffer
//len: IN, sek & sak buffer length. shall be 16
//return value: 0 - success
//                other - failed
int ci_f_sac(unsigned char *ks, unsigned long ci_ks_len,
        unsigned char *slk, unsigned long slk_len,
        unsigned char *sek, unsigned char *sak, unsigned long ci_len);

