#ifndef __allfiles_stdafx_h___
#define __allfiles_stdafx_h___

#ifdef _WIN64
	#ifndef WIN32
		#define WIN32
	#endif
#endif



#ifdef _MSC_VER
#if(_MSC_VER >= 1400)
//#define		_CRT_SECURE_NO_WARNINGS
//#pragma warning(disable : 4996)
//#pragma message(": warning<put what you like here>: blah blah blah")
#endif

//#define __STR__(x) #x
//#define __LOC__ __FILE__ "("__STR__(__LINE__)") : warning D0001: "
//#pragma message(__LOC__"Visual studio c++ compilers older than vs6 is bad for this task")

#else/* #ifdef _MSC_VER */
//#define WARNING(x) #warning (x)
//#define __GENERATOR__(hash,text) hash warning text
//#define WARNING(x) Generator(#,x)
#endif


#ifndef LINKAGE_SRC

#ifdef _CRT_LIB_ /// Creating library

#ifdef _MSC_VER
#define LINKAGE_HDR _declspec(dllexport)
#define LINKAGE_SRC _declspec(dllexport)
#elif defined(__GNUC__)
#define LINKAGE_HDR
#define LINKAGE_SRC
#else
#define LINKAGE_HDR
#define LINKAGE_SRC
#endif

#else /// Using library files from APP

#define LINKAGE_SRC

#ifdef _MSC_VER
//#define LINKAGE_HDR __declspec( dllimport )
#define LINKAGE_HDR
#elif defined(__GNUC__)
#define LINKAGE_HDR
#else  /* #ifdef _MSC_VER */
#define LINKAGE_HDR
#endif  /* #ifdef _MSC_VER */

#endif /* #ifdef ___APP___ */

#endif  /* #ifndef LINKAGE_SRC */


#endif
