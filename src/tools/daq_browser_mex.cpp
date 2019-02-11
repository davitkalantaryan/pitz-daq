#define __MEX__

//#define ROOT_IS_OK_IN_MATLAB

#include <mex.h>
//#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "createmlistener.h"
#include "createnewprocess1.h"
#include "shared_memory_svr.h"
#include "commonheader_root.h"
//#include "browsing_funcs.h"



#define     STD_BUF_LEN_MIN1     2047
//#define     _PROCES_NAME_   "/doocs/develop/kalantar/programs/cpp/works/root_tests/projects/rootreader/rootreader_qt/rootreader.stable"
//#define     _PROCES_NAME_   "/doocs/develop/kalantar/programs/cpp/works/root_tests/projects/rootreader/rootreader_qt_sl5/rootreader"
#define     _PROCES_NAME_   "rootreader"
//#define     _PROCES_NAME_   "/doocs/develop/kalantar/programs/cpp/works/pitz-daq/sys/Santiago/bin/rootreader"
typedef char* TPCHARPTR;

class InitDestroy
{
public:
    InitDestroy()
    {
        //mexPrintf("InitDestroy::InitDestroy()\n");
        if(pipe(m_vnPipes))
        //if(pipe2(m_vnPipes,O_NONBLOCK|O_CLOEXEC))
        {
            mexPrintf("Pipe creation error!\n");
            mexErrMsgTxt("Unable to open pipe");
        }

        int status = fcntl( m_vnPipes[0], F_GETFL, 0 );
        if(status == -1)
        {
            //mexPrintf("fcntl error!\n");
            //return;
            mexErrMsgTxt("fcntl get error!\n");
        }
        //status |= O_NONBLOCK;
        status |= (O_NONBLOCK | FD_CLOEXEC);
        if( fcntl( m_vnPipes[0], F_SETFL, status ) == -1)
        {
            mexErrMsgTxt("fcntl set error!\n");
        }
    }

    ~InitDestroy()
    {
        //mexPrintf("InitDestroy::~InitDestroy()\n");
        close(m_vnPipes[0]);
        close(m_vnPipes[1]);
    }

    char*   GetBuffer() {return m_vcBuffer;}

    int     GetPipe(int index)const{return m_vnPipes[index];}

    int     CreateSharedMem( size_t size)
    {
        return m_Shared.CreateShrdMem(size,SHARED_MEM_NAME);
    }

    void    CloseSharedMem()
    {
        m_Shared.Close2();
    }

    void*   GetSharedMemoryBuffer()
    {
        return m_Shared.GetMemPtr();
    }

private:
    char                m_vcBuffer[STD_BUF_LEN_MIN1+1];
    int                 m_vnPipes[2];
    Shared_Memory_Svr   m_Shared;
};


static InitDestroy s_InDst;


extern "C"
{

void mexFunction( int a_nNumOuts, mxArray *a_Outputs[],
                  int a_nNumInps, const mxArray*a_Inputs[] )
{


    const char* cpcErrorString = NULL;
    //mexPrintf("New tries!\n");
    //mexEvalString("drawnow"); // or mexCallMATLAB(0, NULL, 0, NULL, "drawnow");
    if(a_nNumInps<5)
    {
        //system("ls -al /afs/ifh.de/group/pitz/doocs/develop/kalantar/programs/cpp/works/root_tests/binaries/bin/");
        //mexEvalString("unix('ls -al /afs/ifh.de/group/pitz/doocs/develop/kalantar/programs/cpp/works/root_tests/binaries/@sys/bin/')");
        mexEvalString("unix('ls -al /afs/ifh.de/group/pitz/doocs/develop/kalantar/programs/cpp/works/root_tests/binaries/bin/')");
        mexPrintf("PID = %d, nOuts = %d, nInps = %d, pOuts = %p, pIns = %p\n",(int)getpid(),a_nNumOuts,a_nNumInps,a_Outputs,a_Inputs);
        mexErrMsgTxt("Number of arguments are too low!\n");
        return ;
    }
    int i,j, nIndex0,nIndexF;
    int type_field,time_field,event_field,isAvailable_field,data_field;
    int* pnReturn;
    int* pnOut;
    const char* vcpcFieldNames2[] = {"type","time","event","isAvailable","data"};
    char* pcShared;
    struct19* pBufferForData;
    int *pnTime, *pnEvent, *pnIsAvailable;
    float* pfSpectrum;
    int nRet(-1);
    int nReaded(0);
    int nOutAndErr = STDOUT_FILENO;
    int nVerbosity, nOption;
    long lnPid;
    char vcFrom[16];
    char vcTo[16];
    char vcOption[16];
    int nFrom,nTo;
    double* plfPtrToArg;
    void* pPtrToArg2;
    mwSize vSizes3[2];
    TPCHARPTR* argv = (TPCHARPTR*)malloc(sizeof(TPCHARPTR)*(a_nNumInps+1));

    mexPrintf("!!!!! version 2\n");

    plfPtrToArg = mxGetPr(a_Inputs[0]);
    nVerbosity = (int)(*plfPtrToArg);


    /*****************************************************/
    plfPtrToArg = mxGetPr(a_Inputs[1]);
    nOption = (int)(*plfPtrToArg); // 4.
    /*****************************************************/


    /*****************************************************/
    pPtrToArg2 = mxGetData(a_Inputs[2]);
    nFrom = mxGetClassID(a_Inputs[2])==mxINT32_CLASS ? (int)(*((int*)pPtrToArg2)) : (int)(*((double*)pPtrToArg2)); // 4.
    /*****************************************************/


    /*****************************************************/
    pPtrToArg2 = mxGetData(a_Inputs[3]);
    nTo = mxGetClassID(a_Inputs[3])==mxINT32_CLASS ? (int)(*((int*)pPtrToArg2)) : (int)(*((double*)pPtrToArg2));  // 5.
    /*****************************************************/

    if(nTo<=nFrom)
    {
        mexPrintf("nTo(%d)<=nFrom(%d)\n",nTo,nFrom);
        return;
    }

    /*****************************************************/
    // 1.
    const int cnEventsMaxCount = nOption ? ((nTo - nFrom)*10 + 1) : (nTo - nFrom + 1);
    /*****************************************************/

    /*****************************************************/
    // 3.  Later on for arg. 3 we will use mexPrintf
    /*****************************************************/

    /*****************************************************/
    // 6.
    int nNumberOfSpectra(0);

    for(i = 4; i<a_nNumInps;++i,++nNumberOfSpectra)
    {
        argv[i] = NULL;
        if(CreateAndCopy4(a_Inputs[i],&argv[i],false)!=NO_ERROR__)
        {
            mexPrintf("Couldn't convert!\n");
            goto cleaning_point;
            return;
        }
    }
    /*****************************************************/
#ifdef ROOT_IS_OK_IN_MATLAB
    BranchItem* pBranchItems = (BranchItem*)malloc(sizeof(BranchItem)*nNumberOfSpectra);
    // Fill Branch Items
    ReadDataFromDCacheByEv2(cnEventsMaxCount,pBufferForData,mexPrintf,fromEvent,toEvent,nNumberOfSpectra,pBranchItems,NULL,NULL,0);
    free(pBranchItems);
#else  // #ifdef ROOT_IS_OK_IN_MATLAB
    if(nVerbosity==11)
    {
        void* pShared;
        TYPE_MAP_FILE hMapFile;
        if (Shared_Memory_Base::createSharedMemory(&hMapFile,&pShared,_SIZE_OF_SHARED_(nNumberOfSpectra,cnEventsMaxCount),SHARED_MEM_NAME2,0)<=0)
        {
            mexPrintf("Unable to connetct to shared memory!\n");
            goto cleaning_point;
            return ;
        }
        pcShared = (char*)pShared;
    }
    else
    {
        if(s_InDst.CreateSharedMem( _SIZE_OF_SHARED_(nNumberOfSpectra,cnEventsMaxCount) ) <= 0  )
        {
            mexPrintf("Unable to create shared memory!\n");
            goto cleaning_point;
            return ;
        }
        pcShared = (char*)s_InDst.GetSharedMemoryBuffer();
    }

    pnReturn = (int*)((void*)pcShared);
    /*****************************************************/
    pBufferForData = (struct19*)(pcShared + sizeof(int));   // 2.
    /*****************************************************/

    //////////////////////////////////////////////////////////////////////////////
    argv[0] = const_cast<char*>(_PROCES_NAME_);  // For sure arg[0] is const

    snprintf(vcOption,16,"%d",nOption);
    argv[1] = vcOption;

    snprintf(vcFrom,16,"%d",nFrom);
    argv[2] = vcFrom;

    snprintf(vcTo,16,"%d",nTo);
    argv[3] = vcTo;

    argv[a_nNumInps] = NULL;

    if(nVerbosity==11)
    {
        //
    }
    else
    {
        if(nVerbosity){nOutAndErr = s_InDst.GetPipe(1);}
        //lnPid = CreateNewProcess1(argv,STDIN_FILENO,nOutAndErr,nOutAndErr,NULL,0,NULL);
        //lnPid = CreateNewProcess1(argv,nOutAndErr,nOutAndErr,nOutAndErr,NULL,0,NULL);
        lnPid = CreateNewProcess1(argv,s_InDst.GetPipe(0),nOutAndErr,nOutAndErr,NULL,0,NULL);
        mexPrintf("!!!!!!!! processPid=%d\n",(int)lnPid);
        mexEvalString("drawnow");

        if(nVerbosity)
        {
            int nTry;
            char* pcBuffer = s_InDst.GetBuffer();
            int nPipe = s_InDst.GetPipe(0);

            fd_set rfds;
            FD_ZERO( &rfds );
            FD_SET( (unsigned int)nPipe, &rfds );
            int maxsd = nPipe + 1;
            struct timeval  aTimeout0,aTimeout;
            aTimeout0.tv_sec = 10;
            aTimeout0.tv_usec = 1;

            int bWork = 1;

            while(bWork)
            {
                aTimeout = aTimeout0;
                nTry = select(maxsd, &rfds, (fd_set *) 0, (fd_set *) 0, &aTimeout );

                switch(nTry)
                {
                case 0:	/* time out */
                    bWork = 0;
                    break;
#ifdef	WIN32
                case SOCKET_ERROR:
#else  // #ifdef	WIN32
                case -1:
#endif  // #ifdef	WIN32
                    if( errno == EINTR )
                    {
                        /* interrupted by signal */
                        continue;
                    }
                    else
                    {
                        mexErrMsgTxt("E_SELECT!\n");
                    }
                default:
                    if( !FD_ISSET( nPipe, &rfds ) )
                    {
                        mexErrMsgTxt("E_FATAL!\n");
                    }
                    nReaded = read(s_InDst.GetPipe(0), pcBuffer, STD_BUF_LEN_MIN1) ;
                    if(nReaded>0)
                    {
                        pcBuffer[nReaded] = 0;
                        mexPrintf(pcBuffer);
                        mexEvalString("drawnow"); // or mexCallMATLAB(0, NULL, 0, NULL, "drawnow");
                    }
                    else
                    {
                        //Program finished
                        bWork = 0;
                    }
                    break;
                } //  end switch(nTry)

            } //  end while(bWork)

        } //  end if(nVerbosity)

        nRet = WaitChildProcess1(lnPid,NULL);

    }
#endif  // #ifdef ROOT_IS_OK_IN_MATLAB



    if(*pnReturn<0)
    {
        a_Outputs[0] = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        pnOut = (int*)mxGetData(a_Outputs[0]);
        *pnOut = *pnReturn;
        //mexPrintf("Error returned during root file reading!\n");
        cpcErrorString = "Error returned during root file reading!\n";
        goto cleaning_point;
    }

    if(a_nNumOuts>1)
    {
        a_Outputs[1] = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
        pnOut = (int*)mxGetData(a_Outputs[1]);
        *pnOut = *pnReturn;
    }

    //a_Outputs[0] = mxCreateNumericArray(3,vSizes,mxSINGLE_CLASS,mxREAL);
    a_Outputs[0] = mxCreateStructMatrix(nNumberOfSpectra,cnEventsMaxCount,5,vcpcFieldNames2);
    if(!a_Outputs[0])
    {
        mexPrintf("Out of memory!\n");
        goto cleaning_point;
        return;
    }

    type_field = mxGetFieldNumber(a_Outputs[0],vcpcFieldNames2[0]);
    time_field = mxGetFieldNumber(a_Outputs[0],vcpcFieldNames2[1]);
    event_field = mxGetFieldNumber(a_Outputs[0],vcpcFieldNames2[2]);
    isAvailable_field = mxGetFieldNumber(a_Outputs[0],vcpcFieldNames2[3]);
    data_field = mxGetFieldNumber(a_Outputs[0],vcpcFieldNames2[4]);

    mxArray *pType,*pTime, *pEvent,*pIsAvailable,*pSpectrum;

    for(i=0;i<nNumberOfSpectra;i++)
    {
        vSizes3[0] = i;
        for(j=0;j<cnEventsMaxCount;++j)
        {
            vSizes3[1] = j;
            nIndex0 = _GET_INDEX_(i,j,cnEventsMaxCount);
            nIndexF = mxCalcSingleSubscript(a_Outputs[0],2,vSizes3);
            pSpectrum = mxCreateNumericMatrix(1,_NUMBER_OF_FLOATS_,mxSINGLE_CLASS,mxREAL);
            pfSpectrum = (float*)mxGetData( pSpectrum );
            memcpy(pfSpectrum,&(pBufferForData[nIndex0].array_value),sizeof(float)*_NUMBER_OF_FLOATS_);
            mxSetFieldByNumber(a_Outputs[0],nIndexF,data_field,pSpectrum);

            pType = mxCreateString("spectrum");
            //pnTime = (int*)mxGetData( pTime );
            //*pnTime = pBufferForData[nIndex0].time;
            mxSetFieldByNumber(a_Outputs[0],nIndexF,type_field,pType);

            pTime = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
            pnTime = (int*)mxGetData( pTime );
            *pnTime = pBufferForData[nIndex0].time;
            mxSetFieldByNumber(a_Outputs[0],nIndexF,time_field,pTime);

            pEvent = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
            pnEvent = (int*)mxGetData( pEvent );
            *pnEvent = pBufferForData[nIndex0].buffer;
            mxSetFieldByNumber(a_Outputs[0],nIndexF,event_field,pEvent);

            pIsAvailable = mxCreateNumericMatrix(1,1,mxINT32_CLASS,mxREAL);
            pnIsAvailable = (int*)mxGetData( pIsAvailable );
            *pnIsAvailable = 1;
            mxSetFieldByNumber(a_Outputs[0],nIndexF,isAvailable_field,pIsAvailable);
        }
    }



cleaning_point:

    s_InDst.CloseSharedMem();

    for(i = 4; i<a_nNumInps;++i)
    {
        free(argv[i]);
    }
    free(argv);

    if(cpcErrorString)
    {
        mexErrMsgTxt(cpcErrorString);
    }
}

}
