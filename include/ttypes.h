#if !defined(ttypes__included_)
#define ttypes__included_

#ifdef WIN32
	typedef __int64 int64_tt;
	typedef unsigned __int64 uint64_tt;
	#include <typeinfo.h>
	#include <io.h>

extern int pread(int fd, void *buf, size_t count, int offset);
extern int pwrite(int fd, const void *buf, size_t count, int offset);

#else
	typedef long long int int64_tt;
	typedef unsigned long long int uint64_tt;
	#include <typeinfo>
	#include <unistd.h>
#define _atoi64 atoll
#endif

#include <sys/types.h>
#include <sys/timeb.h>

#ifdef __x86_64__
	typedef long long sysint_t;
	typedef unsigned long long sysuint_t;
#else
	typedef int sysint_t;
	typedef unsigned int sysuint_t;
#endif

typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

#define MAX_SENTENCE_SIZE 10240
#define MAX_WORD_SIZE 1024
#define MAX_FILE_PATH 1024
#define MAX_WORDS_SENTENCE 1024

#endif // !defined(ttypes__included_)
