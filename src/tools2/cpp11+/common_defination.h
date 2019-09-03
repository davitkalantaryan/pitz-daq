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

#ifndef __THISCALL__
#ifdef _MSC_VER
#define __THISCALL__ __thiscall
#else
#define __THISCALL__
#endif
#endif

// Is C++11
#ifndef __NOT_USE_CPP11__
#ifndef __CPP11_DEFINED__
#if defined(_MSC_VER)
#if __cplusplus >= 199711L
#define __CPP11_DEFINED__
#endif // #if __cplusplus >= 199711L
#elif defined(__GNUC__) // #if defined(_MSC_VER)
#if __cplusplus > 199711L
#define __CPP11_DEFINED__
#endif // #if __cplusplus > 199711L
#else // #if defined(_MSC_VER)
#error this compiler is not supported
#endif // #if defined(_MSC_VER)
#endif  // #ifndef __CPP11_DEFINED__
#endif  // #ifndef __NOT_USE_CPP11__

// Is C++14
#ifndef __NOT_USE_CPP14__
#ifndef __CPP14_DEFINED__
#if defined(_MSC_VER)
#if __cplusplus >= 199711L
#define __CPP14_DEFINED__
#endif // #if __cplusplus >= 199711L
#elif defined(__GNUC__) // #if defined(_MSC_VER)
#if __cplusplus > 201103L
#define __CPP14_DEFINED__
#endif // #if __cplusplus > 199711L
#else // #if defined(_MSC_VER)
#error this compiler is not supported
#endif // #if defined(_MSC_VER)
#endif  // #ifndef __CPP14_DEFINED__
#endif  // #ifndef __NOT_USE_CPP14__

// This should be done after check
#ifdef __CPP11_DEFINED__
#define OVERRIDE	override
#define FINAL    	final
#define STATIC_CAST(_Type,_Data)        static_cast<_Type>(_Data)
#define REINTERPRET_CAST(_Type,_Data)   reinterpret_cast<_Type>(_Data)
#define NEWNULLPTR                      nullptr
#else
#define OVERRIDE
#define FINAL
#define STATIC_CAST(_Type,_Data)        ( (_Type)(_Data) )
#define REINTERPRET_CAST(_Type,_Data)   ( (_Type)(_Data) )
#define NEWNULLPTR                      NULL
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
