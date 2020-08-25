#ifndef UTILITY_H
#define UTILITY_H

/* Used for tools that can analyze code w/ recursion bounds. */
#ifndef uassert
#define uassert(x)
#endif /* !assert */

/* Gives explicit recursion bound. */
static void  _itoa (char** cursor, long long value, unsigned n) {
	/* Math is done with negative numbers. */
	/* This way, we can deal with INT_MIN. */

	if ( n  >  20 ) return uassert ( 0 ); /* 20: Digits in 2**64. */
	if (value < -9)
		_itoa (cursor, value / 10, n+1);

	*(*cursor)++ = -(value % -10) + '0';
}

/* Prettyprint "value" in base-10. */
static void  itoa (char** cursor, long long value) {
	if (value < 0) *(*cursor)++ = '-';
	else             value = -(value);

	_itoa (cursor, value, 0);
}

/* Append "str" to another string. */
static void  sadd (char** cursor, const char* str) {
	for (;*str;++str) *(*cursor)++ = *str;
}

/* Just like the C library memcpy. */
static void* mcpy (void*  destin, const void* src, unsigned n) {
	while (n--) *((char*)destin) = *((char*)src); return destin;
}

#undef uassert

#endif /* UTILITY_H */
