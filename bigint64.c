#include "bigint.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <intrin.h>
#include <assert.h>

#ifndef _M_X64
#error This source builds on x64 only.
#endif

#define MAXVAL 0xFFFFFFFFULL 

/*
* TODO LIST:
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
* Now all routines that return bigints through pointers (not the return value)
* check and free the result bigint first.
* 
* Need:
* - something like big_int_copy that links the new integer[] and frees the original
* - big_int_mult_by_uint32_t (and add, sub, ...) - possibly faster than bigint versions?
* 
* 
* 
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
#if 0 // ############ TEMP until we have a div ###########
	case 3:
		// Decimal
		{
			char* revbuf = malloc(a->size * 16);	// should be big enough (double the hex size)
			BigInt c = { 0, };
			BigInt ten = big_int_from_uint_t(10);
			BigInt q = { 0, };						// quotient and remainder
			BigInt r = { 0, };
			int i = 0;
			big_int_copy(a, &c);					// don't clobber the input
			while (!big_int_is_zero(&c))
			{
				big_int_div(&c, &ten, &q, &r);		// TODO: this will free q/r integers, but watch out for inconsistencies elsewhere. Need div_by_uint_t here.
				revbuf[i++] = r.integer[0] + '0';
				big_int_copy(&q, &c);				// Can be faster as q not rrequired aagain, but see above
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
#endif // 0 ########### TEMP ###########
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

// These routines need completely rewriting for x64. Don't hold your breath.
#ifndef _M_X64

int big_int_div(BigInt *u, BigInt *v,BigInt *q, BigInt *r){
	
	//Knuth-D implementation based on https://github.com/hcs0/Hackers-Delight/
	
	/*
    1. Space q for the quotient, m - n + 1 words (at least one).
   	2. Space r for the remainder (optional), n words.
   	3. The dividend u, m words, m >= 1.
   	4. The divisor v, n words, n >= 2.
   	*/
	

	uint64_t BASE = 0x100000000;		// number base (2^32)
	uint32_t *un,*vn;			//normalised u,v
	
	uint64_t q_hat;
	uint64_t r_hat;
	uint64_t p;				//product of 2 digits(chunks) 
	
	size_t m = u->size;
	size_t n = v->size;

	check_and_free(q);
	check_and_free(r);

	if(n > m){
		*q = big_int_from_uint32_t(0);
		if (r)
			big_int_copy(u, r);	
		return 0;
	}
	
	size_t sizeQ = m - n + 1;
	size_t sizeR = n;
	
	long long t, k;
	unsigned int i;
	int j;

	q->integer = malloc(sizeof(uint32_t) * sizeQ);
	q->size = sizeQ;
	
	if(r){
		r->integer = malloc(sizeof(uint32_t) * sizeR);
		r->size = sizeR;
	}
	
	if (m < n || n <= 0 || v->integer[n - 1] == 0) {
		free(q->integer);
		if (r) free(r->integer);
		return 1;
	}
	
	//easy casse hard coded
	if(n == 1){
		k = 0;
		for(j = m - 1 ; j >= 0 ; --j){
			q->integer[j] = (uint32_t)((k * BASE + u->integer[j])/v->integer[0]);
			k = (k * BASE + u->integer[j]) - q->integer[j]*v->integer[0];
		}
		if(r){
			r->integer[0] = (uint32_t)k;
		}
		return 0;
	}
	
	
	//normalising v
	int s = big_int_count_leading_zeros(v);
	
	vn = (uint32_t*)malloc(sizeof(uint32_t) * n);
	un = (uint32_t*)malloc(sizeof(uint32_t) * (m + 1));
	
	for(i = n - 1; i > 0 ; i --){
		uint32_t shifted = v->integer[i] << s;
		uint32_t carried = (uint64_t)v->integer[i-1] >> (32 - s);
		
		vn[i] = shifted | carried;
	}
	vn[0] = v->integer[0] << s;
	
	
	
	//normalising u
	un[m] = (uint64_t)u->integer[m-1] >> (32 - s);
	
	for(i = m - 1 ; i > 0 ; --i){
		uint32_t shifted = u->integer[i] << s;
		uint32_t carried = (uint64_t)u->integer[i-1] >> (32 - s);
		
		un[i] = shifted | carried;
	}
	un[0] = u->integer[0] << s;
	
	//main loop
	for(j = m-n; j >= 0; j--){ //for each quotient digit
		//estimate qhat of q[j] using top 2 digits of dividend(u)
		uint64_t numerator = ((uint64_t)un[j+n] << 32) | un[j+n-1]; //top 2 digits of u(chunks)
		q_hat = numerator / vn[n-1];				//top 1 digit of v
		r_hat = numerator % vn[n-1];
		
		
		//refine qhat
		while(q_hat >= BASE || (q_hat * vn[n-2]) > ((r_hat << 32) + un[j+n-2])){
			q_hat = q_hat - 1;
			r_hat = r_hat + vn[n-1];
			if(r_hat >= BASE){
				break;
			}
		}
	
		
		//main procedure un[j..... j+n] - vn * qhat
		k = 0;
		for(i = 0 ; i < n  ; i++){
		
			p = q_hat * vn[i];
			t = (uint64_t)un[j+i] - k - (p & MAXVAL);
			un[j+i] = (uint32_t)(t & MAXVAL);
			// Borrow = 1 if upper 32 bits are non-zero, else 0
			k = (p >> 32) - (t >> 32);

		}
		t = (uint64_t)un[j+n] - k;
		un[j+n] = (uint32_t)(t & MAXVAL);
		
		
		//store q
		q->integer[j] = (uint32_t)(q_hat & MAXVAL);
		
		//subtraction underflowed implied q_hat is large or overestimated
		if ((int64_t)t < 0) {
			q->integer[j] -= 1;
			
			//add back vn to current slice of un to restore remainder for next iter
			uint64_t carry = 0;
			for (i = 0; i < n; i++) {
				uint64_t sum = (uint64_t)un[j+i] + vn[i] + carry;
			 	un[j+i] = (uint32_t)sum;
				carry = sum >> 32;
			}
			un[j+n] += (uint32_t)carry;
		}
	}
	//main loop done
	
	if(r){
		//unnormalise(reverse what we did) to remainder
		for(i = 0 ; i < n - 1 ; ++i){
			uint32_t shifted = (un[i] >> s);
			uint32_t carried = (uint64_t)un[i+1] << (32-s);
			
			r->integer[i] = shifted | carried;
		}
		r->integer[n-1] = un[n-1] >> s;
	
		i = sizeR-1;
		while( i > 0 && r->integer[i] == 0x0){
			i--;
		}
		if (!big_int_safe_realloc(r, i+1)) {
			fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (long unsigned int)i+1);
			exit(EXIT_FAILURE);
		}
	}

	i = sizeQ-1;
	while( i > 0 && q->integer[i] == 0x0){
		i--;
	}
	if (!big_int_safe_realloc(q, i+1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (long unsigned int)i+1);
		exit(EXIT_FAILURE);
	}
	
	free(un);
	free(vn);
	return 0;
	
}

int big_int_mod(BigInt *a,BigInt *b, BigInt * c){
	BigInt t = {NULL,0,0};
	int res =  big_int_div(a,b,&t,c);
	big_int_destructor(&t);
	return res;
}

#endif // !_M_X64