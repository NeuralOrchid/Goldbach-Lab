import math
from typing import List

def sieve_primes(limit: int) -> List[int]:
    """Return a list of all primes <= limit using a simple Sieve of Eratosthenes."""
    if limit < 2:
        return []
    is_prime = [True] * (limit + 1)
    is_prime[0] = is_prime[1] = False
    for p in range(2, int(limit ** 0.5) + 1):
        if is_prime[p]:
            for multiple in range(p * p, limit + 1, p):
                is_prime[multiple] = False
    return [i for i, prime in enumerate(is_prime) if prime][1:] # except for 2

def prime_factors(n: int, primes: List[int]) -> List[int]:
    """
    Return the prime factors of n (repeated according to multiplicity)
    using a precomputed list of primes up to sqrt(original_max_n).
    """
    factors = []
    temp = n
    for p in primes:
        if p * p > temp:
            break
        while temp % p == 0:
            factors.append(p)
            temp //= p
    if temp > 2: # except for 2
        factors.append(temp)  # remaining prime
    return factors

def factorize_numbers(numbers: List[int]) -> List[List[int]]:
    """
    Efficiently factorize a list of integers.
    Returns a list of factor lists (same order as input).
    """
    if not numbers:
        return []
    max_n = max(numbers)
    limit = int(math.isqrt(max_n)) + 1
    primes = sieve_primes(limit)
    return [prime_factors(n, primes) for n in numbers]

def compute(list_of_prime_factors: List[List[int]]) -> List[int]:
    result:List[int] = list()
    for prime_factors in list_of_prime_factors:
        a: List[int] = map(lambda p: 1 - (1/((p-1)**2)), prime_factors)
        b: List[int] = map(lambda p: 1 + (1/((p-1)**3)), prime_factors)
        result.append(math.prod(a) * math.prod(b))
    return result

# ----- Example usage -----
if __name__ == "__main__":
    nums = list(range(840, 2050))
    factorizations = factorize_numbers(nums)
    for n, g in zip(nums, compute(factorizations)):
        print(f"G({n}) = {g}")
    
        