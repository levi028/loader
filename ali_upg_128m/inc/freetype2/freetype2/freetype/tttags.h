/***************************************************************************/
/*                                                                         */
/*  tttags.h                                                               */
/*                                                                         */
/*    Tags for TrueType and OpenType tables (specification only).          */
/*                                                                         */
/*  Copyright 1996-2001, 2004, 2005, 2007, 2008 by                         */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __TTAGS_H__
#define __TTAGS_H__


#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


#define ttag_avar  FT_MAKE_TAG( 'a', 'v', 'a', 'r' )
#define TTAG_BASE  FT_MAKE_TAG( 'B', 'A', 'S', 'E' )
#define ttag_bdat  FT_MAKE_TAG( 'b', 'd', 'a', 't' )
#define TTAG_BDF   FT_MAKE_TAG( 'B', 'D', 'F', ' ' )
#define ttag_bhed  FT_MAKE_TAG( 'b', 'h', 'e', 'd' )
#define ttag_bloc  FT_MAKE_TAG( 'b', 'l', 'o', 'c' )
#define ttag_bsln  FT_MAKE_TAG( 'b', 's', 'l', 'n' )
#define TTAG_CFF   FT_MAKE_TAG( 'C', 'F', 'F', ' ' )
#define TTAG_CID   FT_MAKE_TAG( 'C', 'I', 'D', ' ' )
#define ttag_cmap  FT_MAKE_TAG( 'c', 'm', 'a', 'p' )
#define ttag_cvar  FT_MAKE_TAG( 'c', 'v', 'a', 'r' )
#define ttag_cvt   FT_MAKE_TAG( 'c', 'v', 't', ' ' )
#define TTAG_DSIG  FT_MAKE_TAG( 'D', 'S', 'I', 'G' )
#define TTAG_EBDT  FT_MAKE_TAG( 'E', 'B', 'D', 'T' )
#define TTAG_EBLC  FT_MAKE_TAG( 'E', 'B', 'L', 'C' )
#define TTAG_EBSC  FT_MAKE_TAG( 'E', 'B', 'S', 'C' )
#define ttag_feat  FT_MAKE_TAG( 'f', 'e', 'a', 't' )
#define TTAG_FOND  FT_MAKE_TAG( 'F', 'O', 'N', 'D' )
#define ttag_fpgm  FT_MAKE_TAG( 'f', 'p', 'g', 'm' )
#define ttag_fvar  FT_MAKE_TAG( 'f', 'v', 'a', 'r' )
#define ttag_gasp  FT_MAKE_TAG( 'g', 'a', 's', 'p' )
#define TTAG_GDEF  FT_MAKE_TAG( 'G', 'D', 'E', 'F' )
#define ttag_glyf  FT_MAKE_TAG( 'g', 'l', 'y', 'f' )
#define TTAG_GPOS  FT_MAKE_TAG( 'G', 'P', 'O', 'S' )
#define TTAG_GSUB  FT_MAKE_TAG( 'G', 'S', 'U', 'B' )
#define ttag_gvar  FT_MAKE_TAG( 'g', 'v', 'a', 'r' )
#define ttag_hdmx  FT_MAKE_TAG( 'h', 'd', 'm', 'x' )
#define ttag_head  FT_MAKE_TAG( 'h', 'e', 'a', 'd' )
#define ttag_hhea  FT_MAKE_TAG( 'h', 'h', 'e', 'a' )
#define ttag_hmtx  FT_MAKE_TAG( 'h', 'm', 't', 'x' )
#define TTAG_JSTF  FT_MAKE_TAG( 'J', 'S', 'T', 'F' )
#define ttag_just  FT_MAKE_TAG( 'j', 'u', 's', 't' )
#define ttag_kern  FT_MAKE_TAG( 'k', 'e', 'r', 'n' )
#define ttag_lcar  FT_MAKE_TAG( 'l', 'c', 'a', 'r' )
#define ttag_loca  FT_MAKE_TAG( 'l', 'o', 'c', 'a' )
#define TTAG_LTSH  FT_MAKE_TAG( 'L', 'T', 'S', 'H' )
#define TTAG_LWFN  FT_MAKE_TAG( 'L', 'W', 'F', 'N' )
#define TTAG_MATH  FT_MAKE_TAG( 'M', 'A', 'T', 'H' )
#define ttag_maxp  FT_MAKE_TAG( 'm', 'a', 'x', 'p' )
#define TTAG_META  FT_MAKE_TAG( 'M', 'E', 'T', 'A' )
#define TTAG_MMFX  FT_MAKE_TAG( 'M', 'M', 'F', 'X' )
#define TTAG_MMSD  FT_MAKE_TAG( 'M', 'M', 'S', 'D' )
#define ttag_mort  FT_MAKE_TAG( 'm', 'o', 'r', 't' )
#define ttag_morx  FT_MAKE_TAG( 'm', 'o', 'r', 'x' )
#define ttag_name  FT_MAKE_TAG( 'n', 'a', 'm', 'e' )
#define ttag_opbd  FT_MAKE_TAG( 'o', 'p', 'b', 'd' )
#define TTAG_OS2   FT_MAKE_TAG( 'O', 'S', '/', '2' )
#define TTAG_OTTO  FT_MAKE_TAG( 'O', 'T', 'T', 'O' )
#define TTAG_PCLT  FT_MAKE_TAG( 'P', 'C', 'L', 'T' )
#define TTAG_POST  FT_MAKE_TAG( 'P', 'O', 'S', 'T' )
#define ttag_post  FT_MAKE_TAG( 'p', 'o', 's', 't' )
#define ttag_prep  FT_MAKE_TAG( 'p', 'r', 'e', 'p' )
#define ttag_prop  FT_MAKE_TAG( 'p', 'r', 'o', 'p' )
#define ttag_sfnt  FT_MAKE_TAG( 's', 'f', 'n', 't' )
#define TTAG_SING  FT_MAKE_TAG( 'S', 'I', 'N', 'G' )
#define ttag_trak  FT_MAKE_TAG( 't', 'r', 'a', 'k' )
#define ttag_true  FT_MAKE_TAG( 't', 'r', 'u', 'e' )
#define ttag_ttc   FT_MAKE_TAG( 't', 't', 'c', ' ' )
#define ttag_ttcf  FT_MAKE_TAG( 't', 't', 'c', 'f' )
#define TTAG_TYP1  FT_MAKE_TAG( 'T', 'Y', 'P', '1' )
#define ttag_typ1  FT_MAKE_TAG( 't', 'y', 'p', '1' )
#define TTAG_VDMX  FT_MAKE_TAG( 'V', 'D', 'M', 'X' )
#define ttag_vhea  FT_MAKE_TAG( 'v', 'h', 'e', 'a' )
#define ttag_vmtx  FT_MAKE_TAG( 'v', 'm', 't', 'x' )


FT_END_HEADER

#endif /* __TTAGS_H__ */


/* END */
