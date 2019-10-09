//
// file:        common/inthash.hpp
// created on:  2019 Oct 09
// author:      D. Kalantaryan
//

#ifndef COMMON_INTHASH_HPP
#define COMMON_INTHASH_HPP

#include <stddef.h>

#ifndef NEWNULLPTR2
#define NEWNULLPTR2     nullptr
#endif

namespace common{

template <typename DataType>
class IntHash
{
protected:
    struct HashItem;
public:
    IntHash(size_t tableSize);
    virtual ~IntHash();

    void clear();
    size_t size()const;
    template <typename IntType>
    bool AddNewElement(IntType key, const DataType& data);
    template <typename IntType>
    void RemoveElement(IntType key);
    template <typename IntType>
    bool FindElement(IntType key, DataType* pData)const;

protected:
    template <typename IntType>
    struct HashItem* FindElementPrivate(IntType key,size_t* a_pIndex)const;

protected:
    struct HashItem{
        struct HashItem *prev, *next;
        size_t          key;
        DataType        data;
        HashItem(size_t a_key, const DataType& a_data):key(a_key),data(a_data){this->prev=this->next=NEWNULLPTR2;}
    };
protected:
    size_t              m_unNumberOfElements;
    const size_t        m_cunHashSize;
    HashItem**const     m_pTable;
};

} // namespace common{

#ifndef COMMON_INTHASH_IMPL_HPP
#include "inthash.impl.hpp"
#endif

#endif  // #ifndef COMMON_INTHASH_HPP
