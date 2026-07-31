# Dependencies
from typing import List, Optional, Set, Tuple
from math import sqrt, ceil

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


## Prime factorization
def primeFactors(n: int) -> Set[int]:
    factors: Set[int] = set()
    primes: List[int] = sieveOfEratosthenes(n, True)
    for p in primes:
        if p * p > n:
            break
        while n % p == 0:
            n //= p
            factors.add(p)
    if n > 1:
        factors.add(n)
    return factors


# => Main computation
def compute(n: int) -> Tuple[int]:
    isPrime: List[bool] = sieveOfEratosthenes(2*n)
    if isPrime[n]:
        return [(n, n)]

    # n is composite
    for p in primeFactors(2*n):
        isPrime[p] = False

    answer: List[Tuple[int]] = list()
    for p in [i for i, prime in enumerate(isPrime[n:], n) if prime]:
        if isPrime[p]:
            answer.append((p, 2*n - p))

    return answer



if __name__ == "__main__":
    print(compute(40))