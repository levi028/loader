#ifndef __IRC_H__
#define __IRC_H__

#ifdef __cplusplus
extern "C" {
#endif
// add some functiong declaration for irc.c, so create it in 2013926
extern UINT16 key_cnt;
extern unsigned short key_cnt;


void irc_m6303irc_init(struct pan_hw_info *hw_info);
void irc_m6303irc_close(void);
void irc_lab_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to);
void irc_nec_mode_set(UINT32 state, UINT32 precision, UINT32 repeat_to);
UINT32 irc_lab_pulse_to_code(UINT32 pulse_width);
UINT32 irc_nec_pulse_to_code(UINT32 pulse_width);
UINT32 irc_pulse_to_code(UINT32 pulse_width, UINT32 pulse_polarity);

#ifdef __cplusplus
}
#endif

#endif
