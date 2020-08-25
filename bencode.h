#ifndef BENCODE_H
#define BENCODE_H
/**
 * This is a parser and serializer for the bencode format (used by bittorrent).
 *
 * Presently, it does not order its output, nor need that its input be ordered.
 * One thing unique here, is that it allocates no memory nor employs recursion.
 * One can always be sure that memory usage is totally static w/in this module.
 *
 * To serialize, one creates a tree out of stack memory representing an object.
 * This is done using the "BC_[INT|STR|LST|DCT]" family of preprocessor macros.
 * The "bc_write" function writes the tree's bencoded contents out to a buffer.
 *
 * To parse, employ the "bc_read_[str|int|len|lst|dct|end]" "bc_istrm" methods.
 * The "bc_read_end" function consumes the end of a collection (a list / dict).
 * Conversely, the "bc_read_lst" and "bc_read_dct" functions consume the start.
 * Otherwise, those remaining consume an atomic object the way one would think.
 *
 * A note: There is an upper limit collection size or nesting size in bc_write.
 *         Thus, ensure your object trees' depths and lists are not very large.
 *         At the moment the lists/dicts' lengths count against this depth cap.
 *         Thus, ensure not only that expression depth is small, but also that
 *         your largest lists and dicts are also relatively small.
 *
 * Plans:
 * In the future I could add a function for ordering a bencode expression tree.
 * It is also possible to have list/dict sizes not count against the depth cap.
 */


/* * * * *
 * TYPES *
 * * * * */

typedef struct bc_str {
	const char* buf;
	   unsigned len; }bc_str;
typedef          char bc_chr;
typedef         _Bool bc_had;
typedef          long bc_int;
typedef unsigned long bc_len;

typedef struct _bcpair _bcpair;
typedef struct  bc_val  bc_val;
typedef struct { union { bc_val* lst; _bcpair* ps; }; unsigned len; } bcelms;
typedef struct { bcelms parent; /* Inheiritance */ } bc_lst;
typedef struct { bcelms parent; /* Inheiritance */ } bc_dct;

/* * * * * * * * *
 * SERIALIZATION *
 * * * * * * * * */

enum   bc_tag { BCK_INT, BCK_STR, BCK_DCT='d', BCK_LST='l', BCK_END };
struct bc_val {
	union {
		bc_int Int;
		bc_str Str;
		bc_dct Dct;
		bc_lst Lst; };
	enum bc_tag which;
};

/* One constructs a value with the BC_* macros, and serializes it here. */
unsigned bc_write (bc_val val, char* out);

/* A single mapping: [0]: Key, [1]: Value */
struct _bcpair { struct bc_val mapping[2]; };

#define BC_INT(num) ((bc_val) { .which=BCK_INT, .Int=(num)                })
#define BC_STR(s,l) ((bc_val) { .which=BCK_STR, .Str={(s), (l)}           })
#define BC_LST(...) ((bc_val) { .which=BCK_LST, .Lst=BC__VEC(__VA_ARGS__) })
#define BC_DCT(...) ((bc_val) { .which=BCK_DCT, .Dct=BC__CNS(__VA_ARGS__) })

/* Helpers */
#define BC__ELM(s,l) { (bcelms) { .s=(l), .len=sizeof(l)/sizeof(bc_val) } }
#define BC__VEC(...) BC__ELM(lst, (( bc_val[]){__VA_ARGS__}))
#define BC__CNS(...) BC__ELM( ps, ((_bcpair[]){__VA_ARGS__}))

/* * * * * * * * * *
 * DESERIALIZATION *
 * * * * * * * * * */

typedef unsigned long bc_error;
typedef struct {
	char   * buf; /* The bencoded string. Mutated for inits. */
	unsigned max; /* The buffer's length, plus one for '\0'. */
	unsigned pos; /* Seek header; i.e., the stream position. */
	bc_error err; /* Record of past errors as boolean flags. */
} bc_istrm;

#define BC_ISTRM_INIT(buff, buflen) { (buff), (buflen), 0, 0 }
bc_str bc_read_str (bc_istrm* strm); /* Read str/key of a dictionary. */
bc_int bc_read_int (bc_istrm* strm); /* Read ordinary signed integer. */
bc_len bc_read_len (bc_istrm* strm); /* Read an unsigned integer val. */
bc_had bc_read_lst (bc_istrm* strm); /* Consume the initial 'l' char. */
bc_had bc_read_dct (bc_istrm* strm); /* Consume the initial 'd' char. */
bc_had bc_read_end (bc_istrm* strm); /* Consume terminating 'e' char. */

#endif /* BENCODE_H */
