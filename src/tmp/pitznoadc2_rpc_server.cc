//
// file eq_pitznoadc2.cc
//
// pitznoadc2 Eq function class

#include	<ctime>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>

#include <sys/stat.h>
#include <sys/shm.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/time.h>
#include <sys/resource.h> 
#include	"printtostderr.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include <TROOT.h>
#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TBasket.h>
#include <TObject.h>
#include <TSystem.h>
#include <TError.h>
#include <TNetFile.h>

#include	"eq_errors.h"
#include	"eq_sts_codes.h"
#include	"eq_fct_errors.h"

#include	"eq_pitznoadc2.h"

using namespace std;

//key_t  SHMKEY = ftok("/export/doocs/server/daqtime_server/adc_daqtime",'g');
key_t  SHMKEY = ftok("/export/doocs/server/daqtimeZMQ_server/adc_daqtimeZMQ",'a');

const char*	object_name = "pitznoadc2";

EqFct*		pitznoadc2_fct;

int	EqFctpitznoadc2::conf_done = 0; 

extern	std::vector<EqFct*>	*eq_list;
extern	int		chan_count;
extern	Config* 	config;
extern	int		fct_code;

extern	int		ring_buffer;

time_t			walltime;

const char* LN = "#_QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm1234567890";

/*
setup:
	::init DOOCS address for .TD	
	eq_XXXX.h
		const int properties = ?;
*/
const int  H_count = 1200;

struct  H_struct
{
  int   seconds;
  int   microseconds;
  int   gen_event;
  int   rep_rate; 
}Hstruct;

int        shmid;
struct H_struct *shareptr = &Hstruct;

int     GENSTATIC;
int     SECSTATIC;

int     CONFIG_COUNTER    = 0;
int     NOALARM_COUNTER   = 0;
int     CONDITION_COUNTER = 0;

extern  void* cond_thread(void *);
extern  void* fill_struct(void *);
extern  void* root_thread(void *);
extern  void* asova_thread(void *);
extern  void* mc_thread(void *);
extern  void* stat_thread(void *);


D_ThreadStatus::D_ThreadStatus(const char* pn, EqFctpitznoadc2* eq_fct) : D_int(pn,eq_fct),eq_fct_(eq_fct) { }
void D_ThreadStatus::set(EqAdr*, EqData* /*sed*/, EqData* /*red*/, EqFct *)
{
	return;
}

D_Command::D_Command(const char* pn, EqFctpitznoadc2* eq_fct) : D_int(pn,eq_fct),eq_fct_(eq_fct) { }
void D_Command::set(EqAdr*, EqData* sed, EqData* /*red*/, EqFct *)
{
	switch ( sed->get_int() ) 
	{
  		case 1 :                                 //  stop  thread
			if( eq_fct_->ThreadStatus_.value() == 1 )			
			{
				eq_fct_->ThreadStatus_.set_value(0);
			}
                        if (get_access()) set_value(0);//???
    	break;
  		default :
                        if (get_access()) set_value(0);//???
		break;
	}
	return;
}


D_Alarm::D_Alarm(const char* pn, EqFctpitznoadc2* eq_fct) : D_int(pn,eq_fct),eq_fct_(eq_fct) { clean = 0; }
void D_Alarm::set(EqAdr*, EqData* sed, EqData* /*red*/, EqFct *)
{
	switch ( sed->get_int() ) 
	{
  		case 0 :
			if( value() > 0 )
			{
				clean = 1;                                 
				set_value(0);
                                if (get_access()) set_value(0);
			}
    	break;
  		default :

		break;
	}
	return;
}


D_WriteFile::D_WriteFile(const char* pn, EqFctpitznoadc2* eq_fct) : D_int(pn,eq_fct),eq_fct_(eq_fct) { ; }
void D_WriteFile::set(EqAdr*, EqData* sed, EqData* /*red*/, EqFct *)
{
	switch ( sed->get_int() ) 
	{
  		case 1 :
			if(eq_fct_->Alarm_.value() > 0)
			{
         	eq_fct_->write_file = 1;
			}                      
			set_value(0);
                        if (get_access()) set_value(0);//???
    	break;
  		default :

		break;
	}
	return;
}


EqFctpitznoadc2::EqFctpitznoadc2() : 
	EqFct		("NAME = location" )
                ,alias_("ALIAS device name",this)
		,Command_("COMMAND_ for threads", this)
		,ThreadStatus_("THREADSTATUS_ status of the threads", this)
		,Conds_("CONDITION_S ok, when =1", this)
		,root_length("ROOT_LENGTH max length of file", this)
		,gen_event_("GEN_EVENT value",this)

		
		,Alarm_("ALARM worse property",this)
		,WriteFile_("WRITE_FILE for worse properties",this)		
		
		,datastream_("DATA_STREAM ok, when =1",this)
		,serverstatus_("SERVER_STATUS ok, when =11",this)
{

				
	if (!conf_done) 
	{
	    list_append();
	    conf_done = 1;
	}
}

void	eq_init_prolog() 	// called once before init of all EqFct's
{
   shmid = shmget(SHMKEY, (sizeof(struct H_struct))*(H_count + 1), 0664);	
   if (shmid < 0)
	{
		printf(" shared memory is not created... Exit !!!\n");
		exit(1);
	}
	if ( (shareptr = (struct H_struct *) shmat(shmid, NULL, 0)) == (struct H_struct *) -1)
	{
		printf(" can't attach to shared memory... Exit !!!\n");
		exit(1);
	}		
	

	for(int i=0; i < properties; i++)
	{
		board_channel[i] = 0;
	}

	GENSTATIC = 0;
	SECSTATIC = 0;
}


//
// The init() method is call for every location during startup of the server
// Initialization of the hardware may be done here
//
void	EqFctpitznoadc2::init()
{
	eq = new EqCall();
	ea = new EqAdr();
	ed = new EqData();
	result = new EqData();

	set_error( no_error );
	ThreadStatus_.set_value(1);
	Command_.set_value(0);		
	Conds_.set_value(1);
	
	Alarm_.set_value(0);
	WriteFile_.set_value(0);
	
	
	write_file = 0;

	datastream_.set_value(1);
	serverstatus_.set_value(11);	
		
	sprintf(ConfigName,"%s",name_.value());
	i=0;
	while (ConfigName[i])
	{
		if(ConfigName[i] == '_') ConfigName[i] = '.';
	 ConfigName[i] = tolower(ConfigName[i]);
	 i++;
	}
	
	CONFIG_COUNTER = 0;
	sprintf(logstr,"/doocs/data/DAQdata/log/%s.log",ConfigName);
	parse_config();
	
	if( CONFIG_COUNTER == 0 )
	{
		log_ptr =  fopen(logstr,"a");	
		walltime = ::time(0);		
		fprintf(log_ptr,"%s : No any properties in CONFIG FILE, server was terminated. %s\n",ConfigName,asctime(localtime(&(walltime))));		
		fclose(log_ptr);
		exit(1);	
	}
	
	sprintf(ConditionName,"%s",name_.value());	
	i=0;
	while (ConditionName[i])
	{
		if(ConditionName[i] == '_') ConditionName[i] = '.';
	 ConditionName[i] = tolower(ConditionName[i]);
	 i++;
	}
	
	CONDITION_COUNTER = 0;	
	parse_condition();	
	
	if( ThreadStatus_.value() == 0 ) { exit(1); }
	
	for(i=0; i < CONFIG_COUNTER; i++)
	{
		AtLeastOne[i] = 0;
		AtLeastOne_cp[i] = 0;		
	}

	ASOVA_true = 0;	
			
	for(i=0; i < CONFIG_COUNTER; i++)
	{
		for(j=0; j < COUNT; j++)
		{
			transit[i][j] = new struct str_all;
			transit[i][j]->time        = 0;
			transit[i][j]->buffer      = 0;
			transit[i][j]->int_value   = 0;
			transit[i][j]->float_value = 0.0;
			sprintf(transit[i][j]->char_array,"%s","               ");
			sprintf(transit[i][j]->IFFF_array,"%s","               ");						
		}	
	}
	
	for(i=0; i < CONFIG_COUNTER; i++)
	{	
		DaqConf[i]->id        = i;
		DaqConf[i]->carS      = 0;
		DaqConf[i]->carR      = 0;		
		DaqConf[i]->eq_fct_   = this;
		DaqConf[i]->statistic = 0;		
	}	
	
	for(i=0; i < CONFIG_COUNTER; i++)
	{
		switch ( DaqConf[i]->type ) 
		{
			case 19 :
				DaqConf[i]->samples = 2048;
			break;
			default :
				DaqConf[i]->samples = 1;
			break;								
		}					
	}	

	for(i=0; i < CONFIG_COUNTER; i++)
	{
		switch ( DaqConf[i]->type ) 
		{
			case  1 :
				D_C01[i] = new struct str_01;		
			break;
			case  2 :
				D_C02[i] = new struct str_02;
			break;
			case  3 :
				D_C03[i] = new struct str_03;
			break;			
			case  4 :
				D_C01[i] = new struct str_01;
			break;
			case  6 :
				D_C02[i] = new struct str_02;
			break;
			case 15 :
				D_C15[i] = new struct str_15;
			break;			
			default :
			break;								
		}											
	}	
	
	StatusCondition = 0;
	
	if( CONDITION_COUNTER > 0 )
	{
		if ( pthread_create(&COND_thread, NULL, cond_thread, (void*)this))
		{
			fprintf(stderr,"could not create cond_thread! exit!\n");			
			exit(1);
		}
	}
	else
	{
		StatusCondition = 1;
	}
	
	if ( pthread_create(&ASOVA_thread, NULL, asova_thread, (void*)this))
	{
		fprintf(stderr,"could not create asova_thread! exit!\n");
		exit(1);
	}
	
	if ( pthread_create(&ROOT_thread, NULL, root_thread, (void*)this))
	{
		fprintf(stderr,"could not create root_thread! exit!\n");
		exit(1);
	}	
		
	for(i=0; i < CONFIG_COUNTER; i++)
	{	
		if( pthread_create(&(thread_id[i]), NULL, fill_struct, (void*)(DaqConf[i])))		
		{
			fprintf(stderr,"could not create fill_thread! exit!\n");				
			exit(1);					
		}
	}
	
	if ( pthread_create(&MC_thread, NULL, mc_thread, (void*)this))
	{
		fprintf(stderr,"could not create mc_thread! exit!\n");
		exit(1);
	}
	
	if ( pthread_create(&STAT_thread, NULL, stat_thread, (void*)this))
	{
		fprintf(stderr,"could not create stat_thread! exit!\n");
		exit(1);
	}	
			
}


void	eq_init_epilog() 	// called once at end of init of all EqFct's
{
}


//
// used during startup of the server to create the locations
//
EqFct* eq_create_derived(int a_eq_code, void* a_arg)
{
    EqFct* eqn( NULL);
    switch(a_eq_code)
    {
    case Codepitznoadc2:
        eqn = new EqFctpitznoadc2();
        break;

    default:
        eqn = NULL;
        break;
    } // switch(a_eq_code)

    return eqn;
}

void refresh_prolog()		// called before "update"
{
}

//
// This "update" method usually does the real work in a DOOCS server
// "update" is called frequently with the rate configured by SVR.RATE
// This method is called for every location, e.g. runs in a loop over all locations
// configured inside the .conf file
//
void	EqFctpitznoadc2::update()
{
	int   error = 0;

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
}


void refresh_epilog()	{}	// called after "update"


//
// The following methods are provided, when the server needs to use SIGUSR1 or SIGUSR2 
// interrupts (from timing system)
//
void interrupt_usr1_prolog(int)  {}
void interrupt_usr2_prolog(void) {}
void interrupt_usr1_epilog(int)  {}
void interrupt_usr2_epilog(void) {}
void post_init_prolog(void)  	 {}
void post_init_epilog(void)	 {}
void eq_cancel(void)
{
}

#include "stringparser1.h"
#define EMAIL_BUF_LEN   2047
#define SUBJ_BUF_LEN    511
#ifndef MAX_HOSTNAME_LENGTH
#define MAX_HOSTNAME_LENGTH 64
#endif

#include	"mailsender.h"

const int g_nMaxNumber = 10;


void EqFctpitznoadc2::parse_config()
{

        bool bDelete;
        int nRecipts(0);
        const char* tos[g_nMaxNumber];
        char* pcFileBuffer = NULL;
        if( StringParser1::FileBuffer2("/doocs/data/DAQdata/config/aacommon.conf",&pcFileBuffer) > 0 )
        {
            bDelete = true;
            StringParser1::TakeAllLineComments(pcFileBuffer,"//");
            StringParser1::TakeAllLineComments(pcFileBuffer,"#");
            StringParser1::TakeAllMLComments(pcFileBuffer,"/*","*/");

            char* pcAddress = NULL;
            while( StringParser1::FindFirstString(pcFileBuffer,&pcAddress) && nRecipts<g_nMaxNumber )
            {
                tos[nRecipts++]=pcAddress;
                pcAddress = NULL;
            }

            free(pcFileBuffer);
        }
        else
        {
            bDelete = false;
            nRecipts = 1;
            tos[0]="davit.kalantaryan@desy.de";
        }

        char vcEmailBuffer[EMAIL_BUF_LEN+1];
        char vcSubjectBuffer[SUBJ_BUF_LEN+1];
        int nWrited(0);
        int nRemains(EMAIL_BUF_LEN);
        int nSingle;
        struct  DaqConfig* pTemporal;

        char vcSenderName[MAX_HOSTNAME_LENGTH];
        snprintf(vcSenderName,MAX_HOSTNAME_LENGTH,"%s@ifh.am",ConfigName);


	StatusConfig = 1;
	sprintf(ConfigFile,"/doocs/data/DAQdata/config/%s.config",ConfigName);						 
	config_ptr =	fopen(ConfigFile,"r");
	CONFIG_COUNTER = 0;
	while ( fgets(data, positions, config_ptr) != NULL)
	{	
		pn = strpbrk(data,LN);
		if( ( pn == 0 ) || ( pn[0] == '#' ) )		continue;
		DaqConf[CONFIG_COUNTER] = new struct DaqConfig;

                sscanf(&data[0],"%s %s %d %d %d %d",DaqConf[CONFIG_COUNTER]->daq_name,
                                                    DaqConf[CONFIG_COUNTER]->doocs_url,
                                                    &DaqConf[CONFIG_COUNTER]->from,
                                                    &DaqConf[CONFIG_COUNTER]->size,
                                                    &DaqConf[CONFIG_COUNTER]->step,
                                                    &DaqConf[CONFIG_COUNTER]->type);
		CONFIG_COUNTER++;
	}		
	fclose(config_ptr);
	
	log_ptr =  fopen(logstr,"w");	
	fclose(log_ptr);

	log_ptr =  fopen(logstr,"a");	
	fprintf(log_ptr,"%s : ------ CONFIG FILE TESTING  ------\n",ConfigName);
	fclose(log_ptr);

	if(CONFIG_COUNTER > 1)
	{
		for(i=1; i < CONFIG_COUNTER; i++)
		{	
			for(j=0; j < i; j++)
			{	
				if( ! (strcmp(DaqConf[i]->daq_name,DaqConf[j]->daq_name)) )
				{
					log_ptr =  fopen(logstr,"a");	
					fprintf(log_ptr,"%s : Error. Daq name \"%s\" already exists\n",ConfigName,DaqConf[j]->daq_name);
                                        nSingle = snprintf(vcEmailBuffer,nRemains,"%s : Error. Daq name \"%s\" already exists\n",ConfigName,DaqConf[j]->daq_name);
                                        nWrited += nSingle;
                                        nRemains -= nSingle;
                                        nRemains = nRemains<0 ? 0:nRemains;
                                        pTemporal = DaqConf[i];
                                        memmove(DaqConf+i,DaqConf+i+1,sizeof(struct DaqConfig*)*(CONFIG_COUNTER-i-1));
                                        --CONFIG_COUNTER;
                                        delete pTemporal;
                                        //StatusConfig = 0;
					fclose(log_ptr);
				}
			}
			for(j=0; j < i; j++)
			{	
				if( ! (strcmp(DaqConf[i]->doocs_url,DaqConf[j]->doocs_url)) )
				{
					log_ptr =  fopen(logstr,"a");	
					fprintf(log_ptr,"%s : Attention. DOOCS address \"%s\" is already used\n",ConfigName,DaqConf[j]->doocs_url);
					fclose(log_ptr);
				}
			}			
		}
	}
	for(i=0; i < CONFIG_COUNTER; i++)
	{
		ea->adr( (char *)DaqConf[i]->doocs_url);
		ed->init();			
		result = eq->get(ea,ed);

		if(result->error() != 0) 	
		{
			log_ptr =  fopen(logstr,"a");	
			fprintf(log_ptr,"%s : Attention. DOOCS address \"%s\" does not respond. Not possibly to check up TYPE\n",ConfigName,DaqConf[i]->doocs_url);
			fclose(log_ptr);
		}
		else 
		{
			if( result->type() != DaqConf[i]->type )
			{
				log_ptr =  fopen(logstr,"a");	
				fprintf(log_ptr,"%s : Error. Wrong TYPE for the DOOCS address \"%s\"\n",ConfigName,DaqConf[i]->doocs_url);
                                nSingle=snprintf(vcEmailBuffer,nRemains,"%s : Error. Wrong TYPE for the DOOCS address \"%s\"\n",ConfigName,DaqConf[i]->doocs_url);
                                nWrited += nSingle;
                                nRemains -= nSingle;
                                nRemains = nRemains<0 ? 0:nRemains;
                                pTemporal = DaqConf[i];
                                memmove(DaqConf+i,DaqConf+i+1,sizeof(struct DaqConfig*)*(CONFIG_COUNTER-i-1));
                                --CONFIG_COUNTER;
                                delete pTemporal;
                                //StatusConfig = 0;
				fclose(log_ptr);
			}
		}			
	}
	if( StatusConfig == 1 )
	{
		log_ptr =  fopen(logstr,"a");	
		walltime = ::time(0);		
		fprintf(log_ptr,"%s : Testing of \"CONFIG\" file passed SUCCESSFULLY  %s\n",ConfigName,asctime(localtime(&(walltime))));		
		fclose(log_ptr);
	}
	else
	{
		log_ptr =  fopen(logstr,"a");	
		walltime = ::time(0);		
		fprintf(log_ptr,"%s : ERROR ! Testing of \"CONFIG\" file passed NOT SUCCESSFULLY  %s\n",ConfigName,asctime(localtime(&(walltime))));
		fclose(log_ptr);
	}
	if ( StatusConfig == 0 ) ThreadStatus_.set_value(0);

        if(nWrited)
        {
            snprintf(vcSubjectBuffer,SUBJ_BUF_LEN,"Email from %s",ConfigName);
            SendMail1(vcSenderName,nRecipts,tos,0,NULL,vcSubjectBuffer, vcEmailBuffer);
        }

        char* pcName;

        for(int nIndex(0); nIndex < nRecipts && bDelete; ++nIndex)
        {
            pcName = const_cast<char*>(tos[nIndex]);
            free(pcName);
        }
	return ;
}


void EqFctpitznoadc2::parse_condition()
{
	StatusCondition = 1;
	
	sprintf(ConditionFile,"/doocs/data/DAQdata/conditions/%s.cond",ConditionName);					 
	cond_ptr =	fopen(ConditionFile,"r");
	CONDITION_COUNTER = 0;	
	
	while ( fgets(data, positions, cond_ptr) != NULL)
	{	
		pn = strpbrk(data,LN);
		if( ( pn == 0 ) || ( pn[0] == '#' ) )		continue;
		DaqCond[CONDITION_COUNTER] = new struct DaqCondition;		
		
    	sscanf(&data[0],"%s %f %f %d",DaqCond[CONDITION_COUNTER]->doocs_url,
											  &DaqCond[CONDITION_COUNTER]->min,
											  &DaqCond[CONDITION_COUNTER]->max,
											  &DaqCond[CONDITION_COUNTER]->type);
		CONDITION_COUNTER++;
	}		
	fclose(cond_ptr);	

	log_ptr =  fopen(logstr,"a");	
	fprintf(log_ptr,"%s : ------ CONDITION FILE TESTING  ------\n",ConditionName);
	fclose(log_ptr);
		
	if(CONDITION_COUNTER > 0 )
	{
		for(i=0; i < CONDITION_COUNTER; i++)
		{
			ea->adr( (char *)DaqCond[i]->doocs_url);
			ed->init();			
			result = eq->get(ea,ed);

			if(result->error() != 0) 	
			{	
				log_ptr =  fopen(logstr,"a");	
				fprintf(log_ptr,"%s : Attention. DOOCS address \"%s\" does not respond. Not possibly to check up TYPE\n",ConditionName,DaqCond[i]->doocs_url);
				fclose(log_ptr);
			}
			else 
			{
				if( result->type() != DaqCond[i]->type )
				{
					log_ptr =  fopen(logstr,"a");	
					fprintf(log_ptr,"%s : Error. Wrong TYPE for the DOOCS address \"%s\"\n",ConditionName,DaqCond[i]->doocs_url);
					StatusCondition = 0;
					fclose(log_ptr);
				}
			}			
		}	
	}	
	if( StatusCondition == 1 )
	{
		log_ptr =  fopen(logstr,"a");	
		walltime = ::time(0);			
		fprintf(log_ptr,"%s : Testing of \"CONDITION\" file passed SUCCESSFULLY  %s\n",ConditionName,asctime(localtime(&(walltime))));
		fclose(log_ptr);
	}
	else
	{
		log_ptr =  fopen(logstr,"a");
		walltime = ::time(0);			
		fprintf(log_ptr,"%s : ERROR ! Testing of \"CONDITION\" file passed NOT SUCCESSFULLY  %s\n",ConditionName,asctime(localtime(&(walltime))));
		fclose(log_ptr);
	}	
	if ( StatusCondition == 0 ) ThreadStatus_.set_value(0);	
	return ;
}


void* cond_thread(void *arg)
{
	EqFctpitznoadc2* eq_fct_ = (EqFctpitznoadc2*) arg;
	
	EqAdr      *ea;
	EqData     *ed;
	EqData     *result;	
	EqCall     *eq;	
	
	int     StatusCondition_test;
	float   float_;
	int     i;
	
	eq     = new EqCall();
	ea     = new EqAdr();
	ed     = new EqData();
	result = new EqData();
	
	struct timespec ts;
	ts.tv_sec = 2;
	ts.tv_nsec = 0;	
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );	
	
	while( eq_fct_->ThreadStatus_.value() == 1 )
	{
		StatusCondition_test = 1;

		for(i=0; i < CONDITION_COUNTER; i++)
		{
			switch ( DaqCond[i]->type ) 
			{
			case 1 :
				ea->adr( (char *)DaqCond[i]->doocs_url);
				ed->init();			
				result = eq->get(ea,ed);
				if(result->error() == 0)
				{ 				
					float_ = (float)(result->get_int());
					if( float_ < DaqCond[i]->min ) StatusCondition_test = 0;
					if( float_ > DaqCond[i]->max ) StatusCondition_test = 0;	
				}					
				break;
			case 2 :
				ea->adr( (char *)DaqCond[i]->doocs_url);
				ed->init();			
				result = eq->get(ea,ed);
				if(result->error() == 0)
				{ 				
					float_ = (float)(result->get_float());
					if( float_ < DaqCond[i]->min ) StatusCondition_test = 0;
					if( float_ > DaqCond[i]->max ) StatusCondition_test = 0;	
				}					
				break;
			case 4 :
				ea->adr( (char *)DaqCond[i]->doocs_url);
				ed->init();			
				result = eq->get(ea,ed);
				if(result->error() == 0)
				{				
					float_ = (float)(result->get_int());
					if( float_ < DaqCond[i]->min ) StatusCondition_test = 0;
					if( float_ > DaqCond[i]->max ) StatusCondition_test = 0;	
				}					
				break;
				default :
				break;				
			}
		}

		StatusCondition = StatusCondition_test;
		if(StatusCondition == 1) {	eq_fct_->Conds_.set_value(1); } else {	eq_fct_->Conds_.set_value(0); }	
			
		nanosleep(&ts, NULL);
	}
	pthread_exit(arg);	
}


void *fill_struct(void *arg)
{
	struct DaqConfig *message;
	message = (struct DaqConfig *) arg;
	
	time_t ltime;	
	
	IFFF*	  POLYPARA;
	int     samples;
	
	EqAdr*  adr;
	EqData* src;
	EqData* dst;
	EqCall* call;
	
	call = new EqCall();
	adr  = new EqAdr();
	src  = new EqData();
	dst  = new EqData();
	
	samples = message->samples;
	
	time(&ltime);
	sprintf(message->statistic_time,"%s",(char *)(ctime(&ltime)));	
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 1000000;	
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );		
	
	while( message->eq_fct_->ThreadStatus_.value() == 1 )
	{
	
		if(board_channel[(message->id)] == 1)
		{
			board_channel[(message->id)] = 0;
			
			if( StatusCondition == 1 )
			{
				adr->adr(message->doocs_url);						
				src->init();
				dst = call->get(adr, src);

				if(dst->error() == 0) 	
				{
					switch ( message->type ) 
					{
						case  1 :
							transit[message->id][message->carS]->int_value = dst->get_int();
							transit[message->id][message->carS]->buffer    = GENSTATIC;
							transit[message->id][message->carS]->time      = SECSTATIC;

							if( message->carS == (COUNT - 1) ) { message->carS = 0; }
							else { message->carS = message->carS + 1; }						
						break;
						case  2 :
							transit[message->id][message->carS]->float_value = dst->get_float();
							transit[message->id][message->carS]->buffer      = GENSTATIC;
							transit[message->id][message->carS]->time        = SECSTATIC;

							if( message->carS == (COUNT - 1) ) { message->carS = 0; }
							else { message->carS = message->carS + 1; }						
						break;
						case  3 :
							sprintf(transit[message->id][message->carS]->char_array,"%s",dst->get_char_array());
							transit[message->id][message->carS]->buffer      = GENSTATIC;
							transit[message->id][message->carS]->time        = SECSTATIC;

							if( message->carS == (COUNT - 1) ) { message->carS = 0; }
							else { message->carS = message->carS + 1; }						
						break;					
						case  4 :
							transit[message->id][message->carS]->int_value = dst->get_int();
							transit[message->id][message->carS]->buffer    = GENSTATIC;
							transit[message->id][message->carS]->time      = SECSTATIC;

							if( message->carS == (COUNT - 1) ) { message->carS = 0; }
							else { message->carS = message->carS + 1; }						
						break;
						case  6 :
							transit[message->id][message->carS]->float_value = (float)(dst->get_double());
							transit[message->id][message->carS]->buffer      = GENSTATIC;
							transit[message->id][message->carS]->time        = SECSTATIC;

							if( message->carS == (COUNT - 1) ) { message->carS = 0; }
							else { message->carS = message->carS + 1; }						
						break;
						case 15 :
							POLYPARA = dst->get_ifff();
							sprintf(transit[message->id][message->carS]->IFFF_array,"%d  %e  %e  %e",
													POLYPARA->i1_data,POLYPARA->f1_data,POLYPARA->f2_data,POLYPARA->f3_data);
							transit[message->id][message->carS]->buffer      = GENSTATIC;
							transit[message->id][message->carS]->time        = SECSTATIC;

							if( message->carS == (COUNT - 1) ) { message->carS = 0; }
							else { message->carS = message->carS + 1; }						
						break;					
						default :
						break;				
					}
					
					if(NOALARM_COUNTER <= CONFIG_COUNTER)
					{
						NOALARM_COUNTER = NOALARM_COUNTER + 1;
					}
					else
					{
						if( (message->eq_fct_->Alarm_.value()) == 1 ) { message->eq_fct_->Alarm_.set_value(2); }
					}
					
				}
				else
				{
					if( (message->eq_fct_->Alarm_.clean == 0) && (message->eq_fct_->write_file == 0) )
					{
						NOALARM_COUNTER = 0;
						message->statistic = message->statistic + 1;
						for(int i=0; i < 6; i++)
						{
							time(&ltime);
							sprintf(message->statistic_time,"%s",(char *)(ctime(&ltime)));

							if( strcmp(message->statistic_time,"") != 0) { break; }	

						}
						message->eq_fct_->Alarm_.set_value(1);
					}
					nanosleep(&ts, NULL);
				}					
			}
			else
			{
				nanosleep(&ts, NULL);
			}				
		}
		else
		{
			nanosleep(&ts, NULL);
		}
	}
	pthread_exit(arg);
}


void* root_thread(void *arg)
{
	EqFctpitznoadc2* eq_fct_ = (EqFctpitznoadc2*) arg;

	struct tm * timeinfo;
	
	time_t  walltime;		
	
	char    root_fn[positions];
	char    cache_fn[positions];		
	char    write_dir[positions];
	char    writecache_dir[positions];		
	char    dir_name[positions];
	
	struct dirent *drent;	
	char    delfile[positions];
	char    delfile1[positions];		
	DIR    *dir;	
	
	char    cachedir_name[positions];	
	char    year[40];
	char    month[40];
	char    day[40];
	char    hour[40];
	char    minute[40];
	
	int i;
			
	TFile*  file;
	TTree*  tree[properties];
	
	int files_in_dir = 2;
	
	int	Close_File_R;
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 10000000;
	
	struct timespec ts1;
	ts1.tv_sec = 0;
	ts1.tv_nsec = 1000000;	
	
	struct timespec ts2;
	ts2.tv_sec = 61;
	ts2.tv_nsec = 0;		
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );		
	
	while( StatusCondition == 0 ) 
	{
		if( eq_fct_->ThreadStatus_.value() == 0 ) { exit(0); }			 
		nanosleep(&ts, NULL); 
	}			
	
	while( eq_fct_->ThreadStatus_.value() == 1 )
	{
		walltime = ::time(0);	
		timeinfo = localtime( &(walltime) );
		
		strftime (year,40,"%Y",timeinfo);
		strftime (month,40,"%m",timeinfo);
		strftime (day,40,"%d",timeinfo);
		strftime (hour,40,"%H",timeinfo);
		strftime (minute,40,"%M",timeinfo);		
		
		sprintf(writecache_dir,"/acs/pitz/daq");	

		sprintf(cachedir_name,"%s",writecache_dir);		

		sprintf(cachedir_name,"%s/%s",cachedir_name,year);	
		mkdir(cachedir_name, S_IRWXU | S_IRWXG | S_IRWXO);

		sprintf(cachedir_name,"%s/%s",cachedir_name,month);	
		mkdir(cachedir_name, S_IRWXU | S_IRWXG | S_IRWXO);

		sprintf(cachedir_name,"%s/%s",cachedir_name,eq_fct_->ConfigName);	
		mkdir(cachedir_name, S_IRWXU | S_IRWXG | S_IRWXO);		

		sprintf(cache_fn,"%s/PITZ_DATA.%s.%s-%s-%s-%s%s.root",cachedir_name,eq_fct_->ConfigName,year,month,day,hour,minute);

		sprintf(write_dir,"/doocs/data/DAQdata/daqL");	
//		sprintf(write_dir,"/export/doocs/daq/daqL");	
		sprintf(dir_name,"%s",write_dir);	

		sprintf(dir_name,"%s/%s",dir_name,year);
		mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);		

		sprintf(dir_name,"%s/%s",dir_name,month);	
		mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);

		sprintf(dir_name,"%s/%s",dir_name,eq_fct_->ConfigName);	
		mkdir(dir_name, S_IRWXU | S_IRWXG | S_IRWXO);

		do
		{
			dir = opendir(dir_name);
			rewinddir(dir);
			i = 0;
			while( (drent = readdir(dir)) != NULL )			
			{
				if( (strcmp(drent->d_name,"..") != 0) && (strcmp(drent->d_name,".") != 0) )
				{
					if( i == 0 )
					{
						sprintf(delfile,"%s/%s",dir_name,drent->d_name);						
					}
					sprintf(delfile1,"%s/%s",dir_name,drent->d_name);
					if( (strcmp(delfile,delfile1)) > 0 ) { strcpy(delfile,delfile1); }
					i++;	
				} 					
			}
			if(i > files_in_dir) { unlink(delfile); }
			closedir(dir);
		}  while(i > files_in_dir);

		sprintf(root_fn,"%s/PITZ_DATA.%s.%s-%s-%s-%s%s.root",dir_name,eq_fct_->ConfigName,year,month,day,hour,minute);	
		
  		file = new TFile(root_fn,"UPDATE","DATA",1);       // SetCompressionLevel(1)
  		if (file->IsZombie()) 
		{
			walltime = ::time(0);		
			fprintf(stderr,"%s : Error opening ROOT file  %s\n",eq_fct_->ConfigName,asctime(localtime(&(walltime))));
    		exit(-1);
  		}
		file->cd(); gFile = file;

		for(i=0; i < CONFIG_COUNTER; i++)
		{
			switch ( DaqConf[i]->type ) 
			{
				case  1 :
					file->cd(); gFile = file;
					tree[i] = new TTree(DaqConf[i]->daq_name, "DATA");				
					tree[i]->Branch(DaqConf[i]->daq_name, &(D_C01[i]->time),"time/I:buffer/I:int_value/I");									
				break;			
				case  2 :
					file->cd(); gFile = file;				
					tree[i] = new TTree(DaqConf[i]->daq_name, "DATA");				
					tree[i]->Branch(DaqConf[i]->daq_name, &(D_C02[i]->time),"time/I:buffer/I:float_value/F");									
				break;
				case  3 :
					file->cd(); gFile = file;				
					tree[i] = new TTree(DaqConf[i]->daq_name, "DATA");				
					tree[i]->Branch(DaqConf[i]->daq_name, &(D_C03[i]->time),"time/I:buffer/I:char_array[60]/C");									
				break;				
				case  4 :
					file->cd(); gFile = file;				
					tree[i] = new TTree(DaqConf[i]->daq_name, "DATA");				
					tree[i]->Branch(DaqConf[i]->daq_name, &(D_C01[i]->time),"time/I:buffer/I:int_value/I");									
				break;
				case  6 :
					file->cd(); gFile = file;				
					tree[i] = new TTree(DaqConf[i]->daq_name, "DATA");				
					tree[i]->Branch(DaqConf[i]->daq_name, &(D_C02[i]->time),"time/I:buffer/I:float_value/F");									
				break;
				case 15 :
					file->cd(); gFile = file;				
					tree[i] = new TTree(DaqConf[i]->daq_name, "DATA");				
					tree[i]->Branch(DaqConf[i]->daq_name, &(D_C15[i]->time),"time/I:buffer/I:IFFF_array[60]/C");									
				break;				
				default :
				break;								
			}				
		}
			
		ASOVA_start = 0;
		end_TIME_R   = 0;
		end_BUFFER_R = 0;		
		
		Close_File_R = 0;

		while( (Close_File_R == 0) && (eq_fct_->ThreadStatus_.value() == 1) )
		{
		
			for(i=0; i < CONFIG_COUNTER; i++)
			{			
				if( DaqConf[i]->carR != DaqConf[i]->carS )
				{
					
					AtLeastOne[i] = 1;
					
					if(ASOVA_start == 0) 
					{ 
						ASOVA_start = 1; 
						begin_TIME_R   = transit[i][DaqConf[i]->carR]->time; 
						begin_BUFFER_R = transit[i][DaqConf[i]->carR]->buffer;
					}					
					
					switch ( DaqConf[i]->type ) 
					{
						case  1 :
							D_C01[i]->time        = (Int_t)(transit[i][DaqConf[i]->carR]->time);
							D_C01[i]->buffer      = (Int_t)(transit[i][DaqConf[i]->carR]->buffer);
							D_C01[i]->int_value   = (Int_t)(transit[i][DaqConf[i]->carR]->int_value);
							
							file->cd(); gFile = file;	tree[i]->Fill();							
							if( DaqConf[i]->carR == (COUNT - 1) ) { DaqConf[i]->carR = 0; tree[i]->AutoSave("SaveSelf"); }
							else { DaqConf[i]->carR = DaqConf[i]->carR + 1; }								
						break;					
						case  2 :
							D_C02[i]->time        = (Int_t)(transit[i][DaqConf[i]->carR]->time);
							D_C02[i]->buffer      = (Int_t)(transit[i][DaqConf[i]->carR]->buffer);
							D_C02[i]->float_value = (Float_t)(transit[i][DaqConf[i]->carR]->float_value);

							file->cd(); gFile = file;	tree[i]->Fill();							
							if( DaqConf[i]->carR == (COUNT - 1) ) { DaqConf[i]->carR = 0; tree[i]->AutoSave("SaveSelf"); }
							else { DaqConf[i]->carR = DaqConf[i]->carR + 1; }								 
						break;
						case  3 :
							D_C03[i]->time        = (Int_t)(transit[i][DaqConf[i]->carR]->time);
							D_C03[i]->buffer      = (Int_t)(transit[i][DaqConf[i]->carR]->buffer);
							sprintf(D_C03[i]->char_array,"%s", transit[i][DaqConf[i]->carR]->char_array);

							file->cd(); gFile = file;	tree[i]->Fill();							
							if( DaqConf[i]->carR == (COUNT - 1) ) { DaqConf[i]->carR = 0; tree[i]->AutoSave("SaveSelf"); }
							else { DaqConf[i]->carR = DaqConf[i]->carR + 1; }
						break;														
						case  4 :
							D_C01[i]->time        = (Int_t)(transit[i][DaqConf[i]->carR]->time);
							D_C01[i]->buffer      = (Int_t)(transit[i][DaqConf[i]->carR]->buffer);
							D_C01[i]->int_value   = (Int_t)(transit[i][DaqConf[i]->carR]->int_value);
							
							file->cd(); gFile = file;	tree[i]->Fill();							
							if( DaqConf[i]->carR == (COUNT - 1) ) { DaqConf[i]->carR = 0; tree[i]->AutoSave("SaveSelf"); }
							else { DaqConf[i]->carR = DaqConf[i]->carR + 1; }								
						break;
						case  6 :
							D_C02[i]->time        = (Int_t)(transit[i][DaqConf[i]->carR]->time);
							D_C02[i]->buffer      = (Int_t)(transit[i][DaqConf[i]->carR]->buffer);
							D_C02[i]->float_value = (Float_t)(transit[i][DaqConf[i]->carR]->float_value);

							file->cd(); gFile = file;	tree[i]->Fill();							
							if( DaqConf[i]->carR == (COUNT - 1) ) { DaqConf[i]->carR = 0; tree[i]->AutoSave("SaveSelf"); }
							else { DaqConf[i]->carR = DaqConf[i]->carR + 1; }								 
						break;
						case 15 :
							D_C15[i]->time        = (Int_t)(transit[i][DaqConf[i]->carR]->time);
							D_C15[i]->buffer      = (Int_t)(transit[i][DaqConf[i]->carR]->buffer);
							sprintf(D_C15[i]->IFFF_array,"%s", transit[i][DaqConf[i]->carR]->IFFF_array);

							file->cd(); gFile = file;	tree[i]->Fill();							
							if( DaqConf[i]->carR == (COUNT - 1) ) { DaqConf[i]->carR = 0; tree[i]->AutoSave("SaveSelf"); }
							else { DaqConf[i]->carR = DaqConf[i]->carR + 1; }
						break;						
						default :
						break;								
					}											
				}
			}
			nanosleep(&ts1, NULL); 
			if( file->GetSize() >= eq_fct_->root_length.value() ) { Close_File_R = 1; }			
		}
		
		for(i=0; i < CONFIG_COUNTER; i++) 
		{ 
			if(DaqConf[i]->carR != 0)
			{
				file->cd(); gFile = file;	tree[i]->AutoSave("SaveSelf");
			} 
		}		
		
		sprintf(root_fn_old,"%s",root_fn);
		sprintf(cache_fn_old,"%s",cache_fn);
		
		for(i=0; i < CONFIG_COUNTER; i++)
		{	
			if( AtLeastOne[i] == 1 )
			{
				if(DaqConf[i]->carR == 0)
				{
					end_TIME_R_ch   = transit[i][COUNT-1]->time;
					end_BUFFER_R_ch = transit[i][COUNT-1]->buffer;		
				}
				else
				{
					end_TIME_R_ch   = transit[i][(DaqConf[i]->carR) - 1]->time;
					end_BUFFER_R_ch = transit[i][(DaqConf[i]->carR) - 1]->buffer;		
				}
			}
			if( end_TIME_R   < end_TIME_R_ch  ) { end_TIME_R   = end_TIME_R_ch;   }
			if( end_BUFFER_R < end_BUFFER_R_ch) { end_BUFFER_R = end_BUFFER_R_ch; }
		}		
		
		sprintf(ASOVA_str,"%d:%02d,%d:%02d,%s",begin_TIME_R,begin_BUFFER_R,end_TIME_R,end_BUFFER_R,cache_fn);
		
		for(i=0; i < CONFIG_COUNTER; i++)
		{
			AtLeastOne_cp[i] = AtLeastOne[i];
			AtLeastOne[i]    = 0;				
		}		
		
		file->cd();
		gFile = file;
		file->TDirectory::DeleteAll();
		file->TDirectory::Close();
   	delete file;
   	file = 0;	
		
		ASOVA_true = 1;
			
	}
	
	nanosleep(&ts2, NULL); 
	
	exit(0);

	pthread_exit(arg);
}


void* asova_thread(void* arg)
{
	fstream* index_ptr;	
	char  indexstr[positions];
	int i;
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000;
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );		

	while( 1 )
	{
	
		if( ASOVA_true == 1 )
		{
			ASOVA_true = 0;
			
//			sprintf(dccp_char,"/opt/products/bin/dccp %s %s 2>/dev/null", root_fn_old, cache_fn_old);
			sprintf(dccp_char,"/opt/products/bin/dccp -d 0 %s %s", root_fn_old, cache_fn_old);				
				
			system( dccp_char );

			for(i=0; i < CONFIG_COUNTER; i++)
			{
				if( AtLeastOne_cp[i]  == 1 )
				{
// 				sprintf(indexstr,"/doocs/data/DAQdata/INDEXL/%s.idx",DaqConf[i]->daq_name);
//					sprintf(indexstr,"/export/doocs/daq/INDEXL/%s.idx",DaqConf[i]->daq_name);							
	 				sprintf(indexstr,"/doocs/data/DAQdata/INDEX/%s.idx",DaqConf[i]->daq_name);			

					index_ptr = new fstream(indexstr, ios_base::out | ios_base::app );
					if(index_ptr->is_open())
  					{
						*index_ptr << ASOVA_str << endl << flush;				
						index_ptr->flush();
					}
					index_ptr->close();
					delete index_ptr;				

					AtLeastOne_cp[i] = 0;				
				}
			}	
		}
		else
		{
			nanosleep(&ts, NULL); 			
		}
	}	
	pthread_exit(arg);
}


void* mc_thread(void * arg)
{
	EqFctpitznoadc2* eq_fct_ = (EqFctpitznoadc2*) arg;
	
	struct       timeval  tv1;
	struct       timeval  tv2;
	int          tv1s,tv1us;
	int          tv2s,tv2us;	
	
	int   GE;
	int   val;
	
	SECSTATIC = shareptr[H_count].seconds;
	GENSTATIC = shareptr[H_count].gen_event;			
	
	gettimeofday(&tv1, NULL );
	tv1s  = (int)(tv1.tv_sec);
	tv1us = (int)(tv1.tv_usec);	
	
	gettimeofday(&tv2, NULL );
	tv2s  = (int)(tv2.tv_sec);
	tv2us = (int)(tv2.tv_usec);
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 300000000;
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );		
	
	while( eq_fct_->ThreadStatus_.value() == 1 )
	{
		
		gettimeofday(&tv2, NULL );
		tv2s  = (int)(tv2.tv_sec);
		tv2us = (int)(tv2.tv_usec);
		
		if( (tv2s - tv1s) >= 10 )
		{
			eq_fct_->datastream_.set_value(0);	
		}
		else
		{
			eq_fct_->datastream_.set_value(1);
		}	
			
		val = 10*(eq_fct_->Conds_.value()) + eq_fct_->datastream_.value();
		eq_fct_->serverstatus_.set_value(val);
			
		if( (tv2s - tv1s) < 2 )
		{
			while( tv2s > tv1s)	
			{
				tv2s = tv2s - 1;
				tv2us = tv2us + 1000000;	
			}				
		}

		if( ((tv2s - tv1s) >= 2) || ((tv2us - tv1us ) >= 900000) )					
		{
			GE = shareptr[H_count].gen_event;
					
			if( GE != GENSTATIC )
			{
				GENSTATIC = GE;
				SECSTATIC = shareptr[H_count].seconds;

				for(int i=0; i < CONFIG_COUNTER; i++) 					 
				{ 
					board_channel[i] = 1;
				}					
				if( (DaqConf[0]->eq_fct_->ThreadStatus_.value()) == 1 )
				{
					DaqConf[0]->eq_fct_->gen_event_.set_value(GENSTATIC);
				}

				gettimeofday(&tv1, NULL );
				tv1s  = (int)(tv1.tv_sec);
				tv1us = (int)(tv1.tv_usec);											

			}	

		}		
		nanosleep(&ts, NULL); 	
	}	
	pthread_exit(arg);
}


void* stat_thread(void * arg)
{
	EqFctpitznoadc2* eq_fct_ = (EqFctpitznoadc2*) arg;
	
	FILE* statistic_ptr;

	char  StatFile[positions];
	
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = 100000000;
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );
			
	for(int i=0; i < CONFIG_COUNTER; i++)
	{		
		if(DaqConf[i]->statistic > 0)
		{
			eq_fct_->Alarm_.set_value(2);
		}
	}
	
	while( eq_fct_->ThreadStatus_.value() == 1 )
	{
	
		if(eq_fct_->Alarm_.clean == 1)
		{
			nanosleep(&ts, NULL);
		
			for(int i=0; i < CONFIG_COUNTER; i++)
			{
				DaqConf[i]->statistic = 0;
			}
			sprintf(StatFile,"/doocs/data/DAQdata/statistic/%s.txt",eq_fct_->ConfigName);
			statistic_ptr = fopen(StatFile,"w");
			fclose(statistic_ptr);
			
			eq_fct_->Alarm_.clean = 0;	
		}
		
		if( (eq_fct_->write_file == 1) && (eq_fct_->Alarm_.value() > 0) )
		{
			nanosleep(&ts, NULL);		
		
			sprintf(StatFile,"/doocs/data/DAQdata/statistic/%s.txt",eq_fct_->ConfigName);
			statistic_ptr = fopen(StatFile,"w");

			for(int i=0; i < CONFIG_COUNTER; i++)
			{		
				if(DaqConf[i]->statistic > 0)
				{
					fprintf(statistic_ptr,"%-40s %-60s %-14d    %-60s\n",
							DaqConf[i]->daq_name,DaqConf[i]->doocs_url,DaqConf[i]->statistic,DaqConf[i]->statistic_time);
				}
			}
			fclose(statistic_ptr);
			eq_fct_->write_file = 0;
		}

		nanosleep(&ts, NULL);
	}		
		

	pthread_exit(arg);
}















