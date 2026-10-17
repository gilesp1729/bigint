#ifndef BIGINT_H
#define BIGINT_H


#include <stdlib.h>
#include <stdint.h>

typedef struct {
	uint32_t * integer;
	size_t size;		//no of elements in array
	short sign;
} BigInt;


BigInt big_int_constructor(short sign, size_t size, ...);
int big_int_safe_realloc(BigInt *a, size_t new_size_words);
void big_int_destructor(BigInt * bignum);

BigInt big_int_from_uint64_t(uint64_t num);
BigInt big_int_from_uint32_t(uint32_t num);
BigInt big_int_from_byte_array_le(unsigned char * buff,size_t buff_len);
BigInt big_int_from_byte_array_be(unsigned char * buff,size_t buff_len);

void big_int_uadd(BigInt *a, BigInt *b,BigInt *c);
void big_int_add(BigInt *a, BigInt *b,BigInt *c);
void big_int_usub(BigInt *a, BigInt *b,BigInt *c);
void big_int_sub(BigInt *a, BigInt *b,BigInt *c);
void big_int_mult(BigInt *a, BigInt *b,BigInt *c);
int big_int_div(BigInt *u, BigInt *v,BigInt *q, BigInt *r);		//divmod
int big_int_mod(BigInt *a,BigInt *b, BigInt * c);
int big_int_modpow(BigInt * a,BigInt *b, BigInt * c, BigInt * d);

void big_int_gcd(BigInt *a, BigInt *b, BigInt *c);
void big_int_xgcd(BigInt *a, BigInt *b, BigInt *gcd, BigInt *x_out, BigInt *y_out);
void big_int_modinv(BigInt *a, BigInt *b, BigInt *c);

void big_int_inc(BigInt *a);
void big_int_bit_shift_r(BigInt *a,size_t s);
void big_int_bit_shift_l(BigInt *a,size_t s);
void big_int_word_shift_r(BigInt *a,size_t s);
void big_int_word_shift_l(BigInt *a,size_t s);

int big_int_is_zero(BigInt *a);
int big_int_compare(BigInt *a,BigInt *b, int modulus);

void big_int_print(BigInt * a,int mode);
void big_int_swap(BigInt * a, BigInt * b);
int big_int_count_leading_zeros(BigInt *a);
BigInt big_int_copy(BigInt a);

size_t big_int_bit_length(BigInt *a);
unsigned char * big_int_to_byte_array_be(BigInt * a, size_t * len);

#endif