#include <cstdint>

#include <gtest/gtest.h>
#include <b3d/b3d.h>

using namespace b3d;
using std::uint32_t;

TEST(compress, empty) {
    using namespace b3d;

    // Empty block for compression only arises in the case where the
    // input data is empty. This fixes the values for t, b, and d below.

    block_t m = {
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0
    };

    uint32_t t = 0; // chunk index
    uint32_t b = 0; // block size in bytes
    uint32_t d = CHUNK_START | CHUNK_END | ROOT;

    // With default key (verified against b3sum):

    chaining_t result0 = compress(default_key, m, t, b, d);
    chaining_t expected0 = {
        0xb94913af, 0xa6a1f9f5, 0xea4d40a0, 0x49c9dc36,
        0xc925cb9b, 0xb712c1ad, 0xca939acc, 0x62321fe4
    };

    EXPECT_EQ(expected0, result0);

    // With arbitrary key (verified against b3sum in keyed mode):

    chaining_t h = {
        0x2b895f87, 0xc88145f4, 0x4bc7c164, 0x7cada9fd,
        0x9eeb6dbb, 0x1e7f8536, 0x85ff300b, 0xfd7bb8c8
    };

    chaining_t result1 = compress(h, m, t, b, d);
    chaining_t expected1 = {
        0x5d8f24a7, 0xd1a05519, 0x4ea08933, 0xa9c53f19,
        0x5a809df8, 0xa06ae32d, 0x91b33050, 0xb0dc047c
    };


    EXPECT_EQ(expected0, result0);
}

#if 0

// Check results of compress against precomputed results with various inputs.

TEST(compress, various) {
    using namespace b3d;

    chaining_t h[3] = {
        {0x19da393e, 0xfdebb620, 0xdb5ebb63, 0x4f2212ad,
         0xc0c4378c, 0x83da807c, 0x0f027b97, 0xa5a88085},

        {0x5f16bc8f, 0xcb5d5346, 0x268be478, 0x823de91f,
         0xa9172b11, 0x92749faa, 0x04c0a9dd, 0x4e33dcaf}

        {0x04599985, 0x8ef0c377, 0x59f44253, 0x49fb405d,
         0x40fb3fe9, 0xd6a17bc9, 0x1c6b9063, 0xebb03b47}
    };

    // Block data; bytes 
    block_t block_t m[3] = {
        {0x72e454a6, 0xb1968a27, 0x9125b1b4, 0xfdaf58c0,
         0x3823f11e, 0xded07b32, 0x6cc5ba9b, 0xd9130b0a,
         0xe69e2c1b, 0xe26fd0cd, 0x66abd4f1, 0x036f3628,
         0x3e128223, 0xe7a0553b, 0x2e0f17a0, 0xf467333c},

        {0x93098cbe, 0x5b44424e, 0x6c4c1b19, 0x4d8b95cd,
         0xb797493f, 0x42723944, 0x2dcdcf2f, 0xad5f4b61,
         0xeae33790, 0x6b56c7ad, 0xdd35001d, 0xdf05ec50,
         0xda05434f, 0x6c91e4be, 0xdd623b9c, 0xb34a7992},

        {0x5630ca4e, 0x2573aed8, 0x8253856f, 0x32ec3020,
         0xfc15276e, 0xf6912e93, 0x5d187282, 0x44aa3068,
         0x38241b06, 0x871cbe3f, 0x16e736c2, 0x43d32d62,
         0xecc3c5a2, 0x03e92e70, 0x542a1851, 0xc38a8fcb}
    };

    // Chunk indices
    uint32_t t[3] = {
        0, 123, 0xfe1cca95
    };

    uint32_t b[3] = {
        0, 123, 0xfe1cca95
    };

    {
        block_t
}
#endif
