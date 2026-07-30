import cmath

PRIMES = (2, 3, 5, 7, 11, 13, 17, 19, 23, 29) # Prime numbers less than N

def compute(N: int) -> complex:
    total = 0+0j

    for k in range(N):
        s = 0
        for p in PRIMES:
            s += cmath.exp(2j * cmath.pi * k * p / N)
        total += s**2 * cmath.exp(2j * cmath.pi * k * -1)

    return total / N

print(compute(30)) # (6+0j); so the answer is 6