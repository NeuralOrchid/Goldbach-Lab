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
    is_prime = sieveOfEratosthenes(2310)
    primes = [i for i, prime in enumerate(is_prime) if prime if i%210 in (1, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199)]
    nums = [210*i + j for i in range(1, 11) for j in range(0, 210, 2)]
    sums = [i + j for i in primes for j in primes]
    for num in nums:
        if not num in sums:
            print("counterexample:", num)

if __name__ == "__main__":
    print(" => Running...")
    main()
    print(" => Done!")