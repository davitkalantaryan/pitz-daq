
// main_all_test.cpp
// 2017 Sep 5


// This code investigates possibility
// to obtail Epoch seconds, using tm structure
//
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <zmq/zmq.h>

//#define STRING_TO_TIME (_a_string)


#define  STRING_TO_PARSE "2017.10.26-10:41"

int main()
{
#if 0
    struct tm aTm;
	std::thread aThr;

    aTm.tm_year = 2017-1900;
    aTm.tm_mon = 10-1;
    aTm.tm_mday = 26;
    aTm.tm_hour = 9;
    aTm.tm_min= 50;
    aTm.tm_sec = 1;
    aTm.tm_isdst = -1;

    time_t aTime = mktime(&aTm);

    try{
        //time_t aTime2 = STRING_TO_EPOCH(STRING_TO_PARSE);
		time_t aTime2 = 0;
        printf("thr=%d, epoch=%ld, time_str=\"%s\"\n",(int)aThr.native_handle(),(long)aTime, ctime(&aTime));
        printf("str=\"" STRING_TO_PARSE "\", finStr=\"%s\"\n", ctime(&aTime2));
    }
    catch(...)
    {
    }

#endif

    return 0;
}


// Code that shows problem with DOOCS client lib 18.10.11
// The problem is following:
//  In the case if ENSHOST variable is not defined, then hos
//  application simply crashes
//  To do -> should be tested with new version of DOOCS
//           in the case if problem still there, should be
//           reported to developers
#if 0
#include <eq_client.h>
int main()
{
    EqAdr  dcsAddr;
    EqData dataIn, dataOut;
    EqCall eqCall;

    dcsAddr.adr("PITZ.DIAG/FASTADC/DIAG.ADC0/CH00.TD");
    dataIn.init();dataOut.init();
    eqCall.get(&dcsAddr,&dataIn,&dataOut);

    return 0;
}
#endif


#if 0

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <string>
#include <time.h>
#include <iostream>
#include <eq_client.h>

#define BUF_LEN_FOR_STRFTIME 64
#define DUMMY_ARGS(...)

int mkdir_p(const char *a_path, mode_t a_mode);

void CalcRootFilePathPrivate(
        const std::string& a_dirNameBase,
        std::string* a_result,std::string* a_dirName,
        bool a_bCalcTime, char* a_pcYear, char* a_pcMonth,
        char* a_day, char* a_hour, char* a_minute);

//int main(int argc, char* argv[])
int main()
{

    EqAdr  dcsAddr;
    EqData dataIn, dataOut;
    EqCall eqCall;
    float* fpValue;
    int nArrayLen;

    dcsAddr.adr("PITZ.DIAG/FASTADC/DIAG.ADC0/CH00.TD");
    dataIn.init();dataOut.init();
    int nErr = eqCall.get(&dcsAddr,&dataIn,&dataOut);

    printf("error=%d\n",nErr);

    if(nErr==0){
        nArrayLen = dataOut.array_length();
        printf("type=%d, size=%d\n",dataOut.type(),nArrayLen);
        fpValue = dataOut.get_float_array();
        for(int i(0);i<nArrayLen;++i){
            printf("%f\t",fpValue[i]);
            if(i%8==0){printf("\n");}
        }
        printf("\n");
    }

    return 0;

    std::string fileName, dirName;
    char
            vcYear[BUF_LEN_FOR_STRFTIME],vcMonth[BUF_LEN_FOR_STRFTIME],
            vcDay[BUF_LEN_FOR_STRFTIME],vcHour[BUF_LEN_FOR_STRFTIME],
            vcMinute[BUF_LEN_FOR_STRFTIME];

    CalcRootFilePathPrivate("/doocs/data/DAQdata/daqL",&fileName, &dirName,1,
                            vcYear,vcMonth,vcDay,vcHour,vcMinute);

    std::cout<<"dir ="<<dirName <<std::endl;
    std::cout<<"file="<<fileName<<std::endl;

    return 0;
}


//#define mkdir_p_debug(...) printf(__VA_ARGS__)
#define mkdir_p_debug(...)

static int mkdir_p_raw(const char *a_path, mode_t a_mode);

int mkdir_p(const char *a_path, mode_t a_mode)
{
    int nError = mkdir_p_raw(a_path,a_mode);

    switch(nError)
    {
    case 0:
        break;
    case ENOENT:
        mkdir_p_debug("(ENOENT)");
    {
        std::string aNewPath(a_path);
        char* basePath = const_cast<char*>(aNewPath.c_str());
        char *lastDir = strrchr(basePath,'/');
        int i,nDeep=0;

        while(lastDir && (nError==ENOENT)){
            ++nDeep;
            *lastDir = 0;
            //aNewPath = std::string(a_path,(size_t)(lastDir-aNewPath.c_str()));
            mkdir_p_debug("new_try=%s",aNewPath.c_str());
            nError=mkdir_p_raw(basePath,a_mode);
            lastDir = strrchr(basePath,'/');
        }

        for(i=0;(i<nDeep)&&(nError==0);++i){
            basePath[strlen(basePath)]='/';
            nError=mkdir_p_raw(basePath,a_mode);
        }

    }
        break;
    case ELOOP:
        mkdir_p_debug("(ELOOP)");break;
    case EMLINK:
        mkdir_p_debug("(EMLINK)");break;
    default:
        mkdir_p_debug("(unknown)");break;
    }

    return nError;
}


static int mkdir_p_raw(const char *a_path, mode_t a_mode)
{
    int nReturn = mkdir(a_path,a_mode);

    if(nReturn){
        nReturn = errno;
        mkdir_p_debug("errno=%d ",nReturn);
        switch(nReturn)
        {
        case EEXIST:
            mkdir_p_debug("(EEXIST)");
            nReturn = 0;
            break;
        default:
            mkdir_p_debug("(unknown)");
            break;
        }

        mkdir_p_debug("\n");

    } // if(nReturn){

    return nReturn;
}


class MyClass
{
public:
    MyClass(const std::string& a_str):m_str(a_str){}
    const std::string& value()const{return m_str;}
private:
    std::string m_str;
};

static MyClass m_usedName("pitznoadc0");
static MyClass m_rootFileNameBase("pitznoadc0");



void CalcRootFilePathPrivate(
        const std::string& a_dirNameBase,
        std::string* a_result,std::string* a_dirName,
        bool a_bCalcTime, char* a_pcYear, char* a_pcMonth,
        char* a_day, char* a_hour, char* a_minute)
{
    std::string& retStr(*a_result);

            if(a_bCalcTime){
                time_t  aWalltime (::time(0));
                struct tm * timeinfo(localtime(&aWalltime));
                strftime (a_pcYear,BUF_LEN_FOR_STRFTIME,"%Y",timeinfo);
                strftime (a_pcMonth,BUF_LEN_FOR_STRFTIME,"%m",timeinfo);
                strftime (a_day,BUF_LEN_FOR_STRFTIME,"%d",timeinfo);
                strftime (a_hour,BUF_LEN_FOR_STRFTIME,"%H",timeinfo);
                strftime (a_minute,BUF_LEN_FOR_STRFTIME,"%M",timeinfo);
                DUMMY_ARGS(aWalltime);
            }

            retStr = a_dirNameBase+"/"+std::string(a_pcYear);
            mkdir(retStr.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            retStr += "/"+std::string(a_pcMonth); // month
            mkdir(retStr.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            retStr += std::string("/")+m_usedName.value();
            mkdir(retStr.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
            *a_dirName=retStr;
            retStr += std::string("/")+m_rootFileNameBase.value()+"."+
                    std::string(a_pcYear)+"-"+std::string(a_pcMonth)+"-"+
                    std::string(a_day)+"-"+std::string(a_hour)+a_minute +
                    std::string(".root");
}

#endif  // #####################if 0
