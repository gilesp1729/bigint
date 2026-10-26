#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "uint128.hpp"
#include "bigint.h"

/**
 * Multiprecision Division (Knuth's Algorithm D)
 * Computes Q = N / D and updates N to be the Remainder R = N % D.
 *
// Pass bigints here instead of arrays, so correct sizes can be returned.
 * Params derived from the bigints:
 * @param n  Array containing the dividend (size n_size)
 * @param n_size Size of the dividend array
 * @param d  Array containing the divisor (size m_size)
 * @param m_size Size of the divisor array
 * @param q  Array to store the quotient (size at least n - m + 1)
 * @param r  Array to store the remainder (size at least m) or NULL if not used
 * @return 0 on success, -1 on division by zero
 */
int mp_div(BigInt* dividend,  BigInt *divisor, BigInt *quotient, BigInt* remainder) 
{
    // Copy the sizes. 
    uint_t* n = dividend->integer;
    size_t n_size = dividend->size;
    uint_t* d = divisor->integer;
    size_t m_size = divisor->size;
    uint_t* q = quotient->integer;
    uint_t* r = NULL;
    if (remainder)
        r = remainder->integer;

#if 0 // Not needed, we assume all bigints are well-formed
    // Strip leading zeros from the divisor as a sanity check
    while (m_size > 0 && d[m_size - 1] == 0) {
        m_size--;
    }
    if (m_size == 0) return -1; // Division by zero

    while (n_size > 0 && n[n_size - 1] == 0) {
        n_size--;
    }
#endif

    // If dividend is smaller than divisor, quotient is 0 and remainder is dividend
    // Catches the case n_size = 0 (which is not well formed, but you know what we mean)
    if (n_size < m_size) {
        memset(q, 0, (n_size == 0 ? 1 : n_size) * sizeof(uint_t));
        if (r)
            memcpy(r, n, m_size * sizeof(uint_t));
        return 0;
    }

    size_t q_size = n_size - m_size + 1;

    // Base case: Divisor is exactly 1 limb long (Single-limb division)
    if (m_size == 1) {
        uint_t rem = 0;
        uint_t d0 = d[0];
#if 0
        for (size_t i = n_size; i > 0; i--) {
            limb2_t alloc = ((limb2_t)rem << 64) | n[i - 1];
            q[i - 1] = (uint_t)(alloc / d0);
            rem = (uint_t)(alloc % d0);
        }
#else
        for (size_t i = n_size; i > 0; i--) {
            q[i - 1] = _udiv128(rem, n[i - 1], d0, &rem);
        }
#endif
        // Trim leading zeros, leaving a minimum size of 1.
        while (q_size > 1 && q[q_size - 1] == 0) {
            q_size--;
        }
        quotient->size = q_size;
        if (r)
        {
            r[0] = rem;
            //memset(r + 1, 0, (m_size - 1) * sizeof(uint_t));
            remainder->size = 1;
        }
        return 0;
    }

    // Step D1: Normalize the divisor and dividend
    int shift = (int)__lzcnt64(d[m_size - 1]);
    uint_t* vn = (uint_t *)malloc(m_size * sizeof(uint_t));

    // Normalize divisor
    uint_t carry = 0;
    for (size_t i = 0; i < m_size; i++) {
        vn[i] = (d[i] << shift) | (shift ? (carry >> (64 - shift)) : 0);
        carry = d[i];
    }
    if (shift) vn[m_size - 1] |= (carry >> (64 - shift));

    // Normalize dividend (requires an extra limb at the top for potential overflow)
    uint_t* un = (uint_t *)malloc((n_size + 1) * sizeof(uint_t));
    carry = 0;
    for (size_t i = 0; i < n_size; i++) {
        un[i] = (n[i] << shift) | (shift ? (carry >> (64 - shift)) : 0);
        carry = n[i];
    }
    un[n_size] = shift ? (carry >> (64 - shift)) : 0;

    // Step D2 to D7: Loop through quotient digits
    for (size_t j = q_size; j > 0; j--) {
        size_t idx = j - 1;

#if 0
        // Step D3: Calculate the estimated quotient digit q_hat
        limb2_t dividend_hat = ((limb2_t)un[idx + m_size] << 64) | un[idx + m_size - 1];
        limb2_t q_hat = dividend_hat / vn[m_size - 1];
        limb2_t r_hat = dividend_hat % vn[m_size - 1];

        // Refine q_hat estimation
        while (q_hat >= ((limb2_t)1 << 64) ||
            (q_hat * vn[m_size - 2] > ((r_hat << 64) | un[idx + m_size - 2]))) {
            q_hat--;
            r_hat += vn[m_size - 1];
            if (r_hat >= ((limb2_t)1 << 64)) break;
        }
#else
        uint128_t dividend_hat = { un[idx + m_size - 1], un[idx + m_size] };
        uint128_t q_hat;
        uint128_t r_hat = 0;
        uint128_t prod;

        q_hat = udiv128_by_64_to_128(dividend_hat, vn[m_size - 1], &r_hat.low);

        // Refine q_hat estimation
        while (1)
        {
            if (q_hat.high == 0) 
                break;
            uint128_t tst = { un[idx + m_size - 2], r_hat.low };
            if (q_hat * vn[m_size - 2] <= tst)  
                break;
            q_hat = q_hat - 1;                  // no ++ or += ops so write it out long hand
            r_hat = r_hat + vn[m_size - 1];
            if (r_hat.high != 0) 
                break;
        }
#endif

        // Step D4: Multiply and subtract
#if 0
        limb2_t borrow = 0;
        for (size_t i = 0; i < m_size; i++) {
            limb2_t prod = q_hat * vn[i];                                                   // 128x64 multiply
            limb2_t sub = (limb2_t)un[idx + i] - borrow - (prod & 0xFFFFFFFFFFFFFFFFULL);  
            un[idx + i] = (uint_t)sub;
            borrow = (prod >> 64) - (sub >> 63); // Handle borrow propagation safely
        }

        limb2_t sub_top = (limb2_t)un[idx + m_size] - borrow;
        un[idx + m_size] = (uint_t)sub_top;
#else
        uint128_t borrow = 0;
        for (size_t i = 0; i < m_size; i++) 
        {
            uint128_t prod = q_hat * vn[i];            // Need a 128x64 multiply op - casting up will work but is wasteful of multiplies
            uint128_t sub = (uint128_t)un[idx + i] - borrow - prod.low;
            un[idx + i] = sub.low;
            borrow = (prod >> 64) - (sub >> 63); // Handle borrow propagation safely
        }

        uint128_t sub_top = (uint128_t)un[idx + m_size] - borrow;
        un[idx + m_size] = sub_top.low;

#endif



#if 0
        // Step D6: Add back if the quotient estimate was 1 too large
        if ((sub_top >> 63) != 0) {
            q_hat--;
            uint_t carry_back = 0;
            for (size_t i = 0; i < m_size; i++) {
                limb2_t sum = (limb2_t)un[idx + i] + vn[i] + carry_back;
                un[idx + i] = (uint_t)sum;
                carry_back = (uint_t)(sum >> 64);
            }
            un[idx + m_size] += carry_back;
        }

        q[idx] = (uint_t)q_hat;
#else
        // Step D6: Add back if the quotient estimate was 1 too large
        if ((sub_top >> 63) != 0) {
            q_hat = q_hat - 1;
            uint_t carry_back = 0;
            for (size_t i = 0; i < m_size; i++) {
                uint128_t sum = (uint128_t)un[idx + i] + vn[i] + carry_back;
                un[idx + i] = sum.low;
                carry_back = sum.high;
            }
            un[idx + m_size] += carry_back;
        }

        q[idx] = q_hat.low;
#endif
    }

    // Step D8: Denormalize the remainder (leave original dividend intact)
    if (r)
    {
        memcpy(r, n, m_size * sizeof(uint_t));
        carry = 0;
        for (size_t i = m_size; i > 0; i--) {
            uint_t val = un[i - 1];
            r[i - 1] = (val >> shift) | (shift ? (carry << (64 - shift)) : 0);
            carry = val;
        }
    }

    free(vn);
    free(un);

    // Reset correct sizes for quotient and remainder by stripping leading zero limbs
    // Quotient cannot be zero (that case is trapped at top) and remainder must
    // be returned as a single liumb even if it's zero.
    while (q_size > 1 && q[q_size - 1] == 0) {
        q_size--;
    }
    quotient->size = q_size;
    if (r)
    {
        while (m_size > 1 && r[m_size - 1] == 0) {
            m_size--;
        }
        remainder->size = m_size;
    }

    return 0;
}

