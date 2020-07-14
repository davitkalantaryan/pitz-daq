//
// file:        pitz/daq/base.hpp
//

#ifndef __pitz_daq_base_hpp__
#define __pitz_daq_base_hpp__

#include <common_daq_definations.h>

namespace pitz{ namespace daq {

namespace log{
extern ptrdiff_t  g_nLogLevel;
typedef int (*TypeReportOwn)(void*,const char*,...);
typedef int (*TypeReportNoOwn)(const char*,...);
extern TypeReportNoOwn g_globalLogger;
extern TypeReportNoOwn g_globalErrorLogger;
}  // namespace log{


class Base
{
public:
    Base();
    virtual ~Base(){}

    uint64_t isDebug()const{return m_isDebug;}
    void SetIsDebug(uint64_t a_isDebug){ m_isDebug = a_isDebug;}

protected:
    void*       m_logOwner;
    union{
        log::TypeReportOwn own;
        log::TypeReportNoOwn noOwn;
    }rep, err;

    uint64_t    m_isDebug : 1;
    uint64_t    m_isInited : 1;
    uint64_t    m_isLogOwned : 1;
    ptrdiff_t   m_logLevel : 10;
};


}}  // namespace pitz{ namespace daq {


#ifndef _FILE_DELIM_
#ifdef _WIN32
#define _FILE_DELIM_ '\\'
#else
#define _FILE_DELIM_ '/'
#endif
#endif

#ifndef _FILE_FROM_PATH_
#define _FILE_FROM_PATH_(_path)   ( strrchr((_path),_FILE_DELIM_) ? (strrchr((_path),_FILE_DELIM_) + 1) : (_path) )
#endif

#define MAKE_DEBUG_ANY_NO_OWN(_function,_addString,_logLevel,...) \
    do{ \
        if(  (_logLevel)<=pitz::daq::log::g_nLogLevel   ) { \
            _function(_addString "fl:%s,ln:%d -> ",_FILE_FROM_PATH_(__FILE__),__LINE__); \
            _function(__VA_ARGS__); _function("\n"); \
         }\
    }while(0)


#define MAKE_DEBUG_THIS(_which,_addString,_logLevel,...)  \
    do{ \
        if(   ((_logLevel)<=this->m_logLevel) || ((_logLevel)<=log::g_nLogLevel)   ) { \
            if(this->m_isLogOwned) { \
                (*this->_which.own)(this->m_logOwner,_addString "fl:%s,ln:%d -> ",_FILE_FROM_PATH_(__FILE__),__LINE__); \
                (*this->_which.own)(this->m_logOwner,__VA_ARGS__); (*this->_which.own)(this->m_logOwner,"\n"); \
            } \
            else{ \
                MAKE_DEBUG_ANY_NO_OWN((*this->_which.noOwn),_addString,_logLevel,__VA_ARGS__); \
            } \
        } \
    }while(0)


#define MAKE_DEBUG_THIS_RAW(_which,_addString,_logLevel,...)  \
    do{ \
        if(   ((_logLevel)<=this->m_logLevel) || ((_logLevel)<=log::g_nLogLevel)   ) { \
            if(this->m_isLogOwned) { (*this->_which.own)(this->m_logOwner,__VA_ARGS__);  } \
            else{ (*this->_which.noOwn)(__VA_ARGS__);} \
        } \
    }while(0)


#define MAKE_REPORT_GLOBAL(_logLevel,...)   MAKE_DEBUG_ANY_NO_OWN((*pitz::daq::log::g_globalLogger),"",_logLevel,__VA_ARGS__)
#define MAKE_ERROR_GLOBAL(...)              MAKE_DEBUG_ANY_NO_OWN((*pitz::daq::log::g_globalErrorLogger),"ERROR: ",0,__VA_ARGS__)
#define MAKE_WARNING_GLOBAL(...)            MAKE_DEBUG_ANY_NO_OWN((*pitz::daq::log::g_globalLogger),"WARNING: ",0,__VA_ARGS__)

#define MAKE_REPORT_THIS(_logLevel,...) MAKE_DEBUG_THIS(rep,"",(_logLevel),__VA_ARGS__)
#define MAKE_ERROR_THIS(...)            MAKE_DEBUG_THIS(err,"ERROR: ",0,__VA_ARGS__)
#define MAKE_WARNING_THIS(...)          MAKE_DEBUG_THIS(rep,"WARNING: ",0,__VA_ARGS__)

#ifndef HANDLE_MEM_DEF
#define HANDLE_MEM_DEF(_mem,...)    do{if( !(_mem) ){MAKE_ERROR_GLOBAL(__VA_ARGS__);exit(1);}}while(0)
#endif


#endif  // #ifndef __pitz_daq_base_hpp__
