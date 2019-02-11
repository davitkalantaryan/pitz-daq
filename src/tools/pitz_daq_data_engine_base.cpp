//
// file:        mex_simple_root_reader.cpp
//

#include "pitz/daq/data/engine/base.hpp"
#include <functional>
#include <stdio.h>
#include <stdarg.h>

using namespace pitz::daq;


data::engine::Base::Base()
{
    clbk.sp.p1 = clbk.sp.p2 = NULL;
    m_clbkData = NULL;
    this->m_clbkType = callbackN::Type::NoClbk;
}


void data::engine::Base::SetCallbacks(void* a_clbkData, const callbackN::SFncsFileEntriesInfo& a_clbk)
{
    this->m_clbkData = a_clbkData;
    this->m_clbkType = callbackN::Type::Info;
    this->clbk.m_flInfo = a_clbk;
}


void data::engine::Base::SetCallbacks(void* a_clbkData, const callbackN::SFncsMultiEntries& a_clbk)
{
    this->m_clbkData = a_clbkData;
    this->m_clbkType = callbackN::Type::MultiEntries;
    this->clbk.m_multiEntries = a_clbk;
}
