// Created by ChatGPT (OpenAI)
// Date: 2026-08-30

#include <bits/stdc++.h>
#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using u64 = uint64_t;

int main(int argc, char* argv[]) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " LIMIT [output.csv]\n";
        return 1;
    }

    const u64 L = stoull(argv[1]);
    const string filename =
        (argc >= 3) ? argv[2] : "results.csv";

    if (L < 2) {
        cerr << "LIMIT must be >= 2\n";
        return 1;
    }

    // We need primality up to N+i < 2N <= 2L.
    const u64 M = 2 * L;

    // --------------------------------------------------------
    // Odd-only Sieve of Eratosthenes
    // prime[x >> 1] represents odd x
    // --------------------------------------------------------
    vector<uint8_t> prime((M >> 1) + 1, 1);
    prime[0] = 0; // number 1

    for (u64 p = 3; p * p <= M; p += 2) {
        if (!prime[p >> 1])
            continue;

        for (u64 x = p * p; x <= M; x += 2 * p)
            prime[x >> 1] = 0;
    }

    auto is_prime = [&](u64 n) -> bool {
        if (n == 2) return true;
        if (n < 2 || !(n & 1)) return false;
        return prime[n >> 1];
    };

    // answer[N] = smallest i
    vector<u64> answer(L + 1);

    // --------------------------------------------------------
    // Every N is independent, so parallelize the search.
    // --------------------------------------------------------
    #pragma omp parallel for schedule(dynamic, 256)
    for (long long nll = 2; nll <= (long long)L; ++nll) {
        const u64 N = (u64)nll;

        // i = 0 works immediately when N itself is prime.
        if (is_prime(N)) {
            answer[N] = 0;
            continue;
        }

        // For composite N, i must have opposite parity to N.
        u64 i = (N & 1) ? 2 : 1;

        for (; i < N; i += 2) {
            if (is_prime(N - i) && is_prime(N + i)) {
                answer[N] = i;
                break;
            }
        }
    }

    // --------------------------------------------------------
    // CSV
    // --------------------------------------------------------
    ofstream out(filename);

    if (!out) {
        cerr << "Cannot open " << filename << '\n';
        return 1;
    }

    out << "N,i,N_minus_i,N_plus_i\n";

    for (u64 N = 2; N <= L; ++N) {
        const u64 i = answer[N];

        out << N << ','
            << i << ','
            << N - i << ','
            << N + i << '\n';
    }

    cerr << "Finished: " << filename << '\n';

#ifdef _OPENMP
    cerr << "Threads: " << omp_get_max_threads() << '\n';
#endif
}
