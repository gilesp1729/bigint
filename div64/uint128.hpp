#include <cstdint>
#include <intrin.h>
#include <immintrin.h>

typedef uint64_t uint_t;

class uint128_t
{
public:
    // Constructors
    uint128_t() {};
    uint128_t(uint_t lo) { low = lo; high = 0; }
    uint128_t(uint_t lo, uint_t hi) { low = lo; high = hi; }
    ~uint128_t() {};

    // The 128-bit number structure
    uint_t low = 0;
    uint_t high = 0;

    // Operators
    uint128_t operator+(uint128_t b)
    {
        uint128_t r;
        unsigned char c = _addcarry_u64(0, low, b.low, &r.low);
        (void)_addcarry_u64(c, high, b.high, &r.high);
        return r;
    }

    uint128_t operator-(uint128_t b)
    {
        uint128_t r;
        unsigned char c = _subborrow_u64(0, low, b.low, &r.low);
        (void)_subborrow_u64(c, high, b.high, &r.high);
        return r;
    }

    uint128_t operator*(uint128_t b)
    {
        // we want the low 128 bits of:
        // (a.low + 2^64 a.high) * (b.low + 2^64 b.high)
        //
        // r.low  = lo64(a.low * b.low)     (a = this)
        // r.high = hi64(a.low * b.low)
        //        + lo64(a.low * b.high)
        //        + lo64(a.high * b.low)        (mod 2^64)

        uint128_t r;

        uint_t p0_hi;
        r.low = _umul128(low, b.low, &p0_hi);

        // cross terms: only the low 64 bits contribute to r.high
        uint_t t1_hi;
        uint_t t1_lo = _umul128(low, b.high, &t1_hi);

        uint_t t2_hi;
        uint_t t2_lo = _umul128(high, b.low, &t2_hi);

        // 64-bit add is sufficient: carries would land in bit 128 and are discarded
        r.high = p0_hi + t1_lo + t2_lo;

        return r;
    }

    // TODO: >> operators and maybe ++ += 
    // TODO: 128x64 multiply, simplified form of the above 128x128

    uint128_t operator<<(int shift)
    {
        if (shift == 64)
        {
            high = low;
            low = 0;
            return *this;
        }
        // The shift must be 64 or less
        uint_t carry = 0;
        uint_t current = low;
        low = current << shift;
        carry = current >> (64 - shift); 
        high = high << shift | carry;
        return *this;
    }

    uint128_t operator>>(int shift)
    {
        if (shift == 64)
        {
            low = high;
            high = 0;
            return *this;
        }
        // The shift must be 64 or less
        uint_t current = low >> shift;
        uint_t carry = high << (64 - shift);
        low = current | carry;
        high = high >> shift;
        return *this;
    }

    bool operator==(uint128_t b)
    {
        return low == b.low && high == b.high;
    }

    bool operator!=(uint128_t b)
    {
        return low != b.low || high != b.high;
    }

    bool operator<(uint128_t b)
    {
        // This is actually faster than the simple compare.
        uint_t dont_care;
        unsigned char borrow = _subborrow_u64(0, low, b.low, &dont_care);
        // final borrow tells us if a < b in 128-bit unsigned.
        borrow = _subborrow_u64(borrow, high, b.high, &dont_care);

        return borrow != 0;
    }

    bool operator<=(uint128_t b)
    {
        if (low == b.low && high == b.high)
            return true;

        uint_t dont_care;
        unsigned char borrow = _subborrow_u64(0, low, b.low, &dont_care);
        borrow = _subborrow_u64(borrow, high, b.high, &dont_care);

        return borrow != 0;
    }
};

// Divide primitive: only 128/64 supported.
uint128_t udiv128_by_64_to_128(uint128_t dividend, uint_t divisor, uint_t* remainder)
{
    uint128_t quotient = 0;
    uint_t rem_high = 0;

    // If high part is smaller than divisor, quotient.hi is 0 and we just do 1 div
    if (dividend.high < divisor) {
        quotient.high = 0;
        quotient.low = _udiv128(dividend.high, dividend.low, divisor, remainder);
    }
    else {
        // High part is >= divisor, so quotient.hi is non-zero
        uint_t rem1;
        quotient.high = _udiv128(0, dividend.high, divisor, &rem1);
        quotient.low = _udiv128(rem1, dividend.low, divisor, remainder);
    }

    return quotient;
}

