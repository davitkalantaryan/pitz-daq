//
// file:
// created on:
//

#include "common_daq_definations.h"
#include "pitz/daq/base.hpp"
//#include <stdarg.h>
//#include <stdio.h>
#ifdef _WIN32
#else
#include <unistd.h>
#endif


namespace pitz{ namespace daq {

static int DefaultReport(const char* a_format,...);
static int DefaultError(const char* a_format,...);

namespace log{
ptrdiff_t  g_nLogLevel = 0;
TypeReportNoOwn g_globalLogger = DefaultReport;
TypeReportNoOwn g_globalErrorLogger = DefaultError;
}

static int DefaultReport(const char* a_format,...)
{
    int nReturn;
    va_list aList;
    va_start(aList,a_format);
    nReturn = vdprintf(STDOUT_FILENO,a_format,aList);
    va_end(aList);
    return nReturn;
}


static int DefaultError(const char* a_format,...)
{
    int nReturn;
    va_list aList;
    va_start(aList,a_format);
    nReturn = vdprintf(STDERR_FILENO,a_format,aList);
    va_end(aList);
    return nReturn;
}

}}  // namespace pitz{ namespace daq {


/***********************************************************************************************************************/
pitz::daq::Base::Base()
    :
      m_logOwner(NULL)
{
    m_isDebug=m_isInited=m_isLogOwned=0;
    m_logLevel = log::g_nLogLevel;

    rep.noOwn = &DefaultReport;
    err.noOwn = &DefaultError;

}
