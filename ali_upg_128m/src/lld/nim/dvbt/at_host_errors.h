/**
 *
 * \file  host_errors.h
 *
 * \brief: error codes of the host AT810 driver
 *
 * \date   08/10/14
 *
 *
 */


#ifndef _HOST_ERROR_H_
#define _HOST_ERROR_H_

/* Error codes values */
typedef enum {
   HOST_RC_LOG_ERROR                   = -105,
   HOST_RC_MEMORY_ERROR                = -104,
   HOST_RC_FIRMWARE_UPLOAD_FAILED      = -103,
   HOST_RC_FIRMWARE_BAD_FILE           = -102,
   HOST_RC_FIRMWARE_FILE_NOT_FOUND     = -101,
   HOST_RC_ERROR                       = -100,
   HOST_RC_OK                          = 0
} host_error_code_t;


#endif
