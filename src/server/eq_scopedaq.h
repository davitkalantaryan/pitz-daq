/**
 * @file eq_scopedaq.h
 *
 * FastADC_SCOPE Eq function class
 * @date 12.6.2004
 * @author O.Hensler
 */

#ifndef eq_scopedaq_h
#define eq_scopedaq_h

#include	"eq_fct.h"
#include "eq_client.h"
#ifdef SOLARIS
#include "solaris_timer_io.h"
#else
//#include "linux_timer_io.h"
#include <linux/ioctl.h>
#endif

#include	"ScopeAdc.h"

#define MAX_BUFFER_NO	16
#define MAX_CH_NUM      64
#define MAX_BRD_NUMBERS  16

#define	Codescopedaq  301

enum spect_type { spect_raw, spect_ampl, 
                  spect_phase, spect_bpm_ampl, 
		  spect_bpm_phase, spect_bpm_ampl_DX,
                  spect_bpm_ampl_DY, spec_mcp, spec_torr 
};

class       EqFctADC;

class D_Command : public D_int
{
public:
	EqFctADC* eq_fct_;
	D_Command(char* pn, EqFctADC* eq_fct);
	void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
};

class myClass
{
public:
	EqFctADC* eq_fct_;

	myClass(int,EqFctADC*);
//	void  read_adcL();
	void  read_adcL( ADC_MAP_Helper<ADC_MAP_CLT>* a_pAdcmap, int& a_nPrevEvent );		
	int  	read_shm_buf_rwL(float *usr_buf, int buf_num, int ch_num,
		                   int usr_start, int usr_smpl, int usr_step);	
    void  evaluateL(int chanNum,int i1_data, float f1_data, float f2_data, float f3_data,
								 float* src, float* dst, int smp);

	int            BOARD_NUMBER;
	
	int            tmp_gen_eventL;
	int            tmp_cur_bufL;
	int            tmp_secondsL;	

	int            gen_expected;

//==	ADC_MAP_CLT::refresh_for_curbuf()
	
	int         mperror;
	int         tmp_brd_num;
	int         tmp_ch_num;
	int         tmp_maxsmpl;
	int         tmp_ch_size;
	int         tmp_bunch_size;
	int         tmp_allchn_size;
	int         tmp_sampl_set[MAX_BRD_NUM];
	int         tmp_brd_channel[MAX_BRD_NUM];
	int         tmp_brd_type[MAX_BRD_NUM];
	char        tmp[80];
	int         tmp_ch;
	int	      tmp_cur_buf; 
	int         tmp_cur_event;
	int         tmp_gen_event;
	int         tmp_gen_even_errt;
	int         tmp_brd_err;
	int         tmp_cur_sts;

	struct  timeval   tmp_tp;	
		
//==ADC_MAP_CLT::read_shm_buf_rw		
   float   tm_fvalue;
   int     tm_ivalue;
   u_short tm_svalue;
   int     shmem_offset;
   int     bunch_offset;
   int     cur_brd;
   int     cur_brd_chan;
   int     all_chan;
   int     all_chan_prev;
   int     chan_inboard[MAX_BRD_NUM];
   int     tmp_adc_offset;
   int     tmp_smpl;
   int     tmp_smp_num;
   int     smpl_read;
   int     tmp_max_smpl;
   int     tmp_brd_typeL;
   int     MAX_SAMPLES;
   int     rr;	
//==		

};

// =========================THE MAIN EqFct CLASS=====================

class EqFctADC : public EqFct {

public:
	D_int			is_interrupt_;
	
	D_int      	*adcscope_nm[MAX_CH_NUM];	
	D_polynom  	*adcscope_poly[MAX_CH_NUM];	
	D_Command	  *daq_start[MAX_CH_NUM];
	
	
	int   BLOCK_rdbk_thread_id[MAX_CH_NUM + 1];	
	int         start_inter[MAX_CH_NUM + 1];		
	
	myClass        *myclass[MAX_CH_NUM + 1];
	pthread_t       thread_id[MAX_CH_NUM + 1];	
	
	pthread_t       SEND_thread;		
	int  BLOCK_rdbk_SENDpthread;		
	
	pthread_t       PROP_thread;
	int  BLOCK_rdbk_PROPpthread;		
	
	IFFF         IFFFL;	
public:

	EqFctADC();	
	void		interrupt_usr1 	(int signo);	// called on interrupt
	void		update 		();		// called on timer
	void		init 		();		// started after creation of all Eq's
	int		fct_code()	{ return Codescopedaq; }
	void		post_init();
	
	static int	conf_done;

};


#endif
