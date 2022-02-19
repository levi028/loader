/***************************************************************************/
/*                                                                         */
/*  ftrender.h                                                             */
/*                                                                         */
/*    FreeType renderer modules public interface (specification).          */
/*                                                                         */
/*  Copyright 1996-2001, 2005, 2006, 2010 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTRENDER_H__
#define __FTRENDER_H__


#include <ft2build.h>
#include FT_MODULE_H
#include FT_GLYPH_H


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    module_management                                                  */
  /*                                                                       */
  /*************************************************************************/


  /* create a new glyph object */
  typedef ft_error
  (*ft_glyph_init_func_x)( ft_glyph      glyph,
                        ft_glyph_slot  slot );

  /* destroys a given glyph object */
  typedef void
  (*ft_glyph_done_func_x)( ft_glyph  glyph );

  typedef void
  (*ft_glyph_transform_func_x)( ft_glyph          glyph,
                             const ft_matrix*  matrix,
                             const ft_vector*  delta );

  typedef void
  (*ft_glyph_get_bbox_func)( ft_glyph  glyph,
                           ft_bbox*  abbox );

  typedef ft_error
  (*ft_glyph_copy_func_x)( ft_glyph   source,
                        ft_glyph   target );

  typedef ft_error
  (*ft_glyph_prepare_func_x)( ft_glyph      glyph,
                           ft_glyph_slot  slot );

/* deprecated */
#define ft_glyph_init_func       ft_glyph_init_func_x
#define ft_glyph_done_func       ft_glyph_done_func_x
#define ft_glyph_transform_func  ft_glyph_transform_func_x
#define ft_glyph_bbox_func       ft_glyph_get_bbox_func
#define ft_glyph_copy_func       ft_glyph_copy_func_x
#define ft_glyph_prepare_func    ft_glyph_prepare_func_x


  struct  ft_glyph_class_
  {
    ft_long                 glyph_size;
    ft_glyph_format         glyph_format;
    ft_glyph_init_func_x       glyph_init;
    ft_glyph_done_func_x       glyph_done;
    ft_glyph_copy_func_x       glyph_copy;
    ft_glyph_transform_func_x  glyph_transform;
    ft_glyph_get_bbox_func    glyph_bbox;
    ft_glyph_prepare_func_x    glyph_prepare;
  };


  typedef ft_error
  (*ft_renderer_render_func)( ft_renderer       renderer,
                             ft_glyph_slot      slot,
                             ft_uint           mode,
                             const ft_vector*  origin );

  typedef ft_error
  (*ft_renderer_transform_func)( ft_renderer       renderer,
                                ft_glyph_slot      slot,
                                const ft_matrix*  matrix,
                                const ft_vector*  delta );


  typedef void
  (*ft_renderer_get_cbox_func)( ft_renderer   renderer,
                              ft_glyph_slot  slot,
                              ft_bbox*      cbox );


  typedef ft_error
  (*ft_renderer_set_mode_func)( ft_renderer  renderer,
                              ft_ulong     mode_tag,
                              ft_pointer   mode_ptr );

/* deprecated identifiers */
#define ftrenderer_render  ft_renderer_render_func
#define ftrenderer_transform  ft_renderer_transform_func
#define ftrenderer_get_cbox  ft_renderer_get_cbox_func
#define ftrenderer_set_mode  ft_renderer_set_mode_func


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Renderer_Class                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The renderer module class descriptor.                              */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    root            :: The root @FT_Module_Class fields.               */
  /*                                                                       */
  /*    glyph_format    :: The glyph image format this renderer handles.   */
  /*                                                                       */
  /*    render_glyph    :: A method used to render the image that is in a  */
  /*                       given glyph slot into a bitmap.                 */
  /*                                                                       */
  /*    transform_glyph :: A method used to transform the image that is in */
  /*                       a given glyph slot.                             */
  /*                                                                       */
  /*    get_glyph_cbox  :: A method used to access the glyph's cbox.       */
  /*                                                                       */
  /*    set_mode        :: A method used to pass additional parameters.    */
  /*                                                                       */
  /*    raster_class    :: For @FT_GLYPH_FORMAT_OUTLINE renderers only.    */
  /*                       This is a pointer to its raster's class.        */
  /*                                                                       */
  typedef struct  ft_renderer_class_
  {
    ft_module_class            root;

    ft_glyph_format            glyph_format;

    ft_renderer_render_func     render_glyph;
    ft_renderer_transform_func  transform_glyph;
    ft_renderer_get_cbox_func    get_glyph_cbox;
    ft_renderer_set_mode_func    set_mode;

    ft_raster_funcs*           raster_class;

  } ft_renderer_class;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Get_Renderer                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the current renderer for a given glyph format.            */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to the library object.                         */
  /*                                                                       */
  /*    format  :: The glyph format.                                       */
  /*                                                                       */
  /* <Return>                                                              */
  /*    A renderer handle.  0~if none found.                               */
  /*                                                                       */
  /* <Note>                                                                */
  /*    An error will be returned if a module already exists by that name, */
  /*    or if the module requires a version of FreeType that is too great. */
  /*                                                                       */
  /*    To add a new renderer, simply use @FT_Add_Module.  To retrieve a   */
  /*    renderer by its name, use @FT_Get_Module.                          */
  /*                                                                       */
  FT_EXPORT( ft_renderer )
  ft_get_renderer( ft_library       library,
                   ft_glyph_format  format );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Set_Renderer                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Set the current renderer to use, and set additional mode.          */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    library    :: A handle to the library object.                      */
  /*                                                                       */
  /* <Input>                                                               */
  /*    renderer   :: A handle to the renderer object.                     */
  /*                                                                       */
  /*    num_params :: The number of additional parameters.                 */
  /*                                                                       */
  /*    parameters :: Additional parameters.                               */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    In case of success, the renderer will be used to convert glyph     */
  /*    images in the renderer's known format into bitmaps.                */
  /*                                                                       */
  /*    This doesn't change the current renderer for other formats.        */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_set_renderer( ft_library     library,
                   ft_renderer    renderer,
                   ft_uint        num_params,
                   ft_parameter*  parameters );


  /* */


FT_END_HEADER

#endif /* __FTRENDER_H__ */


/* END */
