#include "../bencode.h"
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

#define STATIC_SERALIZARTION_LIMIT (1024*10)

/** Common helper functions. **/

/* String equality for bc_str objects. */
static bc_had bc_seq (const bc_str lhs, const bc_str rhs) {
	return lhs.len == rhs.len && !memcmp (lhs.buf, rhs.buf, rhs.len); }

static char* format_to (bc_val val, char* out, unsigned limit) {
	unsigned amount = bc_write (val, out);
	assert (amount > 0    );
	assert (amount < limit);

	out[amount] = '\0';
	return out;
}

/* Serialize to a tmpbuf. */
static char* serialize (bc_val val) {
	static char buffer[STATIC_SERALIZARTION_LIMIT];
	return format_to (val, buffer, sizeof(buffer));
}

/** Testing framework funcs. **/

#define ASSERT_EQUALITY_LIM  5 /* Bound assert_equality's recursion. */

/* Assert equality between a value and a stream representing output. */
static void assert_equality (unsigned n, bc_val v, bc_istrm* s) {
	if ( n > ASSERT_EQUALITY_LIM ) return assert (0u);

	switch (v.which) {
	case BCK_END: assert (0 /* Shouldn't occur. */);        break;
	case BCK_INT: assert (v.Int == bc_read_int (s));        break;
	case BCK_STR: assert (bc_seq (v.Str, bc_read_str (s))); break;
	case BCK_DCT: case BCK_LST:
		assert (v.which == BCK_LST
		        ? bc_read_lst (s)
		        : bc_read_dct (s));

		for (bcelms l = *((bcelms*)&v); l.len--; l.lst++)
			assert_equality (n+1, *l.lst, s);

		assert (bc_read_end (s));
	}

	assert (!s->err);
}

/** Static objects for testing. **/
#define STATIC_OBJECTS                                       \
const bc_val static_objects [] = {                           \
    BC_INT(3),                                               \
    BC_INT(0),                                               \
    BC_STR("\0", 0),                                         \
    BC_STR( "!", 1),                                         \
    BC_LST(BC_INT(1)),                                       \
    BC_LST(),                                                \
    BC_DCT(),                                                \
    BC_LST(BC_LST(BC_INT(1), BC_INT(2)), BC_INT(3)),         \
    BC_LST(BC_INT(1), BC_STR("hi", 2)),                      \
    BC_DCT({BC_STR("!!", 2), BC_INT(3)}),                    \
}

/** Essential smoke testing. **/

static void smoke_main () {
	STATIC_OBJECTS;

	for (unsigned i=0; i < sizeof(static_objects)/sizeof(*static_objects); ++i)
		printf ("%s\n", serialize (static_objects [i]));
}

/** CBMC Verification tests. **/

static void verif_main () {
	STATIC_OBJECTS;

	/* Assert that serialization and parsing work on the static test objects. */
	for (unsigned i=0; i < sizeof(static_objects)/sizeof(*static_objects); ++i)
		assert_equality (0, static_objects [i],
		               &(bc_istrm) { .buf = serialize (static_objects [i]),
		                             .max = STATIC_SERALIZARTION_LIMIT,
		                             .pos = 0,
		                             .err = 0, });
}

int main () {
	smoke_main();
	verif_main();
}
