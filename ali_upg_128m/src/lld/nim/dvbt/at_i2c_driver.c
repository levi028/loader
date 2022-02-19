#include <string.h>
#include <stdlib.h>
#include "at_i2c_driver.h"

#define AT810_I2C_COMMAND_SIZE         (0x1)
#define AT810_I2C_ADDRESS_SIZE         (0x3)
#define AT_I2C_CMD_MAX_OVERHEAD_BYTE   (AT810_I2C_COMMAND_SIZE+AT810_I2C_ADDRESS_SIZE)


int32_t at_i2c_format_frame(frame_param_t * frame_param, uint8_t * p_i2c_frame_out, uint32_t i2c_frame_out_length)
{
    uint32_t out_buf_expected_size = AT810_I2C_COMMAND_SIZE;
    uint32_t offset = 0;

    /* Control output buffer size validity */
    if(frame_param->at810_address != 0xFFFFFFFF)
        out_buf_expected_size += AT810_I2C_ADDRESS_SIZE;
    out_buf_expected_size += frame_param->data_in_length;
    if(out_buf_expected_size > i2c_frame_out_length)
        return HOST_RC_ERROR;

    /* Control input buffer size validity */
    if(frame_param->data_in_length % 4)
        return HOST_RC_ERROR;

    /* Append command to output buffer */
    MEMCPY(p_i2c_frame_out + offset, &frame_param->command, sizeof(uint8_t));
    offset += AT810_I2C_COMMAND_SIZE;

    /* Append address to output buffer */
    if(frame_param->at810_address != 0xFFFFFFFF){
        MEMCPY(p_i2c_frame_out + offset, &frame_param->at810_address, AT810_I2C_ADDRESS_SIZE * sizeof(uint8_t));
        offset += AT810_I2C_ADDRESS_SIZE;
    }

    /* Append input buffer to output buffer */
    MEMCPY(p_i2c_frame_out + offset, frame_param->p_data_in, frame_param->data_in_length * sizeof(uint8_t));
    offset += frame_param->data_in_length;

    return offset;
}

int32_t at_read_block(handle_t * p_handle, uint32_t address, uint16_t nb_bytes, uint8_t * p_buffer_out)
{
    frame_param_t frame_param;
    int32_t nb_bytes_formatted = 0;

    /* Configure frame_param */
    frame_param.command = AT_MEM_ACCESS;
    frame_param.at810_address = address;
    frame_param.data_in_length = 0;
    frame_param.p_data_in = 0;

    /* Format the first frame to be sent (I2C write frame) */
    if((nb_bytes_formatted = at_i2c_format_frame(&frame_param, p_handle->buffer, p_handle->max_transfer_size)) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    /* Write the first frame on I2C bus */
    if(p_handle->i2c_write_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes_formatted, p_handle->buffer) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    /* Read the second frame on I2C Bus */
    if(p_handle->i2c_read_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes, p_buffer_out) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    return HOST_RC_OK;
}

int32_t at_write_block(handle_t * p_handle, uint32_t address, uint16_t nb_bytes, uint8_t * p_buffer_in)
{
    frame_param_t frame_param;
    int32_t nb_bytes_formatted = 0;

    /* Configure frame_param */
    frame_param.command = AT_MEM_ACCESS;
    frame_param.at810_address = address;
    frame_param.data_in_length = nb_bytes;
    frame_param.p_data_in = p_buffer_in;

    /* Format the frame to be sent (I2C write frame) */
    if((nb_bytes_formatted = at_i2c_format_frame(&frame_param, p_handle->buffer, p_handle->max_transfer_size)) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    /* Write the frame on I2C bus */
    return p_handle->i2c_write_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes_formatted, p_handle->buffer);
}

int32_t at_read(handle_t * p_handle, uint32_t address, uint32_t * p_data_out)
{
    frame_param_t frame_param;
    int32_t nb_bytes_formatted = 0;

    /* Configure frame_param */
    frame_param.command = AT_MEM_ACCESS;
    frame_param.at810_address = address;
    frame_param.data_in_length = 0;
    frame_param.p_data_in = 0;

    /* Format the first frame to be sent (I2C write frame) */
    if((nb_bytes_formatted = at_i2c_format_frame(&frame_param, p_handle->buffer, p_handle->max_transfer_size)) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    /* Write the first frame on I2C bus */
    if(p_handle->i2c_write_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes_formatted, p_handle->buffer) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    /* Read the second frame on I2C Bus */
    if(p_handle->i2c_read_func(p_handle->p_user_data, p_handle->i2c_address, 4, (uint8_t *)p_data_out) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    return HOST_RC_OK;
}

int32_t at_write(handle_t * p_handle, uint32_t address, uint32_t data_in)
{
    frame_param_t frame_param;
    int32_t nb_bytes_formatted = 0;

    /* Configure frame_param */
    frame_param.command = AT_MEM_ACCESS;
    frame_param.at810_address = address;
    frame_param.data_in_length = 4;
    frame_param.p_data_in = (uint8_t *)&data_in;

    /* Format the frame to be sent (I2C write frame) */
    if((nb_bytes_formatted = at_i2c_format_frame(&frame_param, p_handle->buffer, p_handle->max_transfer_size)) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    /* Write the frame on I2C bus */
    return p_handle->i2c_write_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes_formatted, p_handle->buffer);
}

int32_t at_reset(handle_t * p_handle)
{
    frame_param_t frame_param;
    int32_t nb_bytes_formatted = 0;

    /* Configure frame_param */
    frame_param.command = AT_RESET;
    frame_param.at810_address = 0xFFFFFFFF;
    frame_param.data_in_length = 0;
    frame_param.p_data_in = 0;

    /* Format the frame to be sent (I2C write frame) */
    if((nb_bytes_formatted = at_i2c_format_frame(&frame_param, p_handle->buffer, p_handle->max_transfer_size)) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    if(p_handle->i2c_write_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes_formatted, p_handle->buffer) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    return HOST_RC_OK;
}


int32_t at_fw_reboot(handle_t * p_handle)
{
    frame_param_t frame_param;
    int32_t nb_bytes_formatted = 0;

    /* Configure frame_param */
    frame_param.command = AT_FW_REBOOT;
    frame_param.at810_address = 0xFFFFFFFF;
    frame_param.data_in_length = 0;
    frame_param.p_data_in = 0;

    /* Format the frame to be sent (I2C write frame) */
    if((nb_bytes_formatted = at_i2c_format_frame(&frame_param, p_handle->buffer, p_handle->max_transfer_size)) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    if(p_handle->i2c_write_func(p_handle->p_user_data, p_handle->i2c_address, nb_bytes_formatted, p_handle->buffer) == HOST_RC_ERROR)
        return HOST_RC_ERROR;

    return HOST_RC_OK;
}

handle_t * at_create_channel(uint8_t i2c_address, uint32_t max_transfer_size, void * p_user_data, i2c_transfer_func_t i2c_read_func, i2c_transfer_func_t i2c_write_func)
{
    handle_t * p_handle;

    if(max_transfer_size < AT_I2C_CMD_MAX_OVERHEAD_BYTE + sizeof(uint32_t))
        return NULL;

    p_handle = (handle_t *) malloc(sizeof(handle_t));

    if(!p_handle)
        return p_handle;

    p_handle->buffer = malloc(max_transfer_size);
    if(!p_handle->buffer){
        free(p_handle);
        return NULL;
    }

    p_handle->i2c_address = i2c_address;
    p_handle->max_transfer_size = max_transfer_size;
    p_handle->i2c_read_func = i2c_read_func;
    p_handle->i2c_write_func = i2c_write_func;
    p_handle->p_user_data = p_user_data;

    return p_handle;
}

void at_delete_channel(handle_t * p_handle)
{
   if(p_handle!=NULL){
      if(p_handle->buffer!=NULL)
         free(p_handle->buffer);
      free(p_handle);
   }
}

uint16_t at_get_max_size_rdwr(handle_t * p_handle)
{
    return(p_handle->max_transfer_size - AT_I2C_CMD_MAX_OVERHEAD_BYTE - p_handle->max_transfer_size%sizeof(uint32_t));
}
