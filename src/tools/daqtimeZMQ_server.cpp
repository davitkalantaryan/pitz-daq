
//#define CONFIGURABLE

#ifndef  CONFIGURABLE
//#define     PUBLISHER_INFO      "tcp://vmepitz14:5566"
//#define     PUBLISHER_INFO      "tcp://mtcapitzcpu3:5566"
#define     PUBLISHER_INFO      "tcp://mtcapitzcpu4:5566"
#endif

#include <iostream>
#include	<cstdio>
#include	<cstdlib>
#include <cmath>
#include	<ctime>
#include <fstream>
#include <cstring>
#include <cerrno>

#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/ioctl.h>
#include	<sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <dirent.h>
#include <libgen.h>
#include <sys/time.h> 
#include <sys/resource.h> 
#include	<setjmp.h>
#include <stdlib.h>

//#include "zmq.hpp"
#include "daqtimeZMQ_server.h"

#include "stringparser1.h"




//////////////////////////////////////////////////////////////////////////////////////////////////////
ZMQsuscrb::MutexDv::MutexDv()
{
#ifdef WIN32
        m_MutexLock = CreateMutex( NULL, FALSE, NULL );
#else

        pthread_mutexattr_t attr;
        pthread_mutexattr_init( &attr );
        pthread_mutexattr_settype( &attr, PTHREAD_MUTEX_ERRORCHECK );

        pthread_mutex_init( &m_MutexLock, &attr );

        pthread_mutexattr_destroy( &attr );
#endif
}



ZMQsuscrb::MutexDv::~MutexDv()
{

#ifdef WIN32
        CloseHandle( m_MutexLock );
#else
        pthread_mutex_destroy( &m_MutexLock );
#endif

}


/*
 * EDEADLK	-	The current thread already owns the mutex.
 *
 */
int ZMQsuscrb::MutexDv::Lock()
{
#ifdef WIN32
        return WaitForSingleObject( m_MutexLock, INFINITE ) == WAIT_OBJECT_0 ? 0 : EDEADLK ;
#else
        return pthread_mutex_lock( &m_MutexLock );
#endif
}



/*
 * EINVAL	-	Mutex is not an initialized mutex.
 * EFAULT	-	Mutex is an invalid pointer.
 * EPERM	-	The calling thread does not own the mutex.
 *
 */
int ZMQsuscrb::MutexDv::UnLock()
{
#ifdef WIN32
        return ReleaseMutex( m_MutexLock ) ? 0 : EPERM;
#else
        return pthread_mutex_unlock( &m_MutexLock );
#endif
}
////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
void* ZMQsuscrb::m_pContextZMQ = NULL;
ZMQsuscrb::MutexDv ZMQsuscrb::m_MutexContext;

void AtExitZMQ(void)
{
    printf("\n\n\nAtExitZMQ\n\n");
    if(ZMQsuscrb::m_pContextZMQ)
    {
        zmq_term(ZMQsuscrb::m_pContextZMQ);
        zmq_ctx_destroy(ZMQsuscrb::m_pContextZMQ);
    }
}



ZMQsuscrb::ZMQsuscrb()
        :   m_pSocketZMQ(NULL)
{
    m_MutexContext.Lock();
    if(!m_pContextZMQ)
    {
        m_pContextZMQ = zmq_ctx_new ();

        if(m_pContextZMQ)
        {
            atexit(&AtExitZMQ);
        }
    }
    m_MutexContext.UnLock();

    if(!m_pContextZMQ)throw "No contest!";

    m_pSocketZMQ = zmq_socket (m_pContextZMQ, ZMQ_SUB);

    if(!m_pSocketZMQ)throw "No socket";

    //subscriber->setsockopt(ZMQ_SUBSCRIBE, "", 0);
    zmq_setsockopt (m_pSocketZMQ, ZMQ_SUBSCRIBE, NULL, 0);
}



ZMQsuscrb::~ZMQsuscrb()
{
    ZMQsuscrb::Close();
    //zmq_ctx_term()
}



int ZMQsuscrb::Connect(const char* a_cpcPublisher)
{
    return zmq_connect (m_pSocketZMQ, a_cpcPublisher);
}



void ZMQsuscrb::Close()
{
    if(m_pSocketZMQ)zmq_close(m_pSocketZMQ);
}






using namespace std;

extern  void* zmq_thread(void *);

key_t  SHMKEY = ftok("/export/doocs/server/daqtimeZMQ_server/adc_daqtimeZMQ",'a');

//========================================================================================
const int  H_count = 1200;

struct  H_struct
{
  int   seconds;
  int   microseconds;
  int   gen_event;
  int   rep_rate; 
}Hstruct;

struct H_struct *shareptr = &Hstruct;


int        shmid;
int        bad_cond;
int        restart;

struct       timeval  tv1;
struct       timeval  tv2;
int          tv1s,tv1us;
int          tv2s,tv2us;
int          differ;
	
char       charptr1[400];
char       charptr10[400];
char       charptr2[400];
char       charptr20[400];

char       info[40];
//========================================================================================
struct TTelegram_L 
{
    int32_t genEvent;
    int32_t sec;    
    int32_t usec;    
};
const int TTelegram_L_l = sizeof(TTelegram_L);
union DATA_union_L
{
	char ch[TTelegram_L_l];
	struct TTelegram_L ttelegram_L;
};
union DATA_union_L  data_unionL;	
//========================================================================================

pthread_t        ZMQ_thread;

void printtostderrL (const char *dev, const char *str)
{
	struct tm           t;
	long                atime;
	char                buf [80];

	fseek (stderr, 0, SEEK_END);
	atime = time (0);
	localtime_r (&atime, &t);
	strftime (buf, (int) 80,"%H:%M.%S  %e.%m.%Y", &t);
	fprintf(stderr, "%-16s %-20s -> %s\n",dev, buf, str);
}

void enco(int &arg)
{
	union integer
	{
		int argc;
		char ch[4];
	} integ;
	
	char ch;
	
	integ.argc = arg;
	
	ch = integ.ch[0];
	integ.ch[0] = integ.ch[3];
	integ.ch[3] = ch;
	
	ch = integ.ch[1];
	integ.ch[1] = integ.ch[2];
	integ.ch[2] = ch;
	
	arg = integ.argc;	
	
	return;	
}


static inline void fastSwap(void* a_pNew)
{
    char* pcValueToSwap = (char*)a_pNew;
    char cTemp = pcValueToSwap[0];
    pcValueToSwap[0] = pcValueToSwap[3];
    pcValueToSwap[3] = cTemp;

    cTemp = pcValueToSwap[1];
    pcValueToSwap[1] = pcValueToSwap[2];
    pcValueToSwap[2] = cTemp;

    ///////////////////////////////////
    pcValueToSwap = (char*)a_pNew + 4;
    cTemp = pcValueToSwap[0];
    pcValueToSwap[0] = pcValueToSwap[3];
    pcValueToSwap[3] = cTemp;

    cTemp = pcValueToSwap[1];
    pcValueToSwap[1] = pcValueToSwap[2];
    pcValueToSwap[2] = cTemp;


    ///////////////////////////////////
    pcValueToSwap = (char*)a_pNew + 8;
    cTemp = pcValueToSwap[0];
    pcValueToSwap[0] = pcValueToSwap[3];
    pcValueToSwap[3] = cTemp;

    cTemp = pcValueToSwap[1];
    pcValueToSwap[1] = pcValueToSwap[2];
    pcValueToSwap[2] = cTemp;
}



void* zmq_thread(void *arg)
{
        ZMQsuscrb *p = (ZMQsuscrb*)arg;

	int nbytes;
	int not_first = 0;
	int Old_genEvent = 0;
	
	char       charptr[400];
		
	struct timespec ts2;
	ts2.tv_sec = 0;
	ts2.tv_nsec = 1000000;	
	
	sigset_t sigset;	
	sigfillset( &sigset );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );	
		

	while( 1 )
	{
		nanosleep(&ts2, NULL);

                nbytes = p->recv(data_unionL.ch, TTelegram_L_l, 0);
		restart  = 0;	
#ifdef LITTLE_ENDIAN_BP		
		enco(data_unionL.ttelegram_L.genEvent);
		enco(data_unionL.ttelegram_L.sec);
		enco(data_unionL.ttelegram_L.usec);	
#endif

                fastSwap(data_unionL.ch);

                //printf( "seconds = %.6d, mcSec = %.6d, EvNum = %d\n",data_unionL.ttelegram_L.sec,
                //        data_unionL.ttelegram_L.usec,data_unionL.ttelegram_L.genEvent);

		shareptr[ ((data_unionL.ttelegram_L.genEvent)% H_count) ].seconds      = data_unionL.ttelegram_L.sec;
		shareptr[ ((data_unionL.ttelegram_L.genEvent)% H_count) ].microseconds = data_unionL.ttelegram_L.usec;		
		shareptr[ ((data_unionL.ttelegram_L.genEvent)% H_count) ].gen_event    = data_unionL.ttelegram_L.genEvent;		
		shareptr[ ((data_unionL.ttelegram_L.genEvent)% H_count) ].rep_rate     = 0;	
		
		shareptr[H_count].seconds      = data_unionL.ttelegram_L.sec;
		shareptr[H_count].microseconds = data_unionL.ttelegram_L.usec;		
		shareptr[H_count].gen_event    = data_unionL.ttelegram_L.genEvent;		
		shareptr[H_count].rep_rate     = 0;	
		
		
		if(not_first == 0)
		{
			not_first = 1;
		}
		else
		{
			if( (data_unionL.ttelegram_L.genEvent - Old_genEvent) != 1 )
			{
				sprintf(charptr,"Old_genEvent: %d       New_genEvent: %d",Old_genEvent, data_unionL.ttelegram_L.genEvent);
				printtostderrL("JUMP",charptr);
			}
		}
		
		Old_genEvent = data_unionL.ttelegram_L.genEvent;
		
/*	
if( ((data_unionL.ttelegram_L.genEvent)% H_count) == 0  )  
{
		fprintf(stderr,"%10d   %10d   %10d   %10d\n",
					shareptr[H_count].seconds,shareptr[H_count].microseconds,shareptr[H_count].gen_event,shareptr[H_count].rep_rate);
}
*/

		if(nbytes != TTelegram_L_l)
		{
			bad_cond = 1;
		}

		gettimeofday(&tv1, NULL );
		tv1s  = (int)(tv1.tv_sec);
		tv1us = (int)(tv1.tv_usec);
	
	}
	return (void*)NULL;
}


int main (int /*argc*/, char */*argv*/[ ])
{
#ifdef CONFIGURABLE
        char* pcPUBLISHER_INFO = NULL;
#endif
	int check_number;
	int res;

	struct timespec ts1;
	ts1.tv_sec = 0;
	ts1.tv_nsec = 1000000;
	
	struct timespec ts3;
	ts3.tv_sec = 60;
	ts3.tv_nsec = 0;		
	
	sigset_t sigset;	
	sigfillset( &sigset );
        sigdelset( &sigset, SIGINT );
	pthread_sigmask( SIG_BLOCK, &sigset, NULL );		
	
	bad_cond = 0;
	restart  = 0;

#ifdef CONFIGURABLE

        char* pcBuffer = NULL;
        if( StringParser1::FileBuffer2("daqtimeZMQ_server.conf",&pcBuffer)<=0 )
        {
            fprintf(stderr,"daqtimeZMQ_server.conf doesn't exist!  Program will be switched off\n");
            exit(1);
        }


        if (!StringParser1::FindVariable(pcBuffer,"PUBLISHER_INFO",&pcPUBLISHER_INFO) )
        {
            free(pcBuffer);
            fprintf(stderr,"Field \"PUBLISHER_INFO\" doesn't exist in file \"daqtimeZMQ_server.conf\"\
                    \n Program will be terminated!\n");
            exit(1);
        }

        free(pcBuffer);
#endif

	sprintf(info,"%s","INFO");

	sprintf(charptr10,"%s"," ");
	sprintf(charptr20,"%s"," ");

	sprintf(charptr1,"%s","daqtime_server is started");	
	if( strcmp(charptr1,charptr10) )
	{
		sprintf(charptr10,"%s",charptr1);
		printtostderrL(info,charptr1);
	}
	
        ZMQsuscrb  *zmqclass;
        zmqclass = new ZMQsuscrb();
        zmqclass->Connect(PUBLISHER_INFO);

   shmid = shmget(SHMKEY, (sizeof(struct H_struct))*(H_count + 1), IPC_CREAT | 0664);			
   if (shmid < 0)
	{
		sprintf(charptr1,"%s"," shared memory is not created... Exit !!!");	
		if( strcmp(charptr1,charptr10) )
		{
			sprintf(charptr10,"%s",charptr1);
			printtostderrL(info,charptr1);
		}		
		exit(1);
	}
	
	if ( (shareptr = (struct H_struct *) shmat(shmid, NULL, 0)) == (struct H_struct *) -1)	
	{
		sprintf(charptr1,"%s"," can't attach to shared memory... Exit !!!");	
		if( strcmp(charptr1,charptr10) )
		{
			sprintf(charptr10,"%s",charptr1);
			printtostderrL(info,charptr1);
		}			
		exit(1);
	}	

	for(int i=0; i < (H_count + 1); i++)
	{
		shareptr[i].seconds      = 0;
		shareptr[i].microseconds = 0;		
		shareptr[i].gen_event    = 0;		
		shareptr[i].rep_rate     = 0;	
	}
	
	gettimeofday(&tv1, NULL );
	tv1s  = (int)(tv1.tv_sec);
	tv1us = (int)(tv1.tv_usec);

	gettimeofday(&tv2, NULL );
	tv2s  = (int)(tv2.tv_sec);
	tv2us = (int)(tv2.tv_usec);
	
	if ( pthread_create(&ZMQ_thread, NULL, zmq_thread, (void*)zmqclass))		
	{
		fprintf(stderr,"Could not create zmq_thread : exit!\n");		
		exit(1);
	}	

	while( 1 )
	{
		nanosleep(&ts1, NULL);
		
		gettimeofday(&tv2, NULL );
		tv2s  = (int)(tv2.tv_sec);
		tv2us = (int)(tv2.tv_usec);
		differ = tv2s - tv1s;				
	
                if(  (differ >= 200) || (bad_cond == 1) )
		{
		
			check_number = 1;					
			do
			{
				res = pthread_cancel(ZMQ_thread);
				sprintf(charptr2,"ZMQ_thread cancel error: %d",res);						
				if( strcmp(charptr2,charptr20) )
				{
					sprintf(charptr20,"%s",charptr2);
					printtostderrL("INFO",charptr2);
				}							
				if(res != 0) { check_number++; }						
			} while( (res != 0) && (check_number < 4) );
			
			if(differ >= 2)
			{
				sprintf(charptr2,"%s","No events anymore");	
				if( strcmp(charptr2,charptr20) )
				{
					sprintf(charptr20,"%s",charptr2);
					printtostderrL(info,charptr2);
				}
			}
			else
			{
			
				if(bad_cond == 1)
				{
					sprintf(charptr2,"%s","Bad data are coming !");	
					if( strcmp(charptr2,charptr20) )
					{
						sprintf(charptr20,"%s",charptr2);
						printtostderrL(info,charptr2);
					}
					bad_cond = 0;
				}	
			}					
			
                        //zmq_close (*(zmqclass->subscriber));
                        zmqclass->Close();
                        //zmq_ctx_destroy(*(zmqclass->context));
			
			if(restart < 21) { restart++; }
			for(int i=1; i < restart; i++) { nanosleep(&ts3, NULL); }
					
                        //zmqclass->initZMQ();
                        zmqclass->Connect(PUBLISHER_INFO);
			
			check_number = 1;
			do
			{
				res = pthread_create(&(ZMQ_thread), NULL, zmq_thread, (void *)zmqclass);
				sprintf(charptr2,"ZMQ_thread creation error: %d",res);						
				if( strcmp(charptr2,charptr20) )
				{
					sprintf(charptr20,"%s",charptr2);
					printtostderrL("INFO",charptr2);
				}							
				if(res != 0) { check_number++; }							
			} while( (res != 0) && (check_number < 4) );
			if(res != 0) { exit(1); } 
			
			gettimeofday(&tv1, NULL );
			tv1s  = (int)(tv1.tv_sec);
			tv1us = (int)(tv1.tv_usec);										

		}
		else
		{
			sprintf(charptr2,"%s","New events are O.K.");	
			if( strcmp(charptr2,charptr20) )
			{
				sprintf(charptr20,"%s",charptr2);
				printtostderrL(info,charptr2);
			}		
		}
	}

#ifdef CONFIGURABLE
        free(pcPUBLISHER_INFO);
#endif

	return 0;
}






















 
