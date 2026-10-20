#include "bigint.h"
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <intrin.h>

#define MAXVAL 0xFFFFFFFFULL 

/*
* TODO LIST:
* Some routines declare bigints and return them in their return value.
* They are:
* big_int_constructor
* big_int_from_uint64_t
* big_int_from_uint32_t
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
* - big_int_mult_by_uint32_t (and add, sub, ...)
* 
* 
* 
* 
*/

//stored as little endian

int count_leading_zeros(uint32_t x) {
    if (x == 0) return 32;

    int count = 0;
    if ((x >> 16) == 0) { count += 16; x <<= 16; }
    if ((x >> 24) == 0) { count += 8;  x <<= 8;  }
    if ((x >> 28) == 0) { count += 4;  x <<= 4;  }
    if ((x >> 30) == 0) { count += 2;  x <<= 2;  }
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

// Dangerous old code. Too prone to leaving the result integer array orphaned if it's reused.
// But left in purely for use in declarations with initialisation.
BigInt big_int_copy_from(BigInt a)
{
	BigInt b = { 0, };
	big_int_copy(&a, &b);
	return b;
}

// The new version checks and frees the result first.
void big_int_copy(BigInt *a, BigInt *b)
{
	check_and_free(b);
	b->sign = a->sign;
	b->size = a->size;
	
	b->integer = malloc(sizeof(uint32_t) * a->size);
	if (b->integer == NULL) {
		fprintf(stderr, "Memory allocation failed in big_int_copy.\n");
		exit(EXIT_FAILURE);
	}
	memcpy(b->integer, a->integer, sizeof(uint32_t) * a->size);
}

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


int big_int_safe_realloc(BigInt *a, size_t new_size_words) {
	if (!a) return 0;

	//printf("Realloc requested for side = %zu\n",new_size_words);

#if 0
	uint32_t *new_mem = realloc(a->integer, sizeof(uint32_t) * new_size_words);
	if (!new_mem) {
		return 0;
	}

	if (new_size_words > a->size) {
		size_t old_size_bytes = sizeof(uint32_t) * a->size;
		size_t new_size_bytes = sizeof(uint32_t) * new_size_words;
		memset(((uint8_t *)new_mem) + old_size_bytes, 0, new_size_bytes - old_size_bytes);
	}

	a->integer = new_mem;

#else // lazy realloc (no need to actually if it's getting smaller, just change the size field)

	if (new_size_words > a->size) {
		uint32_t* new_mem = realloc(a->integer, sizeof(uint32_t) * new_size_words);
		if (!new_mem) {
			return 0;
		}

		size_t old_size_bytes = sizeof(uint32_t) * a->size;
		size_t new_size_bytes = sizeof(uint32_t) * new_size_words;
		memset(((uint8_t*)new_mem) + old_size_bytes, 0, new_size_bytes - old_size_bytes);
		a->integer = new_mem;
	}

#endif
	a->size = new_size_words;

	return 1; // success
}

void big_int_swap(BigInt * a, BigInt * b){
	if(!a || !b){
		fprintf(stderr,"Pointers supplied is not valid\n");
		return;
	}
	BigInt t = *a;
	*a = *b;
	*b = t;
}

int big_int_is_zero(BigInt *a){
	if(!a){
		fprintf(stderr,"Pointer supplied is not valid\n");
		return -1;
	}
	for(size_t i = 0 ; i < a->size ; i++){
		if(a->integer[i] != 0x0){
			return 0;
		}
	}
	return 1;
}

int big_int_compare(BigInt *a,BigInt *b, int modulus){
	//a == b   ->   0
	//a > b    ->   +ve
	//a < b	   ->   -ve
	if(!a || !b){
		fprintf(stderr,"Pointers supplied is not valid\n");
		exit(EXIT_FAILURE);
		return 10;
	}

	if(!modulus){
		if(a->sign == 1 && b->sign == 0){
			return -1;
		}
		if(a->sign == 0 && b->sign == 1){
			return 1;
		}
	}
	
	//same sign

	int result = 0;
	size_t sizeA = a->size;
	size_t sizeB = b->size;	
	if(sizeA > sizeB){
			result =  1;
	}
	
	else if(sizeA < sizeB){
			result= -1;
		
	}
	
	else{
		//sizeA = sizeB here
		for(int i = sizeA - 1 ; i >= 0 ; --i){
			if(a->integer[i] > b->integer[i]){
					result = 1;
					break;
				
			}
			if(a->integer[i] < b->integer[i]){
					result = -1;
					break;
				
			}
		}
	}

	if(!modulus && a->sign){
		result = !result;
	}
	return result;
}


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
			printf("0x%08X ",a->integer[i]);
		break;
	case 1:
		// Hex, MS word first
		for(int i = (a->size)-1; i >=0 ; --i)
			printf("0x%08X ",a->integer[i]);
		break;
	case 2:
		// Hex, MS word first, with 0x prepended
		printf("0x");
		for(int i = (a->size)-1; i >=0 ; --i)
			printf("%08X",a->integer[i]);
		break;
	case 3:
		// Decimal
		{
			char* revbuf = malloc(a->size * 16);	// should be big enough (double the hex size)
			BigInt c = { 0, };
			BigInt ten = big_int_from_uint32_t(10);
			BigInt q = { 0, };						// quotient and remainder
			BigInt r = { 0, };
			int i = 0;
			big_int_copy(a, &c);					// don't clobber the input
			while (!big_int_is_zero(&c))
			{
				big_int_div(&c, &ten, &q, &r);		// TODO: this will free q/r integers, but watch out for inconsistencies elsewhere
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
	}
	printf("\n");
}

// Constructors.
// - from variable argument list
// - from array of uint32_t
// - from string with optional leading sign and 0x for hex

// TODO: Change names big_int_new, new_from_string, etc.

BigInt big_int_constructor(short sign, size_t size, ...){
	BigInt number;
	number.integer = malloc(sizeof(uint32_t) * size);
	va_list blocks;
	va_start(blocks,size);
	for(size_t i = 0; i < size ; i++){
		number.integer[i] = va_arg(blocks,uint32_t);
	}
	number.sign = sign;
	number.size = size;
	return number;
}

BigInt big_int_from_string(char *string)
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

	bigbase = big_int_from_uint32_t(base);			// TODO: Need a big_int_mult_by_uint32_t or some such.
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
		bigdigit = big_int_from_uint32_t((uint32_t)ch);
		big_int_add(&temp, &bigdigit, &temp2);		// TODO: Need big_int_add_uint32
		big_int_copy(&temp2, &a);
		big_int_destructor(&temp);
		big_int_destructor(&temp2);
		big_int_destructor(&bigdigit);
	}
	big_int_destructor(&bigbase);
	return a;
}


void big_int_destructor(BigInt *bignum) {
    if (bignum && bignum->integer) {
        free(bignum->integer);
        bignum->integer = NULL;
        bignum->size = 0;
    }
}


BigInt big_int_from_uint64_t(uint64_t num){
	return big_int_constructor(0,2,(uint32_t)(num & MAXVAL),(uint32_t)(num >> 32));
}

BigInt big_int_from_uint32_t(uint32_t num){
	return big_int_constructor(0,1,(uint32_t)(num & MAXVAL));
}


BigInt big_int_from_byte_array_le(unsigned char * buff,size_t buff_len){
	size_t sizeA = (buff_len + 3)/4;		//32 bit word size from 8 bit array
	BigInt a;
	a.sign = 0;
	a.size = sizeA;
	
	a.integer = malloc(sizeof(uint32_t) *sizeA);
	if (a.integer == NULL) {
		fprintf(stderr, "Memory allocation failed in byte array concversion.\n");
		exit(EXIT_FAILURE);
	}

	for (size_t i = 0; i < sizeA; i++) {
        a.integer[i] = 0;
    }

	for(size_t i = 0 ; i < sizeA ; i++){
		size_t buff_index = i * 4;
		size_t bytes_to_copy = 4;
		uint32_t word = 0;

        if (buff_index + 4 > buff_len) {
            bytes_to_copy = buff_len - buff_index;
        }
		memcpy(&word,buff + buff_index,bytes_to_copy);
		a.integer[i] = word;
	}
	return a;
}


BigInt big_int_from_byte_array_be(unsigned char  *bytes, size_t len) {
	BigInt result = big_int_from_uint32_t(0); // or init with 0
    
	// printf("len = %zu\n",len);
    for (size_t i = 0; i < len; ++i) {
        big_int_bit_shift_l(&result, 8); // multiply result by 256

        BigInt tmp = big_int_from_uint32_t(bytes[i]);

		BigInt new_result = {NULL, 0, 0};
		big_int_add(&result, &tmp, &new_result);  // new_result = result + tmp

		big_int_destructor(&result);             // free old result
		result.integer = new_result.integer;                     // assign the new result
		result.sign = new_result.sign;    
		result.size = new_result.size;    

		big_int_destructor(&tmp); 
    }
	// big_int_print(&result,0);
	// printf("\n");
    return result;
}


unsigned char * big_int_to_byte_array_be(BigInt * a, size_t * len){
	if(!a){
		return NULL;
	}

	size_t sizeA = a->size;
	unsigned char * temp = malloc(4 * sizeA); //malloc

	size_t j = 0;
	int leading = 1;
	for(int i = sizeA-1; i >=0 ; --i){

		uint32_t word = a->integer[i];
		for (int byte = 3; byte >= 0; --byte) {
            unsigned char b = (word >> (8 * byte)) & 0xFF;
            if (leading && b == 0) continue;  // Skip leading zero bytes
            leading = 0;
            temp[j++] = b;
        }
	}

	if(len) *len = j;
	unsigned char * result = malloc(j);
	memcpy(result,temp,j);
	free(temp);
	return result;
}

int big_int_count_leading_zeros(BigInt *a){
	return count_leading_zeros(a->integer[(a->size)-1]);
}

void big_int_inc(BigInt *a){
	if(!a){
		fprintf(stderr,"Pointer supplied is not valid, Cant increment\n");
		return;
	}

	short int carry = 0;
	size_t sizeA = a->size;
	
	for(size_t i = 0; i < sizeA ; i++){
		uint64_t temp;
		if(i == 0){
			temp = (uint64_t)a->integer[i] + carry + (uint64_t)0x1;
		}
		else{
			temp = (uint64_t)a->integer[i] + carry;
		}

	
		//printf("Inc\ntemp(unsigned) = %lX  MAXVAL = %llX  res & MAXVAL  = %d\n",temp,MAXVAL,temp > MAXVAL);

		carry = (temp > MAXVAL);
		a->integer[i] = (temp & MAXVAL);
	}
	
	if(carry){
		//a->integer = realloc(a->integer,sizeof(uint32_t) * (sizeA + 1));
		if (!big_int_safe_realloc(a, sizeA  +1)) {
			fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", sizeA + 1);
			exit(EXIT_FAILURE);
		}
		a->integer[sizeA] = 0x1;
	}
}




void big_int_word_shift_r(BigInt *a,size_t s){
	
	if(a->size == 0 || s==0){
		return;
	}
	
	if(s >= a->size){
		if (!big_int_safe_realloc(a, 1)) {
			fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (long unsigned int)1);
			exit(EXIT_FAILURE);
		}
		a->integer[0] = 0x0;
		return;
	}
	
	//s < a->size
	size_t sizeA = a->size; 
	
	memmove(a->integer,a->integer + s, (sizeA - s)*sizeof(uint32_t));
	
	if (!big_int_safe_realloc(a, sizeA - s)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", sizeA - s);
		exit(EXIT_FAILURE);
	}
}

void big_int_word_shift_l(BigInt *a,size_t s){
	
	if(a->size == 0 || s==0){
		return;
	}
	size_t oldSize = a->size;
	


	if (!big_int_safe_realloc(a, oldSize + s)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", oldSize + s);
		exit(EXIT_FAILURE);
	}

	
	memmove(a->integer+s,a->integer, oldSize*sizeof(uint32_t));
	memset(a->integer,0,s * sizeof(uint32_t));
	a->size = oldSize + s;
}




void big_int_bit_shift_r(BigInt *a,size_t s){
	
	if(a->size == 0 || s==0){
		return;
	}
	
	if (s >= 32) {
		size_t s_word = s / 32;
		big_int_word_shift_r(a, s_word);
		s = s % 32;
		if (s == 0) return;  // already done
	}
	
	size_t i = 0;
	size_t sizeA = a->size; 
	for(i = 0 ; i < sizeA - 1 ; ++i){
		uint32_t shifted = ((a->integer[i]) >> s);
		uint32_t carried = (a->integer[i+1]) << (32-s);
		
		a->integer[i] = shifted | carried;
	}
	a->integer[sizeA-1] = a->integer[sizeA-1] >> s;
	
	//shrinking
	i = sizeA-1;
	while( i > 0 && a->integer[i] == 0x0){
		i--;
	}
	//printf("Final size after right shifting %zu \n",i+1);

	if (!big_int_safe_realloc(a, i+1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (long unsigned int)i+1);
		exit(EXIT_FAILURE);
	}
}

void big_int_bit_shift_l(BigInt *a, size_t s) {
    if (!a || a->size == 0 || s == 0) return;

    // Word-level shift first
    if (s >= 32) {
        size_t s_word = s / 32;
        big_int_word_shift_l(a, s_word);
        s %= 32;
        if (s == 0) return;  // No remaining bit shift
    }

    size_t old_size = a->size;
    uint32_t high_word = a->integer[old_size - 1];
    uint32_t carry_out = high_word >> (32 - s);

    // If carry_out is non-zero, add one more word
    if (carry_out) {
        uint32_t *new_data = realloc(a->integer, sizeof(uint32_t) * (old_size + 1));
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
    uint32_t carry = 0;
    for (size_t i = 0; i < old_size; ++i) {
        uint32_t current = a->integer[i];
        a->integer[i] = (current << s) | carry;
        carry = current >> (32 - s);
    }
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

	check_and_free(c);

	//a is always the larger number 
	size_t sizeA = a->size;
	size_t sizeB = b->size;
	
	c->integer = malloc(sizeof(uint32_t) * sizeA);
	c->size = sizeA;
	
	short int carry = 0;
	
	size_t i;
	for(i = 0 ; i < sizeB ; i++){
#if 0  // 32-bit original code
		uint64_t temp = (uint64_t)a->integer[i] + (uint64_t)b->integer[i] + carry;
		carry = (temp > MAXVAL);
		c->integer[i] = (temp & MAXVAL);
#else 
		carry = _addcarry_u32(carry, a->integer[i], b->integer[i], &c->integer[i]);
#endif
		
	}
	
	for(i = sizeB ; i < sizeA ; i++){
#if 0  // 32-bit original code
		uint64_t temp = (uint64_t)a->integer[i] + carry;
		carry = (temp > MAXVAL);
		c->integer[i] = (temp & MAXVAL);
#else 
		carry = _addcarry_u32(carry, a->integer[i], 0, &c->integer[i]);
#endif
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

	check_and_free(c);

	if(big_int_compare(b,a,1) > 0){
		// |b| > |a|
		//printf("\t\t\t|b| > |a|\n");
		big_int_usub(b,a,c);
		c->sign = 1;
		return;
	}

	
	
	size_t sizeA = a->size;
	size_t sizeB = b->size;
	size_t sizeC = (sizeA > sizeB ? sizeA : sizeB);

/*
	if (c->integer == NULL) {
		c->integer = malloc(sizeC * sizeof(uint32_t));
		c->size = sizeC;
	} else {
		uint32_t *new_data = realloc(c->integer, sizeC * sizeof(uint32_t));
		if (new_data == NULL) {
			fprintf(stderr, "Error: realloc failed\n");
			exit(EXIT_FAILURE);
		}
		c->integer = new_data;
		c->size = sizeC;
	}
	*/
	/*
	if (c->integer != NULL) {
		free(c->integer);
	}
	*/


	c->integer = malloc(sizeof(uint32_t) * sizeC);
	c->size = sizeC;
	

	
	short int borrow = 0;
	
	size_t i;
#if 0	// Old code
	//printf("\t\t\t\t\t\t\tsizeC = %zu\n",sizeC);
	for(i = 0 ; i < sizeC ; i++){
		//printf("i = %zu\n",i);
		uint64_t temp1 = (i < sizeA ? (uint64_t)a->integer[i] : (uint64_t)0x0)  + (MAXVAL + 1);
		uint64_t temp2 = (i < sizeB ? (uint64_t)b->integer[i] : (uint64_t)0x0)  + (uint64_t)borrow;
		uint64_t res = temp1 - temp2;
		//printf("Sub\n temp1 = %lX , temp2 = %lX , %d \n",temp1,temp2,temp1 > temp2);
		c->integer[i] = (uint32_t)(res & MAXVAL);	
		//printf("res(unsigned) = %lX  MAXVAL = %llX  res & MAXVAL  = %X\n",res,MAXVAL,res & MAXVAL);
		borrow = (res <= MAXVAL);
	}
#else
	// do it like big_int_add does - separate loops 0-sizeB and sizeB-sizeA


		borrow = _subborrow_u32(borrow, a->integer[i], b->integer[i], &c->integer[i]);
#endif


	/*
	if(borrow){
		c->sign = 1;
		for(i = 0 ; i < sizeC ; i++){
			c->integer[i] = ~c->integer[i];
		}
		big_int_inc(c);
	}
	*/
	



	i = sizeC-1;
	while( i > 0 && c->integer[i] == 0x0){
		i--;
	}
	if (!big_int_safe_realloc(c, i+1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (long unsigned int)i+1);
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
	
	check_and_free(c);

	//a is always the larger number 
	size_t sizeA = a->size;
	size_t sizeB = b->size;
	size_t sizeC = sizeA + sizeB;
	//printf("A sign = %d B sign =%d\n", a->sign,b->sign);
	
	c->sign = a->sign ^ b->sign;
	
	c->integer = malloc(sizeof(uint32_t) * sizeC);
	memset(c->integer, 0, sizeof(uint32_t) * sizeC); //
	c->size = sizeC;
	
	
	for(size_t i = 0 ; i < sizeB ; ++i){	//for all elemnets of b
		uint32_t carry = 0x0;

		size_t j;
		for(j = i ; j < sizeA + i ;  j++){//for all elements of a
			uint64_t product = (uint64_t)a->integer[j-i] * (uint64_t)b->integer[i];
			uint64_t sum = (uint64_t)c->integer[j] + product + carry;
			c->integer[j] = (uint32_t)(sum & MAXVAL);
			carry = (uint32_t)(sum >> 32);
		}
		
		if(carry){
			uint64_t sum = (uint64_t)c->integer[j] + carry;
			c->integer[j] = (uint32_t)(sum & MAXVAL);
		}
	}
	
	size_t i = sizeC-1;
	while( i > 0 && c->integer[i] == 0x0){
		i--;
	}
	if (!big_int_safe_realloc(c, i+1)) {
		fprintf(stderr, "Error: Failed to reallocate BigInt to size %zu words\n", (long unsigned int)i+1);
		exit(EXIT_FAILURE);
	}
}

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
   	int i, j;

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
			q->integer[j] = (k * BASE + u->integer[j])/v->integer[0];
			k = (k * BASE + u->integer[j]) - q->integer[j]*v->integer[0];
		}
		if(r){
			r->integer[0] = k;
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
			un[j+n] += carry;
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

//mod pow by squaring,binary exponentiation
//d = a^b mod c
int big_int_modpow(BigInt * a,BigInt *b, BigInt * c, BigInt * d){
	BigInt base = big_int_copy_from(*a);
	BigInt exponent = big_int_copy_from(*b);
	
	check_and_free(d);
	*d = big_int_constructor(0,1,0x1);
	
	while(!big_int_is_zero((&exponent))){
		if(exponent.integer[0] % 2 == 1){

			BigInt p;
			big_int_mult(d,&base,&p);		//p  = res * exponent
			big_int_mod(&p,c,d);				//res = p % c (res  = p % mod)	
			big_int_destructor(&p);
		}
		big_int_bit_shift_r(&exponent,1);		//exponent = exponent/2;
		
		BigInt baseSquare;
		big_int_mult(&base,&base,&baseSquare);		//baseSquare = base * base;
		
		big_int_mod(&baseSquare,c,&base);		//base = baseSquare % c .. base =\ base ** 2 % c;
		big_int_destructor(&baseSquare);
	}
	
	big_int_destructor(&base);
    big_int_destructor(&exponent);
    return 0;
}


void big_int_gcd(BigInt *a, BigInt *b, BigInt *c){
	//binary gcd algo(Stein's algorithm)

	check_and_free(c);

	if(big_int_is_zero(a)){
		big_int_copy(b, c);
		return;
	}
	if(big_int_is_zero(b)){
		big_int_copy(a, c);
		return;
	}
	
	BigInt x = big_int_copy_from(*a);
	BigInt y = big_int_copy_from(*b);
	
	int count = 0;
	while(!(x.integer[0] & 1) && !(y.integer[0] & 1)){
		big_int_bit_shift_r(&x,1);
		big_int_bit_shift_r(&y,1);
		count++;
	}
	
	
	
	BigInt ZERO = big_int_from_uint32_t(0);
	
	
	while((big_int_compare(&y,&ZERO,0) > 0)){
		while(!(x.integer[0] & 1)){
			big_int_bit_shift_r(&x,1);
		}
		while(!(y.integer[0] & 1)){
			big_int_bit_shift_r(&y,1);
		}
		
		if((big_int_compare(&x, &y,0)) > 0){
			//a = a-b
			BigInt t = {NULL, 0,0};
			big_int_sub(&x,&y,&t);
			big_int_destructor(&x);
			x.integer = t.integer;
			x.size = t.size;
			x.sign = t.sign;
			t.integer = NULL;

			//a = a/2
			big_int_bit_shift_r(&x,1);
		}
		else{
			//b = b-a			
			BigInt t = {NULL, 0,0};
			big_int_sub(&y,&x,&t);
			big_int_destructor(&y);
			y.integer = t.integer;
			y.size = t.size;
			y.sign = t.sign;
			t.integer = NULL;
			
			//b = b/2
			big_int_bit_shift_r(&y,1);
		}
	}
	big_int_bit_shift_l(&x,count); //gcd = a << count;
	big_int_copy(&x, c);
	
	big_int_destructor(&x);
    big_int_destructor(&y);
	big_int_destructor(&ZERO);
}

void big_int_xgcd(BigInt *a, BigInt *b, BigInt *gcd, BigInt *x_out,BigInt *y_out){
	if(big_int_is_zero(a)){
		big_int_copy(b, gcd);
		*x_out = big_int_from_uint32_t(0x0);
		*y_out = big_int_from_uint32_t(0x1);
		return;
	}

	if(big_int_is_zero(b)){
		big_int_copy(a, gcd);
		*x_out = big_int_from_uint32_t(0x1);
		*y_out = big_int_from_uint32_t(0x0);
		return;
	}


	BigInt u = big_int_copy_from(*a);
	BigInt v = big_int_copy_from(*b);

	
	BigInt x1 = big_int_from_uint32_t(0x1); //x1 = 1;
	BigInt y1 = big_int_from_uint32_t(0x0); //y1 = 0;
	
	BigInt x2 = big_int_from_uint32_t(0x0); //x2 = 0;
	BigInt y2 = big_int_from_uint32_t(0x1); //y2 = 1;
	
	int count = 0;
	while(!(u.integer[0] & 1) && !(v.integer[0] & 1)){		//while( even(u) &&  even(v)){
		big_int_bit_shift_r(&u,1);							
		big_int_bit_shift_r(&v,1);
		count++;
	}
	
	BigInt u_i = big_int_copy_from(u);
	BigInt v_i = big_int_copy_from(v);


	while(big_int_compare(&u,&v,0) != 0){
		if(!(u.integer[0] & 1)){
			big_int_bit_shift_r(&u,1);
			if(!(x1.integer[0] & 1) && !(y1.integer[0] & 1)){
				big_int_bit_shift_r(&x1,1);
				big_int_bit_shift_r(&y1,1);

			}
			else{
				
				//x1 = x1 + b
				BigInt tx = {NULL, 0, 0};
				BigInt ty = {NULL, 0, 0};

				big_int_add(&x1,&v_i,&tx);
				big_int_destructor(&x1);
				memset(&x1, 0, sizeof(BigInt));  // Safe shallow reset
				x1.integer = tx.integer;
				x1.size = tx.size;
				x1.sign = tx.sign;


				//y1 = y1 - a
				big_int_sub(&y1,&u_i,&ty);
				big_int_destructor(&y1);
				memset(&y1, 0, sizeof(BigInt)); 
				y1.integer = ty.integer;
				y1.size = ty.size;
				y1.sign = ty.sign;

				//x1 = x1/2
				//y1 = y1/2
				big_int_bit_shift_r(&x1,1);
				big_int_bit_shift_r(&y1,1);

			}
		}
		else if(!(v.integer[0] & 1)){
			big_int_bit_shift_r(&v,1);
			if(!(x2.integer[0] & 1) && !(y2.integer[0] & 1)){
				big_int_bit_shift_r(&x2,1);
				big_int_bit_shift_r(&y2,1);
			}
			else{
				//x2 = x2 + b
				BigInt tx = {NULL, 0, 0};
				BigInt ty = {NULL, 0, 0};

				big_int_add(&x2,&v_i,&tx);
				big_int_destructor(&x2);
				x2.integer = tx.integer;
				x2.size = tx.size;
				x2.sign = tx.sign;


				//y2 = y2 - a
				big_int_sub(&y2,&u_i,&ty);
				big_int_destructor(&y2);
				y2.integer = ty.integer;
				y2.size = ty.size;
				y2.sign = ty.sign;

				big_int_bit_shift_r(&x2,1);
				big_int_bit_shift_r(&y2,1);

			}
		}
		else if((big_int_compare(&u, &v,0)) >= 0){
			BigInt t = {NULL, 0, 0};
			
			//u = u-v
			big_int_sub(&u,&v,&t);
			big_int_destructor(&u);
			u.integer = t.integer;
			u.size = t.size;
			u.sign = t.sign;

			//x1 = x1 - x2
			BigInt tx = {NULL, 0, 0};
			big_int_sub(&x1,&x2,&tx);
			big_int_destructor(&x1);
			x1.integer = tx.integer;
			x1.size = tx.size;
			x1.sign = tx.sign;
			
			//y1 = y1 - y2
			BigInt ty = {NULL, 0, 0};
			big_int_sub(&y1,&y2,&ty);
			big_int_destructor(&y1);
			y1.integer = ty.integer;
			y1.size = ty.size;
			y1.sign = ty.sign;

		}
		else{
			//v = v-u			
			BigInt t = {NULL, 0, 0};
		
			big_int_sub(&v,&u,&t);
			big_int_destructor(&v);
			v.integer = t.integer;
			v.size = t.size;
			v.sign = t.sign;

			//x2 = x2 - x1
			BigInt tx = {NULL, 0, 0};
			big_int_sub(&x2,&x1,&tx);
			big_int_destructor(&x2);
			x2.integer = tx.integer;
			x2.size = tx.size;
			x2.sign = tx.sign;

			BigInt ty = {NULL, 0, 0};
			big_int_sub(&y2,&y1,&ty);
			big_int_destructor(&y2);
			y2.integer = ty.integer;
			y2.size = ty.size;
			y2.sign = ty.sign;
			
		}
	}
	big_int_bit_shift_l(&u,count); //gcd = a << count;
	big_int_copy(&u, gcd);
	big_int_copy(&x1, x_out);
	big_int_copy(&y1, y_out);
	
	big_int_destructor(&u);big_int_destructor(&v);
	big_int_destructor(&u_i);big_int_destructor(&v_i);
	big_int_destructor(&x1);big_int_destructor(&y1);
	big_int_destructor(&x2);big_int_destructor(&y2);
}

//c = a^-1 mod b
void big_int_modinv(BigInt *a, BigInt *b, BigInt *c){
	BigInt x = {NULL, 0 , 0};
	BigInt y = {NULL, 0 , 0};
	BigInt gcd = {NULL, 0 , 0};

	check_and_free(c);

	big_int_xgcd(a,b,&gcd,&x,&y);
	BigInt one = big_int_from_uint32_t(0x1);

	if(big_int_compare(&gcd,&one,0) != 0){
		fprintf(stderr,"Inverse doesnt exist\n");
		return;
	}

	BigInt r = {NULL,0,0,};
	BigInt temp = {NULL,0,0};

	if(big_int_mod(&x,b,&r) == 1){		//temp = x mod b(unisgned)
		fprintf(stderr,"Mod failed\n");
		return;
	}

	if(x.sign == 1){		//if x negetive
		big_int_usub(b,&r,&temp);		//r = m - r
		big_int_destructor(&r);
		r.sign = temp.sign;
		r.integer = temp.integer;
		r.size = temp.size;
	}

	big_int_copy(&r, c);
	big_int_destructor(&x);
	big_int_destructor(&y);
	big_int_destructor(&gcd);
	big_int_destructor(&one);
	big_int_destructor(&r);

}

size_t big_int_bit_length(BigInt *a){
	int leadingZero = big_int_count_leading_zeros(a);
	size_t sizeA = a->size;
	size_t bit_len = (sizeA * 32) - leadingZero;
	return bit_len;
}
