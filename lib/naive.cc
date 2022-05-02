#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

#include <b3d/b3d.h>

namespace b3d {

using uint32_t = std::uint32_t;
using uint64_t = std::uint64_t;

using chaining_t = std::array<uint32_t, 8>;
using block_t = std::array<uint32_t, 16>;

template <unsigned c> uint32_t rot32r(uint32_t x) {
    unsigned j = c&31;
    return (x>>j)|(x<<((-j)&31));
}

template <unsigned i> void g(const block_t& m, uint32_t& a, uint32_t& b, uint32_t& c, uint32_t& d) {
    a += b + m[2*i];
    d = rot32r<16>(d^a);
    c += d;
    b = rot32r<12>(b^c);
    a += b + m[2*i+1];
    d = rot32r<8>(d^a);
    c += d;
    b = rot32r<7>(b^c);
}

chaining_t compress(chaining_t h, block_t m, uint64_t t, uint32_t b, uint32_t d) {
    block_t v;
    memcpy(&v[0], h.data(), 32);
    memcpy(&v[8], IV.data(), 16);
    v[12] = static_cast<uint32_t>(t);
    v[13] = static_cast<uint32_t>(t>>32);
    v[14] = b;
    v[15] = d;

    auto round = [&] {
        g<0>(m, v[0], v[4], v[8], v[12]);
        g<1>(m, v[1], v[5], v[9], v[13]);
        g<2>(m, v[2], v[6], v[10], v[14]);
        g<3>(m, v[3], v[7], v[11], v[15]);

        g<4>(m, v[0], v[5], v[10], v[15]);
        g<5>(m, v[1], v[6], v[11], v[12]);
        g<6>(m, v[2], v[7], v[8], v[13]);
        g<7>(m, v[3], v[4], v[9], v[14]);
    };

    auto permute = [](block_t& m) {
        block_t n = {
            m[2], m[6], m[3], m[10], m[7], m[0], m[4], m[13],
            m[1], m[11], m[12], m[5], m[9], m[14], m[15], m[8]
        };
        m = n;
    };

    round();
    for (unsigned j = 0; j<6; ++j) {
        permute(m);
        round();
    }

    for (unsigned j = 0; j<8; ++j) {
        h[j] = v[j] + v[j+8];
    }
    return h;
}

static constexpr block_t zero_block = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

chaining_t process_chunk(chaining_t h, uint64_t chunk_i, const std::byte* chunk, unsigned sz, bool root_node) {
    assert(sz<=1024);
    uint32_t root = root_node?ROOT:0;

    if (sz<=64) {
        block_t m = zero_block;
        memcpy(&m[0], chunk, sz);
        return compress(h, m, chunk_i, sz, CHUNK_START|CHUNK_END|root);
    }
    else {
        unsigned n = 1 + (sz-1)/64;

        block_t m;
        memcpy(&m[0], chunk, 64);
        h = compress(h, m, chunk_i, 64, CHUNK_START|root);

        for (unsigned i = 1; i<n-1; ++i) {
            memcpy(&m[0], chunk+i*64, 64);
            h = compress(h, m, chunk_i, 64, root);
        }

        m = zero_block;
        memcpy(&m[0], chunk+(n-1)*64, sz-((n-1)*64));
        h = compress(h, m, chunk_i, 64, CHUNK_END|root);
    }

    return h;
}

}
