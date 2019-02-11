#ifndef __roundinteger_h__
#define __roundinteger_h__

#include <stddef.h>
#include "ttypes.h"

#ifndef HASH_LITTLE_ENDIAN
extern int HASH_LITTLE_ENDIAN;
#endif

typedef uint32_t (*TYPE_HASH_FUNC)(const void*const&,const uint32_t&);

#define rot(x,k) (((x)<<(k)) | ((x)>>(32-(k))))

/* The mixing step */
#define mix2(a,b,c) \
{ \
  a=a-b;  a=a-c;  a=a^(c>>13); \
  b=b-c;  b=b-a;  b=b^(a<<8);  \
  c=c-a;  c=c-b;  c=c^(b>>13); \
  a=a-b;  a=a-c;  a=a^(c>>12); \
  b=b-c;  b=b-a;  b=b^(a<<16); \
  c=c-a;  c=c-b;  c=c^(b>>5);  \
  a=a-b;  a=a-c;  a=a^(c>>3);  \
  b=b-c;  b=b-a;  b=b^(a<<10); \
  c=c-a;  c=c-b;  c=c^(b>>15); \
}


#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}


#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}


template<typename TypeInt> inline TypeInt RoundInteger( const TypeInt& a_tInitSize )
{

	if( !a_tInitSize )
	{
		return (TypeInt)0;
	}

	int i(0);
	TypeInt tRet(a_tInitSize);

	for( ; tRet; tRet = (a_tInitSize >> ++i) );


	tRet = ((TypeInt)1) << (i-1);

	if( tRet != a_tInitSize )
	{
		tRet <<= 1;
	}

	return tRet;
}


template<typename TypeInt> inline uint32_t hashInt_( const void*const& a_pKey, const uint32_t& a_unKeySize )
{
	return (uint32_t)( (TypeInt)( *((TypeInt*)a_pKey)) );
}


inline uint32_t hash1_( const void*const& a_pKey, const uint32_t& a_unKeySize )
{
	register uint8_t *k = (uint8_t *)a_pKey;
	register uint32_t a,b,c;  /* the internal state */
	
	uint32_t          len;    /* how many key bytes still need mixing */
	
	/* Set up the internal state */
	len = a_unKeySize;
	a = b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */
	c = 13;         /* variable initialization of internal state */
	
	/*---------------------------------------- handle most of the key */
	while (len >= 12)
	{
		a=a+(k[0]+((uint32_t)k[1]<<8)+((uint32_t)k[2]<<16) +((uint32_t)k[3]<<24));
		b=b+(k[4]+((uint32_t)k[5]<<8)+((uint32_t)k[6]<<16) +((uint32_t)k[7]<<24));
		c=c+(k[8]+((uint32_t)k[9]<<8)+((uint32_t)k[10]<<16)+((uint32_t)k[11]<<24));
		mix2(a,b,c);
		k = k+12; len = len-12;
	}
	
	
	/*------------------------------------- handle the last 11 bytes */
	c = c+a_unKeySize;
	
	switch(len)              /* all the case statements fall through */
	{
	case 11: c=c+((uint32_t)k[10]<<24);
	case 10: c=c+((uint32_t)k[9]<<16);
	case 9 : c=c+((uint32_t)k[8]<<8);
		
		/* the first byte of c is reserved for the length */
	case 8 : b=b+((uint32_t)k[7]<<24);
	case 7 : b=b+((uint32_t)k[6]<<16);
	case 6 : b=b+((uint32_t)k[5]<<8);
	case 5 : b=b+k[4];
	case 4 : a=a+((uint32_t)k[3]<<24);
	case 3 : a=a+((uint32_t)k[2]<<16);
	case 2 : a=a+((uint32_t)k[1]<<8);
	case 1 : a=a+k[0];
		/* case 0: nothing left to add */
	}
	mix2(a,b,c);
	/*-------------------------------------------- report the result */
	
	return c;
}


inline uint32_t hash2_( const void*const& a_pKey, const uint32_t& a_unKeySize2 )
{

	uint32_t unKeySize(a_unKeySize2);
	
	uint32_t a,b,c;                                          /* internal state */
	union { const void *ptr; size_t i; } u;     /* needed for Mac Powerbook G4 */
	
	/* Set up the internal state */
	a = b = c = 0xdeadbeef + ((uint32_t)unKeySize) + 0;
	
	u.ptr = a_pKey;
	
	if( HASH_LITTLE_ENDIAN && ((u.i & 0x3) == 0) )
	{
		const uint32_t *k = (const uint32_t *)a_pKey;         /* read 32-bit chunks */
//		const uint8_t  *k8;
		
		
		/*------ all but last block: aligned reads and affect 32 bits of (a,b,c) */
		while( unKeySize > 12 )
		{
			a += k[0];
			b += k[1];
			c += k[2];
			mix(a,b,c);
			unKeySize -= 12;
			k += 3;
		}
		
		/*----------------------------- handle the last (probably partial) block */
		/* 
		 * "k[2]&0xffffff" actually reads beyond the end of the string, but
		 * then masks off the part it's not allowed to read.  Because the
		 * string is aligned, the masked-off tail is in the same word as the
		 * rest of the string.  Every machine with memory protection I've seen
		 * does it on word boundaries, so is OK with this.  But VALGRIND will
		 * still catch it and complain.  The masking trick does make the hash
		 * noticably faster for short strings (like English words).
		*/

#ifndef VALGRIND
		
		switch( unKeySize )
		{
		case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
		case 11: c+=k[2]&0xffffff; b+=k[1]; a+=k[0]; break;
		case 10: c+=k[2]&0xffff; b+=k[1]; a+=k[0]; break;
		case 9 : c+=k[2]&0xff; b+=k[1]; a+=k[0]; break;
		case 8 : b+=k[1]; a+=k[0]; break;
		case 7 : b+=k[1]&0xffffff; a+=k[0]; break;
		case 6 : b+=k[1]&0xffff; a+=k[0]; break;
		case 5 : b+=k[1]&0xff; a+=k[0]; break;
		case 4 : a+=k[0]; break;
		case 3 : a+=k[0]&0xffffff; break;
		case 2 : a+=k[0]&0xffff; break;
		case 1 : a+=k[0]&0xff; break;
		case 0 : return c;              /* zero length strings require no mixing */
		}

#else /* make valgrind happy */
		
		k8 = (const uint8_t *)k;
		switch( unKeySize )
		{
		case 12: c+=k[2]; b+=k[1]; a+=k[0]; break;
		case 11: c+=((uint32_t)k8[10])<<16;  /* fall through */
		case 10: c+=((uint32_t)k8[9])<<8;    /* fall through */
		case 9 : c+=k8[8];                   /* fall through */
		case 8 : b+=k[1]; a+=k[0]; break;
		case 7 : b+=((uint32_t)k8[6])<<16;   /* fall through */
		case 6 : b+=((uint32_t)k8[5])<<8;    /* fall through */
		case 5 : b+=k8[4];                   /* fall through */
		case 4 : a+=k[0]; break;
		case 3 : a+=((uint32_t)k8[2])<<16;   /* fall through */
		case 2 : a+=((uint32_t)k8[1])<<8;    /* fall through */
		case 1 : a+=k8[0]; break;
		case 0 : return c;
		}

#endif /* !valgrind */
	
	}
	else if( HASH_LITTLE_ENDIAN && ((u.i & 0x1) == 0) )
	{
		const uint16_t *k = (const uint16_t *)a_pKey;         /* read 16-bit chunks */
		const uint8_t  *k8;
		
		/*--------------- all but last block: aligned reads and different mixing */
		while( unKeySize > 12 )
		{
			a += k[0] + (((uint32_t)k[1])<<16);
			b += k[2] + (((uint32_t)k[3])<<16);
			c += k[4] + (((uint32_t)k[5])<<16);
			mix(a,b,c);
			unKeySize -= 12;
			k += 6;
		}
		
		/*----------------------------- handle the last (probably partial) block */
		k8 = (const uint8_t *)k;
		switch( unKeySize )
		{
		case 12: 
			c+=k[4]+(((uint32_t)k[5])<<16);
			b+=k[2]+(((uint32_t)k[3])<<16);
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
		case 11: c+=((uint32_t)k8[10])<<16;     /* fall through */
		case 10:
			c+=k[4];
			b+=k[2]+(((uint32_t)k[3])<<16);
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
		case 9 : c+=k8[8];                      /* fall through */
		case 8 :
			b+=k[2]+(((uint32_t)k[3])<<16);
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
		case 7 : b+=((uint32_t)k8[6])<<16;      /* fall through */
		case 6 :
			b+=k[2];
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
		case 5 : b+=k8[4];                      /* fall through */
		case 4 :
			a+=k[0]+(((uint32_t)k[1])<<16);
			break;
		case 3 : a+=((uint32_t)k8[2])<<16;      /* fall through */
		case 2 :
			a+=k[0];
			break;
		case 1 :
			a+=k8[0];
			break;
		case 0 : return c;                     /* zero length requires no mixing */
		}
	
	}
	else
	{                        /* need to read the key one byte at a time */
		const uint8_t *k = (const uint8_t *)a_pKey;
		
		/*--------------- all but the last block: affect some 32 bits of (a,b,c) */
		while( unKeySize > 12 )
		{
			a += k[0];
			a += ((uint32_t)k[1])<<8;
			a += ((uint32_t)k[2])<<16;
			a += ((uint32_t)k[3])<<24;
			b += k[4];
			b += ((uint32_t)k[5])<<8;
			b += ((uint32_t)k[6])<<16;
			b += ((uint32_t)k[7])<<24;
			c += k[8];
			c += ((uint32_t)k[9])<<8;
			c += ((uint32_t)k[10])<<16;
			c += ((uint32_t)k[11])<<24;
			mix(a,b,c);
			unKeySize -= 12;
			k += 12;
		}
		
		/*-------------------------------- last block: affect all 32 bits of (c) */
		switch( unKeySize )                   /* all the case statements fall through */
		{
		case 12: c+=((uint32_t)k[11])<<24;
		case 11: c+=((uint32_t)k[10])<<16;
		case 10: c+=((uint32_t)k[9])<<8;
		case 9 : c+=k[8];
		case 8 : b+=((uint32_t)k[7])<<24;
		case 7 : b+=((uint32_t)k[6])<<16;
		case 6 : b+=((uint32_t)k[5])<<8;
		case 5 : b+=k[4];
		case 4 : a+=((uint32_t)k[3])<<24;
		case 3 : a+=((uint32_t)k[2])<<16;
		case 2 : a+=((uint32_t)k[1])<<8;
		case 1 : a+=k[0]; break;
		case 0 : return c;
		}
	}
	
	
	final(a,b,c);
	return c;

}


inline uint32_t hash3_( const void*const& a_pKey, const uint32_t& a_unKeySize )
{
	
	unsigned long hash = 5381;
	char *str = (char*)a_pKey;
	int c;
	uint32_t i;
	
	for( i = 0, c = *str; i < a_unKeySize; str++, i++ )
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	
	return hash;

}


inline uint32_t hash4_( const void*const& a_pKey, const uint32_t& a_unKeySize )
{
	
	const unsigned char	*s = (const unsigned char *)a_pKey;
	unsigned int		n = 0;
	unsigned int 		i = 0;
	unsigned int 		j = 0;
	
	for (i = a_unKeySize; i > 0 && s; i--, s++)
	{
		j++;
		n ^= 271 * (*s);
	}
	
	i = n ^ (j * 271);
	
	return i;

}


#endif
