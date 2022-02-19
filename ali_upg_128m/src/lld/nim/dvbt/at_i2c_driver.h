/**

 \file at_i2c_driver.h

 \version $Id$

 \author M. Gandillon

 ----------------------------------------------------------------------------\n
   (c) Copyright Abilis Systems SARL 2005-2009 All rigths reserved \n
   www.abilis.com                                                  \n
 ----------------------------------------------------------------------------\n

 \brief AT810 i2c driver

*/
#ifndef AT_I2C_DRIVER_H
#define AT_I2C_DRIVER_H

#include "type_def.h"
#include "at_host_errors.h"

#define FW_CONTROL_ADDRESS          (0x1000)
#define FW_CONTROL_ENABLE           (0x1)
#define FW_CONTROL_DISABLE          (0x0)

#ifdef __cplusplus
extern "C" {
#endif


/** AT810 O2C driver's command */
typedef enum {
    AT_MEM_ACCESS = 0x5A,
    AT_FW_REBOOT = 0xDF,
    AT_RESET =  0xFF
}at_i2c_driver_command_t;

/** I2C read/write function prototype
    \param p_user_data: pointer on specific user data (unspecified)
    \param i2c_address: I2C address
    \param nb_bytes   : number of byte to transfer
    \param p_buffer   : buffer containing data to transmit if 'write' operation or
                        received data in case of 'read'.
    \return Returns HOST_RC_OK if read/write operation succeeded: AT_ERROR otherwise.
*/
typedef int32_t (*i2c_transfer_func_t)(void * p_user_data,
                                    uint8_t i2c_address,
                                    uint16_t nb_bytes,
                                    uint8_t * p_buffer);


/** handle_t structure;
    i2c_address:        Address of slave I2C device
    max_transfer_size:  Maximum size of a transfer in bytes
    p_user_data:        Pointer to a user data
    i2c_read_func:      Callback to platform dependent I2C read function
    i2c_write_func:     Callback to platform dependent I2C write function
*/
typedef struct {
    uint8_t i2c_address;
    uint32_t max_transfer_size;
    void * p_user_data;
    uint8_t *buffer;
    i2c_transfer_func_t i2c_read_func;
    i2c_transfer_func_t i2c_write_func;
}handle_t;

/** frame_param_t structure:
    command:            Command representing the type of transaction
    at810_address:      Address accessed in AT810 memory map
    data_in_length:     Size of the input data buffer
    p_data_in:          Pointer to an input data buffer
*/
typedef struct {
    uint8_t command;
    uint32_t at810_address;
    uint16_t data_in_length;
    uint8_t * p_data_in;
}frame_param_t;

/**
    \fn void
    \brief  Format I2C frame buffer according to input arguments.
    \param  frame_param : pointer to structure of type frame_param_t containing
            the arguments used to create the I2C frame buffer. If the field
            at810_address is equal 0xFFFFFFFF, it is ignored and therefore not
            appended to the output buffer.
    \param  p_i2c_frame_out : pointer to allocated memory buffer big enough to
            contain the frame.
    \param  i2c_frame_out_length : size of p_i2c_frame_out buffer.
    \return Returns the size of the formatted buffer.
    \callgraph
*/
int32_t at_i2c_format_frame(frame_param_t * frame_param, uint8_t * p_i2c_frame_out, uint32_t i2c_frame_out_length);

/**
    \brief  Perform a memory dump on the target represented by \e p_handle of \e nb_bytes starting at
            \e address and put them into \e p_buffer_out.
    \param  p_handle : pointer to handle structure representing the target to be accessed.
    \param  address : first address to be dumped.
    \param  nb_bytes :  number of bytes to be dumped.
    \param  p_buffer_out : pointer to output buffer.
    \return Returns HOST_RC_OK if the dump succeeded: AT_ERROR otherwise.
    \callgraph
*/
int32_t at_read_block(handle_t * p_handle, uint32_t address, uint16_t nb_bytes, uint8_t * p_buffer_out);

/**
    \brief  Perform a memory write of the \e nb_bytes of \e p_buffer_in at \e address
            on the target represented by \e p_handle.
    \param  p_handle : pointer to handle structure representing the target to be accessed.
    \param  address : first address to be written.
    \param  nb_bytes :  number of bytes to be written.
    \param  p_buffer_out : pointer to the data buffer to be written.
    \return Returns HOST_RC_OK if the write succeeded: AT_ERROR otherwise.
    \callgraph
*/
int32_t at_write_block(handle_t * p_handle, uint32_t address, uint16_t nb_bytes, uint8_t * p_buffer_in);

/**
    \brief  Perform a memory dump of 32 bits on the target represented by \e p_handle at
            \e address and put them into \e p_data_out.
    \param  p_handle : pointer to handle structure representing the target to be accessed.
    \param  address : address to be dumped.
    \param  p_data_out : output buffer.
    \return Returns HOST_RC_OK if the dump succeeded: AT_ERROR otherwise.
    \callgraph
*/
int32_t at_read(handle_t * p_handle, uint32_t address, uint32_t * p_data_out);

/**
    \brief  Perform a memory write of 32 bits at \e address on the target represented by \e p_handle.
    \param  p_handle : pointer to handle structure representing the target to be accessed.
    \param  address : address to be written.
    \param  data_in : data to be written.
    \return Returns HOST_RC_OK if the write succeeded: AT_ERROR otherwise.
    \callgraph
*/
int32_t at_write(handle_t * p_handle, uint32_t address, uint32_t data_in);

/**
    \brief  Performs a software reset of the target represented by \e p_handle.
    \param  p_handle : pointer to handle structure representing the target to be accessed.
    \return Returns HOST_RC_OK if the reset succeeded: AT_ERROR otherwise.
    \callgraph
*/
int32_t at_reset(handle_t * p_handle);

/**
    \brief  Performs a software reboot of the target represented by \e p_handle.
    \param  p_handle : pointer to handle structure representing the target to be accessed.
    \return Returns HOST_RC_OK if the reboot succeeded: AT_ERROR otherwise.
    \callgraph
*/
int32_t at_fw_reboot(handle_t * p_handle);

/**
    \brief  Creates a channel to communicate with a target.
    \param  i2c_address : I2C slave address of the target.
    \param  size of the biggest transfer achievable in one operation.
    \param  p_user_data : pointer to user data (used for example in platform
            dependent part of the firmware).
    \param  i2c_read_func : platform dependent function to perform read operation on  target.
    \param  i2c_write_func : platform dependent function to perform write operation on target.
    \return Returns a pointer to the handle structure representing the target to be accessed through
            the created channel if the channel creation succeeded or a NULL pointer otherwise.
    \callgraph
*/
handle_t * at_create_channel(uint8_t i2c_address, uint32_t max_transfer_size, void * p_user_data, i2c_transfer_func_t i2c_read_func, i2c_transfer_func_t i2c_write_func);

/**
    \brief  Delete the channel to communicate with the target represented by
            \e p_handle.
    \param  p_handle : pointer to handle structure representing the channel.
    \callgraph
*/
void at_delete_channel(handle_t * p_handle);

/**
    \brief  Return the max number of bytes the driver can read/write from/to the target.
    \param  p_handle : pointer to handle structure representing the channel.
    \callgraph
*/
uint16_t at_get_max_size_rdwr(handle_t * p_handle);

#ifdef __cplusplus
}
#endif

#endif /* AT_I2C_DRIVER_H */
