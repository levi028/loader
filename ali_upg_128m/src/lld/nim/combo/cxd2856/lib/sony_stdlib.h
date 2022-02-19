/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/06/24
  Modification ID : 3b74e280b7ad8ce430b6a9419ac53e8f2e3737f9
------------------------------------------------------------------------------*/
/**

 @file    sony_stdlib.h

          This file provides the C standard lib function mapping.
*/
/*----------------------------------------------------------------------------*/

#ifndef SONY_STDLIB_H
#define SONY_STDLIB_H

/*
 PORTING. Please modify if ANCI C standard library is not available.
*/
#include <string.h>
/*------------------------------------------------------------------------------
 Defines
------------------------------------------------------------------------------*/

/**
 @brief Alias for memcpy.
*/
#define sony_memcpy  memcpy

/**
 @brief Alias for memset.
*/
#define sony_memset  memset

#endif /* SONY_STDLIB_H */
