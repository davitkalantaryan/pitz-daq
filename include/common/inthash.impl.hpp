//
// file:        common/inthash.hpp
// created on:  2019 Oct 09
// author:      D. Kalantaryan
//

#ifndef COMMON_INTHASH_IMPL_HPP
#define COMMON_INTHASH_IMPL_HPP

#ifndef COMMON_INTHASH_HPP
#if !defined(__GNUC__)
#error "Does not include this file directly"
#endif
#include "inthash.hpp"
#endif

#include <stdlib.h>
#include <new>

#ifndef LIKELY2
#define LIKELY2(_expr)  (_expr)
#endif
#ifndef UNLIKELY2
#define UNLIKELY2(_expr)  (_expr)
#endif

template <typename DataType>
common::IntHash<DataType>::IntHash(size_t a_unTableSize)
    :
      m_cunHashSize(a_unTableSize),
      m_pTable( static_cast<DataType*>( ::calloc(a_unTableSize,sizeof(DataType*)))  )
{
    if(!m_pTable){
        throw ::std::bad_alloc();
    }
}


template <typename DataType>
common::IntHash<DataType>::~IntHash()
{
    clear();
    ::free(m_pTable);
}


template <typename DataType>
void common::IntHash<DataType>::clear()
{
    HashItem *pItem,*pItemTmp ;
    for( size_t i(0); (i<m_cunHashSize)&&(m_unNumberOfElements>0); ++i){
        pItem = m_pTable[i];
        while(pItem){
            pItemTmp = pItem->next;
            delete pItem;
            pItem = pItemTmp;
            --m_unNumberOfElements;
        }
    }
    m_unNumberOfElements = 0; // not needed
}


template <typename DataType>
size_t common::IntHash<DataType>::size()const
{
    return m_unNumberOfElements;
}


template <typename DataType>
template <typename IntType>
bool common::IntHash<DataType>::AddNewElement(IntType a_key, const DataType& a_data)
{
    const size_t cunKey = static_cast<size_t>(a_key);
    const size_t cunIndex = cunKey % m_cunHashSize;
    HashItem* pNewItem ( new HashItem(cunKey,a_data)  );

    if( UNLIKELY2(m_pTable[cunIndex])){
        m_pTable[cunIndex]->prev = pNewItem;
        pNewItem->next = m_pTable[cunIndex];
    }
    m_pTable[cunIndex]=pNewItem;
    ++m_unNumberOfElements;
    return true;
}


template <typename DataType>
template <typename IntType>
void common::IntHash<DataType>::RemoveElement(IntType a_key)
{
    size_t unIndex;
    HashItem* pItem ( FindElementPrivate(a_key,&unIndex) );

    if( LIKELY2(pItem) ){
        if(pItem==m_pTable[unIndex]){
            m_pTable[unIndex] = pItem->next;
        }
        else{  // else means this not first elemet
            pItem->prev->next = pItem->next;
        }

        if(pItem->next){
            pItem->next->prev = pItem->prev;
        }

        delete pItem;
        --m_unNumberOfElements;
    }
}


template <typename DataType>
template <typename IntType>
bool common::IntHash<DataType>::FindElement(IntType a_key, DataType* a_pData)const
{
    size_t unIndex;
    HashItem* pItem ( FindElementPrivate(a_key,&unIndex) );

    if( LIKELY2(pItem) ){
        *a_pData = pItem->data;
        return true;
    }

    return  false;
}


template <typename DataType>
template <typename IntType>
typename common::IntHash<DataType>::HashItem* common::IntHash<DataType>::FindElementPrivate(IntType a_key, size_t* a_pIndex)const
{
    const size_t cunKey = static_cast<size_t>(a_key);
    *a_pIndex = cunKey % m_cunHashSize;
    HashItem* pItem (  m_pTable[*a_pIndex] );

    while(pItem){
        if(pItem->key == cunKey){
            return pItem;
        }
    }

    return  NEWNULLPTR2;
}


#if 0
namespace common{

template <typename DataType>
class IntHash
{
public:
    IntHash();
    virtual ~IntHash();

    template <typename IntType>
    bool AddNewElement(IntType key, const DataType& data);
    template <typename IntType>
    void RemoveElement(IntType key);
    template <typename IntType>
    bool FindElement(IntType key, DataType* pData);

protected:
    struct HashItem{
        struct HashItem *prev, *next;
        DataType        data;
    };
protected:
    const size_t    m_cunHashSize;
    HashItem*       m_pTable;
};

} // namespace common{


#endif   // #if 0


#endif  // #ifndef COMMON_INTHASH_IMPL_HPP
