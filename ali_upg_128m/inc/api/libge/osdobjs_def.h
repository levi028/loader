#ifndef _OSDOBJS_DEF_DUAL_H_
#define _OSDOBJS_DEF_DUAL_H_
#define vratio (0.8)
#define hratio (0.5625)
#define wstylsdoffset (125)
#define bmpidsdoffset (1024)

#define DEF_TEXTFIELD(txt,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,str_id,str)   \
SDTXT txt##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        (UINT8)((ox)*hratio),(UINT8)((oy)*vratio),NULL\
}; \
TEXT_FIELD txt = \
{   \
    {OT_TEXTFIELD, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,str_id,str,(UINT32)&(txt##_sd) \
};

#define DEF_BITMAP(bmp,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,icon_id)   \
SDBMP bmp##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry + wstylsdoffset},    \
        (UINT8)((ox)*hratio),(UINT8)((oy)*vratio),(icon_id + bmpidsdoffset),\
}; \
BITMAP bmp = \
{   \
    {OT_BITMAP, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,icon_id,(UINT32)&(bmp##_sd) \
};

#define DEF_CONTAINER(con,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,chlst,focus_id,all_hilite)   \
SDCON con##_sd = \
{  \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
         {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry + wstylsdoffset},    \
}; \
CONTAINER con = \
{   \
    {OT_CONTAINER, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD)chlst,focus_id,all_hilite,(UINT32)&(con##_sd) \
};


#define DEF_MULTITEXT(mtxt,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,count,rl,rt,rw,rh,sb,txttbl)   \
SD_MULTXT mtxt##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
         {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry + wstylsdoffset},    \
         {(UINT16)((rl)*hratio),(UINT16)((rt)*vratio),(UINT16)((rw)*hratio),(UINT16)((rh)*vratio)},      \
}; \
MULTI_TEXT mtxt = \
{   \
    {OT_MULTITEXT, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,count,0,{rl,rt,rw,rh},sb,txttbl,(UINT32)&(mtxt##_sd) \
};


#define DEF_OBJECTLIST(ol,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,field,sb,mark,style,dep,cnt,selarray)   \
SD_OBJLST ol##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
}; \
OBJLIST ol = \
{   \
    {OT_OBJLIST, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD*)field,sb,style,dep,cnt,0,0,0,0, selarray,mark,(UINT32)&(ol##_sd) \
};


#define DEF_MULTISEL(msel,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,style,ptbl,cur,cnt)   \
SD_MULSEL msel##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        (UINT8)((ox)*hratio),(UINT8)((oy)*vratio),\
}; \
MULTISEL msel = \
{   \
    {OT_MULTISEL, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,style,(void*)ptbl,cnt,cur,(UINT32)&(msel##_sd) \
};

#define DEF_EDITFIELD(edit,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,style,pat,maxlen,cursormode,pre,sub,str)   \
SD_EDF edit##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        (UINT8)((ox)*hratio),(UINT8)((oy)*vratio),\
}; \
EDIT_FIELD edit = \
{   \
    {OT_EDITFIELD, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,maxlen,pat,style,cursormode,0,str,pre,sub, 0,(UINT32)&(edit##_sd) \
};

#define DEF_PROGRESSBAR(bar,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,b_x,b_y,bg,fg,rcl,rct,rcw,rch,min,max,block,pos)    \
SD_PBAR bar##_sd =\
{    \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)+2},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        (UINT8)((b_x)*hratio),(UINT8)((b_y)*vratio),bg + wstylsdoffset,fg + wstylsdoffset,    \
    {(UINT16)((rcl)*hratio),(UINT16)((rct)*vratio),(UINT16)((rcw)*hratio),(UINT16)((rch)*vratio)+2},    \
}; \
PROGRESS_BAR  bar =\
{    \
    {OT_PROGRESSBAR, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
            {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },    \
    style,b_x,b_y,bg,fg,    \
    {rcl,rct,rcw,rch},    \
    min,max,block,pos,(UINT32)&(bar##_sd) \
};

#define DEF_SCROLLBAR(bar,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,page,thumb,bg,rcl,rct,rcw,rch,max,pos)    \
SD_SBAR bar##_sd =\
{    \
            {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        thumb + wstylsdoffset,bg +  + wstylsdoffset,    \
    {(UINT16)((rcl)*hratio),(UINT16)((rct)*vratio),(UINT16)((rcw)*hratio),(UINT16)((rch)*vratio)},    \
}; \
SCROLL_BAR bar =\
{    \
    {OT_SCROLLBAR, attr, font,ID,l_id,r_id,t_id,d_id,   \
            {l,t,w,h},      \
            {sh,hl,sel,gry},    \
            kmap,cb,    \
            (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },    \
    style,page,thumb,bg,    \
    {rcl,rct,rcw,rch},    \
    max,pos,(UINT32)&(bar##_sd) \
};

#define DEF_LIST(ls,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,field,numfield,page,bar,intervaly,msel,cnt,ntop,nsel,npos)   \
SD_LST ls##_sd = \
{   \
       {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        (INT8)((intervaly)*vratio),\
}; \
LIST ls = \
{   \
    {OT_LIST, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    style,(lp_listfield)field,numfield,page,bar,intervaly,(UINT32 *)msel,cnt,ntop,nsel,npos,(UINT32)&(ls##_sd)\
};

#define DEF_MATRIXBOX(mb,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,ctype,cnt,ctable,row,col,assigntype,ix,iy,itt,itl,iit,iil,pos)   \
SD_MTXBOX mb##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry +wstylsdoffset},    \
        (INT16)((ix)*hratio),(INT16)((iy)*vratio),\
}; \
MATRIX_BOX mb = \
{   \
    {OT_MATRIXBOX, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    style,ctype,cnt,(void*)ctable,row,col,assigntype,ix,iy,itt,itl,iit,iil,pos,(UINT32)&(mb##_sd)\
};

#define LDEF_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, WSTL_TXT_3,WSTL_TXT_3,WSTL_TXT_3,WSTL_TXT_3,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)
#endif

