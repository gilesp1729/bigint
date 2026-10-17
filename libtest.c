#include "bigint.h"
#include <stdio.h>


int main(){
	BigInt a1 = big_int_constructor(0, 16,
	    0x97555800 , 0x6A321112 , 0xB9DB9D03 , 0x36A0D281 ,
	    0x1668B5B3 , 0x09E3345E , 0x5C4F2B9D , 0x5DD10C1A ,
	    0xB9B77BFC , 0xBEDA9538 , 0x3668A252 , 0x2A71BE5A ,
	    0x2030D449 , 0x0A12A85F , 0xA7A14B50 , 0x05BBCCE3 );
	    
	
	BigInt zero = big_int_constructor(0,1,0x00000000);

	BigInt b1 = big_int_constructor(0, 16,
	    0x5C189D00 , 0xA3B2E4FB , 0xFCB93DA0 , 0x4453859B ,
	    0xA9222471 , 0x15E747A2 , 0x5D0B2B12 , 0x39DC45CB ,
	    0x669F8809 , 0xC1859E49 , 0xC5FEE768 , 0x20E4FDA8 ,
	    0x19960098 , 0x4F513BA2 , 0x65ACF9F7 , 0x2B5C3D79);
	    
	BigInt a = big_int_constructor(0, 1,
	    65537);
	    
	BigInt b = big_int_constructor(0, 32,
		0xC1A3B2D4, 0xC1A3B2D5, 0x12345678, 0x9ABCDEF0,
		0x0FEDCBA9, 0x98765432, 0xCAFEBABE, 0xDEADBEEF,
		0xFACEB00C, 0x8BADF00D, 0xB16B00B5, 0x0D15EA5E,
		0xBADC0FFEE, 0xFEEDFACE, 0xBAADF00D, 0xDEADC0DE,
		0xC0DEC0DE, 0x0F00D00D, 0xCAFED00D, 0xBAADF00D,
		0xFEEDFACE, 0xBAADF00D, 0xABAD1DEA, 0xDEADBEEF,
		0xCAFEBABE, 0x0BADF00D, 0x1BADB002, 0x8BADF00D,
		0xCAFEBABE, 0x8BADF00D, 0xCAFED00D, 0xFFFFFFFF);
	    
	    
	BigInt x_pred = big_int_constructor(0,16,
					0x028B5309, 0x34714DC6, 0xE7E1BCFE, 0xD78DC380,
					0x6CF31C1D, 0x751C74DC, 0xF0805FE7, 0xEC7332D4,
					0x19F65399, 0xBF9784A5, 0xDF3772E9, 0x0938AD5F,
					0x4278A403, 0xE3EE62DF, 0x1FCC23B6, 0x0A59C314);
					
	BigInt y_pred = big_int_constructor(1,16,
					0x00562023, 0x11FE8C42, 0xEA27FFAA, 0x763995E9,
					0x44B35668, 0x95EB19A3, 0x7F5A8850, 0x8E6BB99E,
					0x5F17EA98, 0xD14CEEAC, 0x66835186, 0x798081C9,
					0x462C828B, 0x2DF1D580, 0x0BD53976, 0xB65A72A6);

	BigInt x,y;		// Don't pass uninitialised memory! if these are ever used
	BigInt modinv = {NULL, 0 , 0};
	
	
	//0xE34A15CD 0xBB237AAD 0x99213458 0xAABBCCDD 0xEEFF0011 0x22334455 0x66778899 
	//0x66778899 , 0x22334455 , 0xEEFF0011 , 0xAABBCCDD,  0x99213458 , 0xBB237AAD ,0xE34A15CD
	
	//0xE34A15CD 0xBB237AAD 0x99213458 0xAABBCCDD 0x11223344 0x55667788 0x9900AABB 0xCCDDEEFF 
	//0xCCDDEEFF, 0x9900AABB,0x55667788,0x11223344 ,0xAABBCCDD,  0x99213458 , 0xBB237AAD ,0xE34A15CD
	
	BigInt t1 = { 0, };	// Size of zero will force an alloc. Don't pass uninitialised memory!
	BigInt t2 = { 0, };
	BigInt t3 = { 0, };

	// TODO:
	// - comment and collect these tests into meaningful blocks.
	// - print out some sort of pass/fail on them.
	// - not just correct output, but make sure reallocing/reusing 
	//   code gets exercised.
	// - Some sort of benchmark so speed optimisations can be checked.


	printf("x+pred = ");
	big_int_print(&x_pred,0);
	printf("\n");
	
	printf("a = ");
	big_int_print(&a,0);
	printf("\n");

	big_int_mult(&x_pred,&a,&t2);
	
	printf("t2 = ");
	big_int_print(&t2,0);
	printf("\n\n\n");
	
	
	printf("y_pred = ");
	big_int_print(&y_pred,0);
	printf("\n");
	
	printf("b = ");
	big_int_print(&b,0);
	printf("\n");

	big_int_mult(&y_pred,&b,&t1);
	
	printf("t1 = ");
	big_int_print(&t1,0);
	printf("\n\n");
	
	
	big_int_add(&t2,&t1,&t3);
	printf("t3 = ");
	big_int_print(&t3,0);
	printf("\n\n");
	
	


	
	printf("a = ");
	big_int_print(&a,0);
	printf("\n");
	
	printf("b = ");
	big_int_print(&b,0);
	printf("\n");
	
	
	big_int_modinv(&a,&b,&modinv);
	printf("mod inv = ");
	big_int_print(&modinv,0);
	printf("\n");
	
	
	printf("a = ");
	big_int_print(&a,0);
	printf("\n");
	
	printf("b = ");
	big_int_print(&b,0);
	printf("\n");
	
	big_int_mult(&a,&modinv,&t1);
	big_int_mod(&t1,&b,&t2);
	
	printf("t1 = ");
	big_int_print(&t1,0);
	printf("\n");
	
	printf("t2 = ");
	big_int_print(&t2,0);
	printf("\n");
	
	/*
	printf("X = ");
	big_int_print(&x,0);
	printf("\n");
	
	printf("Y = ");
	big_int_print(&y,0);
	printf("\n");
	
	
	
	big_int_mult(&a,&x,&t1);
	big_int_mult(&b,&y,&t2);
	
	big_int_add(&t1,&t2,&t3);
	printf("t1 = ");
	big_int_print(&t1,0);
	printf("\n");
	
	printf("t2 = ");
	big_int_print(&t2,0);
	printf("\n");
	
	printf("t3 = ");
	big_int_print(&t3,0);
	printf("\n");
	
	*/
	
	
	
	
	//0x05DAB7FA 0xA150868B
	// 0x00000005 0x64B68791 
	
	/*
	BigInt t = big_int_constructor(0,2,  0xA150868B,0x05DAB7FA );
	big_int_mult(&t,&t,&m);
	big_int_print(&t,1);
	big_int_print(&m,1);
	
	*/
	
	/*
	BigInt f = big_int_constructor(0,7,  0x66778899 , 0x22334455 , 0xEEFF0011 , 0xAABBCCDD,  0x99213458 , 0xBB237AAD ,0xE34A15CD);
	BigInt g = big_int_constructor(0,8,  0xCCDDEEFF, 0x9900AABB,0x55667788,0x11223344 ,0xAABBCCDD,  0x99213458 , 0xBB237AAD ,0xE34A15CD);
	BigInt h;
	big_int_sub(&g,&f,&h);
	big_int_print(&h,1);
	*/
	
	
	big_int_destructor(&a);
	big_int_destructor(&b);
	//big_int_destructor(&tx);
	//big_int_destructor(&r);


	getchar();  // so we can see the bloody output...
}
