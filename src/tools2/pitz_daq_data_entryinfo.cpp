/*/////////////////////////////////////////////////////////////////////////////////*/

#include "pitz/daq/data/entryinfo.hpp"
#include <cpp11+/common_defination.h>

using namespace pitz::daq;

uint32_t data::EntryInfoBase::memorySize()const
{
    uint32_t oneItemSize;

    switch(this->dataType){
    case type::Int:
    case type::Float:
        oneItemSize = 4;
        break;
    default:
        oneItemSize = 8;
        break;
    }

    return oneItemSize*(STATIC_CAST(uint32_t,this->itemsCountPerEntry)) + DAQ_HEADER_SIZE;
}
