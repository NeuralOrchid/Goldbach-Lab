# =============================================================================
# Created by ChatGPT (OpenAI)
# Date: 2026-07-29
# Purpose: Plot S(a, N) and h(a, N) for Goldbach-related experiments.
# =============================================================================

import numpy as np
import matplotlib.pyplot as plt

def sieve_primes(n: int) -> np.ndarray:
    """Return all primes < n using a simple sieve."""
    if n <= 2:
        return np.array([], dtype=int)
    sieve = np.ones(n, dtype=bool)
    sieve[:2] = False
    limit = int(np.sqrt(n)) + 1
    for i in range(2, limit):
        if sieve[i]:
            sieve[i*i:n:i] = False
    return np.flatnonzero(sieve)

def S(a: np.ndarray, N: int) -> np.ndarray:
    """S(a, N) = sum_{p < N} exp(i 2 pi p a)."""
    primes = sieve_primes(N)
    p = primes[:, None]  # shape: (num_primes, 1)
    return np.sum(np.exp(1j * 2 * np.pi * p * a[None, :]), axis=0)

def h(a: np.ndarray, N: int) -> np.ndarray:
    """h(N) = S(a, N)^2 * exp(-i 2 pi N a)."""
    Sa = S(a, N)
    return (Sa ** 2) * np.exp(-1j * 2 * np.pi * N * a)

# Choose a large even N
N = 2000

# Sample a in [0, 1]
a = np.linspace(0, 1, 24000)
h_vals = h(a, N)

# Plot magnitude
plt.figure(figsize=(10, 4))
plt.plot(a, np.abs(h_vals))
plt.xlabel("a")
plt.ylabel("|h(a, N)|")
plt.title(f"Magnitude of h(a, N) for N = {N}")
plt.tight_layout()
plt.show()

# Plot real part
plt.figure(figsize=(10, 4))
plt.plot(a, np.real(h_vals))
plt.xlabel("a")
plt.ylabel("Re(h(a, N))")
plt.title(f"Real part of h(a, N) for N = {N}")
plt.tight_layout()
plt.show()

# Plot imaginary part
plt.figure(figsize=(10, 4))
plt.plot(a, np.imag(h_vals))
plt.xlabel("a")
plt.ylabel("Im(h(a, N))")
plt.title(f"Imaginary part of h(a, N) for N = {N}")
plt.tight_layout()
plt.show()

def integral_of_h_exact(N):
    primes = sieve_primes(N)
    prime_set = set(primes.tolist())
    count = 0
    for p in primes:
        q = N - p
        if q in prime_set and q < N:
            count += 1
    return count  # ordered pairs

# example
N = 2000
print("Exact integral of h over [0,1]:", integral_of_h_exact(N))