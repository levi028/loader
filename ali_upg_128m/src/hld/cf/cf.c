#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/cf/cf.h>

static inline struct cf_dev *_cf_dev(void)
{
    return (struct cf_dev *)dev_get_by_id(HLD_DEV_TYPE_CF, 0);
}

int cf_open(void)
{
    struct cf_dev *dev = _cf_dev();

    if(dev && dev->open)
    {
        return dev->open();
    }
    else
    {
        return -CF_ERROR_NODEV;
    }
}

int cf_close(int chan)
{
    struct cf_dev *dev = _cf_dev();

    if(dev && dev->close)
    {
        return dev->close(chan);
    }
    else
    {
        return -CF_ERROR_NODEV;
    }
}


int cf_ioctl(int chan, int cmd, void *param)
{
    struct cf_dev *dev = _cf_dev();

	if (!param)
		return -CF_ERROR_INVALID_PARAM;

    if(dev && dev->ioctl)
    {
        return dev->ioctl(chan, cmd, param);
    }
    else
    {
        return -CF_ERROR_NODEV;
    }
}

int cf_write(int chan, UINT8 *buf, int count)
{
    struct cf_dev *dev = _cf_dev();

	if (!buf)
		return -CF_ERROR_INVALID_PARAM;

    if(dev && dev->write)
    {
        return dev->write(chan, buf, count);
    }
    else
    {
       return -CF_ERROR_NODEV;
    }
}

int cf_read(int chan, UINT8 *buf, int count)
{
    struct cf_dev *dev = _cf_dev();

	if (!buf)
		return -CF_ERROR_INVALID_PARAM;

    if(dev && dev->read)
    {
        return dev->read(chan, buf, count);
    }
    else
    {
       return -CF_ERROR_NODEV;
    }
}

int cf_set_target(int pos)
{
    struct cf_dev *dev = _cf_dev();

    if(dev && dev->set_target)
    {
        return dev->set_target(pos);
    }
    else
    {
       return -CF_ERROR_NODEV;
    }
}

