/***************************************************************************/
/*                                                                         */
/*  ftsystem.h                                                             */
/*                                                                         */
/*    FreeType low-level system interface definition (specification).      */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2005, 2010 by                               */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTSYSTEM_H__
#define __FTSYSTEM_H__


#include <ft2build.h>


FT_BEGIN_HEADER


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*   system_interface                                                    */
  /*                                                                       */
  /* <Title>                                                               */
  /*   System Interface                                                    */
  /*                                                                       */
  /* <Abstract>                                                            */
  /*   How FreeType manages memory and i/o.                                */
  /*                                                                       */
  /* <Description>                                                         */
  /*   This section contains various definitions related to memory         */
  /*   management and i/o access.  You need to understand this             */
  /*   information if you want to use a custom memory manager or you own   */
  /*   i/o streams.                                                        */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /*                  M E M O R Y   M A N A G E M E N T                    */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************
   *
   * @type:
   *   FT_Memory
   *
   * @description:
   *   A handle to a given memory manager object, defined with an
   *   @FT_MemoryRec structure.
   *
   */
  typedef struct ft_memory_rec_*  ft_memory;


  /*************************************************************************
   *
   * @functype:
   *   FT_Alloc_Func
   *
   * @description:
   *   A function used to allocate `size' bytes from `memory'.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   size ::
   *     The size in bytes to allocate.
   *
   * @return:
   *   Address of new memory block.  0~in case of failure.
   *
   */
  typedef void*
  (*ft_alloc_func)( ft_memory  memory,
                    long       size );


  /*************************************************************************
   *
   * @functype:
   *   FT_Free_Func
   *
   * @description:
   *   A function used to release a given block of memory.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   block ::
   *     The address of the target memory block.
   *
   */
  typedef void
  (*ft_free_func)( ft_memory  memory,
                   void*      block );


  /*************************************************************************
   *
   * @functype:
   *   FT_Realloc_Func
   *
   * @description:
   *   A function used to re-allocate a given block of memory.
   *
   * @input:
   *   memory ::
   *     A handle to the source memory manager.
   *
   *   cur_size ::
   *     The block's current size in bytes.
   *
   *   new_size ::
   *     The block's requested new size.
   *
   *   block ::
   *     The block's current address.
   *
   * @return:
   *   New block address.  0~in case of memory shortage.
   *
   * @note:
   *   In case of error, the old block must still be available.
   *
   */
  typedef void*
  (*ft_realloc_func)( ft_memory  memory,
                      long       cur_size,
                      long       new_size,
                      void*      block );


  /*************************************************************************
   *
   * @struct:
   *   FT_MemoryRec
   *
   * @description:
   *   A structure used to describe a given memory manager to FreeType~2.
   *
   * @fields:
   *   user ::
   *     A generic typeless pointer for user data.
   *
   *   alloc ::
   *     A pointer type to an allocation function.
   *
   *   free ::
   *     A pointer type to an memory freeing function.
   *
   *   realloc ::
   *     A pointer type to a reallocation function.
   *
   */
  struct  ft_memory_rec_
  {
    void*            user;
    ft_alloc_func    alloc;
    ft_free_func     free;
    ft_realloc_func  realloc;
  };


  /*************************************************************************/
  /*                                                                       */
  /*                       I / O   M A N A G E M E N T                     */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************
   *
   * @type:
   *   FT_Stream
   *
   * @description:
   *   A handle to an input stream.
   *
   */
  typedef struct ft_stream_rec_*  ft_stream;


  /*************************************************************************
   *
   * @struct:
   *   FT_StreamDesc
   *
   * @description:
   *   A union type used to store either a long or a pointer.  This is used
   *   to store a file descriptor or a `FILE*' in an input stream.
   *
   */
  typedef union  ft_stream_desc_
  {
    long   value;
    void*  pointer;

  } ft_stream_desc;


  /*************************************************************************
   *
   * @functype:
   *   FT_Stream_IoFunc
   *
   * @description:
   *   A function used to seek and read data from a given input stream.
   *
   * @input:
   *   stream ::
   *     A handle to the source stream.
   *
   *   offset ::
   *     The offset of read in stream (always from start).
   *
   *   buffer ::
   *     The address of the read buffer.
   *
   *   count ::
   *     The number of bytes to read from the stream.
   *
   * @return:
   *   The number of bytes effectively read by the stream.
   *
   * @note:
   *   This function might be called to perform a seek or skip operation
   *   with a `count' of~0.  A non-zero return value then indicates an
   *   error.
   *
   */
  typedef unsigned long
  (*ft_stream_io_func)( ft_stream       stream,
                       unsigned long   offset,
                       unsigned char*  buffer,
                       unsigned long   count );


  /*************************************************************************
   *
   * @functype:
   *   FT_Stream_CloseFunc
   *
   * @description:
   *   A function used to close a given input stream.
   *
   * @input:
   *  stream ::
   *     A handle to the target stream.
   *
   */
  typedef void
  (*ft_stream_close_func)( ft_stream  stream );


  /*************************************************************************
   *
   * @struct:
   *   FT_StreamRec
   *
   * @description:
   *   A structure used to describe an input stream.
   *
   * @input:
   *   base ::
   *     For memory-based streams, this is the address of the first stream
   *     byte in memory.  This field should always be set to NULL for
   *     disk-based streams.
   *
   *   size ::
   *     The stream size in bytes.
   *
   *   pos ::
   *     The current position within the stream.
   *
   *   descriptor ::
   *     This field is a union that can hold an integer or a pointer.  It is
   *     used by stream implementations to store file descriptors or `FILE*'
   *     pointers.
   *
   *   pathname ::
   *     This field is completely ignored by FreeType.  However, it is often
   *     useful during debugging to use it to store the stream's filename
   *     (where available).
   *
   *   read ::
   *     The stream's input function.
   *
   *   close ::
   *     The stream's close function.
   *
   *   memory ::
   *     The memory manager to use to preload frames.  This is set
   *     internally by FreeType and shouldn't be touched by stream
   *     implementations.
   *
   *   cursor ::
   *     This field is set and used internally by FreeType when parsing
   *     frames.
   *
   *   limit ::
   *     This field is set and used internally by FreeType when parsing
   *     frames.
   *
   */
  typedef struct  ft_stream_rec_
  {
    unsigned char*       base;
    unsigned long        size;
    unsigned long        pos;

    ft_stream_desc        descriptor;
    ft_stream_desc        pathname;
    ft_stream_io_func     read;
    ft_stream_close_func  close;

    ft_memory            memory;
    unsigned char*       cursor;
    unsigned char*       limit;

  } ft_stream_rec;


  /* */


FT_END_HEADER

#endif /* __FTSYSTEM_H__ */


/* END */
