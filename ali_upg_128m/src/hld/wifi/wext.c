


#include <sys_config.h>

#include <hld/net/net.h>

#include <hld/wifi/iw_handler.h>   //LYH check

#include <hld/wifi/wifi_api.h>

#ifdef WIFI_SUPPORT

/* Special error message for the driver to indicate that we

 * should do a commit after return from the iw_handler */

#define EIWCOMMIT	EINPROGRESS



/* -------------------------- CONSTANTS -------------------------- */

/* Type of headers we know about (basically union iwreq_data) */
#define IW_HEADER_TYPE_NULL	0	/* Not available */
#define IW_HEADER_TYPE_CHAR	2	/* char [IFNAMSIZ] */
#define IW_HEADER_TYPE_UINT	4	/* __u32 */
#define IW_HEADER_TYPE_FREQ	5	/* struct iw_freq */
#define IW_HEADER_TYPE_ADDR	6	/* struct sockaddr */
#define IW_HEADER_TYPE_POINT	8	/* struct iw_point */
#define IW_HEADER_TYPE_PARAM	9	/* struct iw_param */
#define IW_HEADER_TYPE_QUAL	10	/* struct iw_quality */

/* Handling flags */
/* Most are not implemented. I just use them as a reminder of some
 * cool features we might need one day ;-) */
#define IW_DESCR_FLAG_NONE	0x0000	/* Obvious */
/* Wrapper level flags */
#define IW_DESCR_FLAG_DUMP	0x0001	/* Not part of the dump command */
#define IW_DESCR_FLAG_EVENT	0x0002	/* Generate an event on SET */
#define IW_DESCR_FLAG_RESTRICT	0x0004	/* GET : request is ROOT only */
				/* SET : Omit payload from generated iwevent */
#define IW_DESCR_FLAG_NOMAX	0x0008	/* GET : no limit on request size */
/* Driver level flags */
#define IW_DESCR_FLAG_WAIT	0x0100	/* Wait for driver event */

/* ---------------------------- TYPES ---------------------------- */

/*
 * Describe how a standard IOCTL looks like.
 */
struct iw_ioctl_description
{
	UINT8	header_type;		/* NULL, iw_point or other */
	UINT8	token_type;		/* Future */
	UINT16	token_size;		/* Granularity of payload */
	UINT16	min_tokens;		/* Min acceptable token number */
	UINT16	max_tokens;		/* Max acceptable token number */
	UINT32	flags;			/* Special handling of the request */
};



/************************* GLOBAL VARIABLES *************************/

/*

 * You should not use global variables, because of re-entrancy.

 * On our case, it's only const, so it's OK...

 */

/*

 * Meta-data about all the standard Wireless Extension request we

 * know about.

 */

static const struct iw_ioctl_description standard_ioctl[] = {

	[SIOCSIWCOMMIT	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_NULL,

	},

	[SIOCGIWNAME	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_CHAR,

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWNWID	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

		.flags		= IW_DESCR_FLAG_EVENT,

	},

	[SIOCGIWNWID	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWFREQ	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_FREQ,

		.flags		= IW_DESCR_FLAG_EVENT,

	},

	[SIOCGIWFREQ	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_FREQ,

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWMODE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_UINT,

		.flags		= IW_DESCR_FLAG_EVENT,

	},

	[SIOCGIWMODE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_UINT,

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWSENS	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWSENS	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWRANGE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_NULL,

	},

	[SIOCGIWRANGE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= sizeof(struct iw_range),

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWPRIV	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_NULL,

	},

	[SIOCGIWPRIV	- SIOCIWFIRST] = { /* (handled directly by us) */

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= sizeof(struct iw_priv_args),

		.max_tokens	= 16,

		.flags		= IW_DESCR_FLAG_NOMAX,

	},

	[SIOCSIWSTATS	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_NULL,

	},

	[SIOCGIWSTATS	- SIOCIWFIRST] = { /* (handled directly by us) */

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= sizeof(struct iw_statistics),

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWSPY	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= sizeof(struct sockaddr),

		.max_tokens	= IW_MAX_SPY,

	},

	[SIOCGIWSPY	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= sizeof(struct sockaddr) +

				  sizeof(struct iw_quality),

		.max_tokens	= IW_MAX_SPY,

	},

	[SIOCSIWTHRSPY	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= sizeof(struct iw_thrspy),

		.min_tokens	= 1,

		.max_tokens	= 1,

	},

	[SIOCGIWTHRSPY	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= sizeof(struct iw_thrspy),

		.min_tokens	= 1,

		.max_tokens	= 1,

	},

	[SIOCSIWAP	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_ADDR,

	},

	[SIOCGIWAP	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_ADDR,

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWMLME	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.min_tokens	= sizeof(struct iw_mlme),

		.max_tokens	= sizeof(struct iw_mlme),

	},

	[SIOCGIWAPLIST	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= sizeof(struct sockaddr) +

				  sizeof(struct iw_quality),

		.max_tokens	= IW_MAX_AP,

		.flags		= IW_DESCR_FLAG_NOMAX,

	},

	[SIOCSIWSCAN	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.min_tokens	= 0,

		.max_tokens	= sizeof(struct iw_scan_req),

	},

	[SIOCGIWSCAN	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_SCAN_MAX_DATA,

		.flags		= IW_DESCR_FLAG_NOMAX,

	},

	[SIOCSIWESSID	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_ESSID_MAX_SIZE,

		.flags		= IW_DESCR_FLAG_EVENT,

	},

	[SIOCGIWESSID	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_ESSID_MAX_SIZE,

		.flags		= IW_DESCR_FLAG_DUMP,

	},

	[SIOCSIWNICKN	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_ESSID_MAX_SIZE,

	},

	[SIOCGIWNICKN	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_ESSID_MAX_SIZE,

	},

	[SIOCSIWRATE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWRATE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWRTS	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWRTS	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWFRAG	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWFRAG	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWTXPOW	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWTXPOW	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWRETRY	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWRETRY	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWENCODE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_ENCODING_TOKEN_MAX,

		.flags		= IW_DESCR_FLAG_EVENT | IW_DESCR_FLAG_RESTRICT,

	},

	[SIOCGIWENCODE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_ENCODING_TOKEN_MAX,

		.flags		= IW_DESCR_FLAG_DUMP | IW_DESCR_FLAG_RESTRICT,

	},

	[SIOCSIWPOWER	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWPOWER	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWGENIE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_GENERIC_IE_MAX,

	},

	[SIOCGIWGENIE	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.max_tokens	= IW_GENERIC_IE_MAX,

	},

	[SIOCSIWAUTH	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCGIWAUTH	- SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_PARAM,

	},

	[SIOCSIWENCODEEXT - SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.min_tokens	= sizeof(struct iw_encode_ext),

		.max_tokens	= sizeof(struct iw_encode_ext) +

				  IW_ENCODING_TOKEN_MAX,

	},

	[SIOCGIWENCODEEXT - SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.min_tokens	= sizeof(struct iw_encode_ext),

		.max_tokens	= sizeof(struct iw_encode_ext) +

				  IW_ENCODING_TOKEN_MAX,

	},

	[SIOCSIWPMKSA - SIOCIWFIRST] = {

		.header_type	= IW_HEADER_TYPE_POINT,

		.token_size	= 1,

		.min_tokens	= sizeof(struct iw_pmksa),

		.max_tokens	= sizeof(struct iw_pmksa),

	},

};

//static const unsigned standard_ioctl_num = ARRAY_SIZE(standard_ioctl);

static const unsigned int standard_ioctl_num = (sizeof(standard_ioctl) /

						sizeof(struct iw_ioctl_description));




/* Size (in bytes) of the various private data types */

static const char iw_priv_type_size[] = {

	0,				/* IW_PRIV_TYPE_NONE */

	1,				/* IW_PRIV_TYPE_BYTE */

	1,				/* IW_PRIV_TYPE_CHAR */

	0,				/* Not defined */

	sizeof(UINT32),			/* IW_PRIV_TYPE_INT */

	sizeof(struct iw_freq),		/* IW_PRIV_TYPE_FLOAT */

	sizeof(struct sockaddr),	/* IW_PRIV_TYPE_ADDR */

	0,				/* Not defined */

};



/************************ COMMON SUBROUTINES ************************/

/*

 * Stuff that may be used in various place or doesn't fit in one

 * of the section below.

 */

#ifdef WIFI_SUPPORT

/* ---------------------------------------------------------------- */

/*

 * Return the driver handler associated with a specific Wireless Extension.

 */

static iw_handler get_handler(struct net_device *dev, unsigned int cmd)

{

	/* Don't "optimise" the following variable, it will crash */

	unsigned int	index;		/* *MUST* be unsigned */



	/* Check if we have some wireless handlers defined */

	if (dev->wireless_handlers == NULL)

		return NULL;



	/* Try as a standard command */

	index = cmd - SIOCIWFIRST;

	if (index < dev->wireless_handlers->num_standard)

		return dev->wireless_handlers->standard[index];



	/* Try as a private command */

	index = cmd - SIOCIWFIRSTPRIV;

	if (index < dev->wireless_handlers->num_private)

		return dev->wireless_handlers->private[index];



	/* Not found */

	return NULL;

}



/* ---------------------------------------------------------------- */

/*

 * Get statistics out of the driver

 */

static struct iw_statistics *get_wireless_stats(struct net_device *dev)

{

	/* New location */

	if ((dev->wireless_handlers != NULL) &&

	   (dev->wireless_handlers->get_wireless_stats != NULL))

		return dev->wireless_handlers->get_wireless_stats(dev);



	/* Not found */

	return NULL;

}



/* ---------------------------------------------------------------- */

/*

 * Call the commit handler in the driver

 * (if exist and if conditions are right)

 *

 * Note : our current commit strategy is currently pretty dumb,

 * but we will be able to improve on that...

 * The goal is to try to agreagate as many changes as possible

 * before doing the commit. Drivers that will define a commit handler

 * are usually those that need a reset after changing parameters, so

 * we want to minimise the number of reset.

 * A cool idea is to use a timer : at each "set" command, we re-set the

 * timer, when the timer eventually fires, we call the driver.

 * Hopefully, more on that later.

 *

 * Also, I'm waiting to see how many people will complain about the

 * netif_running(dev) test. I'm open on that one...

 * Hopefully, the driver will remember to do a commit in "open()" ;-)

 */

static int call_commit_handler(struct net_device *dev)

{

	if (dev->wireless_handlers->standard[0] != NULL)

		/* Call the commit handler on the driver */

		return dev->wireless_handlers->standard[0](dev, NULL,

							   NULL, NULL);

	else

		return 0;		/* Command completed successfully */

}



/* ---------------------------------------------------------------- */

/*

 * Calculate size of private arguments

 */

static int get_priv_size(UINT16 args)

{

	int	num = args & IW_PRIV_SIZE_MASK;

	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;



	return num * iw_priv_type_size[type];

}



/* ---------------------------------------------------------------- */

/*

 * Re-calculate the size of private arguments

 */

static int adjust_priv_size(UINT32 args, struct iw_point *iwp)

{

	int	num = iwp->length;

	int	max = args & IW_PRIV_SIZE_MASK;

	int	type = (args & IW_PRIV_TYPE_MASK) >> 12;



	/* Make sure the driver doesn't goof up */

	if (max < num)

		num = max;



	return num * iw_priv_type_size[type];

}



/* ---------------------------------------------------------------- */

/*

 * Standard Wireless Handler : get wireless stats

 *	Allow programatic access to /proc/net/wireless even if /proc

 *	doesn't exist... Also more efficient...

 */

static int iw_handler_get_iwstats(struct net_device *		dev,

				  struct iw_request_info *	info,

				  union iwreq_data *		wrqu,

				  char *			extra)

{

	/* Get stats from the driver */

	struct iw_statistics *stats;



	stats = get_wireless_stats(dev);

	if (stats) {

		/* Copy statistics to extra */

		MEMCPY(extra, stats, sizeof(struct iw_statistics));

		wrqu->data.length = sizeof(struct iw_statistics);



		/* Check if we need to clear the updated flag */

		if (wrqu->data.flags != 0)

			stats->qual.updated &= ~IW_QUAL_ALL_UPDATED;

		return 0;

	} else

		return -EOPNOTSUPP;

}



/* ---------------------------------------------------------------- */

/*

 * Standard Wireless Handler : get iwpriv definitions

 * Export the driver private handler definition

 * They will be picked up by tools like iwpriv...

 */

static int iw_handler_get_private(struct net_device *		dev,

				  struct iw_request_info *	info,

				  union iwreq_data *		wrqu,

				  char *			extra)

{

	/* Check if the driver has something to export */

	if ((dev->wireless_handlers->num_private_args == 0) ||

	   (dev->wireless_handlers->private_args == NULL))

		return -EOPNOTSUPP;



	/* Check if there is enough buffer up there */

	if (wrqu->data.length < dev->wireless_handlers->num_private_args) {

		/* User space can't know in advance how large the buffer

		 * needs to be. Give it a hint, so that we can support

		 * any size buffer we want somewhat efficiently... */

		wrqu->data.length = dev->wireless_handlers->num_private_args;

		return -E2BIG;

	}



	/* Set the number of available ioctls. */

	wrqu->data.length = dev->wireless_handlers->num_private_args;



	/* Copy structure to the user buffer. */

	MEMCPY(extra, dev->wireless_handlers->private_args,

	       sizeof(struct iw_priv_args) * wrqu->data.length);



	return 0;

}



/************************** IOCTL SUPPORT **************************/

/*

 * The original user space API to configure all those Wireless Extensions

 * is through IOCTLs.

 * In there, we check if we need to call the new driver API (iw_handler)

 * or just call the driver ioctl handler.

 */



/* ---------------------------------------------------------------- */

static int ioctl_standard_iw_point(struct iw_point *iwp, unsigned int cmd,

				   const struct iw_ioctl_description *descr,

				   iw_handler handler, struct net_device *dev,

				   struct iw_request_info *info)

{

	int err, extra_size, user_length = 0, essid_compat = 0;

	char *extra;



	/* Calculate space needed by arguments. Always allocate

	 * for max space.

	 */

	extra_size = descr->max_tokens * descr->token_size;



	/* Check need for ESSID compatibility for WE < 21 */

	switch (cmd) {

	case SIOCSIWESSID:

	case SIOCGIWESSID:

	case SIOCSIWNICKN:

	case SIOCGIWNICKN:

		if (iwp->length == descr->max_tokens + 1)

			essid_compat = 1;

		else if (IW_IS_SET(cmd) && (iwp->length != 0)) {

			char essid[IW_ESSID_MAX_SIZE + 1];



			err = (MEMCPY(essid, iwp->pointer,

					     iwp->length *

					     descr->token_size) == NULL);

			if (err)

				return -EFAULT;



			if (essid[iwp->length - 1] == '\0')

				essid_compat = 1;

		}

		break;

	default:

		break;

	}



	iwp->length -= essid_compat;



	/* Check what user space is giving us */

	if (IW_IS_SET(cmd)) {

		/* Check NULL pointer */

		if (!iwp->pointer && iwp->length != 0)

			return -EFAULT;

		/* Check if number of token fits within bounds */

		if (iwp->length > descr->max_tokens)

			return -E2BIG;

		if (iwp->length < descr->min_tokens)

			return -EINVAL;

	} else {

		/* Check NULL pointer */

		if (!iwp->pointer)

			return -EFAULT;

		/* Save user space buffer size for checking */

		user_length = iwp->length;



		/* Don't check if user_length > max to allow forward

		 * compatibility. The test user_length < min is

		 * implied by the test at the end.

		 */



		/* Support for very large requests */

		if ((descr->flags & IW_DESCR_FLAG_NOMAX) &&

		    (user_length > descr->max_tokens)) {

			/* Allow userspace to GET more than max so

			 * we can support any size GET requests.

			 * There is still a limit : -ENOMEM.

			 */

			extra_size = user_length * descr->token_size;



			/* Note : user_length is originally a __u16,

			 * and token_size is controlled by us,

			 * so extra_size won't get negative and

			 * won't overflow...

			 */

		}

	}



	/* kzalloc() ensures NULL-termination for essid_compat. */

	extra = MALLOC(extra_size);

	if (!extra)

		return -ENOMEM;

	MEMSET(extra, 0, extra_size);



	/* If it is a SET, get all the extra data in here */

	if (IW_IS_SET(cmd) && (iwp->length != 0)) {

		if (MEMCPY(extra, iwp->pointer,

				   iwp->length *

				   descr->token_size) == NULL) {

			err = -EFAULT;

			goto out;

		}

	}



	err = handler(dev, info, (union iwreq_data *) iwp, extra);



	iwp->length += essid_compat;



	/* If we have something to return to the user */

	if (!err && IW_IS_GET(cmd)) {

		/* Check if there is enough buffer up there */

		if (user_length < iwp->length) {

			err = -E2BIG;

			goto out;

		}



		if (MEMCPY(iwp->pointer, extra,

				 iwp->length *

				 descr->token_size) == NULL) {

			err = -EFAULT;

			goto out;

		}

	}

#if 0   //LYH (not support yet)

	/* Generate an event to notify listeners of the change */

	if ((descr->flags & IW_DESCR_FLAG_EVENT) && err == -EIWCOMMIT) {

		union iwreq_data *data = (union iwreq_data *) iwp;



		if (descr->flags & IW_DESCR_FLAG_RESTRICT)

			/* If the event is restricted, don't

			 * export the payload.

			 */

			wireless_send_event(dev, cmd, data, NULL);

		else

			wireless_send_event(dev, cmd, data, extra);

	}

#endif



out:

	FREE(extra);

	return err;

}



/*

 * Wrapper to call a standard Wireless Extension handler.

 * We do various checks and also take care of moving data between

 * user space and kernel space.

 */

static int ioctl_standard_call(struct net_device *	dev,

			       struct iwreq		*iwr,

			       unsigned int		cmd,

			       struct iw_request_info	*info,

			       iw_handler		handler)

{

	const struct iw_ioctl_description *	descr;

	int					ret = -EINVAL;



	/* Get the description of the IOCTL */

	if ((cmd - SIOCIWFIRST) >= standard_ioctl_num)

		return -EOPNOTSUPP;

	descr = &(standard_ioctl[cmd - SIOCIWFIRST]);



	/* Check if we have a pointer to user space data or not */

	if (descr->header_type != IW_HEADER_TYPE_POINT) {



		/* No extra arguments. Trivial to handle */

		ret = handler(dev, info, &(iwr->u), NULL);

		#if 0   //LYH (not support yet)

		/* Generate an event to notify listeners of the change */

		if ((descr->flags & IW_DESCR_FLAG_EVENT) &&

		   ((ret == 0) || (ret == -EIWCOMMIT)))

			wireless_send_event(dev, cmd, &(iwr->u), NULL);

		#endif

	} else {

		ret = ioctl_standard_iw_point(&iwr->u.data, cmd, descr,

					      handler, dev, info);

	}



	/* Call commit handler if needed and defined */

	if (ret == -EIWCOMMIT)

		ret = call_commit_handler(dev);



	/* Here, we will generate the appropriate event if needed */



	return ret;

}



/* ---------------------------------------------------------------- */

/*

 * Wrapper to call a private Wireless Extension handler.

 * We do various checks and also take care of moving data between

 * user space and kernel space.

 * It's not as nice and slimline as the standard wrapper. The cause

 * is struct iw_priv_args, which was not really designed for the

 * job we are going here.

 *

 * IMPORTANT : This function prevent to set and get data on the same

 * IOCTL and enforce the SET/GET convention. Not doing it would be

 * far too hairy...

 * If you need to set and get data at the same time, please don't use

 * a iw_handler but process it in your ioctl handler (i.e. use the

 * old driver API).

 */

static int get_priv_descr_and_size(struct net_device *dev, unsigned int cmd,

				   const struct iw_priv_args **descrp)

{

	const struct iw_priv_args *descr;

	int i, extra_size;



	descr = NULL;

	for (i = 0; i < dev->wireless_handlers->num_private_args; i++) {

		if (cmd == dev->wireless_handlers->private_args[i].cmd) {

			descr = &dev->wireless_handlers->private_args[i];

			break;

		}

	}



	extra_size = 0;

	if (descr) {

		if (IW_IS_SET(cmd)) {

			int	offset = 0;	/* For sub-ioctls */

			/* Check for sub-ioctl handler */

			if (descr->name[0] == '\0')

				/* Reserve one int for sub-ioctl index */

				offset = sizeof(UINT32);



			/* Size of set arguments */

			extra_size = get_priv_size(descr->set_args);



			/* Does it fits in iwr ? */

			if ((descr->set_args & IW_PRIV_SIZE_FIXED) &&

			   ((extra_size + offset) <= IFNAMSIZ))

				extra_size = 0;

		} else {

			/* Size of get arguments */

			extra_size = get_priv_size(descr->get_args);



			/* Does it fits in iwr ? */

			if ((descr->get_args & IW_PRIV_SIZE_FIXED) &&

			   (extra_size <= IFNAMSIZ))

				extra_size = 0;

		}

	}

	*descrp = descr;

	return extra_size;

}



static int ioctl_private_iw_point(struct iw_point *iwp, unsigned int cmd,

				  const struct iw_priv_args *descr,

				  iw_handler handler, struct net_device *dev,

				  struct iw_request_info *info, int extra_size)

{

	char *extra;

	int err;



	/* Check what user space is giving us */

	if (IW_IS_SET(cmd)) {

		if (!iwp->pointer && iwp->length != 0)

			return -EFAULT;



		if (iwp->length > (descr->set_args & IW_PRIV_SIZE_MASK))

			return -E2BIG;

	} else if (!iwp->pointer)

		return -EFAULT;



	extra = MALLOC(extra_size);

	if (!extra)

		return -ENOMEM;

	MEMSET(extra, 0, extra_size);



	/* If it is a SET, get all the extra data in here */

	if (IW_IS_SET(cmd) && (iwp->length != 0)) {

		if (MEMCPY(extra, iwp->pointer, extra_size) == NULL) {

			err = -EFAULT;

			goto out;

		}

	}



	/* Call the handler */

	err = handler(dev, info, (union iwreq_data *) iwp, extra);



	/* If we have something to return to the user */

	if (!err && IW_IS_GET(cmd)) {

		/* Adjust for the actual length if it's variable,

		 * avoid leaking kernel bits outside.

		 */

		if (!(descr->get_args & IW_PRIV_SIZE_FIXED))

			extra_size = adjust_priv_size(descr->get_args, iwp);



		if (MEMCPY(iwp->pointer, extra, extra_size) == NULL)

			err =  -EFAULT;

	}



out:

	FREE(extra);

	return err;

}



static int ioctl_private_call(struct net_device *dev, struct iwreq *iwr,

			      unsigned int cmd, struct iw_request_info *info,

			      iw_handler handler)

{

	int extra_size = 0, ret = -EINVAL;

	const struct iw_priv_args *descr;



	extra_size = get_priv_descr_and_size(dev, cmd, &descr);



	/* Check if we have a pointer to user space data or not. */

	if (extra_size == 0) {

		/* No extra arguments. Trivial to handle */

		ret = handler(dev, info, &(iwr->u), (char *) &(iwr->u));

	} else {

		ret = ioctl_private_iw_point(&iwr->u.data, cmd, descr,

					     handler, dev, info, extra_size);

	}



	/* Call commit handler if needed and defined */

	if (ret == -EIWCOMMIT)

		ret = call_commit_handler(dev);



	return ret;

}



typedef int (*wext_ioctl_func)(struct net_device *, struct iwreq *,

			       unsigned int, struct iw_request_info *,

			       iw_handler);



/*

 * Main IOCTl dispatcher.

 * Check the type of IOCTL and call the appropriate wrapper...

 */

static int wireless_process_ioctl(struct net_device *net_dev, struct iwreq *iwr,

				  unsigned int cmd,

				  struct iw_request_info *info,

				  wext_ioctl_func standard,

				  wext_ioctl_func private)

{

	//struct iwreq *iwr = (struct iwreq *) ifr;

	struct net_device *dev;

	iw_handler	handler;



	/* Make sure the device exist */

	dev = net_dev;



	/* A bunch of special cases, then the generic case...

	 * Note that 'cmd' is already filtered in dev_ioctl() with

	 * (cmd >= SIOCIWFIRST && cmd <= SIOCIWLAST) */

	if (cmd == SIOCGIWSTATS)

		return standard(dev, iwr, cmd, info,

				&iw_handler_get_iwstats);



	if (cmd == SIOCGIWPRIV && dev->wireless_handlers)

		return standard(dev, iwr, cmd, info,

				&iw_handler_get_private);



#if 0

	/* Basic check */

	if (!netif_device_present(dev))

		return -ENODEV;

#endif



	/* New driver API : try to find the handler */

	handler = get_handler(dev, cmd);

	if (handler) {

		/* Standard and private are not the same */

		if (cmd < SIOCIWFIRSTPRIV)

			return standard(dev, iwr, cmd, info, handler);

		else

			return private(dev, iwr, cmd, info, handler);

	}

#if 0

	/* Old driver API : call driver ioctl handler */

	if (dev->do_ioctl)

		return dev->do_ioctl(dev, ifr, cmd);

#endif

	return -EOPNOTSUPP;

}



/* entry point from dev ioctl */

static int wext_ioctl_dispatch(struct net_device *net_dev, struct iwreq *iwr,

			       unsigned int cmd, struct iw_request_info *info,

			       wext_ioctl_func standard,

			       wext_ioctl_func private)

{

#if 0

	int ret = wext_permission_check(cmd);



	if (ret)

		return ret;



	dev_load(net, ifr->ifr_name);

	rtnl_lock();

	ret = wireless_process_ioctl(net, ifr, cmd, info, standard, private);

	rtnl_unlock();

#else

	int ret;



	ret = wireless_process_ioctl(net_dev, iwr, cmd, info, standard, private);

#endif

	return ret;

}



int wext_handle_ioctl(struct net_device *net_dev, struct iwreq *iwr, unsigned int cmd)

{

	struct iw_request_info info = { .cmd = cmd, .flags = 0 };

	int ret;



	ret = wext_ioctl_dispatch(net_dev, iwr, cmd, &info,

				  ioctl_standard_call,

				  ioctl_private_call);

#if 0   //no need to copy

	if (ret >= 0 &&

	    IW_IS_GET(cmd) &&

	    copy_to_user(arg, ifr, sizeof(struct iwreq)))

		return -EFAULT;

#endif



	return ret;

}

int dev_ioctl(int sock, unsigned int cmd, void *iface)
{
	struct net_device *wifi_dev = NULL;
	struct iwreq *iwr;	/* Use WiFi type to get dev */

	wifi_dev = (struct net_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_WIFI);
	if (wifi_dev == NULL) {
		return -1;	
	}			
	iwr = (struct iwreq*)iface;
	if (cmd >= SIOCIWFIRST && cmd <= SIOCIWLAST) {
		/* Take care of Wireless Extensions */
		return wext_handle_ioctl(wifi_dev, iwr, cmd);
	} else {
		/*Take care of other ioctl cmds */
		switch (cmd) {
			case SIOCGIFHWADDR:
				MEMCPY(iwr->ifr_ifrn.ifrn_name, wifi_dev->dev_addr, ETH_ALEN);
				break;
		} 
	}			
	/* Other cases */
	return 0;
}

#else

int wext_handle_ioctl(struct net_device *net_dev, struct iwreq *iwr, unsigned int cmd)

{

	return -EFAULT;

}

#endif


#endif


