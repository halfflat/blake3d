#ifndef B3D_H_
#define B3D_H_

#include <array>
#include <cstdint>

namespace b3d {

using chaining_t = std::array<std::uint32_t, 8>;
using block_t = std::array<std::uint32_t, 16>;

static constexpr chaining_t IV = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19
};

static constexpr chaining_t default_key = IV;

enum domain_flag: std::uint32_t {
    CHUNK_START = 1,
    CHUNK_END = 2,
    PARENT = 4,
    ROOT = 8,
    KEYED_HASH = 16,
    DERIVE_KEY_CONTEXT = 32,
    DERIVE_KEY_MATERIAL = 64
};

chaining_t compress(chaining_t h, block_t m, std::uint64_t t, std::uint32_t b, std::uint32_t d);
chaining_t process_chunk(chaining_t h, std::uint64_t chunk_i, const std::byte* chunk, unsigned sz, bool root_node);

std::array<std::byte, 32> as_bytes(const chaining_t& h);


} // namespace b3d

#endif // ndef B3D_H_
