import numpy as np
import matplotlib.pyplot as plt

# def sieve_primes(n: int) -> np.ndarray:
#     """Return all primes < n using a simple sieve."""
#     if n <= 2:
#         return np.array([], dtype=int)
#     sieve = np.ones(n, dtype=bool)
#     sieve[:2] = False
#     limit = int(np.sqrt(n)) + 1
#     for i in range(2, limit):
#         if sieve[i]:
#             sieve[i*i:n:i] = False
#     return np.flatnonzero(sieve)

def kronecker_dft_np(x: int, N: int) -> complex:
    k = np.arange(N)
    return np.sum(np.exp(2j * np.pi * k * x / N)) / N

if __name__ == "__main__":
    print(kronecker_dft_np(0, 5))  # (1+0j)
    print(kronecker_dft_np(3, 5))  # (0+0j)