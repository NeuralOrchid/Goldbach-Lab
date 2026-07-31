// Created by ChatGPT (OpenAI)
// Date: 2026-07-31
//
// ---------------------------

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <max_k>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *end = NULL;
    uint64_t max_k = strtoull(argv[1], &end, 10);
    if (!end || *end != '\0') {
        die("Invalid max_k.");
    }

    if (max_k < 1) {
        die("max_k must be at least 1.");
    }

    uint64_t max_n = 6ULL * max_k + 1ULL;   // largest number we need to test

    // Odd-only sieve:
    // index i represents number (2*i + 1)
    // so n -> n/2 for odd n
    uint64_t odd_count_u64 = (max_n >> 1) + 1ULL;
    if (odd_count_u64 > SIZE_MAX) {
        die("Range too large for this build.");
    }

    size_t odd_count = (size_t)odd_count_u64;
    uint8_t *is_prime = (uint8_t *)malloc(odd_count);
    if (!is_prime) {
        die("Memory allocation failed.");
    }

    memset(is_prime, 1, odd_count);

    // 1 is not prime
    is_prime[0] = 0;

    // Sieve only odd numbers
    uint64_t limit = (uint64_t)sqrt((long double)max_n);
    for (uint64_t p = 3; p <= limit; p += 2) {
        if (is_prime[p >> 1]) {
            uint64_t step = 2ULL * p;
            uint64_t start = p * p;
            for (uint64_t m = start; m <= max_n; m += step) {
                is_prime[m >> 1] = 0;
            }
        }
    }

    FILE *fp_plus = fopen("prime_6k_plus_1.csv", "w");
    if (!fp_plus) die("Failed to open prime_6k_plus_1.csv");

    FILE *fp_minus = fopen("prime_6k_minus_1.csv", "w");
    if (!fp_minus) die("Failed to open prime_6k_minus_1.csv");

    // Large buffered output for speed
    static char buf_plus[1 << 20];
    static char buf_minus[1 << 20];
    setvbuf(fp_plus, buf_plus, _IOFBF, sizeof(buf_plus));
    setvbuf(fp_minus, buf_minus, _IOFBF, sizeof(buf_minus));

    fprintf(fp_plus, "k,prime\n");
    fprintf(fp_minus, "k,prime\n");

    for (uint64_t k = 1; k <= max_k; ++k) {
        uint64_t n_minus = 6ULL * k - 1ULL;
        uint64_t n_plus  = 6ULL * k + 1ULL;

        if (n_minus <= max_n && (n_minus == 2 || ((n_minus & 1ULL) && is_prime[n_minus >> 1]))) {
            fprintf(fp_minus, "%" PRIu64 ",%" PRIu64 "\n", k, n_minus);
        }

        if (n_plus <= max_n && (n_plus == 2 || ((n_plus & 1ULL) && is_prime[n_plus >> 1]))) {
            fprintf(fp_plus, "%" PRIu64 ",%" PRIu64 "\n", k, n_plus);
        }
    }

    fclose(fp_plus);
    fclose(fp_minus);
    free(is_prime);

    return EXIT_SUCCESS;
}