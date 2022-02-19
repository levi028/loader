/***************************************************************************/
/*                                                                         */
/*  t1tables.h                                                             */
/*                                                                         */
/*    Basic Type 1/Type 2 tables definitions and interface (specification  */
/*    only).                                                               */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2006, 2008, 2009 by             */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __T1TABLES_H__
#define __T1TABLES_H__


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
  /*    type1_tables                                                       */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Type 1 Tables                                                      */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Type~1 (PostScript) specific font tables.                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the definition of Type 1-specific tables,    */
  /*    including structures related to other PostScript font formats.     */
  /*                                                                       */
  /*************************************************************************/


  /* Note that we separate font data in PS_FontInfoRec and PS_PrivateRec */
  /* structures in order to support Multiple Master fonts.               */


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_FontInfoRec                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a Type~1 or Type~2 FontInfo dictionary.  */
  /*    Note that for Multiple Master fonts, each instance has its own     */
  /*    FontInfo dictionary.                                               */
  /*                                                                       */
  typedef struct  ps_font_info_rec_
  {
    ft_string*  version;
    ft_string*  notice;
    ft_string*  full_name;
    ft_string*  family_name;
    ft_string*  weight;
    ft_long     italic_angle;
    ft_bool     is_fixed_pitch;
    ft_short    underline_position;
    ft_ushort   underline_thickness;

  } ps_font_info_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_FontInfo                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a @PS_FontInfoRec structure.                           */
  /*                                                                       */
  typedef struct ps_font_info_rec_*  ps_font_info;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    T1_FontInfo                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This type is equivalent to @PS_FontInfoRec.  It is deprecated but  */
  /*    kept to maintain source compatibility between various versions of  */
  /*    FreeType.                                                          */
  /*                                                                       */
  typedef ps_font_info_rec  t1_font_info;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_PrivateRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model a Type~1 or Type~2 private dictionary.   */
  /*    Note that for Multiple Master fonts, each instance has its own     */
  /*    Private dictionary.                                                */
  /*                                                                       */
  typedef struct  ps_private_rec_
  {
    ft_int     unique_id;
    ft_int     len_iv;

    ft_byte    num_blue_values;
    ft_byte    num_other_blues;
    ft_byte    num_family_blues;
    ft_byte    num_family_other_blues;

    ft_short   blue_values[14];
    ft_short   other_blues[10];

    ft_short   family_blues      [14];
    ft_short   family_other_blues[10];

    ft_fixed   blue_scale;
    ft_int     blue_shift;
    ft_int     blue_fuzz;

    ft_ushort  standard_width[1];
    ft_ushort  standard_height[1];

    ft_byte    num_snap_widths;
    ft_byte    num_snap_heights;
    ft_bool    force_bold;
    ft_bool    round_stem_up;

    ft_short   snap_widths [13];  /* including std width  */
    ft_short   snap_heights[13];  /* including std height */

    ft_fixed   expansion_factor;

    ft_long    language_group;
    ft_long    password;

    ft_short   min_feature[2];

  } ps_private_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    PS_Private                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a @PS_PrivateRec structure.                            */
  /*                                                                       */
  typedef struct ps_private_rec_*  ps_private;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    T1_Private                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*   This type is equivalent to @PS_PrivateRec.  It is deprecated but    */
  /*   kept to maintain source compatibility between various versions of   */
  /*   FreeType.                                                           */
  /*                                                                       */
  typedef ps_private_rec  t1_private;


  /*************************************************************************/
  /*                                                                       */
  /* <Enum>                                                                */
  /*    T1_Blend_Flags                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A set of flags used to indicate which fields are present in a      */
  /*    given blend dictionary (font info or private).  Used to support    */
  /*    Multiple Masters fonts.                                            */
  /*                                                                       */
  typedef enum  t1_blend_flags_
  {
    /*# required fields in a FontInfo blend dictionary */
    T1_BLEND_UNDERLINE_POSITION = 0,
    T1_BLEND_UNDERLINE_THICKNESS,
    T1_BLEND_ITALIC_ANGLE,

    /*# required fields in a Private blend dictionary */
    T1_BLEND_BLUE_VALUES,
    T1_BLEND_OTHER_BLUES,
    T1_BLEND_STANDARD_WIDTH,
    T1_BLEND_STANDARD_HEIGHT,
    T1_BLEND_STEM_SNAP_WIDTHS,
    T1_BLEND_STEM_SNAP_HEIGHTS,
    T1_BLEND_BLUE_SCALE,
    T1_BLEND_BLUE_SHIFT,
    T1_BLEND_FAMILY_BLUES,
    T1_BLEND_FAMILY_OTHER_BLUES,
    T1_BLEND_FORCE_BOLD,

    /*# never remove */
    T1_BLEND_MAX

  } t1_blend_flags;

  /* */


  /*# backwards compatible definitions */
#define t1_blend_underline_position   T1_BLEND_UNDERLINE_POSITION
#define t1_blend_underline_thickness  T1_BLEND_UNDERLINE_THICKNESS
#define t1_blend_italic_angle         T1_BLEND_ITALIC_ANGLE
#define t1_blend_blue_values          T1_BLEND_BLUE_VALUES
#define t1_blend_other_blues          T1_BLEND_OTHER_BLUES
#define t1_blend_standard_widths      T1_BLEND_STANDARD_WIDTH
#define t1_blend_standard_height      T1_BLEND_STANDARD_HEIGHT
#define t1_blend_stem_snap_widths     T1_BLEND_STEM_SNAP_WIDTHS
#define t1_blend_stem_snap_heights    T1_BLEND_STEM_SNAP_HEIGHTS
#define t1_blend_blue_scale           T1_BLEND_BLUE_SCALE
#define t1_blend_blue_shift           T1_BLEND_BLUE_SHIFT
#define t1_blend_family_blues         T1_BLEND_FAMILY_BLUES
#define t1_blend_family_other_blues   T1_BLEND_FAMILY_OTHER_BLUES
#define t1_blend_force_bold           T1_BLEND_FORCE_BOLD
#define t1_blend_max                  T1_BLEND_MAX


  /* maximum number of Multiple Masters designs, as defined in the spec */
#define T1_MAX_MM_DESIGNS     16

  /* maximum number of Multiple Masters axes, as defined in the spec */
#define T1_MAX_MM_AXIS        4

  /* maximum number of elements in a design map */
#define T1_MAX_MM_MAP_POINTS  20


  /* this structure is used to store the BlendDesignMap entry for an axis */
  typedef struct  ps_design_map_
  {
    ft_byte    num_points;
    ft_long*   design_points;
    ft_fixed*  blend_points;

  } ps_design_map_rec, *ps_design_map;

  /* backwards-compatible definition */
  typedef ps_design_map_rec  t1_design_map;


  typedef struct  ps_blend_rec_
  {
    ft_uint          num_designs;
    ft_uint          num_axis;

    ft_string*       axis_names[T1_MAX_MM_AXIS];
    ft_fixed*        design_pos[T1_MAX_MM_DESIGNS];
    ps_design_map_rec  design_map[T1_MAX_MM_AXIS];

    ft_fixed*        weight_vector;
    ft_fixed*        default_weight_vector;

    ps_font_info      font_infos[T1_MAX_MM_DESIGNS + 1];
    ps_private       privates  [T1_MAX_MM_DESIGNS + 1];

    ft_ulong         blend_bitflags;

    ft_bbox*         bboxes    [T1_MAX_MM_DESIGNS + 1];

    /* since 2.3.0 */

    /* undocumented, optional: the default design instance;   */
    /* corresponds to default_weight_vector --                */
    /* num_default_design_vector == 0 means it is not present */
    /* in the font and associated metrics files               */
    ft_uint          default_design_vector[T1_MAX_MM_DESIGNS];
    ft_uint          num_default_design_vector;

  } ps_blend_rec, *ps_blend;


  /* backwards-compatible definition */
  typedef ps_blend_rec  t1_blend;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    CID_FaceDictRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to represent data in a CID top-level dictionary.  */
  /*                                                                       */
  typedef struct  cid_face_dict_rec_
  {
    ps_private_rec  private_dict;

    ft_uint        len_buildchar;
    ft_fixed       forcebold_threshold;
    ft_pos         stroke_width;
    ft_fixed       expansion_factor;

    ft_byte        paint_type;
    ft_byte        font_type;
    ft_matrix      font_matrix;
    ft_vector      font_offset;

    ft_uint        num_subrs;
    ft_ulong       subrmap_offset;
    ft_int         sd_bytes;

  } cid_face_dict_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    CID_FaceDict                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a @CID_FaceDictRec structure.                          */
  /*                                                                       */
  typedef struct cid_face_dict_rec_*  cid_face_dict;

  /* */


  /* backwards-compatible definition */
  typedef cid_face_dict_rec  cid_font_dict;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    CID_FaceInfoRec                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to represent CID Face information.                */
  /*                                                                       */
  typedef struct  cid_face_info_rec_
  {
    ft_string*      cid_font_name;
    ft_fixed        cid_version;
    ft_int          cid_font_type;

    ft_string*      registry;
    ft_string*      ordering;
    ft_int          supplement;

    ps_font_info_rec  font_info;
    ft_bbox         font_bbox;
    ft_ulong        uid_base;

    ft_int          num_xuid;
    ft_ulong        xuid[16];

    ft_ulong        cidmap_offset;
    ft_int          fd_bytes;
    ft_int          gd_bytes;
    ft_ulong        cid_count;

    ft_int          num_dicts;
    cid_face_dict    font_dicts;

    ft_ulong        data_offset;

  } cid_face_info_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    CID_FaceInfo                                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a @CID_FaceInfoRec structure.                          */
  /*                                                                       */
  typedef struct cid_face_info_rec_*  cid_face_info;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    CID_Info                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*   This type is equivalent to @CID_FaceInfoRec.  It is deprecated but  */
  /*   kept to maintain source compatibility between various versions of   */
  /*   FreeType.                                                           */
  /*                                                                       */
  typedef cid_face_info_rec  cid_info;


  /************************************************************************
   *
   * @function:
   *    FT_Has_PS_Glyph_Names
   *
   * @description:
   *    Return true if a given face provides reliable PostScript glyph
   *    names.  This is similar to using the @FT_HAS_GLYPH_NAMES macro,
   *    except that certain fonts (mostly TrueType) contain incorrect
   *    glyph name tables.
   *
   *    When this function returns true, the caller is sure that the glyph
   *    names returned by @FT_Get_Glyph_Name are reliable.
   *
   * @input:
   *    face ::
   *       face handle
   *
   * @return:
   *    Boolean.  True if glyph names are reliable.
   *
   */
  FT_EXPORT( ft_int )
  ft_has_ps_glyph_names( ft_face  face );


  /************************************************************************
   *
   * @function:
   *    FT_Get_PS_Font_Info
   *
   * @description:
   *    Retrieve the @PS_FontInfoRec structure corresponding to a given
   *    PostScript font.
   *
   * @input:
   *    face ::
   *       PostScript face handle.
   *
   * @output:
   *    afont_info ::
   *       Output font info structure pointer.
   *
   * @return:
   *    FreeType error code.  0~means success.
   *
   * @note:
   *    The string pointers within the font info structure are owned by
   *    the face and don't need to be freed by the caller.
   *
   *    If the font's format is not PostScript-based, this function will
   *    return the `FT_Err_Invalid_Argument' error code.
   *
   */
  FT_EXPORT( ft_error )
  ft_get_ps_font_info( ft_face      face,
                       ps_font_info  afont_info );


  /************************************************************************
   *
   * @function:
   *    FT_Get_PS_Font_Private
   *
   * @description:
   *    Retrieve the @PS_PrivateRec structure corresponding to a given
   *    PostScript font.
   *
   * @input:
   *    face ::
   *       PostScript face handle.
   *
   * @output:
   *    afont_private ::
   *       Output private dictionary structure pointer.
   *
   * @return:
   *    FreeType error code.  0~means success.
   *
   * @note:
   *    The string pointers within the @PS_PrivateRec structure are owned by
   *    the face and don't need to be freed by the caller.
   *
   *    If the font's format is not PostScript-based, this function returns
   *    the `FT_Err_Invalid_Argument' error code.
   *
   */
  FT_EXPORT( ft_error )
  ft_get_ps_font_private( ft_face     face,
                          ps_private  afont_private );

  /* */


FT_END_HEADER

#endif /* __T1TABLES_H__ */


/* END */
