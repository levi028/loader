#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/cert/cert.h>

static struct cert_asa_dev *_cert_asa_dev(void)
{
    return (struct cert_asa_dev *)dev_get_by_id(HLD_DEV_TYPE_ASA, 0);
}

static struct cert_akl_dev *_cert_akl_dev(void)
{
    return (struct cert_akl_dev *)dev_get_by_id(HLD_DEV_TYPE_AKL, 0);
}

int cert_asa_open(void)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->open)
    {
        return dev->open();
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_asa_close(int sess)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->close)
    {
        return dev->close(sess);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}


int cert_asa_set_format(int sess, int format)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->setfmt)
    {
        return dev->setfmt(sess, format);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_asa_add_pid(int sess, struct cert_asa_pid *param)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->addpid)
    {
        return dev->addpid(sess, param);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_asa_del_pid(int sess, struct cert_asa_pid *param)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->delpid)
    {
        return dev->delpid(sess, param);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}


int cert_asa_decrypt(int sess, UINT8 *input, UINT8 *output, UINT32 length)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->decrypt)
    {
        return dev->decrypt(sess, input, output, length);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_asa_decrypt_nodes(int sess, void *buf, UINT32 count)
{
    struct cert_asa_dev *dev = _cert_asa_dev();

    if(dev && dev->decrypt_nodes)
    {
        return dev->decrypt_nodes(sess, buf, count);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_akl_open(void)
{
    struct cert_akl_dev *dev = _cert_akl_dev();

    if(dev && dev->open)
    {
        return dev->open();
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_akl_close(int sess)
{
    struct cert_akl_dev *dev = _cert_akl_dev();

    if(dev && dev->close)
    {
        return dev->close(sess);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_akl_exchange(int sess, struct cert_akl_cmd *param)
{
    struct cert_akl_dev *dev = _cert_akl_dev();

    if(dev && dev->exchange)
    {
        return dev->exchange(sess, param);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_akl_save_key(int sess, struct cert_akl_savekey *param)
{
    struct cert_akl_dev *dev = _cert_akl_dev();

    if(dev && dev->savekey)
    {
        return dev->savekey(sess, param);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}

int cert_akl_ack(int sess)
{
    struct cert_akl_dev *dev = _cert_akl_dev();

    if(dev && dev->ack)
    {
        return dev->ack(sess);
    }
    else
    {
        return (-CERT_ERROR_NODEV);
    }
}


