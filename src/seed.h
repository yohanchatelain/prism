#ifndef __PRISM_SEED_H__
#define __PRISM_SEED_H__

#include <cstdint>

// Random number generator seeding state, shared by every PRISM library loaded
// in the process. Defined in libprism-config (prism_config.cpp); see the note
// on the configuration state in utils.h.

// Number identifying the calling thread. Threads are numbered in the order in
// which they first ask, starting at 0. The number selects the thread's random
// stream, so it must be unique across libprism-static and libprism-dynamic.
auto get_thread_id() -> std::uint64_t;

// Returns the seed, after setting it if `set` is true. The seed is taken from
// the PRISM_SEED environment variable, or drawn at random, on first use.
auto seed_state(bool set = false, std::uint64_t new_seed = 0) -> std::uint64_t;

__attribute__((unused)) inline auto get_user_seed() -> std::uint64_t {
  return seed_state();
}

__attribute__((unused)) inline auto set_user_seed(std::uint64_t seed) -> void {
  seed_state(true, seed);
}

#endif // __PRISM_SEED_H__
