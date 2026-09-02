// Created by ChatGPT (OpenAI)
// Date: 2026-09-02
//
// ---------------------------
// computation_2026-09-02_18-57.cpp
// Build: g++ -O3 -march=native -flto -fopenmp computation_2026-09-02_18-57.cpp -o goldbach_bound
// Usage: goldbach_bound 100000000 1000 failures.csv

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

// ------------------------------------------------------------
// Packed primality table for odd integers only.
//
// Bit = 1  -> composite
// Bit = 0  -> prime
//
// Index for odd x is x >> 1.
// ------------------------------------------------------------

class OddSieve {
private:
    uint64_t N;
    std::vector<uint64_t> composite;

public:
    explicit OddSieve(uint64_t n) : N(n) {
        const uint64_t odd_count = (N >> 1) + 1;
        composite.resize((odd_count + 63) >> 6, 0ULL);

        sieve();
    }

    inline void set_composite(uint64_t x) {
        const uint64_t i = x >> 1;
        composite[i >> 6] |= 1ULL << (i & 63);
    }

    inline bool is_prime(uint64_t x) const {
        if (x == 2) return true;
        if (x < 2 || !(x & 1ULL)) return false;

        const uint64_t i = x >> 1;
        return ((composite[i >> 6] >> (i & 63)) & 1ULL) == 0;
    }

    void sieve() {
        if (N < 3) return;

        for (uint64_t p = 3; p <= N / p; p += 2) {
            if (!is_prime(p)) continue;

            const uint64_t step = p << 1;

            // Start at p^2; only odd multiples need marking.
            for (uint64_t x = p * p; x <= N; x += step) {
                set_composite(x);
            }
        }
    }

    std::vector<uint32_t> primes_up_to(uint64_t limit) const {
        std::vector<uint32_t> primes;

        if (limit >= 2)
            primes.push_back(2);

        if (limit >= 3) {
            for (uint64_t p = 3; p <= limit; p += 2) {
                if (is_prime(p))
                    primes.push_back(static_cast<uint32_t>(p));
            }
        }

        return primes;
    }
};

// ------------------------------------------------------------
// Main experiment
//
// Usage:
//   ./goldbach_bound N B output.csv
//
// Example:
//   ./goldbach_bound 100000000 1000 failures.csv
//
// Meaning:
//   Test every even n <= N.
//   Look for a Goldbach pair n = p + q with p <= B.
//   Since p is scanned from smallest upward, p is the minimum
//   possible smaller prime for that n.
//
// CSV contains ONLY failures: values of n for which no
// Goldbach representation exists with a prime <= B.
// ------------------------------------------------------------

int main(int argc, char** argv) {

    if (argc != 4) {
        std::cerr
            << "Usage: " << argv[0]
            << " N B output.csv\n\n"
            << "Example:\n"
            << "  " << argv[0]
            << " 100000000 1000 failures.csv\n";

        return 1;
    }

    const uint64_t N = std::stoull(argv[1]);
    const uint64_t B = std::stoull(argv[2]);
    const std::string output_file = argv[3];

    if (N < 6) {
        std::cerr << "N must be at least 6.\n";
        return 1;
    }

    std::cerr << "Building sieve up to " << N << "...\n";

    OddSieve sieve(N);

    // We never need to try p > min(B, n/2).
    const uint64_t prime_limit = std::min(B, N / 2);

    std::cerr
        << "Generating primes up to "
        << prime_limit << "...\n";

    const std::vector<uint32_t> primes =
        sieve.primes_up_to(prime_limit);

    std::cerr
        << "Number of candidate small primes: "
        << primes.size() << "\n";

    std::ofstream out(output_file);

    if (!out) {
        std::cerr << "Could not open output file.\n";
        return 1;
    }

    out << "n,B,status\n";

    uint64_t tested = 0;
    uint64_t failures = 0;
    uint64_t largest_required = 0;

    // --------------------------------------------------------
    // Each n is independent, so parallelize over even n.
    // --------------------------------------------------------

    #pragma omp parallel for schedule(dynamic, 4096) \
        reduction(+:tested,failures) \
        reduction(max:largest_required)
    for (int64_t nn = 6; nn <= static_cast<int64_t>(N); nn += 2) {

        const uint64_t n = static_cast<uint64_t>(nn);

        ++tested;

        bool found = false;

        // Scan p in increasing order.
        // Therefore, the first hit is the minimum possible
        // smaller prime in a Goldbach representation.
        for (uint32_t p : primes) {

            if (static_cast<uint64_t>(p) > n / 2)
                break;

            const uint64_t q = n - p;

            if (sieve.is_prime(q)) {
                found = true;

                if (p > largest_required)
                    largest_required = p;

                break;
            }
        }

        if (!found) {

            ++failures;

            #pragma omp critical
            {
                out << n << ',' << B << ",FAIL\n";
            }
        }
    }

    out.close();

    std::cout
        << "\n========== RESULT ==========\n"
        << "N                     = " << N << '\n'
        << "Bound B               = " << B << '\n'
        << "Even numbers tested   = " << tested << '\n'
        << "Failures              = " << failures << '\n'
        << "Largest required p    = " << largest_required << '\n'
        << "CSV                   = " << output_file << '\n';

    if (failures == 0) {
        std::cout
            << "\nNo even n <= N required both primes > B.\n"
            << "Thus computationally, B is a valid one-prime upper bound "
            << "for this range.\n";
    } else {
        std::cout
            << "\nThere are " << failures
            << " exceptions where every tested Goldbach pair has "
               "both primes > B.\n";
    }

    return 0;
}
