//
// file:        impl.mapandhash.hpp
// created on:  2018 Oct 29
//

#ifndef IMPL_MAPANDHASH_HPP
#define IMPL_MAPANDHASH_HPP

#ifndef COMMON_MAPANDHASH_HPP
#error Do not include this file directly
#include "mapandhash.hpp"
#endif

template <typename Type, typename TypeMapKey, typename Compare>
common::MapAndHash<Type,TypeMapKey, Compare>::MapAndHash()
{
    //
}



template <typename Type, typename TypeMapKey, typename Compare>
common::MapAndHash<Type,TypeMapKey, Compare>::~MapAndHash()
{
    //
}


template <typename Type, typename TypeMapKey, typename Compare>
typename common::MapAndHash<Type,TypeMapKey, Compare>::SMapAndHashItem*
common::MapAndHash<Type,TypeMapKey, Compare>::AddData(const Type& a_newData, const TypeMapKey& a_mapKey, const void* a_key, size_t a_keyLen)
{
    SMapAndHashItem* pItem;
    if( !m_hash.FindEntry(a_key,a_keyLen,&pItem) ){
        pItem = new SMapAndHashItem(a_newData);
        pItem->mapIt = m_map.insert( ::std::pair<TypeMapKey,SMapAndHashItem*>(a_mapKey,pItem) );
        pItem->key = m_hash.AddEntry2(a_key,a_keyLen,pItem);
        return pItem;
    }

    return NULL;
}



template <typename Type, typename TypeMapKey, typename Compare>
bool common::MapAndHash<Type,TypeMapKey, Compare>::Erase(const void* a_key, size_t a_keyLen)
{
    SMapAndHashItem* pItem;
    if( m_hash.RemoveAndGet(a_key,a_keyLen,&pItem) ){
        m_map.erase(pItem->mapIt);
        return true;
    }

    return false;
}


template <typename Type, typename TypeMapKey, typename Compare>
bool common::MapAndHash<Type,TypeMapKey, Compare>::FindEntry(const void* a_key, size_t a_keyLen, Type* a_ppData)const
{
    SMapAndHashItem* pItem;
    if( m_hash.FindEntry(a_key,a_keyLen,&pItem) ){
        *a_ppData = pItem->data;
        return true;
    }
    return false;
}


template <typename Type, typename TypeMapKey, typename Compare>
template <typename ClsType>
void common::MapAndHash<Type,TypeMapKey, Compare>::IterateOverEntries( ClsType* a_pOwner, void(ClsType::*a_fpClb)(Type& ) )const
{
    typename ::std::multimap<TypeMapKey, SMapAndHashItem* >::const_iterator itEnd = m_map.end();
    for( typename ::std::multimap<TypeMapKey, SMapAndHashItem* >::const_iterator it=m_map.begin();it!=itEnd;++it ){
        (a_pOwner->*a_fpClb)(it->second->data);
    }

}


template <typename Type, typename TypeMapKey, typename Compare>
template <typename ClsType>
void common::MapAndHash<Type,TypeMapKey, Compare>::IterateOverEntriesWithPossibleRemove( ClsType* a_pOwner, void(ClsType::*a_fpClb)(Type& ) )
{
    typename ::std::multimap<TypeMapKey, SMapAndHashItem* >::iterator itNext,itEnd = m_map.end();
    for( typename ::std::multimap<TypeMapKey, SMapAndHashItem* >::iterator it=m_map.begin();it!=itEnd; ){
        itNext = ++it;
        (a_pOwner->*a_fpClb)(it->second->data);
        it = itNext;
    }

}



template <typename Type, typename TypeMapKey, typename Compare>
ptrdiff_t common::MapAndHash<Type,TypeMapKey, Compare>::index(SMapAndHashItem* a_pItem)const
{
    //::std::vector<int> vect;
    //return ::std::distance(vect.begin(), vect.end());
    ptrdiff_t unIndex(0);
    typename ::std::multimap<TypeMapKey,SMapAndHashItem*>::const_iterator itEnd = m_map.end();
    for( typename ::std::multimap<TypeMapKey,SMapAndHashItem*>::const_iterator it = m_map.begin(); it!=itEnd; ++it,++unIndex ){
        if(it==a_pItem->mapIt){return unIndex;}
    }

    return -1;

}



#endif // IMPL_MAPANDHASH_HPP
