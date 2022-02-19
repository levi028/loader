/*****************************************************************************
*    Copyright (c) 2013 ALi Corp. All Rights Reserved
*    This source is confidential and is ALi's proprietary information.
*    This source is subject to ALi License Agreement, and shall not be
     disclosed to unauthorized individual.
*    File: osdobjs_def.h
*
*    Description: define osd objects.
*    THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*      KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
*      IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
*      PARTICULAR PURPOSE.
*****************************************************************************/

#ifndef _OSDOBJS_DEF_H_
#define _OSDOBJS_DEF_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define DEF_TEXTFIELD(txt,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,str_id,str)   \
TEXT_FIELD txt = \
{   \
    {OT_TEXTFIELD, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,str_id,str \
};

#define DEF_BITMAP(bmp,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,icon_id)   \
BITMAP bmp = \
{   \
    {OT_BITMAP, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,icon_id \
};

#define DEF_CONTAINER(con,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,chlst,focus_id,all_hilite)   \
CONTAINER con = \
{   \
    {OT_CONTAINER, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD)chlst,focus_id,all_hilite \
};

#define DEF_CONTAINER_EX(con,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,chlst,focus_id,all_hilite,draweffect)   \
CONTAINER con = \
{   \
    {OT_CONTAINER, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD)chlst,focus_id,(draweffect<<2)|(all_hilite&0x03) \
};



#define DEF_MULTITEXT(mtxt,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,count,rl,rt,rw,rh,sb,txttbl)   \
MULTI_TEXT mtxt = \
{   \
    {OT_MULTITEXT, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,count,0,{rl,rt,rw,rh},sb,txttbl \
};


#define DEF_OBJECTLIST(ol,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,field,sb,mark,style,dep,cnt,selarray)   \
OBJLIST ol = \
{   \
    {OT_OBJLIST, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    (POBJECT_HEAD*)field,sb,style,dep,cnt,0,0,0,0, selarray,mark\
};

#define DEF_MULTISEL(msel,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,style,ptbl,cur,cnt)   \
MULTISEL msel = \
{   \
    {OT_MULTISEL, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,style,(void*)ptbl,cnt,cur \
};

#define DEF_EDITFIELD(edit,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,style,pat,maxlen,cursormode,pre,sub,str)   \
EDIT_FIELD edit = \
{   \
    {OT_EDITFIELD, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,maxlen,pat,style,cursormode,0,str,pre,sub, 0\
};

#define DEF_PROGRESSBAR(bar,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,b_x,b_y,bg,fg,rcl,rct,rcw,rch,min,max,block,pos)    \
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
    min,max,block,pos \
};

#define DEF_SCROLLBAR(bar,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,page,thumb,bg,rcl,rct,rcw,rch,max,pos)    \
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
    max,pos \
};

#define DEF_LIST(ls,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,field,numfield,page,bar,intervaly,msel,cnt,ntop,nsel,npos)   \
LIST ls = \
{   \
    {OT_LIST, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    style,(lp_listfield)field,numfield,page,bar,intervaly,(UINT32 *)msel,cnt,ntop,nsel,npos\
};

#define DEF_MATRIXBOX(mb,root,next,attr,font,ID,l_id,r_id,t_id,d_id,l,t,w,h,sh,hl,sel,gry,kmap,cb,style,ctype,cnt,ctable,row,col,assigntype,ix,iy,itt,itl,iit,iil,pos)   \
MATRIX_BOX mb = \
{   \
    {OT_MATRIXBOX, attr, font,ID,l_id,r_id,t_id,d_id,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    style,ctype,cnt,(void*)ctable,row,col,assigntype,ix,iy,itt,itl,iit,iil,pos\
};


#define LDEF_LINE(root,var_txt,nxt_obj,ID,idl,idr,idu,idd,l,t,w,h,res_id,str)        \
    DEF_TEXTFIELD(var_txt,root,nxt_obj,C_ATTR_ACTIVE,0, \
    ID,idl,idr,idu,idd, l,t,w,h, WSTL_LINE_HD,WSTL_LINE_HD,WSTL_LINE_HD,WSTL_LINE_HD,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,res_id,str)

#ifdef __cplusplus
}
#endif

#endif//_OSDOBJS_DEF_H_

