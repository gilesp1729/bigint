#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include <assert.h>
#include "bigint.h"

#ifndef _M_X64
#error This source builds on x64 only.
#endif

/*
* Some routines declare bigints and return them in their return value.
* They are:
* big_int_constructor
* big_int_from_uint64_t
* big_int_from_uint32_t
* big_int_from_uint_t
* big_int_from_byte_array_le
* big_int_from_byte_array_be
* big_int_from_string
* This is OK-ish, because the bigints are never malloc/freed (only their underlying
* integer arrays are)
*
* Note: all routines that return bigints through pointers (not through the return value)
* check and reuse their return argument, reallocating if necessary. This means that
* the destructor can be avoided in these cases.
*
* Need:
* - big_int_add_uint_t
* - big_int_mult_by_uint_t
* - big_int_div_by_uint_t
*
*/

void big_int_print(BigInt *a,int mode,char *caption){
	if(!a){
		fprintf(stderr,"%s: Pointer supplied to print is invalid\n", caption);
		return;
	}
	if(a->integer == NULL){
		fprintf(stderr,"%s, Pointer supplied has no integer\n", caption);
		return;
	}
	
	if(a->size == 0){
		fprintf(stderr,"%s: 0 size pointer\n", caption);
		return;
	}
	else{
		printf("%s: print size = %zu\n", caption,a->size);
	}
	//printf("Sign = %d\n",a->sign);
	if(a->sign == 1){
		printf("-");
	}

	switch(mode)
	{
	case 0:
		// Hex, LS word first
		for(size_t i = 0; i < a->size ; i++)
			printf("0x%016I64X ",a->integer[i]);
		break;
	case 1:
		// Hex, MS word first
		for(int i = (a->size)-1; i >=0 ; --i)
			printf("0x%016I64X ",a->integer[i]);
		break;
	case 2:
		// Hex, MS word first, with 0x prepended
		printf("0x");
		for(int i = (a->size)-1; i >=0 ; --i)
			printf("%016I64X",a->integer[i]);
		break;

	case 3:
		// Decimal
		{
			size_t bufsiz = 1024;
			char* revbuf = malloc(bufsiz);			// start with a power of 2 and realloc as needed
			BigInt c = { 0, };
			BigInt ten = big_int_from_uint_t(10);
			BigInt q = { 0, };						// quotient and remainder
			BigInt r = { 0, };
			int i = 0;
			big_int_copy(a, &c);					// don't clobber the input
			while (!big_int_is_zero(&c))
			{
				if (i == bufsiz)	
				{
					bufsiz <<= 1;					// next power of 2	
					revbuf = realloc(revbuf, bufsiz);
				}
				big_int_div(&c, &ten, &q, &r);
				revbuf[i++] = r.integer[0] + '0';
				big_int_copy(&q, &c);
			}
			// Print reversed digits forward. Handle zero specially.
			if (i == 0)
				putchar('0');
			else
			{
				while (i-- > 0)
					putchar(revbuf[i]);
			}
			free(revbuf);
			big_int_destructor(&q);
			big_int_destructor(&r);
			big_int_destructor(&c);
			big_int_destructor(&ten);
	}
		break;
	}
	printf("\n");
}


void big_int_uadd(BigInt *a, BigInt *b,BigInt *c){
	// printf("\t\t\tinside add\n");
	// printf("\t\t\ta = ");
	// big_int_print(a,1);

	// printf("\t\t\tb = ");
	// big_int_print(b,1);

	if(b->size > a->size){
		//printf("\t\t\tb bigger than a by size\n");
		big_int_uadd(b,a,c);
		return;
	}

	// a is always the larger number 
	size_t sizeA = a->size;
	size_t sizeB = b->size;
	assert(sizeA >= sizeB);

	check_and_reuse(c, sizeA);
	
	unsigned char carry = 0;
	
	size_t i;
	for(i = 0 ; i < sizeB ; i++){
		carry = _addcarry_u64(carry, a->integer[i], b->integer[i], &c->integer[i]);
	}
	
	for(i = sizeB ; i < sizeA ; i++){
		carry = _addcarry_u64(carry, a->integer[i], 0, &c->integer[i]);
	}
	
	if(carry){
		if (!big_int_safe_realloc(c, sizeA + 1)) {
			fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", sizeA + 1);
			exit(EXIT_FAILURE);
		}
		c->integer[sizeA] = carry; 
		c->size = sizeA + 1;
	}
	c->sign = 0;
}

void big_int_add(BigInt *a, BigInt *b,BigInt *c){
	if(a->sign == 0 && b->sign == 0){
		big_int_uadd(a,b,c);
		return;
	}
	else if(a->sign == 1 && b->sign == 1){
		big_int_uadd(a,b,c);
		c->sign = 1;
		return;
	}
	else if(a->sign  == 0 && b->sign == 1){
		big_int_usub(a,b,c);
		c->sign = big_int_compare(a, b, 1) >= 0 ? 0 : 1;
		return;

	}
	else if(a->sign == 1 && b->sign == 0){
		big_int_usub(b,a,c);
		c->sign = big_int_compare(b, a, 1) >= 0 ? 0 : 1;
		return;
	}
}

void big_int_usub(BigInt *a, BigInt *b,BigInt *c){
	
	// printf("\t\t\tinside sub\n");
	// printf("\t\t\ta = ");
	// big_int_print(a,1);

	// printf("\t\t\tb = ");
	// big_int_print(b,1);

	if(big_int_compare(b,a,1) > 0){
		// |b| > |a|
		//printf("\t\t\t|b| > |a|\n");
		big_int_usub(b,a,c);
		c->sign = 1;
		return;
	}

	size_t sizeA = a->size;
	size_t sizeB = b->size;

	// Bollocks. sizeA is always >= sizeB.
	//size_t sizeC = (sizeA > sizeB ? sizeA : sizeB);
	assert(sizeA >= sizeB);

	check_and_reuse(c, sizeA);
	
	size_t i;
	// do it like big_int_add does - separate loops 0-sizeB and sizeB-sizeA
	unsigned char borrow = 0;
	for (i = 0; i < sizeB; i++) {
		borrow = _subborrow_u64(borrow, a->integer[i], b->integer[i], &c->integer[i]);
	}
	for (i = sizeB; i < sizeA; i++) {
		borrow = _subborrow_u64(borrow, a->integer[i], 0, &c->integer[i]);
	}

	i = sizeA-1;
	while( i > 0 && c->integer[i] == 0x0){
		i--;
	}
	if (!big_int_safe_realloc(c, i+1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (uint_t)i+1);
		exit(EXIT_FAILURE);
	}
	c->sign = 0;
		
}

void big_int_sub(BigInt * a,BigInt *b,BigInt * c){
	if(a->sign == 0 && b->sign == 0){
		//printf("both zer0\n");
		big_int_usub(a,b,c);
		return;
	}
	else if(a->sign == 1 && b->sign == 1){
		big_int_usub(b,a,c);
		return;
	}
	else if(a->sign  == 0 && b->sign == 1){
		//printf("case 3\n");
		big_int_uadd(a,b,c);
		return; 
	}
	else if(a->sign == 1 && b->sign == 0){
		big_int_uadd(a,b,c);
		c->sign = 1;
		return;

	}

}


void big_int_mult(BigInt *a, BigInt *b,BigInt *c){
	if(b->size > a->size){
		big_int_mult(b, a,c);
		return;
	}
	
	// a is always the larger number 
	size_t sizeA = a->size;
	size_t sizeB = b->size;
	assert(sizeA >= sizeB);
	size_t sizeC = sizeA + sizeB;
	//printf("A sign = %d B sign =%d\n", a->sign,b->sign);
	
	c->sign = a->sign ^ b->sign;
	
	check_and_reuse(c, sizeC);
	memset(c->integer, 0, sizeof(uint_t) * sizeC);
	
	
	for(size_t i = 0 ; i < sizeB ; ++i){	//for all elemnets of b
		uint_t lo_prod, hi_prod, lo_temp, hi_temp;
		uint_t prod_carry = 0;

		size_t j;
		for(j = i ; j < sizeA + i ;  j++){  //for all elements of a
#if 0 // orig 32-bit code			
			uint64_t product = (uint64_t)a->integer[j-i] * (uint64_t)b->integer[i];
			uint64_t sum = (uint64_t)c->integer[j] + product + carry;		// does not overflow, believe it or not.
			c->integer[j] = (uint32_t)(sum & MAXVAL);
			carry = (uint32_t)(sum >> 32);
#else
			lo_prod = _umul128(a->integer[j - i], b->integer[i], &hi_prod);

			// sum = c->integer[j] + (hi_prod,lo_prod) + prod_carry
			unsigned char carry = 0;
			carry = _addcarry_u64(carry, lo_prod, c->integer[j], &lo_temp);
			carry = _addcarry_u64(carry, hi_prod, 0, &hi_temp);

			carry = 0;
			carry = _addcarry_u64(carry, lo_temp, prod_carry, &c->integer[j]);
			carry = _addcarry_u64(carry, hi_temp, 0, &prod_carry);
#endif
		}
#if 0 // orig 32-bit code
		if(carry){
			uint64_t sum = (uint64_t)c->integer[j] + carry;
			c->integer[j] = (uint32_t)(sum & MAXVAL);
#else
		if (prod_carry) {
			c->integer[j] += prod_carry;
#endif
		}
	}
	
	size_t i = sizeC-1;	
	while( i > 0 && c->integer[i] == 0x0){
		i--;
	}
	if (!big_int_safe_realloc(c, i+1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (uint_t)i+1);
		exit(EXIT_FAILURE);
	}
}


int big_int_div(BigInt *u, BigInt *v,BigInt *q, BigInt *r)
{
	int rc;

	// Allocate q and r to be big enough (we'll trim them later,
	// as mp_div doesn't return their true sizes)
	// Quotient size mirrors calc in mp_dev (TODO: Fix this)
	check_and_reuse(q, u->size - v->size + 1);
	if (r)
		check_and_reuse(r, v->size);
	rc = mp_div(u, v, q, r);

	//TODO: trim leading zero words? NO, better to return the true sizes.

	return rc;
}

int big_int_mod(BigInt *a,BigInt *b, BigInt * c){
	BigInt t = {NULL,0,0};
	int res =  big_int_div(a,b,&t,c);
	big_int_destructor(&t);
	return res;
}

