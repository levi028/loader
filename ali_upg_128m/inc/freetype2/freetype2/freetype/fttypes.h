/***************************************************************************/
/*                                                                         */
/*  fttypes.h                                                              */
/*                                                                         */
/*    FreeType simple types definitions (specification only).              */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2004, 2006, 2007, 2008 by                   */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTTYPES_H__
#define __FTTYPES_H__


#include <ft2build.h>
#include FT_CONFIG_CONFIG_H
#include FT_SYSTEM_H
#include FT_IMAGE_H

#include <stddef.h>


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    basic_types                                                        */
  /*                                                                       */
  /* <Title>                                                               */
  /*    Basic Data Types                                                   */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*    The basic data types defined by the library.                       */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This section contains the basic data types defined by FreeType~2,  */
  /*    ranging from simple scalar types to bitmap descriptors.  More      */
  /*    font-specific structures are defined in a different section.       */
  /*                                                                       */
  /* <Order>                                                               */
  /*    FT_Byte                                                            */
  /*    FT_Bytes                                                           */
  /*    FT_Char                                                            */
  /*    FT_Int                                                             */
  /*    FT_UInt                                                            */
  /*    FT_Int16                                                           */
  /*    FT_UInt16                                                          */
  /*    FT_Int32                                                           */
  /*    FT_UInt32                                                          */
  /*    FT_Short                                                           */
  /*    FT_UShort                                                          */
  /*    FT_Long                                                            */
  /*    FT_ULong                                                           */
  /*    FT_Bool                                                            */
  /*    FT_Offset                                                          */
  /*    FT_PtrDist                                                         */
  /*    FT_String                                                          */
  /*    FT_Tag                                                             */
  /*    FT_Error                                                           */
  /*    FT_Fixed                                                           */
  /*    FT_Pointer                                                         */
  /*    FT_Pos                                                             */
  /*    FT_Vector                                                          */
  /*    FT_BBox                                                            */
  /*    FT_Matrix                                                          */
  /*    FT_FWord                                                           */
  /*    FT_UFWord                                                          */
  /*    FT_F2Dot14                                                         */
  /*    FT_UnitVector                                                      */
  /*    FT_F26Dot6                                                         */
  /*                                                                       */
  /*                                                                       */
  /*    FT_Generic                                                         */
  /*    FT_Generic_Finalizer                                               */
  /*                                                                       */
  /*    FT_Bitmap                                                          */
  /*    FT_Pixel_Mode                                                      */
  /*    FT_Palette_Mode                                                    */
  /*    FT_Glyph_Format                                                    */
  /*    FT_IMAGE_TAG                                                       */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Bool                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef of unsigned char, used for simple booleans.  As usual,   */
  /*    values 1 and~0 represent true and false, respectively.             */
  /*                                                                       */
  typedef unsigned char  ft_bool;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_FWord                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 16-bit integer used to store a distance in original font  */
  /*    units.                                                             */
  /*                                                                       */
  typedef signed short  ft_fword;   /* distance in FUnits */


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UFWord                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An unsigned 16-bit integer used to store a distance in original    */
  /*    font units.                                                        */
  /*                                                                       */
  typedef unsigned short  ft_ufword;  /* unsigned distance */


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Char                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _signed_ char type.                       */
  /*                                                                       */
  typedef signed char  ft_char;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Byte                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the _unsigned_ char type.                     */
  /*                                                                       */
  typedef unsigned char  ft_byte;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Bytes                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for constant memory areas.                               */
  /*                                                                       */
  typedef const ft_byte*  ft_bytes;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Tag                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for 32-bit tags (as used in the SFNT format).            */
  /*                                                                       */
  typedef ft_uint32  ft_tag;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_String                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for the char type, usually used for strings.      */
  /*                                                                       */
  typedef char  ft_string;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Short                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed short.                                        */
  /*                                                                       */
  typedef signed short  ft_short;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UShort                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned short.                                      */
  /*                                                                       */
  typedef unsigned short  ft_ushort;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Int                                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the int type.                                        */
  /*                                                                       */
  typedef signed int  ft_int;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_UInt                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for the unsigned int type.                               */
  /*                                                                       */
  typedef unsigned int  ft_uint;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Long                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for signed long.                                         */
  /*                                                                       */
  typedef signed long  ft_long;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_ULong                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A typedef for unsigned long.                                       */
  /*                                                                       */
  typedef unsigned long  ft_ulong;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_F2Dot14                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 2.14 fixed float type used for unit vectors.              */
  /*                                                                       */
  typedef signed short  ft_f2dot14;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_F26Dot6                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A signed 26.6 fixed float type used for vectorial pixel            */
  /*    coordinates.                                                       */
  /*                                                                       */
  typedef signed long  ft_f26dot6;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Fixed                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This type is used to store 16.16 fixed float values, like scaling  */
  /*    values or matrix coefficients.                                     */
  /*                                                                       */
  typedef signed long  ft_fixed;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Error                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    The FreeType error code type.  A value of~0 is always interpreted  */
  /*    as a successful operation.                                         */
  /*                                                                       */
  typedef int  ft_error;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Pointer                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple typedef for a typeless pointer.                           */
  /*                                                                       */
  typedef void*  ft_pointer;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_Offset                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This is equivalent to the ANSI~C `size_t' type, i.e., the largest  */
  /*    _unsigned_ integer type used to express a file size or position,   */
  /*    or a memory block size.                                            */
  /*                                                                       */
  typedef size_t  ft_offset;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_PtrDist                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This is equivalent to the ANSI~C `ptrdiff_t' type, i.e., the       */
  /*    largest _signed_ integer type used to express the distance         */
  /*    between two pointers.                                              */
  /*                                                                       */
  typedef ft_ptrdiff_t  ft_ptr_dist;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_UnitVector                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2D vector unit vector.  Uses    */
  /*    FT_F2Dot14 types.                                                  */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    x :: Horizontal coordinate.                                        */
  /*                                                                       */
  /*    y :: Vertical coordinate.                                          */
  /*                                                                       */
  typedef struct  ft_unit_vector_
  {
    ft_f2dot14  x;
    ft_f2dot14  y;

  } ft_unit_vector;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Matrix                                                          */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to store a 2x2 matrix.  Coefficients are   */
  /*    in 16.16 fixed float format.  The computation performed is:        */
  /*                                                                       */
  /*       {                                                               */
  /*          x' = x*xx + y*xy                                             */
  /*          y' = x*yx + y*yy                                             */
  /*       }                                                               */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    xx :: Matrix coefficient.                                          */
  /*                                                                       */
  /*    xy :: Matrix coefficient.                                          */
  /*                                                                       */
  /*    yx :: Matrix coefficient.                                          */
  /*                                                                       */
  /*    yy :: Matrix coefficient.                                          */
  /*                                                                       */
  typedef struct  ft_matrix_
  {
    ft_fixed  xx, xy;
    ft_fixed  yx, yy;

  } ft_matrix;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Data                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Read-only binary data represented as a pointer and a length.       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    pointer :: The data.                                               */
  /*                                                                       */
  /*    length  :: The length of the data in bytes.                        */
  /*                                                                       */
  typedef struct  ft_data_
  {
    const ft_byte*  pointer;
    ft_int          length;

  } ft_data;


  /*************************************************************************/
  /*                                                                       */
  /* <FuncType>                                                            */
  /*    FT_Generic_Finalizer                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Describe a function used to destroy the `client' data of any       */
  /*    FreeType object.  See the description of the @FT_Generic type for  */
  /*    details of usage.                                                  */
  /*                                                                       */
  /* <Input>                                                               */
  /*    The address of the FreeType object which is under finalization.    */
  /*    Its client data is accessed through its `generic' field.           */
  /*                                                                       */
  typedef void  (*ft_generic_finalizer)(void*  object);


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_Generic                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Client applications often need to associate their own data to a    */
  /*    variety of FreeType core objects.  For example, a text layout API  */
  /*    might want to associate a glyph cache to a given size object.      */
  /*                                                                       */
  /*    Most FreeType object contains a `generic' field, of type           */
  /*    FT_Generic, which usage is left to client applications and font    */
  /*    servers.                                                           */
  /*                                                                       */
  /*    It can be used to store a pointer to client-specific data, as well */
  /*    as the address of a `finalizer' function, which will be called by  */
  /*    FreeType when the object is destroyed (for example, the previous   */
  /*    client example would put the address of the glyph cache destructor */
  /*    in the `finalizer' field).                                         */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    data      :: A typeless pointer to any client-specified data. This */
  /*                 field is completely ignored by the FreeType library.  */
  /*                                                                       */
  /*    finalizer :: A pointer to a `generic finalizer' function, which    */
  /*                 will be called when the object is destroyed.  If this */
  /*                 field is set to NULL, no code will be called.         */
  /*                                                                       */
  typedef struct  ft_generic_
  {
    void*                 data;
    ft_generic_finalizer  finalizer;

  } ft_generic;


  /*************************************************************************/
  /*                                                                       */
  /* <Macro>                                                               */
  /*    FT_MAKE_TAG                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This macro converts four-letter tags which are used to label       */
  /*    TrueType tables into an unsigned long to be used within FreeType.  */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The produced values *must* be 32-bit integers.  Don't redefine     */
  /*    this macro.                                                        */
  /*                                                                       */
#define FT_MAKE_TAG( _x1, _x2, _x3, _x4 ) \
          (ft_tag)                        \
          ( ( (ft_ulong)_x1 << 24 ) |     \
            ( (ft_ulong)_x2 << 16 ) |     \
            ( (ft_ulong)_x3 <<  8 ) |     \
              (ft_ulong)_x4         )


  /*************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                    L I S T   M A N A G E M E N T                      */
  /*                                                                       */
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    list_processing                                                    */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_ListNode                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*     Many elements and objects in FreeType are listed through an       */
  /*     @FT_List record (see @FT_ListRec).  As its name suggests, an      */
  /*     FT_ListNode is a handle to a single list element.                 */
  /*                                                                       */
  typedef struct ft_list_node_rec_*  ft_list_node;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FT_List                                                            */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a list record (see @FT_ListRec).                       */
  /*                                                                       */
  typedef struct ft_list_rec_*  ft_list;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_ListNodeRec                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold a single list element.                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    prev :: The previous element in the list.  NULL if first.          */
  /*                                                                       */
  /*    next :: The next element in the list.  NULL if last.               */
  /*                                                                       */
  /*    data :: A typeless pointer to the listed object.                   */
  /*                                                                       */
  typedef struct  ft_list_node_rec_
  {
    ft_list_node  prev;
    ft_list_node  next;
    void*        data;

  } ft_list_node_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FT_ListRec                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to hold a simple doubly-linked list.  These are   */
  /*    used in many parts of FreeType.                                    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    head :: The head (first element) of doubly-linked list.            */
  /*                                                                       */
  /*    tail :: The tail (last element) of doubly-linked list.             */
  /*                                                                       */
  typedef struct  ft_list_rec_
  {
    ft_list_node  head;
    ft_list_node  tail;

  } ft_list_rec;


  /* */

#define FT_IS_EMPTY( list )  ( (list).head == 0 )

  /* return base error code (without module-specific prefix) */
#define FT_ERROR_BASE( x )    ( (x) & 0xFF )

  /* return module error code */
#define FT_ERROR_MODULE( x )  ( (x) & 0xFF00U )

#define FT_BOOL( x )  ( (ft_bool)( x ) )

FT_END_HEADER

#endif /* __FTTYPES_H__ */


/* END */
