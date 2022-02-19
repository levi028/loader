/***************************************************************************/
/*                                                                         */
/*  ftbitmap.h                                                             */
/*                                                                         */
/*    FreeType utility functions for bitmaps (specification).              */
/*                                                                         */
/*  Copyright 2004, 2005, 2006, 2008 by                                    */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTBITMAP_H__
#define __FTBITMAP_H__


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
  /*    bitmap_handling                                                    */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Bitmap Handling                                                    */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    Handling FT_Bitmap objects.                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains functions for converting FT_Bitmap objects.  */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Bitmap_New                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Initialize a pointer to an @FT_Bitmap structure.                   */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    abitmap :: A pointer to the bitmap structure.                      */
  /*                                                                       */
  FT_EXPORT( void )
  ft_bitmap_new( ft_bitmap  *abitmap );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Bitmap_Copy                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Copy a bitmap into another one.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a library object.                           */
  /*                                                                       */
  /*    source  :: A handle to the source bitmap.                          */
  /*                                                                       */
  /* <Output>                                                              */
  /*    target  :: A handle to the target bitmap.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_bitmap_copy( ft_library        library,
                  const ft_bitmap  *source,
                  ft_bitmap        *target);


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Bitmap_Embolden                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Embolden a bitmap.  The new bitmap will be about `xStrength'       */
  /*    pixels wider and `yStrength' pixels higher.  The left and bottom   */
  /*    borders are kept unchanged.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library   :: A handle to a library object.                         */
  /*                                                                       */
  /*    xStrength :: How strong the glyph is emboldened horizontally.      */
  /*                 Expressed in 26.6 pixel format.                       */
  /*                                                                       */
  /*    yStrength :: How strong the glyph is emboldened vertically.        */
  /*                 Expressed in 26.6 pixel format.                       */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    bitmap    :: A handle to the target bitmap.                        */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The current implementation restricts `xStrength' to be less than   */
  /*    or equal to~8 if bitmap is of pixel_mode @FT_PIXEL_MODE_MONO.      */
  /*                                                                       */
  /*    If you want to embolden the bitmap owned by a @FT_GlyphSlotRec,    */
  /*    you should call @FT_GlyphSlot_Own_Bitmap on the slot first.        */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_bitmap_embolden( ft_library  library,
                      ft_bitmap*  bitmap,
                      ft_pos      x_strength,
                      ft_pos      y_strength );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Bitmap_Convert                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Convert a bitmap object with depth 1bpp, 2bpp, 4bpp, or 8bpp to a  */
  /*    bitmap object with depth 8bpp, making the number of used bytes per */
  /*    line (a.k.a. the `pitch') a multiple of `alignment'.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library   :: A handle to a library object.                         */
  /*                                                                       */
  /*    source    :: The source bitmap.                                    */
  /*                                                                       */
  /*    alignment :: The pitch of the bitmap is a multiple of this         */
  /*                 parameter.  Common values are 1, 2, or 4.             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    target    :: The target bitmap.                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    It is possible to call @FT_Bitmap_Convert multiple times without   */
  /*    calling @FT_Bitmap_Done (the memory is simply reallocated).        */
  /*                                                                       */
  /*    Use @FT_Bitmap_Done to finally remove the bitmap object.           */
  /*                                                                       */
  /*    The `library' argument is taken to have access to FreeType's       */
  /*    memory handling functions.                                         */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_bitmap_convert( ft_library        library,
                     const ft_bitmap  *source,
                     ft_bitmap        *target,
                     ft_int            alignment );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_GlyphSlot_Own_Bitmap                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Make sure that a glyph slot owns `slot->bitmap'.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    slot :: The glyph slot.                                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This function is to be used in combination with                    */
  /*    @FT_Bitmap_Embolden.                                               */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_glyph_slot_own_bitmap( ft_glyph_slot  slot );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FT_Bitmap_Done                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroy a bitmap object created with @FT_Bitmap_New.               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library :: A handle to a library object.                           */
  /*                                                                       */
  /*    bitmap  :: The bitmap object to be freed.                          */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The `library' argument is taken to have access to FreeType's       */
  /*    memory handling functions.                                         */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ft_bitmap_done( ft_library  library,
                  ft_bitmap  *bitmap );


  /* */


FT_END_HEADER

#endif /* __FTBITMAP_H__ */


/* END */
