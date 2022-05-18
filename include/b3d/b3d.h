#ifndef B3D_H_
#define B3D_H_

#include <array>
#include <charconv>
#include <cstdint>
#include <cstddef>
#include <system_error>

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

// Compress one block (64 bytes) to a chaining value given an input chaining value `h`,
// a chunk counter `t`, the length of the data in the block (up to 64 bytes), and the
// bitwise or of any domain flags `d`.

chaining_t compress(chaining_t h, block_t m, std::uint64_t t, std::uint32_t b, std::uint32_t d);

// Convert a single chunk of index `chunk_i` and size `sz` bytes (up to 1024 bytes) given an input
// chaining value `h`. Set `root_node` to true if the chunk comprises the whole data set and hence
// the root node of the hash tree.

chaining_t hash_chunk(chaining_t h, std::uint64_t chunk_i, const std::byte* data, unsigned sz, bool root_node);

// Hash data in memory of length `sz` bytes given an input chaining value `h`.

chaining_t hash(chaining_t h, const std::byte* data, std::uint64_t sz);

// Convert a chaining value or block to equivalent 32-byte array in little-endian order.

std::array<std::byte, 32> as_bytes(const chaining_t&);
std::array<std::byte, 64> as_bytes(const block_t&);

// Convert byte sequence into hexadecimal character sequence.

template <std::size_t N>
std::to_chars_result to_hex_chars(char* first, char* last, const std::array<std::byte, N>& a) {
    static constexpr char xdigit[] = "0123456789abcdef";

    if (last-first<2*N) return {last, std::errc::value_too_large};
    for (std::size_t i = 0; i<N; ++i) {
        auto h = std::to_integer<std::uint8_t>(a[i]>>4);
        auto l = std::to_integer<std::uint8_t>(a[i]&std::byte(0xf));
        first[2*i] = xdigit[h];
        first[2*i+1] = xdigit[l];
    }
    return {first+2*N, {}};
}

// Convert hexadecimal character sequence into (zero-padded) byte sequence.

template <std::size_t N>
std::from_chars_result from_hex_chars(const char* first, const char* last, std::array<std::byte, N>& a) {
    std::size_t i = 0;
    for (const char* p = first; last-p>1; p += 2, ++i) {
        std::uint8_t b = 0;

        if (std::from_chars(p, p+2, b, 16).ptr!=p+2) break;
        a[i] = std::byte(b);
    }
    if (!i) return {first, std::errc::invalid_argument};

    std::fill(a.begin()+i, a.end(), std::byte{});
    return {first+2*i, {}};
}

// TODO:
//
// For SIMD parallel processing of chunks:
// void hash_chunks(std::size_t n_chunk, chaining_t* out, chaining_t h, std::uint64_t chunk_i0, const std::byte* data, unsigned sz, bool root_node);
//
// For extended output mode support:
// chaining_t compress_xof(chaining_t h, block_t m, std::uint64_t t, std::uint32_t b, std::uint32_t d);

} // namespace b3d

#endif // ndef B3D_H_
