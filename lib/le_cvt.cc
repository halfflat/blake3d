#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <b3d/b3d.h>

namespace b3d {

static inline void le_cp_word(std::byte* __restrict__ a, std::uint32_t x) {
#if __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
    memcpy(a, &x, 4);
#elif __BYTE_ORDER__==__ORDER_BIG_ENDIAN__
    std::byte y[4];
    memcpy(y, &x, 4);
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

std::array<std::byte, 32> as_bytes(const chaining_t& h) {
    std::array<std::byte, 32> v;
    for (unsigned i = 0; i<8; ++i) le_cp_word(&v[4*i], h[i]);
    return v;
}

}
