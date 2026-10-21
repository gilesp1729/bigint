// This file contains routines that are unchanged from 32 to 64 bit.

#include "bigint.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include <assert.h>

//stored as little endian

int count_leading_zeros(uint_t x) {
	int count = 0;
#ifdef _M_X64	
	if (x == 0) return 64;
	if ((x >> 32) == 0) { count += 32; x <<= 32; }
#else
	if (x == 0) return 32;
#endif
	if ((x >> 16) == 0) { count += 16; x <<= 16; }
	if ((x >> 24) == 0) { count += 8;  x <<= 8; }
	if ((x >> 28) == 0) { count += 4;  x <<= 4; }
	if ((x >> 30) == 0) { count += 2;  x <<= 2; }
	if ((x >> 31) == 0) { count += 1; }

	return count;
}

// If a contains a valid bigint, clear it by freeing its integer array.
void check_and_free(BigInt* a)
{
	if (a && a->integer != NULL)
	{
		free(a->integer);
		a->integer = NULL;
		a->size = 0;
		a->sign = 0;
	}
}

int big_int_count_leading_zeros(BigInt* a) {
	return count_leading_zeros(a->integer[(a->size) - 1]);
}

// Dangerous old code. Too prone to leaving the result integer array orphaned if it's reused.
// But left in purely for use in declarations with initialisation.
BigInt big_int_copy_from(BigInt a)
{
	BigInt b = { 0, };
	big_int_copy(&a, &b);
	return b;
}

// The new version checks and frees the result first.
void big_int_copy(BigInt* a, BigInt* b)
{
	check_and_free(b);
	b->sign = a->sign;
	b->size = a->size;

	b->integer = malloc(sizeof(uint_t) * a->size);
	if (b->integer == NULL) {
		fprintf(stderr, "Memory allocation failed in big_int_copy.\n");
		exit(EXIT_FAILURE);
	}
	memcpy(b->integer, a->integer, sizeof(uint_t) * a->size);
}

#if 0 // prob not needed
// This is like the copy, but it transfers the source integer array over to the dest.
// The dest is left empty. This saves a malloc, a memcpy, and a destructor call in the caller.
void big_int_transfer(BigInt* a, BigInt* b)
{
	check_and_free(b);
	b->sign = a->sign;
	b->size = a->size;
	b->integer = a->integer;	// copy the pointer
	a->sign = 0;
	a->size = 0;
	a->integer = NULL;			// destroy a
}
#endif // 0

int big_int_safe_realloc(BigInt* a, size_t new_size_words) {
	if (!a) return 0;

	//printf("Realloc requested for side = %zu\n",new_size_words);

#if 0
	uint_t* new_mem = realloc(a->integer, sizeof(uint_t) * new_size_words);
	if (!new_mem) {
		return 0;
	}

	if (new_size_words > a->size) {
		size_t old_size_bytes = sizeof(uint_t) * a->size;
		size_t new_size_bytes = sizeof(uint_t) * new_size_words;
		memset(((uint8_t*)new_mem) + old_size_bytes, 0, new_size_bytes - old_size_bytes);
	}

	a->integer = new_mem;

#else // lazy realloc (no need to actually if it's getting smaller, just change the size field)

	if (new_size_words > a->size) {
		uint_t* new_mem = realloc(a->integer, sizeof(uint_t) * new_size_words);
		if (!new_mem) {
			return 0;
		}

		size_t old_size_bytes = sizeof(uint_t) * a->size;
		size_t new_size_bytes = sizeof(uint_t) * new_size_words;
		memset(((uint8_t*)new_mem) + old_size_bytes, 0, new_size_bytes - old_size_bytes);
		a->integer = new_mem;
	}

#endif
	a->size = new_size_words;

	return 1; // success
}

void big_int_swap(BigInt* a, BigInt* b) {
	if (!a || !b) {
		fprintf(stderr, "Pointers supplied is not valid\n");
		return;
	}
	BigInt t = *a;
	*a = *b;
	*b = t;
}

int big_int_is_zero(BigInt* a) {
	if (!a) {
		fprintf(stderr, "Pointer supplied is not valid\n");
		return -1;
	}
	for (size_t i = 0; i < a->size; i++) {
		if (a->integer[i] != 0x0) {
			return 0;
		}
	}
	return 1;
}

int big_int_compare(BigInt* a, BigInt* b, int modulus) {
	//a == b   ->   0
	//a > b    ->   +ve
	//a < b	   ->   -ve
	if (!a || !b) {
		fprintf(stderr, "Pointers supplied is not valid\n");
		exit(EXIT_FAILURE);
		return 10;
	}

	if (!modulus) {
		if (a->sign == 1 && b->sign == 0) {
			return -1;
		}
		if (a->sign == 0 && b->sign == 1) {
			return 1;
		}
	}

	//same sign

	int result = 0;
	size_t sizeA = a->size;
	size_t sizeB = b->size;
	if (sizeA > sizeB) {
		result = 1;
	}

	else if (sizeA < sizeB) {
		result = -1;

	}

	else {
		//sizeA = sizeB here
		for (int i = sizeA - 1; i >= 0; --i) {
			if (a->integer[i] > b->integer[i]) {
				result = 1;
				break;

			}
			if (a->integer[i] < b->integer[i]) {
				result = -1;
				break;

			}
		}
	}

	if (!modulus && a->sign) {
		result = !result;
	}
	return result;
}


// Constructors.
// - from variable argument list
// - from array of uint_t
// - from string with optional leading sign and 0x for hex

BigInt big_int_constructor(short sign, size_t size, ...) {
	BigInt number;
	number.integer = malloc(sizeof(uint_t) * size);
	va_list blocks;
	va_start(blocks, size);
	for (size_t i = 0; i < size; i++) {
		number.integer[i] = va_arg(blocks, uint_t);
	}
	number.sign = sign;
	number.size = size;
	return number;
}

BigInt big_int_from_string(char* string)
{
	size_t lenstr = strlen(string);
	size_t start = 0;
	short sign = 1;
	int	base = 10;
	BigInt a, bigbase;

	// Check for minus sign
	if (string[0] == '-')
	{
		sign = 1;
		start = 1;
	}

	// Check for hex string
	if (_strnicmp(string, "0x", 2) == 0)
	{
		base = 16;
		start += 2;
	}

	bigbase = big_int_from_uint_t(base);			// TODO: Need a big_int_mult_by_uint_t or some such.
	a = big_int_constructor(0, 1, 0);
	for (size_t i = start; i < lenstr; i++)
	{
		BigInt temp = { 0, };
		BigInt temp2 = { 0, };
		BigInt bigdigit = { 0, };
		big_int_mult(&a, &bigbase, &temp);

		// Extract the digit. TODO: Skip any separating spaces, commas or underscores.
		char ch = string[i];
		if (ch >= 'a')
			ch = ch - 'a' + 10;
		else if (ch >= 'A')
			ch = ch - 'A' + 10;
		else if (ch >= '0')
			ch -= '0';
		bigdigit = big_int_from_uint_t((uint_t)ch);
		big_int_add(&temp, &bigdigit, &temp2);		// TODO: Need big_int_add_uint_t
		big_int_copy(&temp2, &a);
		big_int_destructor(&temp);
		big_int_destructor(&temp2);
		big_int_destructor(&bigdigit);
	}
	big_int_destructor(&bigbase);
	return a;
}


void big_int_destructor(BigInt* bignum) {
	if (bignum && bignum->integer) {
		free(bignum->integer);
		bignum->integer = NULL;
		bignum->size = 0;
	}
}

// These are deprecated.
BigInt big_int_from_uint64_t(uint64_t num) {
	return big_int_constructor(0, 2, (uint32_t)(num & MAXVAL), (uint32_t)(num >> 32));
}

BigInt big_int_from_uint32_t(uint32_t num) {
	return big_int_constructor(0, 1, (uint32_t)(num & MAXVAL));
}

// Use this in preference.
BigInt big_int_from_uint_t(uint_t num) {
	return big_int_constructor(0, 1, num);
}


void big_int_word_shift_r(BigInt* a, size_t s) {

	if (a->size == 0 || s == 0) {
		return;
	}

	if (s >= a->size) {
		if (!big_int_safe_realloc(a, 1)) {
			fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (uint_t)1);
			exit(EXIT_FAILURE);
		}
		a->integer[0] = 0x0;
		return;
	}

	//s < a->size
	size_t sizeA = a->size;

	memmove(a->integer, a->integer + s, (sizeA - s) * sizeof(uint_t));

	if (!big_int_safe_realloc(a, sizeA - s)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", sizeA - s);
		exit(EXIT_FAILURE);
	}
}

void big_int_word_shift_l(BigInt* a, size_t s) {

	if (a->size == 0 || s == 0) {
		return;
	}
	size_t oldSize = a->size;



	if (!big_int_safe_realloc(a, oldSize + s)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", oldSize + s);
		exit(EXIT_FAILURE);
	}


	memmove(a->integer + s, a->integer, oldSize * sizeof(uint_t));
	memset(a->integer, 0, s * sizeof(uint_t));
	a->size = oldSize + s;
}




void big_int_bit_shift_r(BigInt* a, size_t s) {

	if (a->size == 0 || s == 0) {
		return;
	}

	if (s >= WORDSIZE) {
		size_t s_word = s / WORDSIZE;
		big_int_word_shift_r(a, s_word);
		s = s % WORDSIZE;
		if (s == 0) return;  // already done
	}

	size_t i = 0;
	size_t sizeA = a->size;
	for (i = 0; i < sizeA - 1; ++i) {
		uint_t shifted = ((a->integer[i]) >> s);
		uint_t carried = (a->integer[i + 1]) << (WORDSIZE - s);

		a->integer[i] = shifted | carried;
	}
	a->integer[sizeA - 1] = a->integer[sizeA - 1] >> s;

	//shrinking
	i = sizeA - 1;
	while (i > 0 && a->integer[i] == 0x0) {
		i--;
	}
	//printf("Final size after right shifting %zu \n",i+1);

	if (!big_int_safe_realloc(a, i + 1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (uint_t)(i + 1));
		exit(EXIT_FAILURE);
	}
}

void big_int_bit_shift_l(BigInt* a, size_t s) {
	if (!a || a->size == 0 || s == 0) return;

	// Word-level shift first
	if (s >= WORDSIZE) {
		size_t s_word = s / WORDSIZE;
		big_int_word_shift_l(a, s_word);
		s %= WORDSIZE;
		if (s == 0) return;  // No remaining bit shift
	}

	size_t old_size = a->size;
	uint_t high_word = a->integer[old_size - 1];
	uint_t carry_out = high_word >> (WORDSIZE - s);

	// If carry_out is non-zero, add one more word
	if (carry_out) {
		uint_t* new_data = (uint_t*)realloc(a->integer, sizeof(uint_t) * (old_size + 1));
		if (!new_data) {
			fprintf(stderr, "Error: realloc failed\n");
			exit(EXIT_FAILURE);
		}
		a->integer = new_data;
		a->integer[old_size] = 0;  // New top word
		a->size += 1;
		old_size += 1;
	}

	// Perform bit-level shift with carry
	uint_t carry = 0;
	for (size_t i = 0; i < old_size; ++i) {
		uint_t current = a->integer[i];
		a->integer[i] = (current << s) | carry;
		carry = current >> (WORDSIZE - s);
	}
}


