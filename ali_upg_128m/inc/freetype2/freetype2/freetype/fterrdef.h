/***************************************************************************/
/*                                                                         */
/*  fterrdef.h                                                             */
/*                                                                         */
/*    FreeType error codes (specification).                                */
/*                                                                         */
/*  Copyright 2002, 2004, 2006, 2007, 2010 by                              */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


  /*******************************************************************/
  /*******************************************************************/
  /*****                                                         *****/
  /*****                LIST OF ERROR CODES/MESSAGES             *****/
  /*****                                                         *****/
  /*******************************************************************/
  /*******************************************************************/


  /* You need to define both FT_ERRORDEF_ and FT_NOERRORDEF_ before */
  /* including this file.                                           */


  /* generic errors */

  FT_NOERRORDEF_( ok,                                        0x00, \
                  "no error" )

  FT_ERRORDEF_( cannot_open_resource,                        0x01, \
                "cannot open resource" )
  FT_ERRORDEF_( unknown_file_format,                         0x02, \
                "unknown file format" )
  FT_ERRORDEF_( invalid_file_format,                         0x03, \
                "broken file" )
  FT_ERRORDEF_( invalid_version,                             0x04, \
                "invalid FreeType version" )
  FT_ERRORDEF_( lower_module_version,                        0x05, \
                "module version is too low" )
  FT_ERRORDEF_( invalid_argument,                            0x06, \
                "invalid argument" )
  FT_ERRORDEF_( unimplemented_feature,                       0x07, \
                "unimplemented feature" )
  FT_ERRORDEF_( invalid_table,                               0x08, \
                "broken table" )
  FT_ERRORDEF_( invalid_offset,                              0x09, \
                "broken offset within table" )
  FT_ERRORDEF_( array_too_large,                             0x0A, \
                "array allocation size too large" )

  /* glyph/character errors */

  FT_ERRORDEF_( invalid_glyph_index,                         0x10, \
                "invalid glyph index" )
  FT_ERRORDEF_( invalid_character_code,                      0x11, \
                "invalid character code" )
  FT_ERRORDEF_( invalid_glyph_format,                        0x12, \
                "unsupported glyph image format" )
  FT_ERRORDEF_( cannot_render_glyph,                         0x13, \
                "cannot render this glyph format" )
  FT_ERRORDEF_( invalid_outline,                             0x14, \
                "invalid outline" )
  FT_ERRORDEF_( invalid_composite,                           0x15, \
                "invalid composite glyph" )
  FT_ERRORDEF_( too_many_hints,                              0x16, \
                "too many hints" )
  FT_ERRORDEF_( invalid_pixel_size,                          0x17, \
                "invalid pixel size" )

  /* handle errors */

  FT_ERRORDEF_( invalid_handle,                              0x20, \
                "invalid object handle" )
  FT_ERRORDEF_( invalid_library_handle,                      0x21, \
                "invalid library handle" )
  FT_ERRORDEF_( invalid_driver_handle,                       0x22, \
                "invalid module handle" )
  FT_ERRORDEF_( invalid_face_handle,                         0x23, \
                "invalid face handle" )
  FT_ERRORDEF_( invalid_size_handle,                         0x24, \
                "invalid size handle" )
  FT_ERRORDEF_( invalid_slot_handle,                         0x25, \
                "invalid glyph slot handle" )
  FT_ERRORDEF_( invalid_char_map_handle,                      0x26, \
                "invalid charmap handle" )
  FT_ERRORDEF_( invalid_cache_handle,                        0x27, \
                "invalid cache manager handle" )
  FT_ERRORDEF_( invalid_stream_handle,                       0x28, \
                "invalid stream handle" )

  /* driver errors */

  FT_ERRORDEF_( too_many_drivers,                            0x30, \
                "too many modules" )
  FT_ERRORDEF_( too_many_extensions,                         0x31, \
                "too many extensions" )

  /* memory errors */

  FT_ERRORDEF_( out_of_memory,                               0x40, \
                "out of memory" )
  FT_ERRORDEF_( unlisted_object,                             0x41, \
                "unlisted object" )

  /* stream errors */

  FT_ERRORDEF_( cannot_open_stream,                          0x51, \
                "cannot open stream" )
  FT_ERRORDEF_( invalid_stream_seek,                         0x52, \
                "invalid stream seek" )
  FT_ERRORDEF_( invalid_stream_skip,                         0x53, \
                "invalid stream skip" )
  FT_ERRORDEF_( invalid_stream_read,                         0x54, \
                "invalid stream read" )
  FT_ERRORDEF_( invalid_stream_operation,                    0x55, \
                "invalid stream operation" )
  FT_ERRORDEF_( invalid_frame_operation,                     0x56, \
                "invalid frame operation" )
  FT_ERRORDEF_( nested_frame_access,                         0x57, \
                "nested frame access" )
  FT_ERRORDEF_( invalid_frame_read,                          0x58, \
                "invalid frame read" )

  /* raster errors */

  FT_ERRORDEF_( raster_uninitialized,                        0x60, \
                "raster uninitialized" )
  FT_ERRORDEF_( raster_corrupted,                            0x61, \
                "raster corrupted" )
  FT_ERRORDEF_( raster_overflow,                             0x62, \
                "raster overflow" )
  FT_ERRORDEF_( raster_negative_height,                      0x63, \
                "negative height while rastering" )

  /* cache errors */

  FT_ERRORDEF_( too_many_caches,                             0x70, \
                "too many registered caches" )

  /* TrueType and SFNT errors */

  FT_ERRORDEF_( invalid_opcode,                              0x80, \
                "invalid opcode" )
  FT_ERRORDEF_( too_few_arguments,                           0x81, \
                "too few arguments" )
  FT_ERRORDEF_( stack_overflow,                              0x82, \
                "stack overflow" )
  FT_ERRORDEF_( code_overflow,                               0x83, \
                "code overflow" )
  FT_ERRORDEF_( bad_argument,                                0x84, \
                "bad argument" )
  FT_ERRORDEF_( divide_by_zero,                              0x85, \
                "division by zero" )
  FT_ERRORDEF_( invalid_reference,                           0x86, \
                "invalid reference" )
  FT_ERRORDEF_( debug_op_code,                                0x87, \
                "found debug opcode" )
  FT_ERRORDEF_( endf_in_exec_stream,                         0x88, \
                "found ENDF opcode in execution stream" )
  FT_ERRORDEF_( nested_defs,                                 0x89, \
                "nested DEFS" )
  FT_ERRORDEF_( invalid_code_range,                           0x8A, \
                "invalid code range" )
  FT_ERRORDEF_( execution_too_long,                          0x8B, \
                "execution context too long" )
  FT_ERRORDEF_( too_many_function_defs,                      0x8C, \
                "too many function definitions" )
  FT_ERRORDEF_( too_many_instruction_defs,                   0x8D, \
                "too many instruction definitions" )
  FT_ERRORDEF_( table_missing,                               0x8E, \
                "SFNT font table missing" )
  FT_ERRORDEF_( horiz_header_missing,                        0x8F, \
                "horizontal header (hhea) table missing" )
  FT_ERRORDEF_( locations_missing,                           0x90, \
                "locations (loca) table missing" )
  FT_ERRORDEF_( name_table_missing,                          0x91, \
                "name table missing" )
  FT_ERRORDEF_( cmap_table_missing,                          0x92, \
                "character map (cmap) table missing" )
  FT_ERRORDEF_( hmtx_table_missing,                          0x93, \
                "horizontal metrics (hmtx) table missing" )
  FT_ERRORDEF_( post_table_missing,                          0x94, \
                "PostScript (post) table missing" )
  FT_ERRORDEF_( invalid_horiz_metrics,                       0x95, \
                "invalid horizontal metrics" )
  FT_ERRORDEF_( invalid_char_map_format,                      0x96, \
                "invalid character map (cmap) format" )
  FT_ERRORDEF_( invalid_ppem,                                0x97, \
                "invalid ppem value" )
  FT_ERRORDEF_( invalid_vert_metrics,                        0x98, \
                "invalid vertical metrics" )
  FT_ERRORDEF_( could_not_find_context,                      0x99, \
                "could not find context" )
  FT_ERRORDEF_( invalid_post_table_format,                   0x9A, \
                "invalid PostScript (post) table format" )
  FT_ERRORDEF_( invalid_post_table,                          0x9B, \
                "invalid PostScript (post) table" )

  /* CFF, CID, and Type 1 errors */

  FT_ERRORDEF_( syntax_error,                                0xA0, \
                "opcode syntax error" )
  FT_ERRORDEF_( stack_underflow,                             0xA1, \
                "argument stack underflow" )
  FT_ERRORDEF_( ignore,                                      0xA2, \
                "ignore" )
  FT_ERRORDEF_( no_unicode_glyph_name,                       0xA3, \
                "no Unicode glyph name found" )


  /* BDF errors */

  FT_ERRORDEF_( missing_startfont_field,                     0xB0, \
                "`STARTFONT' field missing" )
  FT_ERRORDEF_( missing_font_field,                          0xB1, \
                "`FONT' field missing" )
  FT_ERRORDEF_( missing_size_field,                          0xB2, \
                "`SIZE' field missing" )
  FT_ERRORDEF_( missing_fontboundingbox_field,               0xB3, \
                "`FONTBOUNDINGBOX' field missing" )
  FT_ERRORDEF_( missing_chars_field,                         0xB4, \
                "`CHARS' field missing" )
  FT_ERRORDEF_( missing_startchar_field,                     0xB5, \
                "`STARTCHAR' field missing" )
  FT_ERRORDEF_( missing_encoding_field,                      0xB6, \
                "`ENCODING' field missing" )
  FT_ERRORDEF_( missing_bbx_field,                           0xB7, \
                "`BBX' field missing" )
  FT_ERRORDEF_( bbx_too_big,                                 0xB8, \
                "`BBX' too big" )
  FT_ERRORDEF_( corrupted_font_header,                       0xB9, \
                "Font header corrupted or missing fields" )
  FT_ERRORDEF_( corrupted_font_glyphs,                       0xBA, \
                "Font glyphs corrupted or missing fields" )


/* END */
