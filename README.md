# Big Integer arithmetic (bigint)

Multi-precision integer arithmetic package for Win32 and x64, using compiler intrinsics for MSVC to speed all operations. 

Based on the bigint code in shazilkp/rsa-simple (not a fork, as I didn't want the whole thing) but with the following changes:

* Results returned through pointers reuse existing underlying integer arrays where possible, to save mallocs and prevent orphaned memory allocations

* 64-bit version added using intrinsics (_addcarry_u64, __mul128, _div128)

* Read and print long decimal number strings

* Constructors rationalised (it is preferable to return a result through a pointer)

* Some routines removed as they were not needed (they may find their way back)

Todos:

- Incorporate routines for rational arithmetic aswell as integers

Building:

- build from solution (.sln) file in VC 2019 or later
- libtest contains tests and benchmarks
