// file eq_pitzrf1fpga.h
//
// Test Eq functions class
//
// This is a general test server. The server get the name of the 
// test file and for the history file at compile-time as define parameter
// inside the makefile
//
//
// Olaf Hensler, -MVP2-
//
// last update:
// 	23. Mar. 2000
//
#ifndef eq_pitzrf1fpga_h
#define eq_pitzrf1fpga_h

#include	"eq_fct.h"
#include "eq_client.h"
#include "pitz_daq_collectorproperties.hpp"

#define Codepitzrf1fpga 300	// eq_fct_type number for the .conf file

const int properties = 136;

const int COUNT = 120;

const int positions  = 260;

class EqFctpitzrf1fpga; 

struct DaqConfig
{
	EqFctpitzrf1fpga* eq_fct_;
	int         id;
	int         carS;
	int         carR;
	int         samples;
	char 			daq_name[80];
	char 			doocs_url[80];
	int 			from;	
	int 			size;	
	int 			step;	
	int         type;
	int         statistic;
	char        statistic_time[60];	
};
struct  DaqConfig    *DaqConf[properties];

struct DaqCondition
{
	char        doocs_url[80];
	float       min;
	float       max;
	int         type;	
};
struct DaqCondition 	 *DaqCond[properties];

struct str_all
{
	int         time;
	int         buffer;
	int	      int_value;
	float       float_value;	
	float       array_value[2048];		
};
struct str_all    *transit[properties][COUNT];


struct str_01
{
	Int_t       time;
	Int_t       buffer;
	Int_t       int_value;
};
struct str_01 *D_C01[properties];	


struct str_02
{
	Int_t       time;
	Int_t       buffer;
	Float_t     float_value;
};
struct str_02 *D_C02[properties];


struct str_19
{
	Int_t       time;
	Int_t       buffer;
	Float_t     array_value[2048];
};
struct str_19 *D_C19[properties];

int     begin_TIME_R;
int     begin_BUFFER_R;

int     end_TIME_R;
int     end_BUFFER_R;
int     end_TIME_R_ch;
int     end_BUFFER_R_ch;

char 	  ASOVA_str[positions];
int     ASOVA_true;
int     ASOVA_start;

int     StatusConfig;
int     StatusCondition;

char    root_fn_old[positions];
char    cache_fn_old[positions];	
char 	  dccp_char[260];

pthread_t        COND_thread;
pthread_t        thread_id[properties];
pthread_t        ROOT_thread;
pthread_t        ASOVA_thread;
pthread_t        MC_thread;
pthread_t        STAT_thread;

int      board_channel[properties];

int      AtLeastOne[properties];
int      AtLeastOne_cp[properties];

using namespace pitz::daq;

class EqFctpitzrf1fpga : public EqFctCollector
{
private:

	char ConditionName[positions];
	
	char    data[positions];	
	char    ConfigFile[positions];
	char    ConditionFile[positions];	

	EqAdr      *ea;
	EqData     *ed;
	EqData     *result;	
	EqCall     *eq;
	
	FILE* log_ptr;
	char logstr[positions];
	
	FILE* config_ptr;	
	FILE* cond_ptr;	


protected:
	D_name		alias_;
public:
	EqFctpitzrf1fpga( );

        D_Command2	   Command_;
        D_int Conds_;
        D_int root_length;
        D_int gen_event_;
        D_WriteFile2    WriteFile_;
        D_int datastream_;
        D_int serverstatus_;
	

	char ConfigName[positions];	
		
	void parse_config();
	void parse_condition();		
	
				
	void	update();
	void	init();	// started after creation of all Eq's

        int	fct_code()	{ return Codepitzrf1fpga; }
	
	static int	conf_done;
	
	int     i,j,k;
		
	
	char*   pn;	
	

};


#endif
 
