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
#pragma warning(disable : 4996)
//#pragma message(": warning<put what you like here>: blah blah blah")

#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning Msg: "
//#pragma message(__LOC__"Need to do 3D collision testing")

#endif
#else/* #ifdef _MSC_VER */
#define __GENERATOR__(hash,text) hash warning text
//#define WARNING(x) Generator(#,x)
#endif


/*
#pragma warning( push )                    // Save the current warning state.
#pragma warning( disable : 4723 )          // C4723: potential divide by 0
// Code which would generate warning 4723.
#pragma warning( pop ) 
 */


#endif
