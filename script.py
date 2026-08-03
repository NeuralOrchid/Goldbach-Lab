import numpy as np
from numpy.typing import NDArray
from typing import List, Optional, Literal

## Sieve of Eratosthenes
def sieveOfEratosthenes(
        n: int,
        turn_into_numbers: Optional[bool] = False
) -> List[int] | List[bool]:
    if n < 2:
        return []
    
    is_prime = [True] * (n + 1)
    is_prime[0] = is_prime[1] = False

    p = 2
    while p * p <= n:
        if is_prime[p]:
            for multiple in range(p * p, n + 1, p):
                is_prime[multiple] = False
        p += 1

    if turn_into_numbers:
        primes = [i for i, prime in enumerate(is_prime) if prime]
        return primes
    else:
        return is_prime

def survives(n: int, candidates: NDArray):
    for k in candidates:
        if k == n:
            continue
        for r in (1, 7, 11, 13):
            m = 30 * k - r
            if n % m == k:
                return False
    return True

def main() -> None:
    N: Literal[20] = 20
    candidates = np.arange(-N, N + 1, dtype=int)
    candidates = candidates[candidates != 0]

    primes = np.abs(np.array([n for n in candidates if survives(n, candidates)], dtype=int) * 30)

    is_prime = sieveOfEratosthenes(np.max(primes) + 17)
    for p in primes:
        if not any([is_prime[p + r] for r in (1, 7, 11, 13, -1, -7, -11, -13)]):
            print(f"there's a problem with {p}")

if __name__ == "__main__":
    print(" => Running...")
    main()
    print(" => Done!")