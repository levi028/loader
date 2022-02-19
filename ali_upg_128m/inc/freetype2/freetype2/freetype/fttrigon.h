/***************************************************************************/
/*                                                                         */
/*  fttrigon.h                                                             */
/*                                                                         */
/*    FreeType trigonometric functions (specification).                    */
/*                                                                         */
/*  Copyright 2001, 2003, 2005, 2007 by                                    */
/*  David Turner, Robert Wilhelm, and Werner Lemberg.                      */
/*                                                                         */
/*  This file is part of the FreeType project, and may only be used,       */
/*  modified, and distributed under the terms of the FreeType project      */
/*  license, LICENSE.TXT.  By continuing to use, modify, or distribute     */
/*  this file you indicate that you have read the license and              */
/*  understand and accept it fully.                                        */
/*                                                                         */
/***************************************************************************/


#ifndef __FTTRIGON_H__
#define __FTTRIGON_H__

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
  /*   computations                                                        */
  /*                                                                       */
  /*************************************************************************/


  /*************************************************************************
   *
   * @type:
   *   FT_Angle
   *
   * @description:
   *   This type is used to model angle values in FreeType.  Note that the
   *   angle is a 16.16 fixed float value expressed in degrees.
   *
   */
  typedef ft_fixed  ft_angle;


  /*************************************************************************
   *
   * @macro:
   *   FT_ANGLE_PI
   *
   * @description:
   *   The angle pi expressed in @FT_Angle units.
   *
   */
#define FT_ANGLE_PI  ( 180L << 16 )


  /*************************************************************************
   *
   * @macro:
   *   FT_ANGLE_2PI
   *
   * @description:
   *   The angle 2*pi expressed in @FT_Angle units.
   *
   */
#define FT_ANGLE_2PI  ( FT_ANGLE_PI * 2 )


  /*************************************************************************
   *
   * @macro:
   *   FT_ANGLE_PI2
   *
   * @description:
   *   The angle pi/2 expressed in @FT_Angle units.
   *
   */
#define FT_ANGLE_PI2  ( FT_ANGLE_PI / 2 )


  /*************************************************************************
   *
   * @macro:
   *   FT_ANGLE_PI4
   *
   * @description:
   *   The angle pi/4 expressed in @FT_Angle units.
   *
   */
#define FT_ANGLE_PI4  ( FT_ANGLE_PI / 4 )


  /*************************************************************************
   *
   * @function:
   *   FT_Sin
   *
   * @description:
   *   Return the sinus of a given angle in fixed point format.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   * @return:
   *   The sinus value.
   *
   * @note:
   *   If you need both the sinus and cosinus for a given angle, use the
   *   function @FT_Vector_Unit.
   *
   */
  FT_EXPORT( ft_fixed )
  ft_sin( ft_angle  angle );


  /*************************************************************************
   *
   * @function:
   *   FT_Cos
   *
   * @description:
   *   Return the cosinus of a given angle in fixed point format.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   * @return:
   *   The cosinus value.
   *
   * @note:
   *   If you need both the sinus and cosinus for a given angle, use the
   *   function @FT_Vector_Unit.
   *
   */
  FT_EXPORT( ft_fixed )
  ft_cos( ft_angle  angle );


  /*************************************************************************
   *
   * @function:
   *   FT_Tan
   *
   * @description:
   *   Return the tangent of a given angle in fixed point format.
   *
   * @input:
   *   angle ::
   *     The input angle.
   *
   * @return:
   *   The tangent value.
   *
   */
  FT_EXPORT( ft_fixed )
  ft_tan( ft_angle  angle );


  /*************************************************************************
   *
   * @function:
   *   FT_Atan2
   *
   * @description:
   *   Return the arc-tangent corresponding to a given vector (x,y) in
   *   the 2d plane.
   *
   * @input:
   *   x ::
   *     The horizontal vector coordinate.
   *
   *   y ::
   *     The vertical vector coordinate.
   *
   * @return:
   *   The arc-tangent value (i.e. angle).
   *
   */
  FT_EXPORT( ft_angle )
  ft_atan2( ft_fixed  x,
            ft_fixed  y );


  /*************************************************************************
   *
   * @function:
   *   FT_Angle_Diff
   *
   * @description:
   *   Return the difference between two angles.  The result is always
   *   constrained to the ]-PI..PI] interval.
   *
   * @input:
   *   angle1 ::
   *     First angle.
   *
   *   angle2 ::
   *     Second angle.
   *
   * @return:
   *   Constrained value of `value2-value1'.
   *
   */
  FT_EXPORT( ft_angle )
  ft_angle_diff( ft_angle  angle1,
                 ft_angle  angle2 );


  /*************************************************************************
   *
   * @function:
   *   FT_Vector_Unit
   *
   * @description:
   *   Return the unit vector corresponding to a given angle.  After the
   *   call, the value of `vec.x' will be `sin(angle)', and the value of
   *   `vec.y' will be `cos(angle)'.
   *
   *   This function is useful to retrieve both the sinus and cosinus of a
   *   given angle quickly.
   *
   * @output:
   *   vec ::
   *     The address of target vector.
   *
   * @input:
   *   angle ::
   *     The address of angle.
   *
   */
  FT_EXPORT( void )
  ft_vector_unit( ft_vector*  vec,
                  ft_angle    angle );


  /*************************************************************************
   *
   * @function:
   *   FT_Vector_Rotate
   *
   * @description:
   *   Rotate a vector by a given angle.
   *
   * @inout:
   *   vec ::
   *     The address of target vector.
   *
   * @input:
   *   angle ::
   *     The address of angle.
   *
   */
  FT_EXPORT( void )
  ft_vector_rotate( ft_vector*  vec,
                    ft_angle    angle );


  /*************************************************************************
   *
   * @function:
   *   FT_Vector_Length
   *
   * @description:
   *   Return the length of a given vector.
   *
   * @input:
   *   vec ::
   *     The address of target vector.
   *
   * @return:
   *   The vector length, expressed in the same units that the original
   *   vector coordinates.
   *
   */
  FT_EXPORT( ft_fixed )
  ft_vector_length( ft_vector*  vec );


  /*************************************************************************
   *
   * @function:
   *   FT_Vector_Polarize
   *
   * @description:
   *   Compute both the length and angle of a given vector.
   *
   * @input:
   *   vec ::
   *     The address of source vector.
   *
   * @output:
   *   length ::
   *     The vector length.
   *
   *   angle ::
   *     The vector angle.
   *
   */
  FT_EXPORT( void )
  ft_vector_polarize( ft_vector*  vec,
                      ft_fixed   *length,
                      ft_angle   *angle );


  /*************************************************************************
   *
   * @function:
   *   FT_Vector_From_Polar
   *
   * @description:
   *   Compute vector coordinates from a length and angle.
   *
   * @output:
   *   vec ::
   *     The address of source vector.
   *
   * @input:
   *   length ::
   *     The vector length.
   *
   *   angle ::
   *     The vector angle.
   *
   */
  FT_EXPORT( void )
  ft_vector_from_polar( ft_vector*  vec,
                        ft_fixed    length,
                        ft_angle    angle );

  /* */


FT_END_HEADER

#endif /* __FTTRIGON_H__ */


/* END */
