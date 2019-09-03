//
// file:        pitz/daq/data/entryinfo.hpp
// created on:  2018 Nov 08
//
#ifndef PITZ_DAQ_DATA_ENTRYINFO_HPP
#define PITZ_DAQ_DATA_ENTRYINFO_HPP

#include <stdint.h>
#define DAQ_HEADER_SIZE     8  // 4 Bytes for time and 4 Bytes for gen_event

namespace pitz{ namespace daq{ namespace data{

namespace type{enum Type{Error=-1,NoData=0,Int=1,Float=2,CharAscii=3,IIII_old=4,IFFF_old=5};}

struct EntryInfoBase{
    const char*  dataTypeFromRoot;
    type::Type   dataType;
    int          itemsCountPerEntry;

    EntryInfoBase():dataType(type::NoData),itemsCountPerEntry(0){}
    uint32_t memorySize()const;
};


struct EntryInfoOld : EntryInfoBase{
       int          numberOfEntriesInTheFile;
       EntryInfoOld():numberOfEntriesInTheFile(0){}
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

#endif // PITZ_DAQ_DATA_ENTRYINFO_HPP
