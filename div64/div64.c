
// just for main at end
#include <stdio.h>

// intrinsics
#include <intrin.h>
#include <immintrin.h>

// from google gemini - prob don't need them all

#include <stdint.h>
#include <stddef.h>
#include <string.h>

typedef uint64_t uint_t;

/**
 * Multiprecision Division (Knuth's Algorithm D)
 * Computes Q = N / D and updates N to be the Remainder R = N % D.
 *
 * @param q  Array to store the quotient (size at least n - m + 1)
 * @param n  Array containing the dividend (size n_size, modified to hold remainder)
 * @param n_size Size of the dividend array
 * @param d  Array containing the divisor (size m_size)
 * @param m_size Size of the divisor array
 * @return 0 on success, -1 on division by zero
 */
int mp_div(uint_t *q, uint_t *n, size_t n_size, const uint_t *d, size_t m_size) {
    // Strip leading zeros from the divisor
    while (m_size > 0 && d[m_size - 1] == 0) {
        m_size--;
    }
    if (m_size == 0) return -1; // Division by zero

    // Strip leading zeros from the dividend
    while (n_size > 0 && n[n_size - 1] == 0) {
        n_size--;
    }

    // If dividend is smaller than divisor, quotient is 0 and remainder is dividend
    if (n_size < m_size) {
        memset(q, 0, (n_size == 0 ? 1 : n_size) * sizeof(uint_t));
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
            q[i - 1] =_udiv128(rem, n[i - 1], d0, &rem);
        }
#endif
        n[0] = rem;
        memset(n + 1, 0, (n_size - 1) * sizeof(uint_t));
        return 0;
    }

    // Step D1: Normalize the divisor and dividend
    int shift = (int)__lzcnt64(d[m_size - 1]);
    uint_t *vn = malloc(m_size * sizeof(uint_t));

    // Normalize divisor
    uint_t carry = 0;
    for (size_t i = 0; i < m_size; i++) {
        vn[i] = (d[i] << shift) | (shift ? (carry >> (64 - shift)) : 0);
        carry = d[i];
    }
    if (shift) vn[m_size - 1] |= (carry >> (64 - shift));

    // Normalize dividend (requires an extra limb at the top for potential overflow)
    uint_t* un = malloc((n_size + 1) * sizeof(uint_t));
    carry = 0;
    for (size_t i = 0; i < n_size; i++) {
        un[i] = (n[i] << shift) | (shift ? (carry >> (64 - shift)) : 0);
        carry = n[i];
    }
    un[n_size] = shift ? (carry >> (64 - shift)) : 0;

#if 0
    // Step D2 to D7: Loop through quotient digits
    for (size_t j = q_size; j > 0; j--) {
        size_t idx = j - 1;

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

        // Step D4: Multiply and subtract
        limb2_t borrow = 0;
        for (size_t i = 0; i < m_size; i++) {
            limb2_t prod = q_hat * vn[i];
            limb2_t sub = (limb2_t)un[idx + i] - borrow - (prod & 0xFFFFFFFFFFFFFFFFULL);
            un[idx + i] = (uint_t)sub;
            borrow = (prod >> 64) - (sub >> 63); // Handle borrow propagation safely
        }

        limb2_t sub_top = (limb2_t)un[idx + m_size] - borrow;
        un[idx + m_size] = (uint_t)sub_top;

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
    }

    // Step D8: Denormalize the remainder
    carry = 0;
    for (size_t i = m_size; i > 0; i--) {
        uint_t val = un[i - 1];
        n[i - 1] = (val >> shift) | (shift ? (carry << (64 - shift)) : 0);
        carry = val;
    }
    memset(n + m_size, 0, (n_size - m_size) * sizeof(uint_t));
#endif
    free(vn);
    free(un);
    return 0;
}

int main()
{
  // mp_div(uint_t *q, uint_t *n, size_t n_size, const uint_t *d, size_t m_size)

  uint_t divisor[10], dividend[20], quotient[10];   // remainder gets left in the dividend array

  // Divide a small number by 10
  printf("64-bit Divide Test\n");
  divisor[0] = 10;
  dividend[0] = 12345678;
  mp_div(quotient, dividend, 1, divisor, 1);
  printf("1/1: %lld %lld\n", quotient[0], dividend[0]);

  // Divide 10**30 by 10
  // expect 10**29 = 0x1431e0fae_6d7217caa0000000
  // 10**30 = 0xc9f2c9cd0_4674edea40000000
  dividend[0] = 0x4674edea40000000;
  dividend[1] = 0xc9f2c9cd0;
  quotient[0] = 0;
  quotient[1] = 0;
  mp_div(quotient, dividend, 2, divisor, 1);
  printf("2/1: %016llx %016llx %lld\n", quotient[1], quotient[0], dividend[0]);

  // Divide 10**30 by 10**29, expect 10
  dividend[0] = 0x4674edea40000000;
  dividend[1] = 0xc9f2c9cd0;
  divisor[0] = 0x6d7217caa0000000;
  divisor[1] = 0x1431e0fae;
  quotient[0] = 0;  // don't forget to zero out stale data (it doesn't return the quot/rem sizes)
  quotient[1] = 0;
  mp_div(quotient, dividend, 2, divisor, 2);
  printf("2/2: %016llx %016llx %lld\n", quotient[1], quotient[0], dividend[0]);

  getchar();
}
