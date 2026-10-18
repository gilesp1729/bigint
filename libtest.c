#include "bigint.h"
#include <stdio.h>


int main(){

	// Some strings representing big numbers. They come out of a simple Python test.
	char* ten_to_3000_hex_str = 
		"0x371c933511046bd4cdec92fc6c8178230b2549d97001ca7c02435f365bdbc7192647b82c15aac9821fe6812bf9913d9dfbc622dcf5b337ab4b6f0bb02343d2615c1615232c46ad3933ad83cef354a68ac13429f8c95072e1323afbece5a77e5fd000ecd26452364b2670492ba36553bc34425a1ca2fa6aec951461a0c1a8431e06900f8b2e514012d0710f87e473ed72b042dfadc6db6ef17e6a12837cddd30ff0fd4c8c174efe3b62a9ffe2eba4022e20f6bd7a9060b68321a104866833e7b559cef1b39d32eb9466527916f5afba4a6643c411845c37e7066a6bba98ec72a22fcde648884a9fb73cef9a16bf1b31863b5f556e400cfdf24ac46331e74203182d223a2ab29a27059296f02384c293d8c5e7bdfc683a750b54b08d1a1aeaaaed7f34ccbac3a5d57f08833ea8d2d5ec680953855396e70c06666df22a3c5fc614e5dfdf0d0da39b8967ee826c6268ed07007215f3e9420daad10f27e4badd219cba8d4fb3b195fb7053fec305d89ce7af2eec3fd4f61fd0e4c0f243c017d7f74d81cfa0829ac5f90a8f76b3185c81e116c15f6ba85c86b96bbb2b44914a8f11dca8ff55c194066c5747b9f2a81d7567adacf7d74b30783414bed4acf468576794fbcab256b26941446c050572d6b4b9206c9ff78460c82acc992cc9ec57de57ab5d93bb5ca062f7755ed1dea8902b32a68e721fecfb8d9592c65509b4b5d171bb1dede975aba74de9b5a067b9adf35b081bf1898cf4bc0d4d356e421493bc626f16db5b6fa77606746c1aa9111daacf0d9161735413745625bcf6d9ca151bfe171eac443d5051a9110834bce31d87f019ac2d7de39381214eeef8d2be9aa28eaab55b2205b634e3bf6009a02696a4e56c7662a4f98edd8efab57087139cbabb6b1a69ab63d627cef099ae9d266c72508a13009132f2d208cf0cfcc921005b142e82656531796abe863b296e53060c50fc4b812db280ccd79df9650d738b950f9e61c85542725f607afdedc911d4c090ecc1914faf2eb655054d2b5ccb9bef9cd67ac624e907fb77832c1d8f08c4775c655a4b914aa95b16e6ec7911a9cd88504b056293fbe80de7c08966a5dd32710e9564c95feb31a50ebfad3649cb13decd534f883b6681e8ae6f8ba1139f1457bce783c2d8b0c49457dce5a5bd8da6ef40f4fe3c438c3a84932a3a094b71ce322528b57b2d46fc3f0514690672736307712cac5f4d9fa1df21000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";
	char* ten_to_1500_hex_str =
		"0x76c78ecdd1c287d4d59bc90c35c12b65b0c4209c4c15933b87344984044c585e68ea505c63aaafbfaaef0466d69d5df9b13e87dc0ba69a5b9ab1e836abf300fab7ade954be12149c25e0eedef8fe4c8a99636d8330845e386a399d797184f654b1bd6745384d054149eb29860dae07fb13233ba2b4d7f5fe93002544496d36e6df47196cb1ac8ad495ebbfd5890f96e2340eeb22838f2ee096d815b22766cc30398e63fa0588c904e3ad9ea1ccdddc90128db8b45b4fc7d21e288f74084cc66e9286b2fc916e2e6d431833bf07d470dee1f32d67442d384a2297970bcb09414154fff89474fe840b2cfb4d21fb2ffc90e475132902caeb297bcaf357877fe88a813b1fc1be726b390dccf7bee8675864abf19f1f9b006b91e754055febb334b8012573218529fb4ba29706017c13744a0c1e45815df936d24d9ccc8dceaa604491e57e4a19a8421504025257d53e5027265403b5d777e590cf7bdc9475386f1c0b87878e0b2178c4b7a6c3ae40040de7cf03abaf1ef797f13a4bd3f82d3069b2562a7860bf0dca7f9ba1f4b6bd97a73633ff7d4ecf1cb290b9ec187f6d82fc9f58c0c07c8dd4b52ec3b04ad68126b32bbc87f11000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000";

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
