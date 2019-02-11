// file scopedaq_rpc_server.cc
//
// ADC Eq function class
//
// Levon Hakobyan,
//
// Modified by D. Kalantaryan 2014.08.15
//       to fix problem with 180 spectrum loses
//
// Modified by D. Kalantaryan 2016.01.14
//       to fix ...
//

//#define     ADD_PROPERTY_DIRECT

#include <iostream>
#include	<cstdio>
#include	<cstdlib>

#include	<sys/time.h>
#include	<signal.h>
#include	<setjmp.h>
#include	"eq_errors.h"
#include	"eq_fct_errors.h"
#include	"eq_sts_codes.h"
#include	"printtostderr.h"
#include	<math.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/ioctl.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>

#ifdef SOLARIS
#include "solaris_timer_io.h"
#else
//#include "linux_timer_io.h"
#include <linux/ioctl.h>
#endif

#include "MCclass.h"
#include	"ADCDma.h"
#include "adc_map_helper.h"
#include	"eq_scopedaq.h"

#define     ADC_STORE		"ttf_scopedaq.hist"	// file name:
#define     ADC_SPECT		"ttf_scopedaq.spect"
#define     ADC_DEV		"ttf.scope.adc"		// group names :

using namespace std;

time_t				walltime;

const char* LN = "#_QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";

extern "C" void* fill_thread(void *);
extern "C" void* send_thread(void *);
extern "C" void* prop_thread(void *);

extern ADC_MAP_CLT		adc_map_;

struct  conf_struct
{
	int    conf_nm;
	char   conf_fac_loc[40];
	int    poly1;
	float  poly2;
	float  poly3;
	float  poly4;
	int    conf_daq_start;	
};
	
//===================================================

const char*				object_name = "SCOPE_DAQ";	// name of this object (used in error messages)
EqFct*				adc_fct = 0;

int				EqFctADC::conf_done = 0;

extern	int			fct_code;
extern	int			ring_buffer;

extern	std::vector<EqFct*>	*eq_list;
extern	Config* 		config;

int  CONF_PROPERTIES;

conf_struct   conf_[MAX_CH_NUM];
conf_struct   confcp_[MAX_CH_NUM];
FILE*         config_ptr;

int      check_shmem_ptr = 1;
int      tmp_id;
int      check_reconfig;
int      prop_cond_ = 0;

EqFctADC* pmain;

const int EVENTS = 80;

char         genptr[400]; 
char         sigptr[400];
char         davptr[400];
int          sigres; 
char         charptr[400];
char         charptr0[400];
int          check_number;
int          res;	

const int    MAX_BUFFER = MAX_BUFFER_NO;


//===================================================

struct DATA_struct
{
	int              endian;
	int              branch_num;	
	int              seconds;
	int              gen_event;
	int              samples;
	float            f[2048];
};
const int DATA_struct_l = sizeof(struct DATA_struct);
union DATA_union
{
	char ch[DATA_struct_l];
	struct DATA_struct data_struct;
};

union DATA_union  data_union[MAX_CH_NUM][EVENTS];
int               r_s[MAX_CH_NUM];
int               s_s[MAX_CH_NUM];

//MCsender   *sender = new MCsender(false);
MCsender   *sender = new MCsender;


//===================================================

D_Command::D_Command(char* pn, EqFctADC* a_eq_fct) : D_int(pn,a_eq_fct),eq_fct_(a_eq_fct)
{
}

void D_Command::set(EqAdr*, EqData* sed, EqData* /*red*/, EqFct *)
{
	switch ( sed->get_int() ) 
	{
  	case 0 :                                 
                        if (get_access()) set_value(0);
    break;
	 	
  	case 1 :                                 
                        if (get_access()) set_value(1);
    break;

  	default :
                        if (get_access()) set_value(0);
		break;
	}
	return;
}


myClass::myClass(int BN,EqFctADC* eq_fct) : eq_fct_(eq_fct),BOARD_NUMBER(BN)
{
	gen_expected = 0;
}


//void myClass::read_adcL()
void myClass::read_adcL( ADC_MAP_Helper<ADC_MAP_CLT>* a_pAdcmap, int& a_nPrevEvent )
{
	int      ch_num =0;
	
	mperror           = 0;
   tmp_brd_num       = 0;
   tmp_ch_num        = 0;
   tmp_maxsmpl       = 0;
   tmp_ch_size       = 0;
   tmp_bunch_size    = 0;
   tmp_allchn_size   = 0;

   tmp_cur_buf       = 0; 
   tmp_cur_event     = 0;
   tmp_gen_event     = 0;
   tmp_gen_even_errt = 0;
   tmp_brd_err       = 0;
   tmp_cur_sts       = 0;	

	tmp_cur_buf        = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_buf_num);
	tmp_gen_event      = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_event) ;
	tmp_gen_even_errt  = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_gen_event_err);
	tmp_brd_num        = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_brd_num);
	tmp_ch_num         = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_cnls_num);
	tmp_maxsmpl        = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_maxsmpl);
	tmp_ch_size        = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_ch_size);
	tmp_bunch_size     = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_bunch_size);
	tmp_allchn_size    = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_all_size);	
	
	for (int k =0; k < MAX_BRD_NUM; k++)
	{
		tmp_ch = 0;
		tmp_sampl_set[k] = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_smpl_num_0 + k*sizeof(int));
		tmp_ch = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_brch_num_0 + k*sizeof(int));
		tmp_brd_channel[k] = tmp_ch & 0xFFFF;
		tmp_brd_type[k]    = (tmp_ch>>16)&0xFFFF;
	}	
	
	tmp_cur_event  = *(u_int *)((u_char *)adc_map_.shmem_ptr + (tmp_cur_buf * tmp_bunch_size) + shm_off_event) ;
	tmp_brd_err    = *(u_int *)((u_char *)adc_map_.shmem_ptr + (tmp_cur_buf * tmp_bunch_size) + shm_off_brd_err);
	tmp_cur_sts    = *(u_int *)((u_char *)adc_map_.shmem_ptr + (tmp_cur_buf * tmp_bunch_size) + shm_off_chsts);
	tmp_tp.tv_sec  = *(u_int *)((u_char *)adc_map_.shmem_ptr + (tmp_cur_buf * tmp_bunch_size) + shm_off_time_stamp_sec);
	tmp_tp.tv_usec = *(u_int *)((u_char *)adc_map_.shmem_ptr + (tmp_cur_buf * tmp_bunch_size) + shm_off_time_stamp_usec);		


	if(prop_cond_ == 0)
	{
		conf_[BOARD_NUMBER].conf_nm			=	confcp_[BOARD_NUMBER].conf_nm;
		sprintf(conf_[BOARD_NUMBER].conf_fac_loc,"%s",confcp_[BOARD_NUMBER].conf_fac_loc);
		conf_[BOARD_NUMBER].poly1				=	confcp_[BOARD_NUMBER].poly1;
		conf_[BOARD_NUMBER].poly2				=	confcp_[BOARD_NUMBER].poly2;
		conf_[BOARD_NUMBER].poly3				=	confcp_[BOARD_NUMBER].poly3;
		conf_[BOARD_NUMBER].poly4				=	confcp_[BOARD_NUMBER].poly4;														  
		conf_[BOARD_NUMBER].conf_daq_start	=	confcp_[BOARD_NUMBER].conf_daq_start;	
	}

	ch_num = conf_[BOARD_NUMBER].conf_nm;

	tmp_gen_eventL = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_event) ;
	tmp_cur_bufL   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_gen_buf_num);
	tmp_secondsL   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_time_stamp_sec);
	
	ADC_MAP_Helper<ADC_MAP_CLT>& aAdcMap = *a_pAdcmap;	
	
	
	while( aAdcMap.GetBufferAndEvent(&tmp_cur_bufL, &tmp_gen_eventL) )
	{
		read_shm_buf_rwL(data_union[ch_num][r_s[ch_num]].data_struct.f, tmp_cur_bufL, ch_num, 0, 2048, 1);

		data_union[ch_num][r_s[ch_num]].data_struct.endian = 1;
		data_union[ch_num][r_s[ch_num]].data_struct.branch_num = ch_num;

		data_union[ch_num][r_s[ch_num]].data_struct.seconds   = tmp_secondsL;
		data_union[ch_num][r_s[ch_num]].data_struct.gen_event = tmp_gen_eventL;
		data_union[ch_num][r_s[ch_num]].data_struct.samples = 2048;

        evaluateL(ch_num,conf_[ch_num].poly1,
					 conf_[ch_num].poly2,
					 conf_[ch_num].poly3,
					 conf_[ch_num].poly4,
					 data_union[ch_num][r_s[ch_num]].data_struct.f,
					 data_union[ch_num][r_s[ch_num]].data_struct.f,
					 2048);

		if( r_s[ch_num] < (EVENTS - 1) )
		{
			r_s[ch_num] = r_s[ch_num] + 1;
		}
		else
		{
			r_s[ch_num] = 0;
		}

		if( a_nPrevEvent>0 && (tmp_gen_eventL-a_nPrevEvent)!=1)
		{
		   sprintf(davptr,"NEW gen event: %d     OLD gen event: %d",tmp_gen_eventL,a_nPrevEvent);	
			printtostderr("ERROR",davptr);
		}
		a_nPrevEvent = tmp_gen_eventL;
	}	
	
	return ;	
}


int  myClass::read_shm_buf_rwL(float *usr_buf, int buf_num, int ch_num,
                                int usr_start, int usr_smpl, int usr_step )
{

   tm_fvalue       = 0;
   tm_ivalue       = 0;
   tm_svalue       = 0;
   shmem_offset    = 0;    //????
   bunch_offset    = 0;    //????
   cur_brd        = 0;
   cur_brd_chan   = 0;
   all_chan       = 0;
   all_chan_prev  = 0;
//   int     chan_inboard[MAX_BRD_NUM];
   tmp_adc_offset = 0;
   tmp_smpl       = 0;
   tmp_smp_num    = 0;    //????
   smpl_read      = 0;
   tmp_max_smpl   = 0;
   tmp_brd_typeL  = 0;
   MAX_SAMPLES    = 0;
   rr             = 0;



   buf_num = buf_num & 0xF;
   if (ch_num < 0) ch_num = 0;
	
	for(int i = 0; i< MAX_BRD_NUM; i++)
	{
   	chan_inboard[i] = tmp_brd_channel[i];
   	all_chan += tmp_brd_channel[i];
		if(ch_num < all_chan)
		{
			cur_brd = i;
			cur_brd_chan = ch_num - all_chan_prev;
			break;
		}
   	all_chan_prev = all_chan;
	}
	bunch_offset   = tmp_bunch_size*buf_num;
	tmp_max_smpl   = tmp_maxsmpl;
	MAX_SAMPLES    = tmp_maxsmpl;
	for(int i = 0; i < cur_brd; i++)
	{
   	tmp_adc_offset += tmp_brd_channel[i]* tmp_ch_size;
	}
	tmp_smpl     = tmp_sampl_set[cur_brd];
	tmp_brd_typeL = tmp_brd_type[cur_brd];

   shmem_offset = (bunch_offset + CTRL_STS_MEM + tmp_adc_offset);

   tmp_smp_num = 0;   


   switch(tmp_brd_typeL){
   case FAST_ADC:
	
       shmem_offset += (((cur_brd_chan/2*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
	   if((tmp_smp_num + usr_start) > tmp_smpl) break;
	   tm_ivalue = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
	   if(!(ch_num%2)){
	       tm_ivalue = tm_ivalue>>16;
	       tm_ivalue &= 0x3fff;
	   }
	   else {
	       tm_ivalue &= 0x3fff;
	   }
	   usr_buf[k] = (float)tm_ivalue;
	   tmp_smp_num += usr_step;
	   smpl_read ++;
       }
       break;
   case SIS3300:
       shmem_offset += (((cur_brd_chan/2*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
	   if((tmp_smp_num + usr_start) > tmp_smpl) break;
	   tm_ivalue = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
	   if(!(ch_num%2)){ 
	       tm_ivalue = tm_ivalue>>16;
	       tm_ivalue &= 0x3fff;
	   }
	   else {
	       tm_ivalue &= 0x3fff;
	   }
	   usr_buf[k] = (float)tm_ivalue;
	   tmp_smp_num += usr_step;
	   smpl_read ++;
       }
       break;
   case FNAL1_ADC:
       shmem_offset += (((cur_brd_chan*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
	   if((tmp_smp_num + usr_start) > tmp_smpl) break;
	   tm_ivalue   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
	   if (tm_ivalue&0x800) tm_ivalue|=0xfffff000;
	   usr_buf[k] = (float)tm_ivalue;				
	   tmp_smp_num += usr_step;
	   smpl_read ++;
       }
       break;
   case COMMET_ADC:
       shmem_offset += ((cur_brd_chan*MAX_SAMPLES)*sizeof(int)+usr_start*sizeof(short));
       for(int k = 0; k < usr_smpl; k++){
	   if((tmp_smp_num + usr_start) > tmp_smpl) break;
	   tm_svalue = *(u_short *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(short))) ;
	   tm_svalue = (tm_svalue / 8 ) & 0xfff;
	   usr_buf[k] = (float)tm_svalue;
	   tmp_smp_num += usr_step;
	   smpl_read ++;
       }
       break;
   case ACQIRIS1:
       shmem_offset += (((cur_brd_chan*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
           if((tmp_smp_num + usr_start) > tmp_smpl) break;
           tm_fvalue   = *(float *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
           usr_buf[k] = (float)tm_fvalue;				
           tmp_smp_num += usr_step;
           smpl_read ++;
       }
       break;
   case DAMC:
       shmem_offset += (((cur_brd_chan*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
           if((tmp_smp_num + usr_start) > tmp_smpl) break;
           tm_ivalue   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
           tm_ivalue &= 0xffff;
           usr_buf[k] = (float)tm_ivalue;				
           tmp_smp_num += usr_step;
           smpl_read ++;
       }
       break;
   case TAMC900:
       shmem_offset += (((cur_brd_chan*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
           if((tmp_smp_num + usr_start) > tmp_smpl) break;
           tm_ivalue   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
           tm_ivalue &= 0x3fff;
           usr_buf[k] = (float)tm_ivalue;				
           tmp_smp_num += usr_step;
           smpl_read ++;
       }
       break;
/*	 
   case SIS8300:
       shmem_offset += (((cur_brd_chan*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
           if((tmp_smp_num + usr_start) > tmp_smpl) break;
           tm_ivalue   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
           tm_ivalue &= 0xffff;
           usr_buf[k] = (float)tm_ivalue;
           tmp_smp_num += usr_step;
           smpl_read ++;
       }
       break;
   case LLRF_SIS8300:
       shmem_offset += (((cur_brd_chan*MAX_SAMPLES)+usr_start)*sizeof(int));
       for(int k = 0; k < usr_smpl; k++){
           if((tmp_smp_num + usr_start) > tmp_smpl) break;
           tm_ivalue   = *(u_int *)((u_char *)adc_map_.shmem_ptr + shmem_offset + (tmp_smp_num*sizeof(int))) ;
           tm_ivalue &= 0xffff;
           usr_buf[k] = (float)tm_ivalue;
           tmp_smp_num += usr_step;
           smpl_read ++;
       }
       break;
*/	 		 
   default:
       break;
   }
   return smpl_read;
}


void  myClass::evaluateL(int a_chanNum,int i1_data, float f1_data, float f2_data, float f3_data, float* src, float* dst, int smp)
{
	float		x;
	float		f;
    float fMeanOfSpectrum(0.0f);
	double	d;
	int      boolL;

    //if(a_chanNum==32){printf("!!!!!!!!!!!! i1_data: %d\n",i1_data);}
		
	if(smp > 0)
	{
		switch(i1_data) 
		{
		case 0:
			for(int i=0; i < smp; i++)
			{
				x = src[i];
				f = x;
				dst[i] = f;
            }
            //fMeanOfSpectrum /= (float)smp;
            //if(a_chanNum==32){printf("!!!!!!!!!!!! float after polypara: %f\n",fMeanOfSpectrum);}
            //printf("!!!!!!!!!!!! chanNumber=%d, float after polypara: %f\n",a_chanNum,fMeanOfSpectrum);
			return;

		case 1:
			for(int i=0; i < smp; i++)
			{
				x = src[i];		
				f = f1_data + f2_data * x + f3_data * x * x;
				dst[i] = f;
                fMeanOfSpectrum += f;
            }
            fMeanOfSpectrum /= (float)smp;
            if(a_chanNum==32){printf("!!!!!!!!!!!! float after polypara: %f\n",fMeanOfSpectrum);}
			return;

		case 2:
			for(int i=0; i < smp; i++)
			{
				x = src[i];			
				f = f1_data + f2_data * x + f3_data * x * x;
				dst[i] = (float) exp ((double) f);
			}
			return;

		case 3:
			for(int i=0; i < smp; i++)
			{
				x = src[i];		
				f = f1_data + f2_data * x;
				f = f * f + f3_data;
				dst[i] = f;
			}				
			return;

		case 4:
			for(int i=0; i < smp; i++)
			{
				x = src[i];		
				d = f1_data + f2_data * x;
				if (d > 0.0) f = log ((double) d)  * f3_data;
				else
   			 if (d < 0.0) f = log ((double) -d) * f3_data;
				else f = 0.0;
				dst[i] = f;
			}				
			return;

		case 5:
			// Third order polynom, without offset
			for(int i=0; i < smp; i++)
			{
				x = src[i];	
				f = (f1_data + f2_data * x + f3_data * x * x) * x;
				dst[i] = f;
			}								
			return;
			
      case 6:
			for(int i=0; i < smp; i++)
			{
				x = src[i];				
            f = f1_data + f2_data * pow (fabs ((double) x), (double) f3_data);
				dst[i] = f;				 
			}								
			return;		
		
      case 7:
			for(int i=0; i < smp; i++)
			{
				x = src[i];				
            d = f1_data + f2_data * x;
				dst[i] = (float) (f3_data * pow (10.0, d));				 
			}								
			return;				
			
		case 10:
			// pvak pirani
			for(int i=0; i < smp; i++)
			{
				x = src[i];	
				f = -7.5188931 + 1.44424899e-2 * x + 1.84163771e-5 * x * x -
	   			 7.930881164e-8 * x * x * x + 6.1887331679e-11 * x * x * x * x;
				dst[i] = (float) exp ((double) f);
			}										 
			return;

		case 11:
			// Balzers Pirani TPR 018
			for(int i=0; i < smp; i++)
			{
				x = src[i];	
				f = -6.458298635 + 1.1065126182e-2 * x + 1.724604645308e-5 * x * x -
	   			4.470682241289e-8 * x * x * x + 2.8873048815568e-11 * x * x * x * x;
				dst[i] = (float) exp ((double) f);	
			}								
			return;

		case 12:
			// Alcatel Pirani PA 101-111
			for(int i=0; i < smp; i++)
			{
				x = src[i];	
				f = -4.388932846151 + 2.196812521419e-2 * x - 
	   			 3.891335175617e-5 * x * x + 2.57595961364e-8 * x * x * x;
				dst[i] = (float) exp ((double) f);
			}										 
			return;

		case 13:
			// Alcatel Pirani PB 101-111
			for(int i=0; i < smp; i++)
			{
				x = src[i];
				f = -9.125316008584 + 2.7479261887639e-2 * x - 
	   			 3.27756486380887e-5 * x * x + 1.865511857175e-8 * x * x * x;
				dst[i] = (float) (exp (double (f)));
			}								 
			return;

		case 14:
			// Granville-Phillips Pirani with 275-Display
			for(int i=0; i < smp; i++)
			{
				x = src[i];
				f = -5.80218101625545 + 7.21225921084386e-2 * x - 3.01506446945197e-4 * x * x +
	   			 5.31032962657776e-7 * x * x * x - 2.99101142006525e-10 * x * x * x * x;
				dst[i] = (float) exp ((double) f);
			}					 
			return;

		case 15:
			// MVP pirani with JAD diode
			for(int i=0; i < smp; i++)
			{
				x = src[i];
				f = -6.90752840773383 + 1.56914599457571e-02 * x + -3.71500742123613e-06 * x * x +
	   			 -3.21792780896806e-08 * x * x * x + 3.38803809282301e-11 * x * x * x * x;
				dst[i] = (float) exp ((double) f);
			}					 
			return;

		case 16:
			// MVP penning, curve splitted into two parts
			for(int i=0; i < smp; i++)
			{
				boolL = 1;
				x = src[i];
				if( (x < 860) && (boolL == 1) )  // straight line
				{
					boolL = 0;
					f = -2.46742667494164e1 + 1.58559741926023e-02 * x;
					dst[i] = (float) exp ((double) f);				 					
				}
				if( (x > 980) && (boolL == 1) )
				{
					boolL = 0;
					dst[i] = 0.1;									
				}
				if( boolL == 1 )
				{
					boolL = 0;								
					f = -9.360379498997047e3 + 3.12685547475332e1 * x + -3.48654820147945e-02 * x * x +
						1.29627979745431e-05 * x * x * x;
					dst[i] = f;					
				}	
			}		
			return;

		default:
			for(int i=0; i < smp; i++)
			{
				x = src[i];
				f = x;
				dst[i] = f;
			}		
			return;
		}
	}
} 


// Piti Poxvi

EqFctADC::EqFctADC () :	EqFct 	(const_cast<char*>("NAME = location"))
        ,is_interrupt_	(const_cast<char*>("INTERRUPT checking for interupts"), this)
{

#ifdef ADD_PROPERTY_DIRECT
	D_fct* t;
#endif // #ifdef ADD_PROPERTY_DIRECT

	char	chan[40];

	for(int i = 0; i < CONF_PROPERTIES; i++)
	{

		if( i < 10 )
		{
			sprintf(chan,"CH0%d.NUM",i);
		}
		else
		{
			sprintf(chan,"CH%d.NUM",i);		
		}
		adcscope_nm[i] = new D_int( chan, this );

		if( i < 10 )
		{
			sprintf(chan,"CH0%d.POLYPARA",i);
		}
		else
		{
			sprintf(chan,"CH%d.POLYPARA",i);		
		}
		adcscope_poly[i] = new D_polynom( chan, this );
		
		if( i < 10 )
		{
			sprintf(chan,"CH0%d_DAQ_START",i);
		}
		else
		{
			sprintf(chan,"CH%d_DAQ_START",i);		
		}
                daq_start[i] = new D_Command( chan, this );
#ifdef ADD_PROPERTY_DIRECT
                t = daq_start[i];		fct_list->push_back ( t );
#endif // #ifdef ADD_PROPERTY_DIRECT
	}

	list_append();
	conf_done = 1;
}


int     g_nDebug = 1;
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int GlobFuncMyGetCh()
{
    struct termios oldt, newt;
    int ch;
    tcgetattr(STDIN_FILENO,&oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(STDIN_FILENO,TCSANOW,&newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
    return ch;
}


void	eq_init_prolog ()	// called before init of all EqFct's
{
        int nStopForDebug = 0;
        FILE* fpStopFile = NULL;
	const int positions  = 260;
	char    data[positions];	
	char*   pn;

	for(int i=0; i < MAX_CH_NUM; i++) 
	{ 
		r_s[i] = 0; s_s[i] = 0;
	}

        printf("pid = %d\n",(int)getpid());

        fpStopFile = fopen("stop_file.stp","r");
        if(fpStopFile)
        {
            fscanf(fpStopFile,"%d",&nStopForDebug);
            fclose(fpStopFile);
        }
        else
        {
            fprintf(stderr,"WARNING: \"stop_file.stp\" file does not exist!!!\n");
        }
        if(nStopForDebug)
        {
             printf("press any key to continue!\n");
             GlobFuncMyGetCh();
        }
	
        config_ptr = fopen("./scopedaq_config/scopedaq.conf","r");

        char vcBuffer[16];
        getlogin_r(vcBuffer,15);

        printf("username = \"%s\"\n",vcBuffer);

        if(!config_ptr)
        {
            fprintf(stderr,"\n\n--------\"./scopedaq_config/scopedaq.conf\" doesn't exist!\n\n");
            exit(1);
        }
	
	CONF_PROPERTIES = 0;
	while ( fgets(data, positions, config_ptr) != NULL)
	{	
		pn = strpbrk(data,LN);
		if( ( pn == 0 ) || ( pn[0] == '#' ) )		continue;

                sscanf(&data[0],"%d %s %d %f %f %f %d",
                        &conf_[CONF_PROPERTIES].conf_nm,
                        conf_[CONF_PROPERTIES].conf_fac_loc,
                        &conf_[CONF_PROPERTIES].poly1,
                        &conf_[CONF_PROPERTIES].poly2,
                        &conf_[CONF_PROPERTIES].poly3,
                        &conf_[CONF_PROPERTIES].poly4,
                        &conf_[CONF_PROPERTIES].conf_daq_start);
													  
		CONF_PROPERTIES++;
	}		
	fclose(config_ptr);
	
	for(int i= 0; i < CONF_PROPERTIES; i++)
	{
		confcp_[i].conf_nm			=	conf_[i].conf_nm;
		sprintf(confcp_[i].conf_fac_loc,"%s",conf_[i].conf_fac_loc);
		confcp_[i].poly1				=	conf_[i].poly1;
		confcp_[i].poly2				=	conf_[i].poly2;
		confcp_[i].poly3				=	conf_[i].poly3;
		confcp_[i].poly4				=	conf_[i].poly4;														  
		confcp_[i].conf_daq_start	=	conf_[i].conf_daq_start;	
	}		
	
	return;	
}


EqFct* eq_create(int eq_code, void *)
{
    EqFct    *eqn;

	switch (eq_code) 
	{
		case 778:
			eqn = new D_ADCShm(); // user location
		break;
				 
		case 301:
			eqn =  new EqFctADC();		
		break;			 
		default:
		break;
	}
	return eqn;
}


void	EqFctADC::init( )
{

	pmain = this;
	
	for(int i = 0; i <= CONF_PROPERTIES; i++)
	{
		BLOCK_rdbk_thread_id[i] = 0;
		start_inter[i]          = 0;
	}		
	
	BLOCK_rdbk_SENDpthread = 0;
	BLOCK_rdbk_PROPpthread = 0;
	
	for(int i = 0; i < CONF_PROPERTIES; i++)
	{
		adcscope_nm[i]->set_value(conf_[i].conf_nm);

		IFFFL.i1_data = conf_[i].poly1;
		IFFFL.f1_data = conf_[i].poly2;
		IFFFL.f2_data = conf_[i].poly3;		
		IFFFL.f3_data = conf_[i].poly4;			
		adcscope_poly[i]->set_value(&IFFFL);

		daq_start[i]->set_value(conf_[i].conf_daq_start);
	}
	
	tmp_id = (int)getpid();	
	
	for(int i=0; i <= CONF_PROPERTIES; i++)				
	{		
		myclass[i] = new myClass(i,this);
	}

	for(int i=0; i <= CONF_PROPERTIES; i++)
	{				

		check_number = 1;
		do
		{
			res = pthread_create(&(thread_id[i]), NULL, fill_thread, (void*)(myclass[i]));
			sprintf(charptr,"thread_id[%d] creation error: %d",i,res);						
			if( strcmp(charptr,charptr0) )
			{
				sprintf(charptr0,"%s",charptr);
				printtostderr("INFO",charptr);
			}
			if(res != 0) { check_number++; }							
		} while( (res != 0) && (check_number < 4) );
		if(res != 0) { exit(1); }	
	}	
	
	
	BLOCK_rdbk_SENDpthread = 0;

	check_number = 1;
	do
	{
		res = pthread_create(&(SEND_thread), NULL, send_thread, (void *)(this));
		sprintf(charptr,"SEND_thread creation error: %d",res);						
		if( strcmp(charptr,charptr0) )
		{
			sprintf(charptr0,"%s",charptr);
			printtostderr("INFO",charptr);
		}
		if(res != 0) { check_number++; }							
	} while( (res != 0) && (check_number < 4) );
	if(res != 0) { exit(1); }	
	
	
	BLOCK_rdbk_PROPpthread = 0;

	check_number = 1;
	do
	{
		res = pthread_create(&(PROP_thread), NULL, prop_thread, (void *)(this));
		sprintf(charptr,"PROP_thread creation error: %d",res);						
		if( strcmp(charptr,charptr0) )
		{
			sprintf(charptr0,"%s",charptr);
			printtostderr("INFO",charptr);
		}
		if(res != 0) { check_number++; }							
	} while( (res != 0) && (check_number < 4) );
	if(res != 0) { exit(1); }		
	
	
}

void	eq_init_epilog () 	// called at end of init of all EqFct's
{

}


void 	post_init_prolog() {}

void 	post_init_epilog()
{
    adcscope_post_init_epilog();
    adcscope_setup_interrupt();
	
}

void	EqFctADC::post_init ()
{

}

extern int is_interrupt;

void	EqFctADC::update ()
{
        /*int   error = 0;*/
/*
   if( g_sts_.online() ) 
	{
   	
		//
		// do some hardware readout, e.g. SEDAC, Ethernet ...
		//
		if( !error ) 
		{

//			int_value_.set_value( 1 );
			
   		set_error( no_error );
		}
		else 
		{
  			set_error( device_error );		// set error for this location
		}	
   }
   else 
	{
   	set_error( offline );
   }
*/	
   g_sts_.online( 1 );
}

void	EqFctADC::interrupt_usr1 ( int sig_no )
{
	switch (sig_no) 
	{
		case 1:		// SIGUSR1: ADC trigger interrupt

			for(int i=0; i < CONF_PROPERTIES; i++)				
			{
				if( (BLOCK_rdbk_thread_id[i] == 0) && (start_inter[i] == 0) && (check_shmem_ptr == 1) )
				{
					sigres = pthread_kill(thread_id[i], 0 );
					if(sigres != 0)
					{
						sprintf(sigptr,"Signal 0 sending error: %d, thread: %d",sigres,i);
						printtostderr("ERROR",sigptr);
						BLOCK_rdbk_thread_id[i] = 1;

					}				
					else
					{
						start_inter[i] = 1;					
					}
				}
			}
			
			if( (BLOCK_rdbk_thread_id[CONF_PROPERTIES] == 0) && (start_inter[CONF_PROPERTIES] == 0) )
			{
				sigres = pthread_kill(thread_id[CONF_PROPERTIES], 0 );
				if(sigres != 0)
				{
					sprintf(sigptr,"Signal 0 sending error: %d, thread: %d",sigres,CONF_PROPERTIES);
					printtostderr("ERROR",sigptr);
					BLOCK_rdbk_thread_id[CONF_PROPERTIES] = 1;
				}					
				else
				{
					start_inter[CONF_PROPERTIES] = 1;					
				}
			}			
			
			if( BLOCK_rdbk_SENDpthread == 0 )
			{
				sigres = pthread_kill(SEND_thread, 0 );
				if(sigres != 0)
				{
					sprintf(sigptr,"Signal 0 sending error: %d, thread: SEND_thread",sigres);
					printtostderr("ERROR",sigptr);
					BLOCK_rdbk_PROPpthread = 1;
				}					
			}				
			
			if( BLOCK_rdbk_PROPpthread == 0 )
			{
				sigres = pthread_kill(PROP_thread, 0 );
				if(sigres != 0)
				{
					sprintf(sigptr,"Signal 0 sending error: %d, thread: PROP_thread",sigres);
					printtostderr("ERROR",sigptr);
					BLOCK_rdbk_PROPpthread = 1;
				}					
			}
			
//=========================================================================================	
		
		for(int i=0; i <= CONF_PROPERTIES; i++)
		{
			if( BLOCK_rdbk_thread_id[i]  == 1 )
			{
				check_number = 1;					
				do
				{
					res = pthread_cancel(thread_id[i]);						
					sprintf(charptr,"thread_id[%d] cancel error: %d",i,res);						
					if( strcmp(charptr,charptr0) )
					{
						sprintf(charptr0,"%s",charptr);
						printtostderr("INFO",charptr);
					}							
					if(res != 0) { check_number++; }						
				} while( (res != 0) && (check_number < 4) );				

				check_number = 1;
				do
				{
					res = pthread_create(&(thread_id[i]), NULL , fill_thread, (void*)(myclass[i]));
					sprintf(charptr,"thread_id[%d] creation error: %d",i,res);												
					if( strcmp(charptr,charptr0) )
					{
						sprintf(charptr0,"%s",charptr);
						printtostderr("INFO",charptr);
					}							
					if(res != 0) { check_number++; }							
				} while( (res != 0) && (check_number < 4) );
				if(res != 0) { exit(1); }	else { BLOCK_rdbk_thread_id[i] = 0; }	
											
			}
			
		}				
			
//=========================================================================================			
			
		if( BLOCK_rdbk_SENDpthread == 1 )
		{
			check_number = 1;					
			do
			{
				res = pthread_cancel(SEND_thread);
				sprintf(charptr,"SEND_thread cancel error: %d",res);						
				if( strcmp(charptr,charptr0) )
				{
					sprintf(charptr0,"%s",charptr);
					printtostderr("INFO",charptr);
				}							
				if(res != 0) { check_number++; }						
			} while( (res != 0) && (check_number < 4) );				
 
			check_number = 1;
			do
			{
				res = pthread_create(&(SEND_thread), NULL, send_thread, (void *)(this));
				sprintf(charptr,"SEND_thread creation error: %d",res);						
				if( strcmp(charptr,charptr0) )
				{
					sprintf(charptr0,"%s",charptr);
					printtostderr("INFO",charptr);
				}							
				if(res != 0) { check_number++; }							
			} while( (res != 0) && (check_number < 4) );
			if(res != 0) { exit(1); } else { BLOCK_rdbk_SENDpthread = 0; }								
		}	
				
//=========================================================================================
		
		if( BLOCK_rdbk_PROPpthread == 1 )
		{
			check_number = 1;					
			do
			{
				res = pthread_cancel(PROP_thread);
				sprintf(charptr,"PROP_thread cancel error: %d",res);						
				if( strcmp(charptr,charptr0) )
				{
					sprintf(charptr0,"%s",charptr);
					printtostderr("INFO",charptr);
				}							
				if(res != 0) { check_number++; }						
			} while( (res != 0) && (check_number < 4) );				

			check_number = 1;
			do
			{
				res = pthread_create(&(PROP_thread), NULL, prop_thread, (void *)(this));
				sprintf(charptr,"PROP_thread creation error: %d",res);						
				if( strcmp(charptr,charptr0) )
				{
					sprintf(charptr0,"%s",charptr);
					printtostderr("INFO",charptr);
				}							
				if(res != 0) { check_number++; }							
			} while( (res != 0) && (check_number < 4) );
			if(res != 0) { exit(1); }	else { BLOCK_rdbk_PROPpthread = 0; }								
		}	
	
//=========================================================================================				
			
		break;
		case 2:		// SIGUSR2: reset ADCs interrupt
		break;
		default:
		break;
	}
}


void	refresh_prolog () 	// called before "update"
{
}

void	interrupt_usr1_prolog (int sig_no)
{
    switch (sig_no) {
        case 1:		// SIGUSR1: ADC trigger interrupt
            break;
         case 2:		// SIGUSR2: reset ADCs interrupt
            break;
         default:
            break;
    }
}

void	refresh_epilog ()
{

}

void	interrupt_usr1_epilog (int sig_no)
{
    switch (sig_no) {
        case 1:		// SIGUSR1: ADC trigger interrupt
//                adcscope_interrupt_usr1_epilog ( sig_no );

					
                break;
        case 2:		// SIGUSR2: reset ADCs interrupt
//                adcscope_interrupt_usr1_epilog ( sig_no );
                break;
        default:
                break;
    }
};

void	interrupt_usr2_prolog () {}
void	interrupt_usr2_epilog () {}
void	eq_cancel()
{
}


void *fill_thread(void *arg)
{
	bool bIsConected(false);

	int nPrevEvent(-1);
	ADC_MAP_Helper<ADC_MAP_CLT>  aAdcmapHelper;


	myClass* pmyClass = (myClass*) arg;
	int          BOARD_NUMBER;		
	BOARD_NUMBER = pmyClass->BOARD_NUMBER;		
	
	char       charptr2[400];
	char       charptr20[400];
			
	struct timespec req0;
	req0.tv_sec = 0;
	req0.tv_nsec = 400000000;
	
	int time0L = 2;	
	
	struct timespec reqX;
	reqX.tv_sec = 0;
	reqX.tv_nsec = 20000000;
	
	struct timespec ts;
	ts.tv_sec = 4;
	ts.tv_nsec = 0;		
	
	int  Counter = 0;	
	int  Checker = 0;
		
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );		
	

	if( BOARD_NUMBER == CONF_PROPERTIES )
	{
		nanosleep(&ts, NULL);
	
		while( 1 )
		{
			if(pmyClass->eq_fct_->start_inter[BOARD_NUMBER] == 1)
			{
				Counter = 0;
				Checker = 0;
				sprintf(charptr2,"%s","Interrupts are O.K.");	
				if( strcmp(charptr2,charptr20) )
				{
					sprintf(charptr20,"%s",charptr2);
					printtostderr("INFO",charptr2);
				}
				pmyClass->eq_fct_->start_inter[BOARD_NUMBER] = 0;
			}
			else
			{
				nanosleep(&req0, NULL);
				Counter = Counter + 1;
				if( Counter > time0L )
				{
					Checker = Checker + 1;
					if(Checker < (250*time0L))
					{
						Counter = 0;
					}
					else
					{
						pthread_kill(tmp_id, SIGKILL );
					}
				
					sprintf(charptr2,"%s","No interrupts anymore");	
					if( strcmp(charptr2,charptr20) )
					{
						sprintf(charptr20,"%s",charptr2);
						printtostderr("INFO",charptr2);
					}
				
					check_shmem_ptr = 0;

//					tmp_id = (int)getpid();
					adc_map_.close_shmem();
					check_reconfig = adc_map_.shm_reconfig();				
					if(check_reconfig != -1)
					{
						for(int k = 0; k < ADC_PROC_NUM; k++)
						{
                                                        if( (*(int *)((u_char *)adc_map_.shmem_ptr + shm_off_adcid + k*sizeof(int))) == tmp_id )
							{
								*(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_adcid + k*sizeof(int))= 0;
							}	
						}

						for(int k = 0; k < ADC_PROC_NUM; k++)
						{
							if( (*(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_adcid + k*sizeof(int))) == 0 )
							{
								*(u_int *)((u_char *)adc_map_.shmem_ptr + shm_off_adcid + k*sizeof(int))= tmp_id;
								break;
							}	
						}

						check_shmem_ptr = 1;							
					}
				}
			}	
		}	
	}
	else
	{
		while( 1 )
		{
			if(pmyClass->eq_fct_->start_inter[BOARD_NUMBER] == 1)
			{
				if( conf_[BOARD_NUMBER].conf_daq_start == 1 )
				{	
					if(!bIsConected)
					{
						aAdcmapHelper.AttachToAdcMap( &adc_map_ );
						bIsConected = true;
					}
					pmyClass->read_adcL(&aAdcmapHelper,nPrevEvent);
				}			
				pmyClass->eq_fct_->start_inter[BOARD_NUMBER] = 0;
			}
			else
			{
				nanosleep(&reqX, NULL);
			}
		}
	}


   pthread_exit((void*)NULL);
	return (void*)NULL;
}


void *send_thread(void * /*arg*/)
{	
        //EqFctADC*  p = (EqFctADC*) arg;
	ssize_t  result;
	int      j;
	
	sigset_t sigset;		
	struct timespec ts;	
	
	ts.tv_sec = 0;
	ts.tv_nsec = 1000000;	
		
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );	
		
	while( 1 )	
	{
		for(j = 0; j < CONF_PROPERTIES; j++)		
		{
	
			if( s_s[j] != r_s[j] )
			{
                                result = sendto(sender->sock, data_union[j][s_s[j]].ch, DATA_struct_l, 0, (struct sockaddr *)&(sender->saddr), sender->socklen);
                                //result = DATA_struct_l;
				
				if( ( result == -1 ) || ( result != (ssize_t)(DATA_struct_l) ) )				
				{
					sprintf(genptr,"Result of sendto: %d, length of data structure: %d",(int)(result),DATA_struct_l);
					printtostderr("ERROR",genptr);
					
					delete sender;
					sender = 0;
                                        //sender = new MCsender(false);
                                        sender = new MCsender;
				}
				else
				{
					if( s_s[j] < (EVENTS - 1) )
					{
						s_s[j] = s_s[j] + 1;
					}
					else
					{
						s_s[j] = 0;
					}
				}				
			}
		
		}
		
		nanosleep(&ts, NULL);
	}	
	
	pthread_exit((void*)NULL);
	return (void*)NULL;	
}


void* prop_thread(void* arg)
{
	EqFctADC*  p = (EqFctADC*) arg;
	const int positions  = 260;
	char      conf_addr[positions];		
	
	sigset_t sigset;		
	struct timespec ts;	
	
	EqAdr*      adr_pt;
	EqData*     src_pt;
	EqData*     dst_pt;
	EqCall*     call_pt;	
	
	call_pt = new EqCall();
	adr_pt  = new EqAdr();
	src_pt  = new EqData();
	dst_pt  = new EqData();	
	
	ts.tv_sec = 4;
	ts.tv_nsec = 0;
	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );			
	
	while( 1 )
	{
			
		for(int i= 0; i < CONF_PROPERTIES; i++)
		{
			sprintf(conf_addr,"%s.NUM",confcp_[i].conf_fac_loc);
			adr_pt->adr(conf_addr);
			src_pt->init();
			dst_pt = call_pt->get(adr_pt, src_pt);		
			if(dst_pt->error() == 0) 	
			{		
				confcp_[i].conf_nm = dst_pt->get_int();
			}
		}
			
		for(int i= 0; i < CONF_PROPERTIES; i++)
		{
			sprintf(conf_addr,"%s.POLYPARA",confcp_[i].conf_fac_loc);
			adr_pt->adr(conf_addr);
			src_pt->init();
			dst_pt = call_pt->get(adr_pt, src_pt);		
			if(dst_pt->error() == 0) 	
			{		
				dst_pt->get_ifff(&confcp_[i].poly1,&confcp_[i].poly2,&confcp_[i].poly3,&confcp_[i].poly4);
				p->IFFFL.i1_data = confcp_[i].poly1;
				p->IFFFL.f1_data = confcp_[i].poly2;
				p->IFFFL.f2_data = confcp_[i].poly3;		
				p->IFFFL.f3_data = confcp_[i].poly4;			
				p->adcscope_poly[i]->set_value(&(p->IFFFL));				
			}
		}
	
		for(int i= 0; i < CONF_PROPERTIES; i++)
		{		
			confcp_[i].conf_daq_start = p->daq_start[i]->value();
			conf_[i].conf_daq_start	=	confcp_[i].conf_daq_start;			
		}
		
		prop_cond_ = 1;

		nanosleep(&ts, NULL);
	
		config_ptr = fopen("./scopedaq_config/scopedaq.conf.BAK","w");	

		for(int k= 0; k < CONF_PROPERTIES; k++)
		{

			fprintf(config_ptr,"%2d %-40s %4d %15.5e %15.5e %15.5e %4d\n",
								   	confcp_[k].conf_nm,
										confcp_[k].conf_fac_loc,
								   	confcp_[k].poly1,
								   	confcp_[k].poly2,
								   	confcp_[k].poly3,
								   	confcp_[k].poly4,														  
								   	confcp_[k].conf_daq_start);											

		}
		fclose(config_ptr);

		config_ptr = fopen("./scopedaq_config/scopedaq.conf","w");	

		for(int k= 0; k < CONF_PROPERTIES; k++)
		{

			fprintf(config_ptr,"%2d %-40s %4d %15.5e %15.5e %15.5e %4d\n",
								   	confcp_[k].conf_nm,
										confcp_[k].conf_fac_loc,
								   	confcp_[k].poly1,
								   	confcp_[k].poly2,
								   	confcp_[k].poly3,
								   	confcp_[k].poly4,														  
								   	confcp_[k].conf_daq_start);											

		}
		fclose(config_ptr);
	
		prop_cond_ = 0;				

	}

	pthread_exit((void*)NULL);
	return (void*)NULL;
}






