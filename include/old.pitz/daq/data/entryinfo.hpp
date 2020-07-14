//
// file:        pitz/daq/data/entryinfo.hpp
// created on:  2018 Nov 08
//
#ifndef __PITZ_DAQ_DATA_ENTRYINFO_HPP__
#define __PITZ_DAQ_DATA_ENTRYINFO_HPP__

#include <stdint.h>
#define DAQ_HEADER_SIZE     8  // 4 Bytes for time and 4 Bytes for gen_event

namespace pitz{ namespace daq{ namespace data{

namespace Type{enum Type{Error=-1,NoData=0,Int=1,Float=2,String=3,IIII_old=4,IFFF_old=5};}

struct EntryInfoBase{
       Type::Type   dataType;
       int          itemsCount;

       EntryInfoBase():dataType(Type::NoData),itemsCount(0){}
       uint32_t memorySize()const;
};


struct EntryInfo : EntryInfoBase{
       int          numberOfEntriesInTheFile;
       EntryInfo():numberOfEntriesInTheFile(0){}
};

namespace filter{
namespace Type{enum Type{NoFilter2,FileEntriesInfo,MultyBranchFromFile,ByTime2,ByEvent2};}
struct Data{
    Type::Type  type;
    int start, end;
    Data():type(Type::NoFilter2){this->start=this->end=-1;}
};
}


}}}  // namespace pitz{ namespace daq{ namespace data{

#endif // __PITZ_DAQ_DATA_ENTRYINFO_HPP__
