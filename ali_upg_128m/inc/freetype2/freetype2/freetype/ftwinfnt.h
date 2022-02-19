/***************************************************************************/
/*                                                                         */
/*  ftwinfnt.h                                                             */
/*                                                                         */
/*    FreeType API for accessing Windows fnt-specific data.                */
/*                                                                         */
/*  Copyright 2003, 2004, 2008 by                                          */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTWINFNT_H__
#define __FTWINFNT_H__

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef FREETYPE_H
#error "freetype.h of FreeType 1 has been loaded!"
#error "Please fix the directory search order for header files"
#error "so that freetype.h of FreeType 2 is found first."
#endif


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    winfnt_fonts                                                       */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Window FNT Files                                                   */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Windows FNT specific API.                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the declaration of Windows FNT specific      */
  /*    functions.                                                         */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************
   *
   * @enum:
   *   FT_WinFNT_ID_XXX
   *
   * @description:
   *   A list of valid values for the `charset' byte in
   *   @FT_WinFNT_HeaderRec.  Exact mapping tables for the various cpXXXX
   *   encodings (except for cp1361) can be found at ftp://ftp.unicode.org
   *   in the MAPPINGS/VENDORS/MICSFT/WINDOWS subdirectory.  cp1361 is
   *   roughly a superset of MAPPINGS/OBSOLETE/EASTASIA/KSC/JOHAB.TXT.
   *
   * @values:
   *   FT_WinFNT_ID_DEFAULT ::
   *     This is used for font enumeration and font creation as a
   *     `don't care' value.  Valid font files don't contain this value.
   *     When querying for information about the character set of the font
   *     that is currently selected into a specified device context, this
   *     return value (of the related Windows API) simply denotes failure.
   *
   *   FT_WinFNT_ID_SYMBOL ::
   *     There is no known mapping table available.
   *
   *   FT_WinFNT_ID_MAC ::
   *     Mac Roman encoding.
   *
   *   FT_WinFNT_ID_OEM ::
   *     From Michael Pöttgen <michael@poettgen.de>:
   *
   *       The `Windows Font Mapping' article says that FT_WinFNT_ID_OEM
   *       is used for the charset of vector fonts, like `modern.fon',
   *       `roman.fon', and `script.fon' on Windows.
   *
   *       The `CreateFont' documentation says: The FT_WinFNT_ID_OEM value
   *       specifies a character set that is operating-system dependent.
   *
   *       The `IFIMETRICS' documentation from the `Windows Driver
   *       Development Kit' says: This font supports an OEM-specific
   *       character set.  The OEM character set is system dependent.
   *
   *       In general OEM, as opposed to ANSI (i.e., cp1252), denotes the
   *       second default codepage that most international versions of
   *       Windows have.  It is one of the OEM codepages from
   *
   *         http://www.microsoft.com/globaldev/reference/cphome.mspx,
   *
   *       and is used for the `DOS boxes', to support legacy applications.
   *       A German Windows version for example usually uses ANSI codepage
   *       1252 and OEM codepage 850.
   *
   *   FT_WinFNT_ID_CP874 ::
   *     A superset of Thai TIS 620 and ISO 8859-11.
   *
   *   FT_WinFNT_ID_CP932 ::
   *     A superset of Japanese Shift-JIS (with minor deviations).
   *
   *   FT_WinFNT_ID_CP936 ::
   *     A superset of simplified Chinese GB 2312-1980 (with different
   *     ordering and minor deviations).
   *
   *   FT_WinFNT_ID_CP949 ::
   *     A superset of Korean Hangul KS~C 5601-1987 (with different
   *     ordering and minor deviations).
   *
   *   FT_WinFNT_ID_CP950 ::
   *     A superset of traditional Chinese Big~5 ETen (with different
   *     ordering and minor deviations).
   *
   *   FT_WinFNT_ID_CP1250 ::
   *     A superset of East European ISO 8859-2 (with slightly different
   *     ordering).
   *
   *   FT_WinFNT_ID_CP1251 ::
   *     A superset of Russian ISO 8859-5 (with different ordering).
   *
   *   FT_WinFNT_ID_CP1252 ::
   *     ANSI encoding.  A superset of ISO 8859-1.
   *
   *   FT_WinFNT_ID_CP1253 ::
   *     A superset of Greek ISO 8859-7 (with minor modifications).
   *
   *   FT_WinFNT_ID_CP1254 ::
   *     A superset of Turkish ISO 8859-9.
   *
   *   FT_WinFNT_ID_CP1255 ::
   *     A superset of Hebrew ISO 8859-8 (with some modifications).
   *
   *   FT_WinFNT_ID_CP1256 ::
   *     A superset of Arabic ISO 8859-6 (with different ordering).
   *
   *   FT_WinFNT_ID_CP1257 ::
   *     A superset of Baltic ISO 8859-13 (with some deviations).
   *
   *   FT_WinFNT_ID_CP1258 ::
   *     For Vietnamese.  This encoding doesn't cover all necessary
   *     characters.
   *
   *   FT_WinFNT_ID_CP1361 ::
   *     Korean (Johab).
   */

#define ft_win_fnt_id_cp1252    0
#define ft_win_fnt_id_default   1
#define ft_win_fnt_id_symbol    2
#define ft_win_fnt_id_mac      77
#define ft_win_fnt_id_cp932   128
#define ft_win_fnt_id_cp949   129
#define ft_win_fnt_id_cp1361  130
#define ft_win_fnt_id_cp936   134
#define ft_win_fnt_id_cp950   136
#define ft_win_fnt_id_cp1253  161
#define ft_win_fnt_id_cp1254  162
#define ft_win_fnt_id_cp1258  163
#define ft_win_fnt_id_cp1255  177
#define ft_win_fnt_id_cp1256  178
#define ft_win_fnt_id_cp1257  186
#define ft_win_fnt_id_cp1251  204
#define ft_win_fnt_id_cp874   222
#define ft_win_fnt_id_cp1250  238
#define ft_win_fnt_id_oem     255


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_WinFNT_HeaderRec                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Windows FNT Header info.                                           */
  /*                                                                       */
  typedef struct  ft_win_fnt_header_rec_
  {
    ft_ushort  version;
    ft_ulong   file_size;
    ft_byte    copyright[60];
    ft_ushort  file_type;
    ft_ushort  nominal_point_size;
    ft_ushort  vertical_resolution;
    ft_ushort  horizontal_resolution;
    ft_ushort  ascent;
    ft_ushort  internal_leading;
    ft_ushort  external_leading;
    ft_byte    italic;
    ft_byte    underline;
    ft_byte    strike_out;
    ft_ushort  weight;
    ft_byte    charset;
    ft_ushort  pixel_width;
    ft_ushort  pixel_height;
    ft_byte    pitch_and_family;
    ft_ushort  avg_width;
    ft_ushort  max_width;
    ft_byte    first_char;
    ft_byte    last_char;
    ft_byte    default_char;
    ft_byte    break_char;
    ft_ushort  bytes_per_row;
    ft_ulong   device_offset;
    ft_ulong   face_name_offset;
    ft_ulong   bits_pointer;
    ft_ulong   bits_offset;
    ft_byte    reserved;
    ft_ulong   flags;
    ft_ushort  a_space;
    ft_ushort  b_space;
    ft_ushort  c_space;
    ft_ushort  color_table_offset;
    ft_ulong   reserved1[4];

  } ft_win_fnt_header_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_WinFNT_Header                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an @FT_WinFNT_HeaderRec structure.                     */
  /*                                                                       */
  typedef struct ft_win_fnt_header_rec_*  ft_win_fnt_header;


  /**********************************************************************
   *
   * @function:
   *    FT_Get_WinFNT_Header
   *
   * @description:
   *    Retrieve a Windows FNT font info header.
   *
   * @input:
   *    face    :: A handle to the input face.
   *
   * @output:
   *    aheader :: The WinFNT header.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   This function only works with Windows FNT faces, returning an error
   *   otherwise.
   */
  FT_EXPORT( ft_error )
  ft_get_win_fnt_header( ft_face               face,
                        ft_win_fnt_header_rec  *aheader );


  /* */

FT_END_HEADER

#endif /* __FTWINFNT_H__ */


/* END */


/* Local Variables: */
/* coding: utf-8    */
/* End:             */
