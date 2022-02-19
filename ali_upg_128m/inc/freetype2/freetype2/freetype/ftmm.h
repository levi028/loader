/***************************************************************************/
/*                                                                         */
/*  ftmm.h                                                                 */
/*                                                                         */
/*    FreeType Multiple Master font interface (specification).             */
/*                                                                         */
/*  Copyright 1996-2001, 2003, 2004, 2006, 2009 by                         */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTMM_H__
#define __FTMM_H__


#include <ft2build.h>
#include FT_TYPE1_TABLES_H


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    multiple_masters                                                   */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Multiple Masters                                                   */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    How to manage Multiple Masters fonts.                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The following types and functions are used to manage Multiple      */
  /*    Master fonts, i.e., the selection of specific design instances by  */
  /*    setting design axis coordinates.                                   */
  /*                                                                       */
  /*    George Williams has extended this interface to make it work with   */
  /*    both Type~1 Multiple Masters fonts and GX distortable (var)        */
  /*    fonts.  Some of these routines only work with MM fonts, others     */
  /*    will work with both types.  They are similar enough that a         */
  /*    consistent interface makes sense.                                  */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_MM_Axis                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to model a given axis in design space for  */
  /*    Multiple Masters fonts.                                            */
  /*                                                                       */
  /*    This structure can't be used for GX var fonts.                     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    name    :: The axis's name.                                        */
  /*                                                                       */
  /*    minimum :: The axis's minimum design coordinate.                   */
  /*                                                                       */
  /*    maximum :: The axis's maximum design coordinate.                   */
  /*                                                                       */
  typedef struct  ft_mm_axis_
  {
    ft_string*  name;
    ft_long     minimum;
    ft_long     maximum;

  } ft_mm_axis;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Multi_Master                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model the axes and space of a Multiple Masters */
  /*    font.                                                              */
  /*                                                                       */
  /*    This structure can't be used for GX var fonts.                     */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_axis    :: Number of axes.  Cannot exceed~4.                   */
  /*                                                                       */
  /*    num_designs :: Number of designs; should be normally 2^num_axis    */
  /*                   even though the Type~1 specification strangely      */
  /*                   allows for intermediate designs to be present. This */
  /*                   number cannot exceed~16.                            */
  /*                                                                       */
  /*    axis        :: A table of axis descriptors.                        */
  /*                                                                       */
  typedef struct  ft_multi_master_
  {
    ft_uint     num_axis;
    ft_uint     num_designs;
    ft_mm_axis  axis[T1_MAX_MM_AXIS];

  } ft_multi_master;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Var_Axis                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to model a given axis in design space for  */
  /*    Multiple Masters and GX var fonts.                                 */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    name    :: The axis's name.                                        */
  /*               Not always meaningful for GX.                           */
  /*                                                                       */
  /*    minimum :: The axis's minimum design coordinate.                   */
  /*                                                                       */
  /*    def     :: The axis's default design coordinate.                   */
  /*               FreeType computes meaningful default values for MM; it  */
  /*               is then an integer value, not in 16.16 format.          */
  /*                                                                       */
  /*    maximum :: The axis's maximum design coordinate.                   */
  /*                                                                       */
  /*    tag     :: The axis's tag (the GX equivalent to `name').           */
  /*               FreeType provides default values for MM if possible.    */
  /*                                                                       */
  /*    strid   :: The entry in `name' table (another GX version of        */
  /*               `name').                                                */
  /*               Not meaningful for MM.                                  */
  /*                                                                       */
  typedef struct  ft_var_axis_
  {
    ft_string*  name;

    ft_fixed    minimum;
    ft_fixed    def;
    ft_fixed    maximum;

    ft_ulong    tag;
    ft_uint     strid;

  } ft_var_axis;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Var_Named_Style                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to model a named style in a GX var font.   */
  /*                                                                       */
  /*    This structure can't be used for MM fonts.                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    coords :: The design coordinates for this style.                   */
  /*              This is an array with one entry for each axis.           */
  /*                                                                       */
  /*    strid  :: The entry in `name' table identifying this style.        */
  /*                                                                       */
  typedef struct  ft_var_named_style_
  {
    ft_fixed*  coords;
    ft_uint    strid;

  } ft_var_named_style;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_MM_Var                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to model the axes and space of a Multiple Masters */
  /*    or GX var distortable font.                                        */
  /*                                                                       */
  /*    Some fields are specific to one format and not to the other.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    num_axis        :: The number of axes.  The maximum value is~4 for */
  /*                       MM; no limit in GX.                             */
  /*                                                                       */
  /*    num_designs     :: The number of designs; should be normally       */
  /*                       2^num_axis for MM fonts.  Not meaningful for GX */
  /*                       (where every glyph could have a different       */
  /*                       number of designs).                             */
  /*                                                                       */
  /*    num_namedstyles :: The number of named styles; only meaningful for */
  /*                       GX which allows certain design coordinates to   */
  /*                       have a string ID (in the `name' table)          */
  /*                       associated with them.  The font can tell the    */
  /*                       user that, for example, Weight=1.5 is `Bold'.   */
  /*                                                                       */
  /*    axis            :: A table of axis descriptors.                    */
  /*                       GX fonts contain slightly more data than MM.    */
  /*                                                                       */
  /*    namedstyles     :: A table of named styles.                        */
  /*                       Only meaningful with GX.                        */
  /*                                                                       */
  typedef struct  ft_mm_var_
  {
    ft_uint              num_axis;
    ft_uint              num_designs;
    ft_uint              num_namedstyles;
    ft_var_axis*         axis;
    ft_var_named_style*  namedstyle;

  } ft_mm_var;


  /* */


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Multi_Master                                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the Multiple Master descriptor of a given font.           */
  /*                                                                       */
  /*    This function can't be used with GX fonts.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: A handle to the source face.                            */
  /*                                                                       */
  /* <Output>                                                              */
  /*    amaster :: The Multiple Masters descriptor.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_get_multi_master( ft_face           face,
                       ft_multi_master  *amaster );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_MM_Var                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the Multiple Master/GX var descriptor of a given font.    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    face    :: A handle to the source face.                            */
  /*                                                                       */
  /* <Output>                                                              */
  /*    amaster :: The Multiple Masters/GX var descriptor.                 */
  /*               Allocates a data structure, which the user must free    */
  /*               (a single call to FT_FREE will do it).                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_get_mm_var( ft_face      face,
                 ft_mm_var*  *amaster );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_MM_Design_Coordinates                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    For Multiple Masters fonts, choose an interpolated font design     */
  /*    through design coordinates.                                        */
  /*                                                                       */
  /*    This function can't be used with GX fonts.                         */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face       :: A handle to the source face.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    num_coords :: The number of design coordinates (must be equal to   */
  /*                  the number of axes in the font).                     */
  /*                                                                       */
  /*    coords     :: An array of design coordinates.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_set_mm_design_coordinates( ft_face   face,
                                ft_uint   num_coords,
                                ft_long*  coords );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Var_Design_Coordinates                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    For Multiple Master or GX Var fonts, choose an interpolated font   */
  /*    design through design coordinates.                                 */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face       :: A handle to the source face.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    num_coords :: The number of design coordinates (must be equal to   */
  /*                  the number of axes in the font).                     */
  /*                                                                       */
  /*    coords     :: An array of design coordinates.                      */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_set_var_design_coordinates( ft_face    face,
                                 ft_uint    num_coords,
                                 ft_fixed*  coords );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_MM_Blend_Coordinates                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    For Multiple Masters and GX var fonts, choose an interpolated font */
  /*    design through normalized blend coordinates.                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    face       :: A handle to the source face.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    num_coords :: The number of design coordinates (must be equal to   */
  /*                  the number of axes in the font).                     */
  /*                                                                       */
  /*    coords     :: The design coordinates array (each element must be   */
  /*                  between 0 and 1.0).                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_set_mm_blend_coordinates( ft_face    face,
                               ft_uint    num_coords,
                               ft_fixed*  coords );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Var_Blend_Coordinates                                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This is another name of @FT_Set_MM_Blend_Coordinates.              */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_set_var_blend_coordinates( ft_face    face,
                                ft_uint    num_coords,
                                ft_fixed*  coords );


  /* */


FT_END_HEADER

#endif /* __FTMM_H__ */


/* END */
