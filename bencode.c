#include "bencode.h"
#include "utility.h"

/* Change the last char (normally '\0') in the string to one specified. */
static void delimit (bc_istrm* strm, char m) { strm->buf[strm->max-1] = m; }

/*
 * Attempts to consume one in a set of characters given in the "lst" argument.
 * It will return the matching char if the next one in the stream is in "lst."
 * If no match was found, it may signal an error (s. below), and returns '\0'.
 * One can indicate whether an error is signalled on no match in this fashion:
 *
 * One byte is expected past the terminal '\0': either a '\b' or another '\0'.
 * If it is a '\b', then NO error is indicated if no matches were encountered.
 * If it is a '\0', then an error IS indicated if no matches were encountered.
 */
static bc_int eat_from (bc_istrm* strm, char lst[]) {
	do  if ( *lst == strm->buf [strm->pos] )
	return ++strm->pos, *lst; while (*++lst);
	return *(lst+1) != '\b' ? ++strm->err :0;
}

/* Consume signless part of an integer following the prefix. */
static bc_len eat_uint (bc_istrm* strm, char until) {
	long long v = 0; delimit(strm, until);
	while (strm->buf[strm->pos] != until)
		v *= 10, v += eat_from (strm, "0123456789\0") - '0';

	delimit(strm, '\0'); ++strm->pos;
	return (unsigned) v;
}

/* Consume a full integer literal, with optional +/- prefix. */
static bc_int eat_sint (bc_istrm* strm, char until) {
	bc_chr seen = eat_from (strm, "+-\0\b");
	bc_int sign = (  seen  ==  '-'  ?  -1 : +1  );
	return sign * (bc_int) eat_uint (strm, until);
}

static bc_had eat_char (bc_istrm* strm, char which) {
	return eat_from (strm, (char[]) { which, '\0', '\0' }); }

bc_str bc_read_str (bc_istrm* strm) {
	bc_str str;
	   str.len = eat_uint (strm , ':');
	   str.buf = strm->buf + strm->pos;
	             strm->pos +=  str.len;
	return str;
}

bc_int bc_read_int (bc_istrm* s) { return eat_char (s,'i'), eat_sint (s,'e'); }
bc_len bc_read_len (bc_istrm* s) { return eat_char (s,'i'), eat_sint (s,'e'); }
bc_had bc_read_lst (bc_istrm* s) { return eat_char (s,'l');                   }
bc_had bc_read_dct (bc_istrm* s) { return eat_char (s,'d');                   }
bc_had bc_read_end (bc_istrm* s) { return eat_char (s,'e');                   }

unsigned bc_write (bc_val root, char* o) {
	struct { bc_val* v[768]; unsigned n; } s = { {&root}, .n=1 };
	static bc_val end = { .which=BCK_END };
	void *const saved = o;

	for (bc_val* v; s.n-- && (v=s.v[s.n]);) switch (v->which) {
	case BCK_END:   *o++ = 'e';  break;
	case BCK_STR:
		itoa(&o, v->Str.len); *o++=':';
		sadd(&o, v->Str.buf);    break;
	case BCK_INT:
		*o++ = 'i'; itoa (&o , v->Int);
		*o++ = 'e';              break;
	case BCK_DCT: /* Share the impl. */
	case BCK_LST:*o++ = v->which; s.v[s.n++] = &end;
		for (bcelms l = *((bcelms*)v); l.len--;) s.v[s.n++] = &l.lst[l.len];
	}

	return o - ((char*)saved); /* Calculate write length. */
}
