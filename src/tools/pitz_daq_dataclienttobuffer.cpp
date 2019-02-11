//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/dataclientbase.hpp"
#include <functional>
#include <stdio.h>
#include <stdarg.h>

namespace pitz{ namespace daq { namespace dataClient{
ptrdiff_t  g_nLogLevel = 0;
}}}


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


/***********************************************************************************************************************/
pitz::daq::dataClient::Base::Base()
    :
      m_owner(NULL)
{
    m_isDebug=m_isInited=m_isOwner=0;
    m_logLevel = g_nLogLevel;

    rep.noOwn = &DefaultReport;
    err.noOwn = &DefaultError;
}
