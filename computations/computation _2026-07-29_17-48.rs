// Created by ChatGPT (OpenAI)
// Date: 2026-07-29
//
// ---------------------------

use std::io::{self, Write};

/// ---------- Basic modular arithmetic over u128 ----------

#[inline]
fn add_mod(a: u128, b: u128, m: u128) -> u128 {
    let (sum, overflow) = a.overflowing_add(b);
    if overflow || sum >= m {
        sum.wrapping_sub(m)
    } else {
        sum
    }
}

#[inline]
fn mul_mod(mut a: u128, mut b: u128, m: u128) -> u128 {
    // Russian-peasant multiplication:
    // safe for u128 because we never multiply directly.
    let mut res = 0u128;
    a %= m;

    while b > 0 {
        if b & 1 == 1 {
            res = add_mod(res, a, m);
        }
        b >>= 1;
        if b > 0 {
            a = add_mod(a, a, m);
        }
    }

    res
}

#[inline]
fn pow_mod(mut a: u128, mut e: u128, m: u128) -> u128 {
    let mut res = 1u128;
    a %= m;

    while e > 0 {
        if e & 1 == 1 {
            res = mul_mod(res, a, m);
        }
        a = mul_mod(a, a, m);
        e >>= 1;
    }

    res
}

#[inline]
fn gcd(mut a: u128, mut b: u128) -> u128 {
    while b != 0 {
        let r = a % b;
        a = b;
        b = r;
    }
    a
}

/// ---------- Primality test ----------
///
/// Miller–Rabin with a fixed base set.
/// This is very strong in practice for the size range around 10^23.
///
/// For production-grade full determinism on all u128 values,
/// you would use a more elaborate witness set or a bigint crate.
/// For your target range, this is a good practical choice.
fn is_probably_prime(n: u128) -> bool {
    if n < 2 {
        return false;
    }

    // Small primes first.
    const SMALL_PRIMES: [u128; 12] = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37];
    for &p in &SMALL_PRIMES {
        if n == p {
            return true;
        }
        if n % p == 0 {
            return false;
        }
    }

    // Write n - 1 = d * 2^s with d odd.
    let mut d = n - 1;
    let mut s = 0u32;
    while d % 2 == 0 {
        d /= 2;
        s += 1;
    }

    // Good practical witness set for this range.
    let bases: [u128; 12] = [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37];

    'outer: for &a in &bases {
        if a >= n {
            continue;
        }

        let mut x = pow_mod(a, d, n);
        if x == 1 || x == n - 1 {
            continue;
        }

        for _ in 1..s {
            x = mul_mod(x, x, n);
            if x == n - 1 {
                continue 'outer;
            }
        }

        return false;
    }

    true
}

/// ---------- Pollard Rho factorization ----------
///
/// Fast probabilistic splitter for composite numbers.
///
/// This is much better than trial division for numbers around 10^23.
#[derive(Clone)]
struct XorShift64 {
    state: u64,
}

impl XorShift64 {
    fn new(seed: u64) -> Self {
        Self { state: seed.max(1) }
    }

    fn next_u64(&mut self) -> u64 {
        let mut x = self.state;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        self.state = x;
        x
    }
}

fn pollard_rho(n: u128) -> u128 {
    if n % 2 == 0 {
        return 2;
    }

    let mut rng = XorShift64::new((n as u64) ^ 0x9E3779B97F4A7C15);

    loop {
        let c = (rng.next_u64() as u128 % (n - 1)) + 1;
        let mut x = (rng.next_u64() as u128 % (n - 2)) + 2;
        let mut y = x;
        let mut d = 1u128;

        // Floyd cycle detection
        while d == 1 {
            x = add_mod(mul_mod(x, x, n), c, n);
            y = add_mod(mul_mod(y, y, n), c, n);
            y = add_mod(mul_mod(y, y, n), c, n);

            let diff = x.abs_diff(y);
            d = gcd(diff, n);
        }

        if d != n {
            return d;
        }
    }
}

/// ---------- Factorization ----------
///
/// Returns prime factors with multiplicity.
/// Example: 840 -> [2, 2, 2, 3, 5, 7]
fn factor_rec(n: u128, out: &mut Vec<u128>) {
    if n == 1 {
        return;
    }

    if is_probably_prime(n) {
        out.push(n);
        return;
    }

    let d = pollard_rho(n);
    factor_rec(d, out);
    factor_rec(n / d, out);
}

fn factorize(n: u128) -> Vec<u128> {
    let mut factors = Vec::new();
    factor_rec(n, &mut factors);
    factors.sort_unstable();
    factors
}

/// ---------- Your arithmetic function ----------
///
/// G(n) = Π over prime factors p of
///        (1 - 1/(p-1)^2) * (1 + 1/(p-1)^3)
///
/// Prime 2 is skipped because the formula would divide by zero.
fn compute_g(factors: &[u128]) -> f64 {
    let mut result = 1.0f64;

    for &p in factors {
        if p == 2 {
            continue;
        }

        let x = (p as f64) - 1.0;
        let term = (1.0 - 1.0 / (x * x)) * (1.0 + 1.0 / (x * x * x));
        result *= term;
    }

    result
}

fn main() {
    // Test a window around 2^76.
    // Change the radius to test a wider band.
    let center: u128 = 75_557_863_725_914_323_419_136;
    let radius: u128 = 10;

    let nums: Vec<u128> = (center - radius..=center + radius).collect();

    let mut stdout = io::BufWriter::new(io::stdout());

    for n in nums {
        let factors = factorize(n);
        let g = compute_g(&factors);
        writeln!(stdout, "G({}) = {:.15}", n, g).unwrap();
    }
}

/* 
G(75557863725914323419126) = 0.843749959960026
G(75557863725914323419127) = 0.945392868469307
G(75557863725914323419128) = 0.999994799332045
G(75557863725914323419129) = 0.711706082912944
G(75557863725914323419130) = 0.952145948502848
G(75557863725914323419131) = 0.999997535043637
G(75557863725914323419132) = 0.843732689609084
G(75557863725914323419133) = 0.993630238315169
G(75557863725914323419134) = 0.975628056875642
G(75557863725914323419135) = 0.803356002586783
G(75557863725914323419136) = 1.000000000000000
G(75557863725914323419137) = 0.996336263647376
G(75557863725914323419138) = 0.705481972268994
G(75557863725914323419139) = 0.997084518789480
G(75557863725914323419140) = 0.952102549928912
G(75557863725914323419141) = 0.824110243055535
G(75557863725914323419142) = 0.998765834308272
G(75557863725914323419143) = 0.999796462754881
G(75557863725914323419144) = 0.843749725275351
G(75557863725914323419145) = 0.952148437490018
G(75557863725914323419146) = 0.993630228241629
*/