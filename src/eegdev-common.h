#ifndef EEGDEV_COMMON_H
#define EEGDEV_COMMON_H

/*
 * This file declares common structures that are used in the implementation
 * But that are not exported
 */

 
#include <pthread.h>

#include "eegdev.h"
#include "eegdev-types.h"



// The structure containing the pointer to the methods of the EEG devices
struct eegdev_operations {
/* \param dev	pointer to the eegdev struct of the device
 *
 * Should close the device and free all associated resources. When this
 * function is called, the device implementation should assume that all
 * resources that have not been directly allocated by the device
 * implementation (like those allocated by init_eegdev) have been already
 * freed.
 *
 * Should returns 0 in case of success or -1 if an error occurred (errno
 * should then be set accordingly) */
	int (*close_device)(struct eegdev* dev);


/* \param dev	pointer to the eegdev struct of the device
 * \param ngrp	number of group supplied in the grp array
 * \param grp	pointer to an array of grpconf
 *
 * Called soon after the user has called egd_set_groups. The device
 * implementation can assume that this function will never be called during
 * acquisition (i.e. not between egd_start() and egd_stop())
 *
 * IMPORTANT: in case of success, the device implementation should set
 * dev->in_samlen before returning (unless this has been set once for all
 * at the creation of dev) as well as dev->selch corresponding to the
 * settings describing the transfer between the incoming data from the EEG
 * system to the ringbuffer. The device implementation should assume that
 * dev->selch will allocated before the method is called.
 *
 * Should returns 0 in case of success or -1 if an error occurred (errno
 * should then be set accordingly) */
	int (*set_channel_groups)(struct eegdev* dev, unsigned int ngrp,
					const struct grpconf* grp);


/* \param dev	pointer to the eegdev struct of the device
 *
 * Called when the acquisition is about to start.
 *
 * Should returns 0 in case of success or -1 if an error occurred (errno
 * should then be set accordingly) */
	int (*start_acq)(struct eegdev* dev);


/* \param dev	pointer to the eegdev struct of the device
 *
 * Called when the acquisition is about to stop.
 *
 * Should returns 0 in case of success or -1 if an error occurred (errno
 * should then be set accordingly) */
	int (*stop_acq)(struct eegdev* dev);
};


/* \param dev		pointer to the eegdev struct of the device
 * \param in		pointer to an array of samples
 * \param length	size in bytes of the array
 *
 * update_ringbuffer() should be called by the device implementation
 * whenever a new piece of data is available. This function updates the
 * ringbuffer with the data pointed by the pointer in. The array can be
 * incomplete, i.e. it can start and end at a position not corresponding to
 * a boundary of a samples. */
void update_ringbuffer(struct eegdev* dev, const void* in, size_t length);


/* \param dev		pointer to the eegdev struct of the device
 * \param ops		pointer to an structure holding the methods
 *
 * Initialize the eegdev structure pointed by dev and set the methods of
 * the device according to ops.
 *
 * Returns 0 in case of success or -1 if an error occurred (errno is then
 * set accordingly) */
int init_eegdev(struct eegdev* dev, const struct eegdev_operations* ops);


struct selected_channels {
	unsigned int in_offset;
	unsigned int len;
	unsigned int buff_offset;
	union scale sc;
	cast_function cast_fn;
};

struct array_config {
	unsigned int iarray;
	unsigned int arr_offset;
	unsigned int buff_offset;
	unsigned int len;
};

struct eegdev {
	const struct eegdev_operations ops;
	struct systemcap cap;

	char* buffer;
	size_t buffsize, in_samlen, buff_samlen, in_offset;
	unsigned int ind, last_read, req_to_read;
	unsigned long ns_written, ns_read;
	pthread_mutex_t synclock;
	pthread_cond_t available;
	int acq;

	unsigned int narr;
	size_t *strides;

	unsigned int nsel, nconf;
	struct selected_channels* selch;
	struct array_config* arrconf;
};

#endif //EEGDEV_COMMON_H