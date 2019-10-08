//
// file:        mapandhash.hpp
// created on:  2018 Oct. 29
//

#ifndef COMMON_MAPANDHASH_HPP
#define COMMON_MAPANDHASH_HPP

#include <common/common_hashtbl.hpp>
#include <map>
#include <cpp11+/common_defination.h>
#include  <utility>

namespace common {

template <typename Type, typename TypeMapKey, typename Compare  >
class MapAndHash
{
public:
    struct SMapAndHashItem;
public:
    MapAndHash();
    virtual ~MapAndHash();

    SMapAndHashItem*  AddData(const Type& newData, const TypeMapKey& a_mapKey, const void* a_key, size_t a_keyLen);			// is added, or this is a dublication
    bool   Erase(const void* a_key, size_t a_keyLen);
    bool  FindEntry(const void* key, size_t keyLength, Type* a_ppData)const;
    ptrdiff_t  index(SMapAndHashItem* pItem)const;

    template <typename ClsType>
    void IterateOverEntries( ClsType* a_pOwner, void(ClsType::*a_fpClb)(Type& ) )const;

    template <typename ClsType>
    void IterateOverEntriesWithPossibleRemove( ClsType* a_pOwner, void(ClsType::*a_fpClb)(Type& ) );

    //SMapAndHashItem* first()const;
    //SMapAndHashItem* next(SMapAndHashItem* cur)const;

public:
    struct SMapAndHashItem{
        void* key; Type data; typename ::std::multimap< TypeMapKey, SMapAndHashItem* >::iterator mapIt;
        SMapAndHashItem(const Type& a_dt):data(a_dt){}
        SMapAndHashItem(const SMapAndHashItem& a_cm):data(a_cm.data),mapIt(a_cm.mapIt){}
#ifdef CPP11_DEFINED2
        SMapAndHashItem(Type&& a_dt):data( ::std::move(a_dt) ){}
        SMapAndHashItem(SMapAndHashItem&& a_cm):data( std::move(a_cm.data) ),mapIt( std::move(a_cm.mapIt) ){}
#endif
    };

protected:
    ::std::multimap< TypeMapKey, SMapAndHashItem*, Compare >  m_map;
    HashTbl< SMapAndHashItem* >                      m_hash;
};

}



#include "impl.mapandhash.hpp"

#endif // COMMON_MAPANDHASH_HPP
