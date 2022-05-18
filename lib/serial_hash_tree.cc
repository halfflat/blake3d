#include <array>
#if __cplusplus>=202002
#include <bit>
#endif
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stack>
#include <vector>

#include <b3d/b3d.h>

namespace b3d {

using uint32_t = std::uint32_t;
using uint64_t = std::uint64_t;

using hash_stack = std::stack<chaining_t, std::vector<chaining_t>>;

unsigned countr_one(std::uint64_t i) {
#if __cplusplus>=202002
    return std::countr_one(i);
#elif defined(__GNUC__) || defined(__clang__)
    return i+1?__builtin_ctz(i+1):64;
#else
    unsigned c = 0;
    while (i&1) ++c, i>>=1;
    return c;
#endif
}

static inline block_t concatenate(const chaining_t& left, const chaining_t& right) {
    block_t b;
    std::memcpy(&b[0], left.data(), 32);
    std::memcpy(&b[8], right.data(), 32);
    return b;
}

chaining_t hash(chaining_t h, const std::byte* data, std::uint64_t sz) {
    uint64_t n_chunk = sz? 1+(sz-1)/1024: 1;
    if (n_chunk==1) return hash_chunk(h, 0, data, sz, true);

    std::vector<chaining_t> stack_vec;
    stack_vec.reserve(64);
    hash_stack stack(std::move(stack_vec));

    for (uint64_t chunk_i = 0; chunk_i+1<n_chunk; ++chunk_i) {
        chaining_t k = hash_chunk(h, chunk_i, data+chunk_i*1024, 1024, false);

        // Number of merges off the stack equals number of trailing one bits
        // in the binary representation of chunk_i.

        for (unsigned j = 0; j<countr_one(chunk_i); ++j) {
            // NB: In a parallel context, a schedulded merge on an empty
            // stack indicates we need to send the hash to the thread to the left.
            assert(!stack.empty());

            k = compress(h, concatenate(stack.top(), k), 0, 64, PARENT);
            stack.pop();
        }
        stack.push(k);
    }

    // Last chunk will merge with all remaining items on the stack, and last
    // merge will need the root flag.
    // (In parallel context, merge first with hashes sent from the right.)

    chaining_t k = hash_chunk(h, n_chunk-1, data+(n_chunk-1)*1024, sz-(n_chunk-1)*1024, false);

    while (stack.size()>1) {
        k = compress(h, concatenate(stack.top(), k), 0, 64, PARENT);
        stack.pop();
    }

    assert(stack.size()==1);
    return compress(h, concatenate(stack.top(), k), 0, 64, PARENT|ROOT);
}


} // namespace b3d
