/*/////////////////////////////////////////////////////////////////////////////////*/

#include "pitz/daq/data/entryinfo.hpp"

using namespace pitz::daq;

uint32_t data::EntryInfoBase::memorySize()const
{
    uint32_t oneItemSize;

    switch(this->dataType){
    case Type::Int:
    case Type::Float:
        oneItemSize = 4;
        break;
    default:
        oneItemSize = 8;
        break;
    }

    return oneItemSize*((uint32_t)this->itemsCount) + DAQ_HEADER_SIZE;
}
