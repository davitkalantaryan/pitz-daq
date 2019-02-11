/***************************************************************************** 
 * File		  daq.h
 * created on 26.01.2012
 *****************************************************************************
 * Author:	M.Eng. Dipl.-Ing(FH) Marek Penno, EL/1L23, Tel:033762/77275 marekp
 * Email:	marek.penno@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 * 
 * Changes:
 *  added i4_mod_fadc_toData(val) to readout of fadc/powermeter
 *  added hi peak, lo peak to analog channel
 *  2013-08-20 mp added latched status to controller readout
 *  2014-04-10	MP	added support for snapshot counter
 ****************************************************************************/

#ifndef __I4_DAQ_H__
#define __I4_DAQ_H__



// Interlock Status Structure

// --------------------------------------------- ANAIN / ANAIO Module

#define ANAIN_MODULE_COUNT	8
#define ANAIO_MODULE_COUNT	8


// new
#define I4_MAX_CHANNEL_COUNT	24
#define ANAIN_ADC_CHANNEL_COUNT	16
#define ANAIO_ADC_CHANNEL_COUNT	16
#define ANAIO_DAC_CHANNEL_COUNT	16
#define FADC_ADC_CHANNEL_COUNT	16
#define I4_SLOT_COUNT			13
#define	FADC_MAX_SAMPLES		4098
#define CTRL_FIFO_SIZE			1024
#include <sys/types.h>
#ifndef _KERNEL_TYPES_DEFINED
#include <stdint.h>
typedef uint8_t   u8_t;
typedef uint16_t  u16_t;
typedef uint32_t  u32_t;
typedef uint64_t  u64_t;
typedef int		  bool_t;
#else
#endif

typedef u64_t peakmode_t;
typedef u64_t anaio_dac2_mode_t;

// end new

typedef struct intlk_idrom_s {
	u16_t  val[6];
}intlk_idrom_t;

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct {
	intlk_idrom_t idrom;							// 6*2 = 12
	bool_t		  isPresent;						// +4  = 16
	bool_t		  hasEventStatus;					// +4  = 20
	u32_t 		  inputChannelCount;				// +4  = 24

	u32_t originalInputStatus;						// +4  = 28
	u32_t inputStatus;								// +4  = 32
	u32_t outputStatus;								// +4  = 36
	u32_t masks;									// +4  = 40
	u32_t statusLatchEnable; 						// +4  = 44

	u16_t filterValues[I4_MAX_CHANNEL_COUNT];		//+2*24= 92

	u32_t mclockOutputMask;							// +4  = 96

	u16_t mclockInput;								// +2  = 98
	u16_t reprate;									// +2  = 100

} i4_mod_t; // keep it 4 byte aligned !

typedef struct {
	u16_t index;
	u8_t  counter;
	u8_t  hasInterlock;
} i4_error_event_counter_t;

#define DAQ_MAX_ERROR_EVENT_COUNTER 128
// module readout



// Structure to keep data of one adc channel
typedef struct {
	u32_t value;
	u32_t valueHi;
	u32_t valueLo;
	u32_t min;
	u32_t max;
	peakmode_t peakMode;
} intlk4_adc_channel_t;

typedef struct {
	u16_t 	onValue;
	u16_t 	standbyValue;
	u16_t 	step;
	u16_t	reserved;
	bool_t 	isRampEnabled;
	anaio_dac2_mode_t mode;
} intlk4_dac2_channel_t;

// Structure to keep data of one adc module
typedef struct {
	u32_t slot;
	intlk4_adc_channel_t 	channel[ANAIN_ADC_CHANNEL_COUNT];
} i4_mod_anain_t;

// Structure to keep data of one adc module
typedef struct {
	u32_t slot;
	intlk4_adc_channel_t 	adc[ANAIO_ADC_CHANNEL_COUNT];
	u32_t 					dac[ANAIO_DAC_CHANNEL_COUNT];
	intlk4_dac2_channel_t	rampConfig[ANAIO_DAC_CHANNEL_COUNT];
} i4_mod_anaio_t;

// --------------------------------------------- FADC Module

#define FADC_MODULE_COUNT	4

// Structure to keep data of one fadc channe;
typedef struct {
	u32_t threshold;	// actual threshold
	u32_t val;
	u32_t count;
	u16_t data[FADC_MAX_SAMPLES*4]; // ## warum *4 ?
	bool_t acqActive;
} i4_mod_fadc_channel_t;

#define FADC_ERROR_NONE			0
#define FADC_ERROR_DATA_MISSING	1
#define FADC_ERROR_NO_FIFO		2

// Structure to keep data of all fadc channels of one module
typedef struct {
	u32_t magic;
	u32_t slot;
	u32_t startTime;
	u32_t waitCycles;
	u32_t sampleCount;
	// error code on read out
	u32_t error;
	// fifo words skipped to find start tag
	u32_t skippedData;
	//u32_t fifoCounts[CTRL_FIFO_SIZE];
	u16_t fifoData[CTRL_FIFO_SIZE];
	u32_t fifoDataCount;
	i4_mod_fadc_channel_t channel[FADC_ADC_CHANNEL_COUNT];
} i4_mod_fadc_t;

// --------------------------------------------- Light Module

#define LIGHIO_MODULE_COUNT		4

// Structure to keep data of all fadc channels of one module
typedef struct {
	u32_t slot;
	u32_t eventNumber;
}  i4_mod_lightio_t;

// --------------------------------------------- Interlock Data

typedef struct {
	bool_t hasEvent;
	u32_t error_status;
	u32_t mclock_timestamp;
	u32_t mclock_counter;
} i4_first_event_data_t;

typedef struct {
	u32_t slot;
	u32_t error_status;
	u32_t mclock_timestamp;
	u16_t isEvent;  // not bool_t to save space and to keep 32bit word alignment
	u16_t isMClock;
} i4_status_event_data_t;

#define I4_EVENTDATA_COUNT 500

// Structure to hold Interlock Status Data
typedef struct {
	// Event Number
	u32_t	eventNumber; // contains event number from system (or irq counts, if mclock is not configured)
	// Masks per Module
	u32_t	masks[I4_SLOT_COUNT];
	// Input Status per Module
	u32_t	inputStatus[I4_SLOT_COUNT];
	// Output Status per Module
	u32_t	outputStatus[I4_SLOT_COUNT];
	// Slave SRQ Vector
	u32_t	srqLines;
	// Latched Input Status per Module
	u32_t	latchedInputStatus[I4_SLOT_COUNT];
	// ms snapshot counter
	u32_t	eventCounterMs;
	// mclock delay counter
	u32_t	mclockDelayCounter;

	// controller mclock counter (added with firmware ctrl_d409)
	u32_t	eventCounter;

	// added with genio_dx24

	i4_first_event_data_t firstEventStatus[I4_SLOT_COUNT];
	i4_first_event_data_t latchedFirstEventStatus[I4_SLOT_COUNT];


} i4_mod_ctrl_t;

typedef struct {
	// list of modules to receive data from
	u8_t   anaioSlots[ANAIO_MODULE_COUNT];
	u8_t   anainSlots[ANAIN_MODULE_COUNT];
	u8_t   fadcSlots[FADC_MODULE_COUNT];
	u8_t   lightioSlots[LIGHIO_MODULE_COUNT];

	// error event counter support
	u16_t  ecIndexes[DAQ_MAX_ERROR_EVENT_COUNTER];

} i4_snapshot_cfg_t ;

// Structure to hold all Interlock relevant data for a machine pulse
struct i4_snapshot_s {
	u32_t 			structSize;
	// Controller Data
	i4_mod_ctrl_t 	ctrlModule;
	// Fast ADC Data
	i4_mod_fadc_t 	fadcModule[FADC_MODULE_COUNT];
	// anain data
	i4_mod_anain_t 	anainModule[ANAIN_MODULE_COUNT];
	i4_mod_anaio_t 	anaioModule[ANAIO_MODULE_COUNT];

	// added with genio_dx24
	i4_status_event_data_t eventData[I4_EVENTDATA_COUNT];
	u32_t eventDataCount;

	i4_mod_t modules[I4_SLOT_COUNT];

	u16_t errorEventCounterCount;
	i4_error_event_counter_t errorCounter[DAQ_MAX_ERROR_EVENT_COUNTER];

};

typedef struct i4_snapshot_s i4_snapshot_t;

//// Structure to hold all Interlock relevant data for a machine pulse
//struct i4_snapshot2_s {
//	u32_t 			structSize;
//	// Controller Data
//	i4_mod_ctrl_t 	ctrlModule;
//	// Fast ADC Data
//	i4_mod_fadc_t 	fadcModule[FADC_MODULE_COUNT];
//	// anain data
//	i4_mod_anain_t 	anainModule[ANAIN_MODULE_COUNT];
//	i4_mod_anaio_t 	anaioModule[ANAIO_MODULE_COUNT];
//
//	// added with genio_dx24
//	i4_status_event_data_t eventData[I4_EVENTDATA_COUNT];
//	u32_t eventDataCount;
//
//};

// --------------------------------- Ctrl Readout



// --------------------------------- ADC Readout

// reads out the relevant adc channels of the adc module


// reads out the relevant adc channels of the anaio module

// --------------------------------- FADC Readout

// reads out the relevant adc channels of the anaio module


// reads out the relevant adc channels of the anaio module
// fast variant, saves


// --------------------------------- FADC Readout

// reads out the relevant adc channels of the anaio module


// ---------------------------------



#ifdef __cplusplus
	}
#endif


#endif /* __I4_DAQ_H__ */
