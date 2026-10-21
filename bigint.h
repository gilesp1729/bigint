#ifndef BIGINT_H
#define BIGINT_H

#include <stdlib.h>
#include <stdint.h>

// 64-bit integers need to be the right width
#ifdef _M_X64
	typedef uint64_t  uint_t;
#else
	typedef uint32_t  uint_t;
#endif
typedef struct {
	uint_t* integer;
	size_t size;		//no of elements in array. 
	short sign;
} BigInt;


BigInt big_int_constructor(short sign, size_t size, ...);
int big_int_safe_realloc(BigInt *a, size_t new_size_words);
void big_int_destructor(BigInt * bignum);
void big_int_copy(BigInt* a, BigInt* b);
BigInt big_int_copy_from(BigInt a);
//void big_int_transfer(BigInt* a, BigInt* b);

BigInt big_int_from_uint64_t(uint64_t num);
BigInt big_int_from_uint32_t(uint32_t num);
// Deprecated. Use this instead:
BigInt big_int_from_uint_t(uint_t num);

BigInt big_int_from_string(char* string);

void big_int_uadd(BigInt *a, BigInt *b,BigInt *c);
void big_int_add(BigInt *a, BigInt *b,BigInt *c);
void big_int_usub(BigInt *a, BigInt *b,BigInt *c);
void big_int_sub(BigInt *a, BigInt *b,BigInt *c);
void big_int_mult(BigInt *a, BigInt *b,BigInt *c);
int big_int_div(BigInt *u, BigInt *v,BigInt *q, BigInt *r);	
int big_int_mod(BigInt *a,BigInt *b, BigInt * c);

void big_int_inc(BigInt *a);
void big_int_bit_shift_r(BigInt *a,size_t s);
void big_int_bit_shift_l(BigInt *a,size_t s);
void big_int_word_shift_r(BigInt *a,size_t s);
void big_int_word_shift_l(BigInt *a,size_t s);

int big_int_is_zero(BigInt *a);
int big_int_compare(BigInt *a,BigInt *b, int modulus);

void big_int_print(BigInt * a,int mode,char *caption);
void big_int_swap(BigInt * a, BigInt * b);


#endif