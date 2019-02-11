#ifndef __linkagesdefinations_h__
#define __linkagesdefinations_h__


#if defined (_WIN64)// Or ( || ) Other Windows
	#ifndef WIN32
		#define WIN32
	#endif
#else
	/// ???
#endif


#ifdef LINKAGE_HDR
#undef LINKAGE_HDR
#endif

#ifdef LINKAGE_SRC
#undef LINKAGE_SRC
#endif


#ifdef ___APP___ /// Using DLL files from APP

	#ifdef WIN32
		#define LINKAGE_HDR __declspec( dllimport )
	#else
		#define LINKAGE_HDR
	#endif

#else /// Creating DLL file

	//#if defined(WIN32) && !defined(___QT___)
	#ifdef WIN32
		//#ifdef _WIN32_WINNT
		#ifndef ___QT___
			#define LINKAGE_HDR _declspec(dllexport)
			#define LINKAGE_SRC _declspec(dllexport)
		#else
			#define LINKAGE_HDR
			#define LINKAGE_SRC
		#endif
	#else
		#define LINKAGE_HDR
		#define LINKAGE_SRC
	#endif

#endif /* ___APP___ */



#endif
