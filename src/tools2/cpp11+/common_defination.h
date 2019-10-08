/*****************************************************************************
 * File:    common_defination.h
 * created: 2017 Apr 24
 *****************************************************************************
 * Author:	D.Kalantaryan, Tel:+49(0)33762/77552 kalantar
 * Email:	davit.kalantaryan@desy.de
 * Mail:	DESY, Platanenallee 6, 15738 Zeuthen
 *****************************************************************************
 * Description
 *   ...
 ****************************************************************************/
#ifndef COMMON_DEFINATION_H
#define COMMON_DEFINATION_H

#define	CURRENT_SERIALIZER_VERSION2		5
#define	CURRENT_SERIALIZER_TYPE2		1

#ifndef THISCALL2
#ifdef _MSC_VER
#define THISCALL2 __thiscall
#else
#define THISCALL2
#endif
#endif

// Is C++11
#ifndef NOT_USE_CPP11_2
#ifndef CPP11_DEFINED2
#if defined(_MSC_VER)
#if __cplusplus >= 199711L
#define CPP11_DEFINED2
#endif // #if __cplusplus >= 199711L
#elif defined(__GNUC__) // #if defined(_MSC_VER)
#if __cplusplus > 199711L
#define CPP11_DEFINED2
#endif // #if __cplusplus > 199711L
#else // #if defined(_MSC_VER)
#error this compiler is not supported
#endif // #if defined(_MSC_VER)
#endif  // #ifndef CPP11_DEFINED2
#endif  // #ifndef NOT_USE_CPP11_2

// Is C++14
#ifndef NOT_USE_CPP14_2
#ifndef CPP14_DEFINED2
#if defined(_MSC_VER)
#if __cplusplus >= 199711L
#define CPP14_DEFINED2
#endif // #if __cplusplus >= 199711L
#elif defined(__GNUC__) // #if defined(_MSC_VER)
#if __cplusplus > 201103L
#define CPP14_DEFINED2
#endif // #if __cplusplus > 199711L
#else // #if defined(_MSC_VER)
#error this compiler is not supported
#endif // #if defined(_MSC_VER)
#endif  // #ifndef CPP14_DEFINED2
#endif  // #ifndef NOT_USE_CPP14_2

#ifdef __cplusplus
#define STATIC_CAST2(_Type,_Data)        static_cast<_Type>(_Data)
#define REINTERPRET_CAST2(_Type,_Data)   reinterpret_cast<_Type>(_Data)
#else
#define STATIC_CAST(_Type,_Data)        ( (_Type)(_Data) )
#define REINTERPRET_CAST(_Type,_Data)   ( (_Type)(_Data) )
#endif

// This should be done after check
#ifdef CPP11_DEFINED2
#define OVERRIDE2                        override
#define FINAL2                           final
#define NEWNULLPTR2                      nullptr
#else
#define OVERRIDE2
#define FINAL2
#define NEWNULLPTR2                      NULL
#endif

#include <stdarg.h>

#ifdef __cplusplus
#ifdef FUNCTION_POINTER_TO_VOID_POINTER
template <typename FncType>
static inline void* FUNCTION_POINTER_TO_VOID_POINTER(FncType _a_fnc_)
{
    FncType aFnc = _a_fnc_;
    void** ppFnc = STATIC_CAST(void**,&aFnc);
    void* pRet = *ppFnc;
    return pRet;
}
#endif
#endif // #ifdef __cplusplus

#ifdef __cplusplus
extern "C"{
#endif

//extern void* GetFuncPointer_common(int,...);

#ifdef __cplusplus
}
#endif

#endif // COMMON_DEFINATION_H
