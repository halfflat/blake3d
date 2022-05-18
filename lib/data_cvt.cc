#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <b3d/b3d.h>

namespace b3d {

static inline void le_cp_word(std::byte* __restrict__ a, std::uint32_t x) {
#if __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    std::memcpy(a, &x, 4);
#elif __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    std::byte y[4];
    std::memcpy(y, &x, 4);
    a[0] = y[3];
    a[1] = y[2];
    a[2] = y[1];
    a[3] = y[0];
#else
    a[0] = std::byte(x);
    a[1] = std::byte(x>>8);
    a[2] = std::byte(x>>16);
    a[3] = std::byte(x>>24);
#endif
}

template <std::size_t N>
static std::array<std::byte, N*4> as_bytes_impl(const std::array<std::uint32_t, N>& a) {
    std::array<std::byte, N*4> v;
    for (unsigned i = 0; i<N; ++i) le_cp_word(&v[4*i], a[i]);
    return v;
}

std::array<std::byte, 32> as_bytes(const chaining_t& h) {
    return as_bytes_impl(h);
}

std::array<std::byte, 64> as_bytes(const block_t& m) {
    return as_bytes_impl(m);
}


} // namespace b3d
