#pragma once

// The "xoshiro256** 1.0" generator.
// C++ port by Arthur O'Dwyer (2021).
// Based on the C version by David Blackman and Sebastiano Vigna (2018),
// https://prng.di.unimi.it/xoshiro256starstar.c

#include <cstdint>
using u64 = uint64_t;

inline static constexpr u64 splitmix64(u64 &x) {
  u64 z = (x += 0x9e3779b97f4a7c15uLL);
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9uLL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebuLL;
  return z ^ (z >> 31);
}
inline static constexpr u64 rotl(u64 x, int k) { return (x << k) | (x >> (64 - k)); }

struct xoshiro256ss {
  using result_type = u64;
  u64 s[4]{};

  inline static constexpr u64 min() { return 0; }
  inline static constexpr u64 max() { return u64(-1); }
  inline constexpr explicit xoshiro256ss() : xoshiro256ss(0) {}

  inline constexpr explicit xoshiro256ss(u64 seed) {
    s[0] = splitmix64(seed);
    s[1] = splitmix64(seed);
    s[2] = splitmix64(seed);
    s[3] = splitmix64(seed);
  }

  inline constexpr u64 operator()() {
    u64 result = rotl(s[1] * 5, 7) * 9;
    u64 t = s[1] << 17;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);
    return result;
  }
};

struct xoshiro256pp {
  using result_type = u64;
  u64 s[4]{};

  inline static constexpr u64 min() { return 0; }
  inline static constexpr u64 max() { return u64(-1); }
  inline constexpr explicit xoshiro256pp() : xoshiro256pp(0) {}

  inline constexpr explicit xoshiro256pp(u64 seed) {
    s[0] = splitmix64(seed);
    s[1] = splitmix64(seed);
    s[2] = splitmix64(seed);
    s[3] = splitmix64(seed);
  }

  inline constexpr u64 operator()() {
    u64 result = rotl(s[0] + s[3], 23) + s[0];
    u64 t = s[1] << 17;
    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];
    s[2] ^= t;
    s[3] = rotl(s[3], 45);
    return result;
  }
};