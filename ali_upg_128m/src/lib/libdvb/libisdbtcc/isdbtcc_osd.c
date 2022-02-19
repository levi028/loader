#include <basic_types.h>
#include <mediatypes.h>

#include <api/libc/printf.h>
#include <hld/osd/osddrv.h>
#include <api/libisdbtcc/isdbtcc_osd.h>
#include <api/libisdbtcc/lib_isdbtcc.h>

#include <hld/hld_dev.h>
#include <hld/isdbtcc/isdbtcc_dec.h>
#include <hld/dis/vpo.h>


struct isdbtcc_config_par g_isdbtcc_config_par;//
BOOL g_isdbtcc_init = FALSE;
/*Osd parameter config already:
TRUE: ready to call osd_isdbtcc_enter(),
FASLE: need to call lib_isdbtcc_attach() or first lib_isdbtcc_init()
*/

//
BOOL g_isdbtcc_show_on = FALSE;
BOOL g_isdbtcc_update_palette = FALSE;
extern void isdbtcc_osd_enter(void);
extern void isdbtcc_osd_leave(void);
/////////////////////////////////////////////////////////////////////////////////////////////////
void lib_isdbtcc_attach(struct isdbtcc_config_par *pcc_config_par)
{
    BOOL bl_addr_range_legal = TRUE;

    if(NULL == pcc_config_par)
    {
        ASSERT(0);
    }
    else
    {
        /* Addr legal check */
		#if (defined(_M3715C_) || defined(_M3823C_)) 
		pcc_config_par->g_buf_addr=(UINT8*)__MM_ISDBTCC_PB_START_ADDR;
		pcc_config_par->g_buf_len=__MM_ISDBTCC_PB_LEN;
		#endif
        bl_addr_range_legal  = osal_check_address_range_legal(pcc_config_par->g_buf_addr, pcc_config_par->g_buf_len);
        if (FALSE == bl_addr_range_legal)
        {
            libc_printf("%s:%d Addr Illegal!\n",__func__,__LINE__);
            return;
        }
        MEMCPY(&g_isdbtcc_config_par, pcc_config_par, sizeof(struct isdbtcc_config_par));
    }

    g_isdbtcc_config_par.osd_isdbtcc_enter=isdbtcc_osd_enter;
    g_isdbtcc_config_par.osd_isdbtcc_leave=isdbtcc_osd_leave;
    g_isdbtcc_init=TRUE;
}

void osd_isdbtcc_enter()
{
    if(FALSE==g_isdbtcc_init )
        {return;}
    return g_isdbtcc_config_par.osd_isdbtcc_enter();
}

void osd_isdbtcc_leave()
{
    if(FALSE==g_isdbtcc_init )
        {return;}
    return g_isdbtcc_config_par.osd_isdbtcc_leave();
}

