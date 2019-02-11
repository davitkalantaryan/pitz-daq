// file eq_pitznoadc2.h
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
#ifndef eq_pitznoadc2_h
#define eq_pitznoadc2_h

#include	"eq_fct.h"
#include "eq_client.h"
#include <string>
#include "common_daq_definations.h"

#define Codepitznoadc2 300	// eq_fct_type number for the .conf file

const int properties = 500;

const int COUNT = 120;

const int positions  = 260;

class EqFctpitznoadc2; 

struct DaqConfig
{
	EqFctpitznoadc2* eq_fct_;
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
	char        char_array[60];
	char        IFFF_array[60];		
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


struct str_03
{
	Int_t       time;
	Int_t       buffer;
	char        char_array[60];
};
struct str_03 *D_C03[properties];


struct str_15
{
	Int_t       time;
	Int_t       buffer;
	char        IFFF_array[60];
};
struct str_15 *D_C15[properties];


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

class EqFctpitznoadc2;

class D_ThreadStatus : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{__DEBUG_APP__(0," ");return std::string("Hallo");}
#endif
	EqFctpitznoadc2* eq_fct_;
        D_ThreadStatus(const char* pn, EqFctpitznoadc2* eq_fct);
	void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
protected:
};


class D_Command : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{__DEBUG_APP__(0," ");return std::string("Hallo");}
#endif
	EqFctpitznoadc2* eq_fct_;
        D_Command(const char* pn, EqFctpitznoadc2* eq_fct);
	void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
protected:
};


class D_Alarm : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{__DEBUG_APP__(0," ");return std::string("Hallo");}
#endif
	EqFctpitznoadc2* eq_fct_;
        D_Alarm(const char* pn, EqFctpitznoadc2* eq_fct);
	void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
	bool clean;
protected:
};


class D_WriteFile : public D_int
{
public:
#ifdef FIX_UNRESOLVED
    std::string __xpg_basename() const{__DEBUG_APP__(0," ");return std::string("Hallo");}
#endif
	EqFctpitznoadc2* eq_fct_;
        D_WriteFile(const char* pn, EqFctpitznoadc2* eq_fct);
	void set(EqAdr*, EqData* sed, EqData* red, EqFct *);
protected:
};


class EqFctpitznoadc2 : public EqFct 
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
	EqFctpitznoadc2( );

	D_Command	   Command_;	
	D_ThreadStatus ThreadStatus_;
	D_ThreadStatus Conds_;	
	D_ThreadStatus root_length;
	D_ThreadStatus gen_event_;
	D_Alarm        Alarm_;	
	D_WriteFile    WriteFile_;	
	D_ThreadStatus datastream_;
	D_ThreadStatus serverstatus_;	
	
	int  write_file;
	
	char ConfigName[positions];	
		
	void parse_config();
	void parse_condition();		
	
				
	void	update();
	void	init();	// started after creation of all Eq's

        int	fct_code()	{ return Codepitznoadc2; }
	
	static int	conf_done;
	
	int     i,j,k;
		
	
	char*   pn;	
	

};


#endif
 
