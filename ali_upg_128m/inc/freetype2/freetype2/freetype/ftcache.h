/***************************************************************************/
/*                                                                         */
/*  ftcache.h                                                              */
/*                                                                         */
/*    FreeType Cache subsystem (specification).                            */
/*                                                                         */
/*  Copyright 1996-2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010 by */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTCACHE_H__
#define __FTCACHE_H__


#include <ft2build.h>
#include FT_GLYPH_H


FT_BEGIN_HEADER


  /*************************************************************************
   *
   * <Section>
   *    cache_subsystem
   *
   * <Title>
   *    Cache Sub-System
   *
   * <Abstract>
   *    How to cache face, size, and glyph data with FreeType~2.
   *
   * <Description>
   *   This section describes the FreeType~2 cache sub-system, which is used
   *   to limit the number of concurrently opened @FT_Face and @FT_Size
   *   objects, as well as caching information like character maps and glyph
   *   images while limiting their maximum memory usage.
   *
   *   Note that all types and functions begin with the `FTC_' prefix.
   *
   *   The cache is highly portable and thus doesn't know anything about the
   *   fonts installed on your system, or how to access them.  This implies
   *   the following scheme:
   *
   *   First, available or installed font faces are uniquely identified by
   *   @FTC_FaceID values, provided to the cache by the client.  Note that
   *   the cache only stores and compares these values, and doesn't try to
   *   interpret them in any way.
   *
   *   Second, the cache calls, only when needed, a client-provided function
   *   to convert a @FTC_FaceID into a new @FT_Face object.  The latter is
   *   then completely managed by the cache, including its termination
   *   through @FT_Done_Face.
   *
   *   Clients are free to map face IDs to anything else.  The most simple
   *   usage is to associate them to a (pathname,face_index) pair that is
   *   used to call @FT_New_Face.  However, more complex schemes are also
   *   possible.
   *
   *   Note that for the cache to work correctly, the face ID values must be
   *   *persistent*, which means that the contents they point to should not
   *   change at runtime, or that their value should not become invalid.
   *
   *   If this is unavoidable (e.g., when a font is uninstalled at runtime),
   *   you should call @FTC_Manager_RemoveFaceID as soon as possible, to let
   *   the cache get rid of any references to the old @FTC_FaceID it may
   *   keep internally.  Failure to do so will lead to incorrect behaviour
   *   or even crashes.
   *
   *   To use the cache, start with calling @FTC_Manager_New to create a new
   *   @FTC_Manager object, which models a single cache instance.  You can
   *   then look up @FT_Face and @FT_Size objects with
   *   @FTC_Manager_LookupFace and @FTC_Manager_LookupSize, respectively.
   *
   *   If you want to use the charmap caching, call @FTC_CMapCache_New, then
   *   later use @FTC_CMapCache_Lookup to perform the equivalent of
   *   @FT_Get_Char_Index, only much faster.
   *
   *   If you want to use the @FT_Glyph caching, call @FTC_ImageCache, then
   *   later use @FTC_ImageCache_Lookup to retrieve the corresponding
   *   @FT_Glyph objects from the cache.
   *
   *   If you need lots of small bitmaps, it is much more memory efficient
   *   to call @FTC_SBitCache_New followed by @FTC_SBitCache_Lookup.  This
   *   returns @FTC_SBitRec structures, which are used to store small
   *   bitmaps directly.  (A small bitmap is one whose metrics and
   *   dimensions all fit into 8-bit integers).
   *
   *   We hope to also provide a kerning cache in the near future.
   *
   *
   * <Order>
   *   FTC_Manager
   *   FTC_FaceID
   *   FTC_Face_Requester
   *
   *   FTC_Manager_New
   *   FTC_Manager_Reset
   *   FTC_Manager_Done
   *   FTC_Manager_LookupFace
   *   FTC_Manager_LookupSize
   *   FTC_Manager_RemoveFaceID
   *
   *   FTC_Node
   *   FTC_Node_Unref
   *
   *   FTC_ImageCache
   *   FTC_ImageCache_New
   *   FTC_ImageCache_Lookup
   *
   *   FTC_SBit
   *   FTC_SBitCache
   *   FTC_SBitCache_New
   *   FTC_SBitCache_Lookup
   *
   *   FTC_CMapCache
   *   FTC_CMapCache_New
   *   FTC_CMapCache_Lookup
   *
   *************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                    BASIC TYPE DEFINITIONS                     *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************
   *
   * @type: FTC_FaceID
   *
   * @description:
   *   An opaque pointer type that is used to identity face objects.  The
   *   contents of such objects is application-dependent.
   *
   *   These pointers are typically used to point to a user-defined
   *   structure containing a font file path, and face index.
   *
   * @note:
   *   Never use NULL as a valid @FTC_FaceID.
   *
   *   Face IDs are passed by the client to the cache manager, which calls,
   *   when needed, the @FTC_Face_Requester to translate them into new
   *   @FT_Face objects.
   *
   *   If the content of a given face ID changes at runtime, or if the value
   *   becomes invalid (e.g., when uninstalling a font), you should
   *   immediately call @FTC_Manager_RemoveFaceID before any other cache
   *   function.
   *
   *   Failure to do so will result in incorrect behaviour or even
   *   memory leaks and crashes.
   */
  typedef ft_pointer  ftc_face_id;


  /************************************************************************
   *
   * @functype:
   *   FTC_Face_Requester
   *
   * @description:
   *   A callback function provided by client applications.  It is used by
   *   the cache manager to translate a given @FTC_FaceID into a new valid
   *   @FT_Face object, on demand.
   *
   * <Input>
   *   face_id ::
   *     The face ID to resolve.
   *
   *   library ::
   *     A handle to a FreeType library object.
   *
   *   req_data ::
   *     Application-provided request data (see note below).
   *
   * <Output>
   *   aface ::
   *     A new @FT_Face handle.
   *
   * <Return>
   *   FreeType error code.  0~means success.
   *
   * <Note>
   *   The third parameter `req_data' is the same as the one passed by the
   *   client when @FTC_Manager_New is called.
   *
   *   The face requester should not perform funny things on the returned
   *   face object, like creating a new @FT_Size for it, or setting a
   *   transformation through @FT_Set_Transform!
   */
  typedef ft_error
  (*ftc_face_requester)( ftc_face_id  face_id,
                         ft_library  library,
                         ft_pointer  request_data,
                         ft_face*    aface );

 /* */

#define FT_POINTER_TO_ULONG( p )  ( (ft_ulong)(ft_pointer)(p) )

#define FTC_FACE_ID_HASH( i )                                \
          ((ft_uint32)(( FT_POINTER_TO_ULONG( i ) >> 3 ) ^   \
                       ( FT_POINTER_TO_ULONG( i ) << 7 ) ) )


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                      CACHE MANAGER OBJECT                     *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTC_Manager                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    This object corresponds to one instance of the cache-subsystem.    */
  /*    It is used to cache one or more @FT_Face objects, along with       */
  /*    corresponding @FT_Size objects.                                    */
  /*                                                                       */
  /*    The manager intentionally limits the total number of opened        */
  /*    @FT_Face and @FT_Size objects to control memory usage.  See the    */
  /*    `max_faces' and `max_sizes' parameters of @FTC_Manager_New.        */
  /*                                                                       */
  /*    The manager is also used to cache `nodes' of various types while   */
  /*    limiting their total memory usage.                                 */
  /*                                                                       */
  /*    All limitations are enforced by keeping lists of managed objects   */
  /*    in most-recently-used order, and flushing old nodes to make room   */
  /*    for new ones.                                                      */
  /*                                                                       */
  typedef struct ftc_manager_rec_*  ftc_manager;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTC_Node                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    An opaque handle to a cache node object.  Each cache node is       */
  /*    reference-counted.  A node with a count of~0 might be flushed      */
  /*    out of a full cache whenever a lookup request is performed.        */
  /*                                                                       */
  /*    If you look up nodes, you have the ability to `acquire' them,      */
  /*    i.e., to increment their reference count.  This will prevent the   */
  /*    node from being flushed out of the cache until you explicitly      */
  /*    `release' it (see @FTC_Node_Unref).                                */
  /*                                                                       */
  /*    See also @FTC_SBitCache_Lookup and @FTC_ImageCache_Lookup.         */
  /*                                                                       */
  typedef struct ftc_node_rec_*  ftc_node;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Manager_New                                                    */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a new cache manager.                                        */
  /*                                                                       */
  /* <Input>                                                               */
  /*    library   :: The parent FreeType library handle to use.            */
  /*                                                                       */
  /*    max_faces :: Maximum number of opened @FT_Face objects managed by  */
  /*                 this cache instance.  Use~0 for defaults.             */
  /*                                                                       */
  /*    max_sizes :: Maximum number of opened @FT_Size objects managed by  */
  /*                 this cache instance.  Use~0 for defaults.             */
  /*                                                                       */
  /*    max_bytes :: Maximum number of bytes to use for cached data nodes. */
  /*                 Use~0 for defaults.  Note that this value does not    */
  /*                 account for managed @FT_Face and @FT_Size objects.    */
  /*                                                                       */
  /*    requester :: An application-provided callback used to translate    */
  /*                 face IDs into real @FT_Face objects.                  */
  /*                                                                       */
  /*    req_data  :: A generic pointer that is passed to the requester     */
  /*                 each time it is called (see @FTC_Face_Requester).     */
  /*                                                                       */
  /* <Output>                                                              */
  /*    amanager  :: A handle to a new manager object.  0~in case of       */
  /*                 failure.                                              */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_manager_new( ft_library          library,
                   ft_uint             max_faces,
                   ft_uint             max_sizes,
                   ft_ulong            max_bytes,
                   ftc_face_requester  requester,
                   ft_pointer          req_data,
                   ftc_manager        *amanager );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Manager_Reset                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Empty a given cache manager.  This simply gets rid of all the      */
  /*    currently cached @FT_Face and @FT_Size objects within the manager. */
  /*                                                                       */
  /* <InOut>                                                               */
  /*    manager :: A handle to the manager.                                */
  /*                                                                       */
  FT_EXPORT( void )
  ftc_manager_reset( ftc_manager  manager );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Manager_Done                                                   */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Destroy a given manager after emptying it.                         */
  /*                                                                       */
  /* <Input>                                                               */
  /*    manager :: A handle to the target cache manager object.            */
  /*                                                                       */
  FT_EXPORT( void )
  ftc_manager_done( ftc_manager  manager );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Manager_LookupFace                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the @FT_Face object that corresponds to a given face ID   */
  /*    through a cache manager.                                           */
  /*                                                                       */
  /* <Input>                                                               */
  /*    manager :: A handle to the cache manager.                          */
  /*                                                                       */
  /*    face_id :: The ID of the face object.                              */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aface   :: A handle to the face object.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The returned @FT_Face object is always owned by the manager.  You  */
  /*    should never try to discard it yourself.                           */
  /*                                                                       */
  /*    The @FT_Face object doesn't necessarily have a current size object */
  /*    (i.e., face->size can be 0).  If you need a specific `font size',  */
  /*    use @FTC_Manager_LookupSize instead.                               */
  /*                                                                       */
  /*    Never change the face's transformation matrix (i.e., never call    */
  /*    the @FT_Set_Transform function) on a returned face!  If you need   */
  /*    to transform glyphs, do it yourself after glyph loading.           */
  /*                                                                       */
  /*    When you perform a lookup, out-of-memory errors are detected       */
  /*    _within_ the lookup and force incremental flushes of the cache     */
  /*    until enough memory is released for the lookup to succeed.         */
  /*                                                                       */
  /*    If a lookup fails with `FT_Err_Out_Of_Memory' the cache has        */
  /*    already been completely flushed, and still no memory was available */
  /*    for the operation.                                                 */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_manager_lookup_face( ftc_manager  manager,
                          ftc_face_id   face_id,
                          ft_face     *aface );


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FTC_ScalerRec                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A structure used to describe a given character size in either      */
  /*    pixels or points to the cache manager.  See                        */
  /*    @FTC_Manager_LookupSize.                                           */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face_id :: The source face ID.                                     */
  /*                                                                       */
  /*    width   :: The character width.                                    */
  /*                                                                       */
  /*    height  :: The character height.                                   */
  /*                                                                       */
  /*    pixel   :: A Boolean.  If 1, the `width' and `height' fields are   */
  /*               interpreted as integer pixel character sizes.           */
  /*               Otherwise, they are expressed as 1/64th of points.      */
  /*                                                                       */
  /*    x_res   :: Only used when `pixel' is value~0 to indicate the       */
  /*               horizontal resolution in dpi.                           */
  /*                                                                       */
  /*    y_res   :: Only used when `pixel' is value~0 to indicate the       */
  /*               vertical resolution in dpi.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    This type is mainly used to retrieve @FT_Size objects through the  */
  /*    cache manager.                                                     */
  /*                                                                       */
  typedef struct  ftc_scaler_rec_
  {
    ftc_face_id  face_id;
    ft_uint     width;
    ft_uint     height;
    ft_int      pixel;
    ft_uint     x_res;
    ft_uint     y_res;

  } ftc_scaler_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FTC_Scaler                                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an @FTC_ScalerRec structure.                           */
  /*                                                                       */
  typedef struct ftc_scaler_rec_*  ftc_scaler;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Manager_LookupSize                                             */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve the @FT_Size object that corresponds to a given           */
  /*    @FTC_ScalerRec pointer through a cache manager.                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    manager :: A handle to the cache manager.                          */
  /*                                                                       */
  /*    scaler  :: A scaler handle.                                        */
  /*                                                                       */
  /* <Output>                                                              */
  /*    asize   :: A handle to the size object.                            */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The returned @FT_Size object is always owned by the manager.  You  */
  /*    should never try to discard it by yourself.                        */
  /*                                                                       */
  /*    You can access the parent @FT_Face object simply as `size->face'   */
  /*    if you need it.  Note that this object is also owned by the        */
  /*    manager.                                                           */
  /*                                                                       */
  /* <Note>                                                                */
  /*    When you perform a lookup, out-of-memory errors are detected       */
  /*    _within_ the lookup and force incremental flushes of the cache     */
  /*    until enough memory is released for the lookup to succeed.         */
  /*                                                                       */
  /*    If a lookup fails with `FT_Err_Out_Of_Memory' the cache has        */
  /*    already been completely flushed, and still no memory is available  */
  /*    for the operation.                                                 */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_manager_lookup_size( ftc_manager  manager,
                          ftc_scaler   scaler,
                          ft_size     *asize );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_Node_Unref                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Decrement a cache node's internal reference count.  When the count */
  /*    reaches 0, it is not destroyed but becomes eligible for subsequent */
  /*    cache flushes.                                                     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    node    :: The cache node handle.                                  */
  /*                                                                       */
  /*    manager :: The cache manager handle.                               */
  /*                                                                       */
  FT_EXPORT( void )
  ftc_node_unref( ftc_node     node,
                  ftc_manager  manager );


  /*************************************************************************
   *
   * @function:
   *   FTC_Manager_RemoveFaceID
   *
   * @description:
   *   A special function used to indicate to the cache manager that
   *   a given @FTC_FaceID is no longer valid, either because its
   *   content changed, or because it was deallocated or uninstalled.
   *
   * @input:
   *   manager ::
   *     The cache manager handle.
   *
   *   face_id ::
   *     The @FTC_FaceID to be removed.
   *
   * @note:
   *   This function flushes all nodes from the cache corresponding to this
   *   `face_id', with the exception of nodes with a non-null reference
   *   count.
   *
   *   Such nodes are however modified internally so as to never appear
   *   in later lookups with the same `face_id' value, and to be immediately
   *   destroyed when released by all their users.
   *
   */
  FT_EXPORT( void )
  ftc_manager_remove_face_id( ftc_manager  manager,
                            ftc_face_id   face_id );


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    cache_subsystem                                                    */
  /*                                                                       */
  /*************************************************************************/

  /*************************************************************************
   *
   * @type:
   *   FTC_CMapCache
   *
   * @description:
   *   An opaque handle used to model a charmap cache.  This cache is to
   *   hold character codes -> glyph indices mappings.
   *
   */
  typedef struct ftc_cmap_cache_rec_*  ftc_cmap_cache;


  /*************************************************************************
   *
   * @function:
   *   FTC_CMapCache_New
   *
   * @description:
   *   Create a new charmap cache.
   *
   * @input:
   *   manager ::
   *     A handle to the cache manager.
   *
   * @output:
   *   acache ::
   *     A new cache handle.  NULL in case of error.
   *
   * @return:
   *   FreeType error code.  0~means success.
   *
   * @note:
   *   Like all other caches, this one will be destroyed with the cache
   *   manager.
   *
   */
  FT_EXPORT( ft_error )
  ftc_cmap_cache_new( ftc_manager     manager,
                     ftc_cmap_cache  *acache );


  /************************************************************************
   *
   * @function:
   *   FTC_CMapCache_Lookup
   *
   * @description:
   *   Translate a character code into a glyph index, using the charmap
   *   cache.
   *
   * @input:
   *   cache ::
   *     A charmap cache handle.
   *
   *   face_id ::
   *     The source face ID.
   *
   *   cmap_index ::
   *     The index of the charmap in the source face.  Any negative value
   *     means to use the cache @FT_Face's default charmap.
   *
   *   char_code ::
   *     The character code (in the corresponding charmap).
   *
   * @return:
   *    Glyph index.  0~means `no glyph'.
   *
   */
  FT_EXPORT( ft_uint )
  ftc_cmap_cache_lookup( ftc_cmap_cache  cache,
                        ftc_face_id     face_id,
                        ft_int         cmap_index,
                        ft_uint32      char_code );


  /*************************************************************************/
  /*                                                                       */
  /* <Section>                                                             */
  /*    cache_subsystem                                                    */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/
  /*****                                                               *****/
  /*****                       IMAGE CACHE OBJECT                      *****/
  /*****                                                               *****/
  /*************************************************************************/
  /*************************************************************************/
  /*************************************************************************/


  /*************************************************************************
   *
   * @struct:
   *   FTC_ImageTypeRec
   *
   * @description:
   *   A structure used to model the type of images in a glyph cache.
   *
   * @fields:
   *   face_id ::
   *     The face ID.
   *
   *   width ::
   *     The width in pixels.
   *
   *   height ::
   *     The height in pixels.
   *
   *   flags ::
   *     The load flags, as in @FT_Load_Glyph.
   *
   */
  typedef struct  ftc_image_type_rec_
  {
    ftc_face_id  face_id;
    ft_int      width;
    ft_int      height;
    ft_int32    flags;

  } ftc_image_type_rec;


  /*************************************************************************
   *
   * @type:
   *   FTC_ImageType
   *
   * @description:
   *   A handle to an @FTC_ImageTypeRec structure.
   *
   */
  typedef struct ftc_image_type_rec_*  ftc_image_type;


  /* */


#define FTC_IMAGE_TYPE_COMPARE( d1, d2 )      \
          ( (d1)->face_id == (d2)->face_id && \
            (d1)->width   == (d2)->width   && \
            (d1)->flags   == (d2)->flags   )

#define FTC_IMAGE_TYPE_HASH( d )                          \
          (ft_ufast)( FTC_FACE_ID_HASH( (d)->face_id )  ^ \
                      ( (d)->width << 8 ) ^ (d)->height ^ \
                      ( (d)->flags << 4 )               )


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTC_ImageCache                                                     */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to an glyph image cache object.  They are designed to     */
  /*    hold many distinct glyph images while not exceeding a certain      */
  /*    memory threshold.                                                  */
  /*                                                                       */
  typedef struct ftc_image_cache_rec_*  ftc_image_cache;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_ImageCache_New                                                 */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a new glyph image cache.                                    */
  /*                                                                       */
  /* <Input>                                                               */
  /*    manager :: The parent manager for the image cache.                 */
  /*                                                                       */
  /* <Output>                                                              */
  /*    acache  :: A handle to the new glyph image cache object.           */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_image_cache_new( ftc_manager      manager,
                      ftc_image_cache  *acache );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_ImageCache_Lookup                                              */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Retrieve a given glyph image from a glyph image cache.             */
  /*                                                                       */
  /* <Input>                                                               */
  /*    cache  :: A handle to the source glyph image cache.                */
  /*                                                                       */
  /*    type   :: A pointer to a glyph image type descriptor.              */
  /*                                                                       */
  /*    gindex :: The glyph index to retrieve.                             */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aglyph :: The corresponding @FT_Glyph object.  0~in case of        */
  /*              failure.                                                 */
  /*                                                                       */
  /*    anode  :: Used to return the address of of the corresponding cache */
  /*              node after incrementing its reference count (see note    */
  /*              below).                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The returned glyph is owned and managed by the glyph image cache.  */
  /*    Never try to transform or discard it manually!  You can however    */
  /*    create a copy with @FT_Glyph_Copy and modify the new one.          */
  /*                                                                       */
  /*    If `anode' is _not_ NULL, it receives the address of the cache     */
  /*    node containing the glyph image, after increasing its reference    */
  /*    count.  This ensures that the node (as well as the @FT_Glyph) will */
  /*    always be kept in the cache until you call @FTC_Node_Unref to      */
  /*    `release' it.                                                      */
  /*                                                                       */
  /*    If `anode' is NULL, the cache node is left unchanged, which means  */
  /*    that the @FT_Glyph could be flushed out of the cache on the next   */
  /*    call to one of the caching sub-system APIs.  Don't assume that it  */
  /*    is persistent!                                                     */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_image_cache_lookup( ftc_image_cache  cache,
                         ftc_image_type   type,
                         ft_uint         gindex,
                         ft_glyph       *aglyph,
                         ftc_node       *anode );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_ImageCache_LookupScaler                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A variant of @FTC_ImageCache_Lookup that uses an @FTC_ScalerRec    */
  /*    to specify the face ID and its size.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    cache      :: A handle to the source glyph image cache.            */
  /*                                                                       */
  /*    scaler     :: A pointer to a scaler descriptor.                    */
  /*                                                                       */
  /*    load_flags :: The corresponding load flags.                        */
  /*                                                                       */
  /*    gindex     :: The glyph index to retrieve.                         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    aglyph     :: The corresponding @FT_Glyph object.  0~in case of    */
  /*                  failure.                                             */
  /*                                                                       */
  /*    anode      :: Used to return the address of of the corresponding   */
  /*                  cache node after incrementing its reference count    */
  /*                  (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The returned glyph is owned and managed by the glyph image cache.  */
  /*    Never try to transform or discard it manually!  You can however    */
  /*    create a copy with @FT_Glyph_Copy and modify the new one.          */
  /*                                                                       */
  /*    If `anode' is _not_ NULL, it receives the address of the cache     */
  /*    node containing the glyph image, after increasing its reference    */
  /*    count.  This ensures that the node (as well as the @FT_Glyph) will */
  /*    always be kept in the cache until you call @FTC_Node_Unref to      */
  /*    `release' it.                                                      */
  /*                                                                       */
  /*    If `anode' is NULL, the cache node is left unchanged, which means  */
  /*    that the @FT_Glyph could be flushed out of the cache on the next   */
  /*    call to one of the caching sub-system APIs.  Don't assume that it  */
  /*    is persistent!                                                     */
  /*                                                                       */
  /*    Calls to @FT_Set_Char_Size and friends have no effect on cached    */
  /*    glyphs; you should always use the FreeType cache API instead.      */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_image_cache_lookup_scaler( ftc_image_cache  cache,
                               ftc_scaler      scaler,
                               ft_ulong        load_flags,
                               ft_uint         gindex,
                               ft_glyph       *aglyph,
                               ftc_node       *anode );


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTC_SBit                                                           */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a small bitmap descriptor.  See the @FTC_SBitRec       */
  /*    structure for details.                                             */
  /*                                                                       */
  typedef struct ftc_sbit_rec_*  ftc_sbit;


  /*************************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FTC_SBitRec                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A very compact structure used to describe a small glyph bitmap.    */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    width     :: The bitmap width in pixels.                           */
  /*                                                                       */
  /*    height    :: The bitmap height in pixels.                          */
  /*                                                                       */
  /*    left      :: The horizontal distance from the pen position to the  */
  /*                 left bitmap border (a.k.a. `left side bearing', or    */
  /*                 `lsb').                                               */
  /*                                                                       */
  /*    top       :: The vertical distance from the pen position (on the   */
  /*                 baseline) to the upper bitmap border (a.k.a. `top     */
  /*                 side bearing').  The distance is positive for upwards */
  /*                 y~coordinates.                                        */
  /*                                                                       */
  /*    format    :: The format of the glyph bitmap (monochrome or gray).  */
  /*                                                                       */
  /*    max_grays :: Maximum gray level value (in the range 1 to~255).     */
  /*                                                                       */
  /*    pitch     :: The number of bytes per bitmap line.  May be positive */
  /*                 or negative.                                          */
  /*                                                                       */
  /*    xadvance  :: The horizontal advance width in pixels.               */
  /*                                                                       */
  /*    yadvance  :: The vertical advance height in pixels.                */
  /*                                                                       */
  /*    buffer    :: A pointer to the bitmap pixels.                       */
  /*                                                                       */
  typedef struct  ftc_sbit_rec_
  {
    ft_byte   width;
    ft_byte   height;
    ft_char   left;
    ft_char   top;

    ft_byte   format;
    ft_byte   max_grays;
    ft_short  pitch;
    ft_char   xadvance;
    ft_char   yadvance;

    ft_byte*  buffer;

  } ftc_sbit_rec;


  /*************************************************************************/
  /*                                                                       */
  /* <Type>                                                                */
  /*    FTC_SBitCache                                                      */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A handle to a small bitmap cache.  These are special cache objects */
  /*    used to store small glyph bitmaps (and anti-aliased pixmaps) in a  */
  /*    much more efficient way than the traditional glyph image cache     */
  /*    implemented by @FTC_ImageCache.                                    */
  /*                                                                       */
  typedef struct ftc_sbit_cache_rec_*  ftc_sbit_cache;


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_SBitCache_New                                                  */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Create a new cache to store small glyph bitmaps.                   */
  /*                                                                       */
  /* <Input>                                                               */
  /*    manager :: A handle to the source cache manager.                   */
  /*                                                                       */
  /* <Output>                                                              */
  /*    acache  :: A handle to the new sbit cache.  NULL in case of error. */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_sbit_cache_new( ftc_manager     manager,
                     ftc_sbit_cache  *acache );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_SBitCache_Lookup                                               */
  /*                                                                       */
  /* <Description>                                                         */
  /*    Look up a given small glyph bitmap in a given sbit cache and       */
  /*    `lock' it to prevent its flushing from the cache until needed.     */
  /*                                                                       */
  /* <Input>                                                               */
  /*    cache  :: A handle to the source sbit cache.                       */
  /*                                                                       */
  /*    type   :: A pointer to the glyph image type descriptor.            */
  /*                                                                       */
  /*    gindex :: The glyph index.                                         */
  /*                                                                       */
  /* <Output>                                                              */
  /*    sbit   :: A handle to a small bitmap descriptor.                   */
  /*                                                                       */
  /*    anode  :: Used to return the address of of the corresponding cache */
  /*              node after incrementing its reference count (see note    */
  /*              below).                                                  */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The small bitmap descriptor and its bit buffer are owned by the    */
  /*    cache and should never be freed by the application.  They might    */
  /*    as well disappear from memory on the next cache lookup, so don't   */
  /*    treat them as persistent data.                                     */
  /*                                                                       */
  /*    The descriptor's `buffer' field is set to~0 to indicate a missing  */
  /*    glyph bitmap.                                                      */
  /*                                                                       */
  /*    If `anode' is _not_ NULL, it receives the address of the cache     */
  /*    node containing the bitmap, after increasing its reference count.  */
  /*    This ensures that the node (as well as the image) will always be   */
  /*    kept in the cache until you call @FTC_Node_Unref to `release' it.  */
  /*                                                                       */
  /*    If `anode' is NULL, the cache node is left unchanged, which means  */
  /*    that the bitmap could be flushed out of the cache on the next      */
  /*    call to one of the caching sub-system APIs.  Don't assume that it  */
  /*    is persistent!                                                     */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_sbit_cache_lookup( ftc_sbit_cache    cache,
                        ftc_image_type    type,
                        ft_uint          gindex,
                        ftc_sbit        *sbit,
                        ftc_node        *anode );


  /*************************************************************************/
  /*                                                                       */
  /* <Function>                                                            */
  /*    FTC_SBitCache_LookupScaler                                         */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A variant of @FTC_SBitCache_Lookup that uses an @FTC_ScalerRec     */
  /*    to specify the face ID and its size.                               */
  /*                                                                       */
  /* <Input>                                                               */
  /*    cache      :: A handle to the source sbit cache.                   */
  /*                                                                       */
  /*    scaler     :: A pointer to the scaler descriptor.                  */
  /*                                                                       */
  /*    load_flags :: The corresponding load flags.                        */
  /*                                                                       */
  /*    gindex     :: The glyph index.                                     */
  /*                                                                       */
  /* <Output>                                                              */
  /*    sbit       :: A handle to a small bitmap descriptor.               */
  /*                                                                       */
  /*    anode      :: Used to return the address of of the corresponding   */
  /*                  cache node after incrementing its reference count    */
  /*                  (see note below).                                    */
  /*                                                                       */
  /* <Return>                                                              */
  /*    FreeType error code.  0~means success.                             */
  /*                                                                       */
  /* <Note>                                                                */
  /*    The small bitmap descriptor and its bit buffer are owned by the    */
  /*    cache and should never be freed by the application.  They might    */
  /*    as well disappear from memory on the next cache lookup, so don't   */
  /*    treat them as persistent data.                                     */
  /*                                                                       */
  /*    The descriptor's `buffer' field is set to~0 to indicate a missing  */
  /*    glyph bitmap.                                                      */
  /*                                                                       */
  /*    If `anode' is _not_ NULL, it receives the address of the cache     */
  /*    node containing the bitmap, after increasing its reference count.  */
  /*    This ensures that the node (as well as the image) will always be   */
  /*    kept in the cache until you call @FTC_Node_Unref to `release' it.  */
  /*                                                                       */
  /*    If `anode' is NULL, the cache node is left unchanged, which means  */
  /*    that the bitmap could be flushed out of the cache on the next      */
  /*    call to one of the caching sub-system APIs.  Don't assume that it  */
  /*    is persistent!                                                     */
  /*                                                                       */
  FT_EXPORT( ft_error )
  ftc_sbit_cache_lookup_scaler( ftc_sbit_cache  cache,
                              ftc_scaler     scaler,
                              ft_ulong       load_flags,
                              ft_uint        gindex,
                              ftc_sbit      *sbit,
                              ftc_node      *anode );


 /* */

#ifdef FT_CONFIG_OPTION_OLD_INTERNALS

  /*@***********************************************************************/
  /*                                                                       */
  /* <Struct>                                                              */
  /*    FTC_FontRec                                                        */
  /*                                                                       */
  /* <Description>                                                         */
  /*    A simple structure used to describe a given `font' to the cache    */
  /*    manager.  Note that a `font' is the combination of a given face    */
  /*    with a given character size.                                       */
  /*                                                                       */
  /* <Fields>                                                              */
  /*    face_id    :: The ID of the face to use.                           */
  /*                                                                       */
  /*    pix_width  :: The character width in integer pixels.               */
  /*                                                                       */
  /*    pix_height :: The character height in integer pixels.              */
  /*                                                                       */
  typedef struct  ftc_font_rec_
  {
    ftc_face_id  face_id;
    ft_ushort   pix_width;
    ft_ushort   pix_height;

  } ftc_font_rec;


  /* */


#define FTC_FONT_COMPARE( f1, f2 )                  \
          ( (f1)->face_id    == (f2)->face_id    && \
            (f1)->pix_width  == (f2)->pix_width  && \
            (f1)->pix_height == (f2)->pix_height )

#define FTC_FONT_HASH( f )                              \
          (ft_uint32)( FTC_FACE_ID_HASH((f)->face_id) ^ \
                       ((f)->pix_width << 8)          ^ \
                       ((f)->pix_height)              )

  typedef ftc_font_rec*  ftc_font;


  FT_EXPORT( ft_error )
  ftc_manager_lookup_face( ftc_manager  manager,
                           ftc_face_id   face_id,
                           ft_face     *aface );

  FT_EXPORT( ft_error )
  ftc_manager_lookup_size( ftc_manager  manager,
                           ftc_font     font,
                           ft_face     *aface,
                           ft_size     *asize );

#endif /* FT_CONFIG_OPTION_OLD_INTERNALS */


 /* */

FT_END_HEADER

#endif /* __FTCACHE_H__ */


/* END */
