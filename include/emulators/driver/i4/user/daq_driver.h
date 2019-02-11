/***************************************************************************** 
 * File		  Intlk4DaqDriver.h
 * created on 10.02.2012
 *****************************************************************************
 * Author:	M.Eng. Dipl.-Ing(FH) Marek Penno, EL/1L23, Tel:033762/77275 marekp
 * Email:	marek.penno@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 * 
 ****************************************************************************/

#ifndef I4_USER_DAQ_DRIVER_H_
#define I4_USER_DAQ_DRIVER_H_

#include <sys/types.h>
#include "driver/i4/hal/daq.h"

#define	ERROR_NONE	0
#define I4_DEV_DAQ "/dev/intlkdaq0"

typedef struct {
	// file handle
	int daq_handle;
	i4_snapshot_t* daq_snapshot;
} i4_daq_driver_t;


// opening the device
static inline int i4_daq_open(const char*)
{
	return 0;
}

// close the device
static inline void i4_daq_close() {}

// ioctl based functions

// returns 1 if driver is already open
//int i4_daq_isOpen();

// wait for a irq

// wait for a irq
static inline int i4_daq_waitIrq(unsigned int a_timeout)
{
#ifdef _WIN32
	Sleep(a_timeout);
#else
	usleep(a_timeout*1000);
#endif
	return 0;
}


static inline i4_snapshot_t* i4_daq_getSnapshot()
{
	static i4_snapshot_t  aSnapshot;
	return &aSnapshot;
}


#endif /* INTLK4DAQDRIVER_H_ */
