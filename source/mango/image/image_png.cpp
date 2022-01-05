/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2021 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <cstring>
#include <mango/core/core.hpp>
#include <mango/image/image.hpp>
#include <mango/math/math.hpp>

#include "../../external/zlib/zlib.h"

// https://www.w3.org/TR/2003/REC-PNG-20031110/
// https://wiki.mozilla.org/APNG_Specification

// TODO: discard modes (requires us to keep a copy of main image)
// TODO: check that animations starting with "IDAT" and "fdAT" work correctly
// TODO: SIMD blending (not critical)
// TODO: SIMD color conversions

namespace fpng
{
    using namespace mango;

    static const uint16_t g_defl_len_sym[256] = {
      257,258,259,260,261,262,263,264,265,265,266,266,267,267,268,268,269,269,269,269,270,270,270,270,271,271,271,271,272,272,272,272,
      273,273,273,273,273,273,273,273,274,274,274,274,274,274,274,274,275,275,275,275,275,275,275,275,276,276,276,276,276,276,276,276,
      277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,277,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,278,
      279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,279,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,280,
      281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,281,
      282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,282,
      283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,283,
      284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,284,285 };

    static const uint8_t g_defl_len_extra[256] = {
      0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
      4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
      5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
      5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,0 };

    static const uint8_t g_defl_small_dist_sym[512] = {
      0,1,2,3,4,4,5,5,6,6,6,6,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,11,11,11,11,11,11,
      11,11,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,13,
      13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,14,14,14,14,
      14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,
      14,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,
      15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,16,16,16,16,16,
      16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
      16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
      16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
      17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
      17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,
      17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17,17 };
        
    static const uint32_t g_bitmasks[17] = { 0x0000, 0x0001, 0x0003, 0x0007, 0x000F, 0x001F, 0x003F, 0x007F, 0x00FF, 0x01FF, 0x03FF, 0x07FF, 0x0FFF, 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF };

    static const uint8_t g_dyn_huff_3[] = { 120, 1, 229, 194, 3, 176, 37, 75, 148, 5, 208, 189, 79, 102, 86, 213, 197, 99, 187, 231, 143, 109, 219, 182, 109, 219, 182, 109, 219, 182, 109, 219,
        198, 31, 207, 159, 118, 63, 94, 84, 85, 102, 158, 61, 21, 241, 34, 58, 38, 198, 102, 196 };
    const uint32_t DYN_HUFF_3_BITBUF = 0x2, DYN_HUFF_3_BITBUF_SIZE = 3;
        
    static const struct { uint8_t m_code_size; uint16_t m_code; } g_dyn_huff_3_codes[288] =
    {
        {3,0x0},{3,0x4},{4,0x6},{5,0x1},{5,0x11},{5,0x9},{6,0xD},{6,0x2D},{6,0x1D},{7,0x33},{7,0x73},{7,0xB},{7,0x4B},{8,0x3B},{8,0xBB},{8,0x7B},
        {8,0xFB},{8,0x7},{8,0x87},{9,0x97},{9,0x197},{9,0x57},{9,0x157},{9,0xD7},{9,0x1D7},{9,0x37},{9,0x137},{12,0x24F},{10,0x18F},{12,0xA4F},{12,0x64F},{12,0xE4F},
        {12,0x14F},{12,0x94F},{12,0x54F},{12,0xD4F},{12,0x34F},{12,0xB4F},{12,0x74F},{12,0xF4F},{12,0xCF},{12,0x8CF},{12,0x4CF},{12,0xCCF},{12,0x2CF},{12,0xACF},{12,0x6CF},{12,0xECF},
        {12,0x1CF},{12,0x9CF},{12,0x5CF},{12,0xDCF},{12,0x3CF},{12,0xBCF},{12,0x7CF},{12,0xFCF},{12,0x2F},{12,0x82F},{12,0x42F},{12,0xC2F},{12,0x22F},{12,0xA2F},{12,0x62F},{12,0xE2F},
        {12,0x12F},{12,0x92F},{12,0x52F},{12,0xD2F},{12,0x32F},{12,0xB2F},{12,0x72F},{12,0xF2F},{12,0xAF},{12,0x8AF},{12,0x4AF},{12,0xCAF},{12,0x2AF},{12,0xAAF},{12,0x6AF},{12,0xEAF},
        {12,0x1AF},{12,0x9AF},{12,0x5AF},{12,0xDAF},{12,0x3AF},{12,0xBAF},{12,0x7AF},{12,0xFAF},{12,0x6F},{12,0x86F},{12,0x46F},{12,0xC6F},{12,0x26F},{12,0xA6F},{12,0x66F},{12,0xE6F},
        {12,0x16F},{12,0x96F},{12,0x56F},{12,0xD6F},{12,0x36F},{12,0xB6F},{12,0x76F},{12,0xF6F},{12,0xEF},{12,0x8EF},{12,0x4EF},{12,0xCEF},{12,0x2EF},{12,0xAEF},{12,0x6EF},{12,0xEEF},
        {12,0x1EF},{12,0x9EF},{12,0x5EF},{12,0xDEF},{12,0x3EF},{12,0xBEF},{12,0x7EF},{12,0xFEF},{12,0x1F},{12,0x81F},{12,0x41F},{12,0xC1F},{12,0x21F},{12,0xA1F},{12,0x61F},{12,0xE1F},
        {12,0x11F},{12,0x91F},{12,0x51F},{12,0xD1F},{12,0x31F},{12,0xB1F},{12,0x71F},{12,0xF1F},{12,0x9F},{12,0x89F},{12,0x49F},{12,0xC9F},{12,0x29F},{12,0xA9F},{12,0x69F},{12,0xE9F},
        {12,0x19F},{12,0x99F},{12,0x59F},{12,0xD9F},{12,0x39F},{12,0xB9F},{12,0x79F},{12,0xF9F},{12,0x5F},{12,0x85F},{12,0x45F},{12,0xC5F},{12,0x25F},{12,0xA5F},{12,0x65F},{12,0xE5F},
        {12,0x15F},{12,0x95F},{12,0x55F},{12,0xD5F},{12,0x35F},{12,0xB5F},{12,0x75F},{12,0xF5F},{12,0xDF},{12,0x8DF},{12,0x4DF},{12,0xCDF},{12,0x2DF},{12,0xADF},{12,0x6DF},{12,0xEDF},
        {12,0x1DF},{12,0x9DF},{12,0x5DF},{12,0xDDF},{12,0x3DF},{12,0xBDF},{12,0x7DF},{12,0xFDF},{12,0x3F},{12,0x83F},{12,0x43F},{12,0xC3F},{12,0x23F},{12,0xA3F},{12,0x63F},{12,0xE3F},
        {12,0x13F},{12,0x93F},{12,0x53F},{12,0xD3F},{12,0x33F},{12,0xB3F},{12,0x73F},{12,0xF3F},{12,0xBF},{12,0x8BF},{12,0x4BF},{12,0xCBF},{12,0x2BF},{12,0xABF},{12,0x6BF},{12,0xEBF},
        {12,0x1BF},{12,0x9BF},{12,0x5BF},{12,0xDBF},{12,0x3BF},{12,0xBBF},{12,0x7BF},{12,0xFBF},{12,0x7F},{12,0x87F},{12,0x47F},{10,0x38F},{12,0xC7F},{12,0x27F},{12,0xA7F},{12,0x67F},
        {12,0xE7F},{12,0x17F},{12,0x97F},{12,0x57F},{10,0x4F},{12,0xD7F},{9,0xB7},{9,0x1B7},{9,0x77},{9,0x177},{9,0xF7},{9,0x1F7},{9,0xF},{9,0x10F},{8,0x47},{8,0xC7},
        {8,0x27},{8,0xA7},{8,0x67},{8,0xE7},{7,0x2B},{7,0x6B},{7,0x1B},{7,0x5B},{6,0x3D},{6,0x3},{6,0x23},{5,0x19},{5,0x5},{5,0x15},{4,0xE},{3,0x2},
        {12,0x37F},{6,0x13},{0,0x0},{0,0x0},{8,0x17},{0,0x0},{0,0x0},{9,0x8F},{0,0x0},{12,0xB7F},{0,0x0},{12,0x77F},{12,0xF7F},{12,0xFF},{12,0x8FF},{12,0x4FF},
        {12,0xCFF},{12,0x2FF},{12,0xAFF},{12,0x6FF},{12,0xEFF},{12,0x1FF},{12,0x9FF},{12,0x5FF},{12,0xDFF},{12,0x3FF},{12,0xBFF},{12,0x7FF},{12,0xFFF},{0,0x0},{0,0x0},{0,0x0}
    };

    static const uint8_t g_dyn_huff_4[] = { 120,1,229,195,83,144,37,219,182,0,208,49,87,230,70,177,171,121,204,171,103,219,182,109,219,182,109,219,182,109,219,214,
        197,177,154,213,197,141,204,53,95,228,71,69,116,156,56,207,126,251,99 };
    const uint32_t DYN_HUFF_4_BITBUF = 0x0, DYN_HUFF_4_BITBUF_SIZE = 2;

    static const struct { uint8_t m_code_size; uint16_t m_code; } g_dyn_huff_4_codes[288] =
    {
        {1,0x0},{4,0x1},{5,0x5},{6,0xD},{6,0x2D},{7,0x23},{7,0x63},{7,0x13},{7,0x53},{8,0x6B},{8,0xEB},{8,0x1B},{8,0x9B},{8,0x5B},{8,0xDB},{9,0xA7},
        {8,0x3B},{9,0x1A7},{9,0x67},{9,0x167},{9,0xE7},{9,0x1E7},{9,0x17},{10,0x137},{10,0x337},{10,0xB7},{10,0x2B7},{10,0x1B7},{10,0x3B7},{10,0x77},{10,0x277},{10,0x177},
        {10,0x377},{10,0xF7},{10,0x2F7},{11,0x34F},{11,0x74F},{11,0xCF},{11,0x4CF},{11,0x2CF},{12,0x7CF},{12,0xFCF},{12,0x2F},{12,0x82F},{12,0x42F},{12,0xC2F},{12,0x22F},{12,0xA2F},
        {12,0x62F},{12,0xE2F},{12,0x12F},{12,0x92F},{12,0x52F},{12,0xD2F},{12,0x32F},{12,0xB2F},{12,0x72F},{12,0xF2F},{12,0xAF},{12,0x8AF},{12,0x4AF},{12,0xCAF},{12,0x2AF},{12,0xAAF},
        {12,0x6AF},{12,0xEAF},{12,0x1AF},{12,0x9AF},{12,0x5AF},{12,0xDAF},{12,0x3AF},{12,0xBAF},{12,0x7AF},{12,0xFAF},{12,0x6F},{12,0x86F},{12,0x46F},{12,0xC6F},{12,0x26F},{12,0xA6F},
        {12,0x66F},{12,0xE6F},{12,0x16F},{12,0x96F},{12,0x56F},{12,0xD6F},{12,0x36F},{12,0xB6F},{12,0x76F},{12,0xF6F},{12,0xEF},{12,0x8EF},{12,0x4EF},{12,0xCEF},{12,0x2EF},{12,0xAEF},
        {12,0x6EF},{12,0xEEF},{12,0x1EF},{12,0x9EF},{12,0x5EF},{12,0xDEF},{12,0x3EF},{12,0xBEF},{12,0x7EF},{12,0xFEF},{12,0x1F},{12,0x81F},{12,0x41F},{12,0xC1F},{12,0x21F},{12,0xA1F},
        {12,0x61F},{12,0xE1F},{12,0x11F},{12,0x91F},{12,0x51F},{12,0xD1F},{12,0x31F},{12,0xB1F},{12,0x71F},{12,0xF1F},{12,0x9F},{12,0x89F},{12,0x49F},{12,0xC9F},{12,0x29F},{12,0xA9F},
        {12,0x69F},{12,0xE9F},{12,0x19F},{12,0x99F},{12,0x59F},{12,0xD9F},{12,0x39F},{12,0xB9F},{12,0x79F},{12,0xF9F},{12,0x5F},{12,0x85F},{12,0x45F},{12,0xC5F},{12,0x25F},{12,0xA5F},
        {12,0x65F},{12,0xE5F},{12,0x15F},{12,0x95F},{12,0x55F},{12,0xD5F},{12,0x35F},{12,0xB5F},{12,0x75F},{12,0xF5F},{12,0xDF},{12,0x8DF},{12,0x4DF},{12,0xCDF},{12,0x2DF},{12,0xADF},
        {12,0x6DF},{12,0xEDF},{12,0x1DF},{12,0x9DF},{12,0x5DF},{12,0xDDF},{12,0x3DF},{12,0xBDF},{12,0x7DF},{12,0xFDF},{12,0x3F},{12,0x83F},{12,0x43F},{12,0xC3F},{12,0x23F},{12,0xA3F},
        {12,0x63F},{12,0xE3F},{12,0x13F},{12,0x93F},{12,0x53F},{12,0xD3F},{12,0x33F},{12,0xB3F},{12,0x73F},{12,0xF3F},{12,0xBF},{12,0x8BF},{12,0x4BF},{12,0xCBF},{12,0x2BF},{12,0xABF},
        {12,0x6BF},{12,0xEBF},{12,0x1BF},{12,0x9BF},{12,0x5BF},{12,0xDBF},{12,0x3BF},{12,0xBBF},{12,0x7BF},{12,0xFBF},{12,0x7F},{12,0x87F},{12,0x47F},{12,0xC7F},{12,0x27F},{12,0xA7F},
        {12,0x67F},{12,0xE7F},{12,0x17F},{12,0x97F},{12,0x57F},{12,0xD7F},{12,0x37F},{12,0xB7F},{12,0x77F},{12,0xF7F},{12,0xFF},{11,0x6CF},{11,0x1CF},{11,0x5CF},{11,0x3CF},{10,0x1F7},
        {10,0x3F7},{10,0xF},{10,0x20F},{10,0x10F},{10,0x30F},{10,0x8F},{10,0x28F},{10,0x18F},{10,0x38F},{10,0x4F},{9,0x117},{9,0x97},{9,0x197},{9,0x57},{9,0x157},{9,0xD7},
        {8,0xBB},{9,0x1D7},{8,0x7B},{8,0xFB},{8,0x7},{8,0x87},{8,0x47},{8,0xC7},{7,0x33},{7,0x73},{7,0xB},{7,0x4B},{6,0x1D},{6,0x3D},{5,0x15},{4,0x9},
        {12,0x8FF},{0,0x0},{6,0x3},{0,0x0},{0,0x0},{0,0x0},{8,0x27},{0,0x0},{0,0x0},{9,0x37},{0,0x0},{10,0x24F},{0,0x0},{10,0x14F},{12,0x4FF},{12,0xCFF},
        {12,0x2FF},{12,0xAFF},{12,0x6FF},{12,0xEFF},{12,0x1FF},{12,0x9FF},{12,0x5FF},{12,0xDFF},{12,0x3FF},{12,0xBFF},{12,0x7FF},{12,0xFFF},{7,0x2B},{0,0x0},{0,0x0},{0,0x0},
    };

    constexpr int FPNG_FALSE = 0;
    //constexpr uint8_t FPNG_FDEC_VERSION = 0;
    //constexpr uint32_t FPNG_MAX_SUPPORTED_DIM = 1 << 24;
    //constexpr uint32_t FPNG_ADLER32_INIT = 1;

    // Customized the very common case of reading a 24bpp pixel from memory
    static inline uint32_t READ_RGB_PIXEL(const void* p)
    {
        return uload32le(p) & 0xffffff;
    }

#define PUT_BITS(bb, ll) do { uint32_t b = bb, l = ll; assert((l) >= 0 && (l) <= 16); assert((b) < (1ULL << (l))); bit_buf |= (((uint64_t)(b)) << bit_buf_size); bit_buf_size += (l); assert(bit_buf_size <= 64); } while(0)
#define PUT_BITS_CZ(bb, ll) do { uint32_t b = bb, l = ll; assert((l) >= 1 && (l) <= 16); assert((b) < (1ULL << (l))); bit_buf |= (((uint64_t)(b)) << bit_buf_size); bit_buf_size += (l); assert(bit_buf_size <= 64); } while(0)

#define PUT_BITS_FLUSH do { \
    if ((dst_ofs + 8) > dst_buf_size) \
        return 0; \
    ustore64le(pDst + dst_ofs, bit_buf); \
    uint32_t bits_to_shift = bit_buf_size & ~7; \
    dst_ofs += (bits_to_shift >> 3); \
    assert(bits_to_shift < 64); \
    bit_buf = bit_buf >> bits_to_shift; \
    bit_buf_size -= bits_to_shift; \
} while(0)

#define PUT_BITS_FORCE_FLUSH do { \
    while (bit_buf_size > 0) \
    { \
        if ((dst_ofs + 1) > dst_buf_size) \
            return 0; \
        *(uint8_t*)(pDst + dst_ofs) = (uint8_t)bit_buf; \
        dst_ofs++; \
        bit_buf >>= 8; \
        bit_buf_size -= 8; \
    } \
} while(0)

    enum
    {
        DEFL_MAX_HUFF_TABLES = 3,
        DEFL_MAX_HUFF_SYMBOLS = 288,
        DEFL_MAX_HUFF_SYMBOLS_0 = 288,
        DEFL_MAX_HUFF_SYMBOLS_1 = 32,
        DEFL_MAX_HUFF_SYMBOLS_2 = 19,
        DEFL_LZ_DICT_SIZE = 32768,
        DEFL_LZ_DICT_SIZE_MASK = DEFL_LZ_DICT_SIZE - 1,
        DEFL_MIN_MATCH_LEN = 3,
        DEFL_MAX_MATCH_LEN = 258
    };

    struct defl_huff
    {
        uint16_t m_huff_count[DEFL_MAX_HUFF_TABLES][DEFL_MAX_HUFF_SYMBOLS];
        uint16_t m_huff_codes[DEFL_MAX_HUFF_TABLES][DEFL_MAX_HUFF_SYMBOLS];
        uint8_t m_huff_code_sizes[DEFL_MAX_HUFF_TABLES][DEFL_MAX_HUFF_SYMBOLS];
    };

    struct defl_sym_freq
    {
        uint16_t m_key;
        uint16_t m_sym_index;
    };

#define DEFL_CLEAR_OBJ(obj) memset(&(obj), 0, sizeof(obj))

    static defl_sym_freq* defl_radix_sort_syms(uint32_t num_syms, defl_sym_freq* pSyms0, defl_sym_freq* pSyms1)
    {
        uint32_t total_passes = 2, pass_shift, pass, i, hist[256 * 2]; defl_sym_freq* pCur_syms = pSyms0, * pNew_syms = pSyms1; DEFL_CLEAR_OBJ(hist);
        for (i = 0; i < num_syms; i++) { uint32_t freq = pSyms0[i].m_key; hist[freq & 0xFF]++; hist[256 + ((freq >> 8) & 0xFF)]++; }
        while ((total_passes > 1) && (num_syms == hist[(total_passes - 1) * 256])) total_passes--;
        for (pass_shift = 0, pass = 0; pass < total_passes; pass++, pass_shift += 8)
        {
            const uint32_t* pHist = &hist[pass << 8];
            uint32_t offsets[256], cur_ofs = 0;
            for (i = 0; i < 256; i++) { offsets[i] = cur_ofs; cur_ofs += pHist[i]; }
            for (i = 0; i < num_syms; i++) pNew_syms[offsets[(pCur_syms[i].m_key >> pass_shift) & 0xFF]++] = pCur_syms[i];
            { defl_sym_freq* t = pCur_syms; pCur_syms = pNew_syms; pNew_syms = t; }
        }
        return pCur_syms;
    }

    // defl_calculate_minimum_redundancy() originally written by: Alistair Moffat, alistair@cs.mu.oz.au, Jyrki Katajainen, jyrki@diku.dk, November 1996.
    static void defl_calculate_minimum_redundancy(defl_sym_freq* A, int n)
    {
        int root, leaf, next, avbl, used, dpth;
        if (n == 0) return; else if (n == 1) { A[0].m_key = 1; return; }
        A[0].m_key += A[1].m_key; root = 0; leaf = 2;
        for (next = 1; next < n - 1; next++)
        {
            if (leaf >= n || A[root].m_key < A[leaf].m_key) { A[next].m_key = A[root].m_key; A[root++].m_key = (uint16_t)next; }
            else A[next].m_key = A[leaf++].m_key;
            if (leaf >= n || (root < next && A[root].m_key < A[leaf].m_key)) { A[next].m_key = (uint16_t)(A[next].m_key + A[root].m_key); A[root++].m_key = (uint16_t)next; }
            else A[next].m_key = (uint16_t)(A[next].m_key + A[leaf++].m_key);
        }
        A[n - 2].m_key = 0; for (next = n - 3; next >= 0; next--) A[next].m_key = A[A[next].m_key].m_key + 1;
        avbl = 1; used = dpth = 0; root = n - 2; next = n - 1;
        while (avbl > 0)
        {
            while (root >= 0 && (int)A[root].m_key == dpth) { used++; root--; }
            while (avbl > used) { A[next--].m_key = (uint16_t)(dpth); avbl--; }
            avbl = 2 * used; dpth++; used = 0;
        }
    }

    // Limits canonical Huffman code table's max code size.
    enum { DEFL_MAX_SUPPORTED_HUFF_CODESIZE = 32 };
    static void defl_huffman_enforce_max_code_size(int* pNum_codes, int code_list_len, int max_code_size)
    {
        int i; uint32_t total = 0; if (code_list_len <= 1) return;
        for (i = max_code_size + 1; i <= DEFL_MAX_SUPPORTED_HUFF_CODESIZE; i++) pNum_codes[max_code_size] += pNum_codes[i];
        for (i = max_code_size; i > 0; i--) total += (((uint32_t)pNum_codes[i]) << (max_code_size - i));
        while (total != (1UL << max_code_size))
        {
            pNum_codes[max_code_size]--;
            for (i = max_code_size - 1; i > 0; i--) if (pNum_codes[i]) { pNum_codes[i]--; pNum_codes[i + 1] += 2; break; }
            total--;
        }
    }

    static void defl_optimize_huffman_table(defl_huff* d, int table_num, int table_len, int code_size_limit, int static_table)
    {
        int i, j, l, num_codes[1 + DEFL_MAX_SUPPORTED_HUFF_CODESIZE]; uint32_t next_code[DEFL_MAX_SUPPORTED_HUFF_CODESIZE + 1]; DEFL_CLEAR_OBJ(num_codes);
        if (static_table)
        {
            for (i = 0; i < table_len; i++) num_codes[d->m_huff_code_sizes[table_num][i]]++;
        }
        else
        {
            defl_sym_freq syms0[DEFL_MAX_HUFF_SYMBOLS], syms1[DEFL_MAX_HUFF_SYMBOLS], * pSyms;
            int num_used_syms = 0;
            const uint16_t* pSym_count = &d->m_huff_count[table_num][0];
            for (i = 0; i < table_len; i++) if (pSym_count[i]) { syms0[num_used_syms].m_key = (uint16_t)pSym_count[i]; syms0[num_used_syms++].m_sym_index = (uint16_t)i; }

            pSyms = defl_radix_sort_syms(num_used_syms, syms0, syms1); defl_calculate_minimum_redundancy(pSyms, num_used_syms);

            for (i = 0; i < num_used_syms; i++) num_codes[pSyms[i].m_key]++;

            defl_huffman_enforce_max_code_size(num_codes, num_used_syms, code_size_limit);

            DEFL_CLEAR_OBJ(d->m_huff_code_sizes[table_num]); DEFL_CLEAR_OBJ(d->m_huff_codes[table_num]);
            for (i = 1, j = num_used_syms; i <= code_size_limit; i++)
                for (l = num_codes[i]; l > 0; l--) d->m_huff_code_sizes[table_num][pSyms[--j].m_sym_index] = (uint8_t)(i);
        }

        next_code[1] = 0; for (j = 0, i = 2; i <= code_size_limit; i++) next_code[i] = j = ((j + num_codes[i - 1]) << 1);

        for (i = 0; i < table_len; i++)
        {
            uint32_t rev_code = 0, code, code_size; if ((code_size = d->m_huff_code_sizes[table_num][i]) == 0) continue;
            code = next_code[code_size]++; for (l = code_size; l > 0; l--, code >>= 1) rev_code = (rev_code << 1) | (code & 1);
            d->m_huff_codes[table_num][i] = (uint16_t)rev_code;
        }
    }

#define DEFL_RLE_PREV_CODE_SIZE() { if (rle_repeat_count) { \
  if (rle_repeat_count < 3) { \
    d->m_huff_count[2][prev_code_size] = (uint16_t)(d->m_huff_count[2][prev_code_size] + rle_repeat_count); \
    while (rle_repeat_count--) packed_code_sizes[num_packed_code_sizes++] = prev_code_size; \
  } else { \
    d->m_huff_count[2][16] = (uint16_t)(d->m_huff_count[2][16] + 1); packed_code_sizes[num_packed_code_sizes++] = 16; packed_code_sizes[num_packed_code_sizes++] = (uint8_t)(rle_repeat_count - 3); \
} rle_repeat_count = 0; } }

#define DEFL_RLE_ZERO_CODE_SIZE() { if (rle_z_count) { \
  if (rle_z_count < 3) { \
    d->m_huff_count[2][0] = (uint16_t)(d->m_huff_count[2][0] + rle_z_count); while (rle_z_count--) packed_code_sizes[num_packed_code_sizes++] = 0; \
  } else if (rle_z_count <= 10) { \
    d->m_huff_count[2][17] = (uint16_t)(d->m_huff_count[2][17] + 1); packed_code_sizes[num_packed_code_sizes++] = 17; packed_code_sizes[num_packed_code_sizes++] = (uint8_t)(rle_z_count - 3); \
  } else { \
    d->m_huff_count[2][18] = (uint16_t)(d->m_huff_count[2][18] + 1); packed_code_sizes[num_packed_code_sizes++] = 18; packed_code_sizes[num_packed_code_sizes++] = (uint8_t)(rle_z_count - 11); \
} rle_z_count = 0; } }

    static uint8_t g_defl_packed_code_size_syms_swizzle[] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

#define DEFL_DYN_PUT_BITS(bb, ll) \
do { \
    uint32_t b = (bb), l = (ll); \
    assert((l) >= 1 && (l) <= 16); assert((b) < (1ULL << (l))); \
    bit_buf |= (((uint64_t)(b)) << bit_buf_size); bit_buf_size += (l); assert(bit_buf_size <= 64); \
    while (bit_buf_size >= 8) \
    { \
        if ((dst_ofs + 1) > dst_buf_size) \
            return false; \
        *(uint8_t*)(pDst + dst_ofs) = (uint8_t)bit_buf; \
        dst_ofs++; \
        bit_buf >>= 8; \
        bit_buf_size -= 8; \
    } \
} while(0)

    static bool defl_start_dynamic_block(defl_huff* d, uint8_t* pDst, uint32_t& dst_ofs, uint32_t dst_buf_size, uint64_t& bit_buf, int& bit_buf_size)
    {
        int num_lit_codes, num_dist_codes, num_bit_lengths; uint32_t i, total_code_sizes_to_pack, num_packed_code_sizes, rle_z_count, rle_repeat_count, packed_code_sizes_index;
        uint8_t code_sizes_to_pack[DEFL_MAX_HUFF_SYMBOLS_0 + DEFL_MAX_HUFF_SYMBOLS_1], packed_code_sizes[DEFL_MAX_HUFF_SYMBOLS_0 + DEFL_MAX_HUFF_SYMBOLS_1], prev_code_size = 0xFF;

        d->m_huff_count[0][256] = 1;

        defl_optimize_huffman_table(d, 0, DEFL_MAX_HUFF_SYMBOLS_0, 12, FPNG_FALSE);
        defl_optimize_huffman_table(d, 1, DEFL_MAX_HUFF_SYMBOLS_1, 12, FPNG_FALSE);

        for (num_lit_codes = 286; num_lit_codes > 257; num_lit_codes--) if (d->m_huff_code_sizes[0][num_lit_codes - 1]) break;
        for (num_dist_codes = 30; num_dist_codes > 1; num_dist_codes--) if (d->m_huff_code_sizes[1][num_dist_codes - 1]) break;

        memcpy(code_sizes_to_pack, &d->m_huff_code_sizes[0][0], num_lit_codes);
        memcpy(code_sizes_to_pack + num_lit_codes, &d->m_huff_code_sizes[1][0], num_dist_codes);
        total_code_sizes_to_pack = num_lit_codes + num_dist_codes; num_packed_code_sizes = 0; rle_z_count = 0; rle_repeat_count = 0;

        memset(&d->m_huff_count[2][0], 0, sizeof(d->m_huff_count[2][0]) * DEFL_MAX_HUFF_SYMBOLS_2);
        for (i = 0; i < total_code_sizes_to_pack; i++)
        {
            uint8_t code_size = code_sizes_to_pack[i];
            if (!code_size)
            {
                DEFL_RLE_PREV_CODE_SIZE();
                if (++rle_z_count == 138) { DEFL_RLE_ZERO_CODE_SIZE(); }
            }
            else
            {
                DEFL_RLE_ZERO_CODE_SIZE();
                if (code_size != prev_code_size)
                {
                    DEFL_RLE_PREV_CODE_SIZE();
                    d->m_huff_count[2][code_size] = (uint16_t)(d->m_huff_count[2][code_size] + 1); packed_code_sizes[num_packed_code_sizes++] = code_size;
                }
                else if (++rle_repeat_count == 6)
                {
                    DEFL_RLE_PREV_CODE_SIZE();
                }
            }
            prev_code_size = code_size;
        }
        if (rle_repeat_count) { DEFL_RLE_PREV_CODE_SIZE(); }
        else { DEFL_RLE_ZERO_CODE_SIZE(); }

        defl_optimize_huffman_table(d, 2, DEFL_MAX_HUFF_SYMBOLS_2, 7, FPNG_FALSE);

        // max of 2+5+5+4+18*3+(288+32)*7=2310 bits
        DEFL_DYN_PUT_BITS(2, 2);

        DEFL_DYN_PUT_BITS(num_lit_codes - 257, 5);
        DEFL_DYN_PUT_BITS(num_dist_codes - 1, 5);

        for (num_bit_lengths = 18; num_bit_lengths >= 0; num_bit_lengths--) if (d->m_huff_code_sizes[2][g_defl_packed_code_size_syms_swizzle[num_bit_lengths]]) break;
        num_bit_lengths = std::max<int>(4, (num_bit_lengths + 1)); DEFL_DYN_PUT_BITS(num_bit_lengths - 4, 4);
        for (i = 0; (int)i < num_bit_lengths; i++) DEFL_DYN_PUT_BITS(d->m_huff_code_sizes[2][g_defl_packed_code_size_syms_swizzle[i]], 3);

        for (packed_code_sizes_index = 0; packed_code_sizes_index < num_packed_code_sizes; )
        {
            uint32_t code = packed_code_sizes[packed_code_sizes_index++]; assert(code < DEFL_MAX_HUFF_SYMBOLS_2);
            DEFL_DYN_PUT_BITS(d->m_huff_codes[2][code], d->m_huff_code_sizes[2][code]);
            if (code >= 16) DEFL_DYN_PUT_BITS(packed_code_sizes[packed_code_sizes_index++], "\02\03\07"[code - 16]);
        }

        return true;
    }

    static uint32_t write_raw_block(const uint8_t* pSrc, uint32_t src_len, uint8_t* pDst, uint32_t dst_buf_size)
    {
        if (dst_buf_size < 2)
            return 0;

        pDst[0] = 0x78;
        pDst[1] = 0x01;

        uint32_t dst_ofs = 2;

        uint32_t src_ofs = 0;
        while (src_ofs < src_len)
        {
            const uint32_t src_remaining = src_len - src_ofs;
            const uint32_t block_size = std::min<uint32_t>(UINT16_MAX, src_remaining);
            const bool final_block = (block_size == src_remaining);

            if ((dst_ofs + 5 + block_size) > dst_buf_size)
                return 0;

            pDst[dst_ofs + 0] = final_block ? 1 : 0;

            pDst[dst_ofs + 1] = block_size & 0xFF;
            pDst[dst_ofs + 2] = (block_size >> 8) & 0xFF;

            pDst[dst_ofs + 3] = (~block_size) & 0xFF;
            pDst[dst_ofs + 4] = ((~block_size) >> 8) & 0xFF;

            memcpy(pDst + dst_ofs + 5, pSrc + src_ofs, block_size);

            src_ofs += block_size;
            dst_ofs += 5 + block_size;
        }

        // reserve space for adler
        for (uint32_t i = 0; i < 4; i++)
        {
            if (dst_ofs + 1 > dst_buf_size)
                return 0;

            pDst[dst_ofs] = 0;
            dst_ofs++;
        }

        return dst_ofs;
    }

    static void adjust_freq32(uint32_t num_freq, uint32_t* pFreq, uint16_t* pFreq16)
    {
        uint32_t total_freq = 0;
        for (uint32_t i = 0; i < num_freq; i++)
            total_freq += pFreq[i];

        if (!total_freq)
        {
            memset(pFreq16, 0, num_freq * sizeof(uint16_t));
            return;
        }

        uint32_t total_freq16 = 0;
        for (uint32_t i = 0; i < num_freq; i++)
        {
            uint64_t f = pFreq[i];
            if (!f)
            {
                pFreq16[i] = 0;
                continue;
            }

            pFreq16[i] = (uint16_t)std::max<uint32_t>(1, (uint32_t)((f * UINT16_MAX) / total_freq));

            total_freq16 += pFreq16[i];
        }

        while (total_freq16 > UINT16_MAX)
        {
            total_freq16 = 0;
            for (uint32_t i = 0; i < num_freq; i++)
            {
                if (pFreq[i])
                {
                    pFreq[i] = std::max<uint32_t>(1, pFreq[i] >> 1);
                    total_freq16 += pFreq[i];
                }
            }
        }
    }

    static uint32_t pixel_deflate_dyn_3_rle_one_pass(
        const uint8_t* pImg, uint32_t w, uint32_t h,
        uint8_t* pDst, uint32_t dst_buf_size)
    {
        const uint32_t bpl = 1 + w * 3;

        if (dst_buf_size < sizeof(g_dyn_huff_3))
            return false;
        memcpy(pDst, g_dyn_huff_3, sizeof(g_dyn_huff_3));
        uint32_t dst_ofs = sizeof(g_dyn_huff_3);

        uint64_t bit_buf = DYN_HUFF_3_BITBUF;
        int bit_buf_size = DYN_HUFF_3_BITBUF_SIZE;

        const uint8_t* pSrc = pImg;
        uint32_t src_ofs = 0;

        for (uint32_t y = 0; y < h; y++)
        {
            const uint32_t end_src_ofs = src_ofs + bpl;

            const uint32_t filter_lit = pSrc[src_ofs++];
            PUT_BITS_CZ(g_dyn_huff_3_codes[filter_lit].m_code, g_dyn_huff_3_codes[filter_lit].m_code_size);

            uint32_t prev_lits;

            {
                uint32_t lits = READ_RGB_PIXEL(pSrc + src_ofs);

                PUT_BITS_CZ(g_dyn_huff_3_codes[lits & 0xFF].m_code, g_dyn_huff_3_codes[lits & 0xFF].m_code_size);
                PUT_BITS_CZ(g_dyn_huff_3_codes[(lits >> 8) & 0xFF].m_code, g_dyn_huff_3_codes[(lits >> 8) & 0xFF].m_code_size);
                PUT_BITS_CZ(g_dyn_huff_3_codes[(lits >> 16)].m_code, g_dyn_huff_3_codes[(lits >> 16)].m_code_size);

                src_ofs += 3;
            
                prev_lits = lits;
            }

            PUT_BITS_FLUSH;

            while (src_ofs < end_src_ofs)
            {
                uint32_t lits = READ_RGB_PIXEL(pSrc + src_ofs);

                if (lits == prev_lits)
                {
                    uint32_t match_len = 3;
                    uint32_t max_match_len = std::min<int>(255, (int)(end_src_ofs - src_ofs));

                    while (match_len < max_match_len)
                    {
                        if (READ_RGB_PIXEL(pSrc + src_ofs + match_len) != lits)
                            break;
                        match_len += 3;
                    }
                                        
                    uint32_t adj_match_len = match_len - 3;

                    PUT_BITS_CZ(g_dyn_huff_3_codes[g_defl_len_sym[adj_match_len]].m_code, g_dyn_huff_3_codes[g_defl_len_sym[adj_match_len]].m_code_size);
                    PUT_BITS(adj_match_len & g_bitmasks[g_defl_len_extra[adj_match_len]], g_defl_len_extra[adj_match_len] + 1); // up to 6 bits, +1 for the match distance Huff code which is always 0

                    src_ofs += match_len;
                }
                else
                {
                    PUT_BITS_CZ(g_dyn_huff_3_codes[lits & 0xFF].m_code, g_dyn_huff_3_codes[lits & 0xFF].m_code_size);
                    PUT_BITS_CZ(g_dyn_huff_3_codes[(lits >> 8) & 0xFF].m_code, g_dyn_huff_3_codes[(lits >> 8) & 0xFF].m_code_size);
                    PUT_BITS_CZ(g_dyn_huff_3_codes[(lits >> 16)].m_code, g_dyn_huff_3_codes[(lits >> 16)].m_code_size);
                    
                    prev_lits = lits;

                    src_ofs += 3;
                }

                PUT_BITS_FLUSH;

            } // while (src_ofs < end_src_ofs)

        } // y

        assert(src_ofs == h * bpl);
        
        assert(bit_buf_size <= 7);

        PUT_BITS_CZ(g_dyn_huff_3_codes[256].m_code, g_dyn_huff_3_codes[256].m_code_size);

        PUT_BITS_FORCE_FLUSH;

        // Write zlib adler32 placeholder
        for (uint32_t i = 0; i < 4; i++)
        {
            if ((dst_ofs + 1) > dst_buf_size)
                return 0;
            *(uint8_t*)(pDst + dst_ofs) = 0;
            dst_ofs++;
        }

        return dst_ofs;
    }

    static uint32_t pixel_deflate_dyn_4_rle_one_pass(
        const uint8_t* pImg, uint32_t w, uint32_t h,
        uint8_t* pDst, uint32_t dst_buf_size)
    {
        const uint32_t bpl = 1 + w * 4;

        if (dst_buf_size < sizeof(g_dyn_huff_4))
            return false;
        memcpy(pDst, g_dyn_huff_4, sizeof(g_dyn_huff_4));
        uint32_t dst_ofs = sizeof(g_dyn_huff_4);

        uint64_t bit_buf = DYN_HUFF_4_BITBUF;
        int bit_buf_size = DYN_HUFF_4_BITBUF_SIZE;

        const uint8_t* pSrc = pImg;
        uint32_t src_ofs = 0;

        for (uint32_t y = 0; y < h; y++)
        {
            const uint32_t end_src_ofs = src_ofs + bpl;

            const uint32_t filter_lit = pSrc[src_ofs++];
            PUT_BITS_CZ(g_dyn_huff_4_codes[filter_lit].m_code, g_dyn_huff_4_codes[filter_lit].m_code_size);

            PUT_BITS_FLUSH;

            uint32_t prev_lits;
            {
                uint32_t lits = uload32le(pSrc + src_ofs);

                PUT_BITS_CZ(g_dyn_huff_4_codes[lits & 0xFF].m_code, g_dyn_huff_4_codes[lits & 0xFF].m_code_size);
                PUT_BITS_CZ(g_dyn_huff_4_codes[(lits >> 8) & 0xFF].m_code, g_dyn_huff_4_codes[(lits >> 8) & 0xFF].m_code_size);
                PUT_BITS_CZ(g_dyn_huff_4_codes[(lits >> 16) & 0xFF].m_code, g_dyn_huff_4_codes[(lits >> 16) & 0xFF].m_code_size);

                if (bit_buf_size >= 49)
                {
                    PUT_BITS_FLUSH;
                }
                
                PUT_BITS_CZ(g_dyn_huff_4_codes[(lits >> 24)].m_code, g_dyn_huff_4_codes[(lits >> 24)].m_code_size);

                src_ofs += 4;
                
                prev_lits = lits;
            }

            PUT_BITS_FLUSH;

            while (src_ofs < end_src_ofs)
            {
                uint32_t lits = uload32le(pSrc + src_ofs);
                                
                if (lits == prev_lits)
                {
                    uint32_t match_len = 4;
                    uint32_t max_match_len = std::min<int>(252, (int)(end_src_ofs - src_ofs));

                    while (match_len < max_match_len)
                    {
                        if (uload32le(pSrc + src_ofs + match_len) != lits)
                            break;
                        match_len += 4;
                    }

                    uint32_t adj_match_len = match_len - 3;

                    const uint32_t match_code_bits = g_dyn_huff_4_codes[g_defl_len_sym[adj_match_len]].m_code_size;
                    const uint32_t len_extra_bits = g_defl_len_extra[adj_match_len];

                    if (match_len == 4)
                    {
                        // This check is optional - see if just encoding 4 literals would be cheaper than using a short match.
                        uint32_t lit_bits = g_dyn_huff_4_codes[lits & 0xFF].m_code_size + g_dyn_huff_4_codes[(lits >> 8) & 0xFF].m_code_size + 
                            g_dyn_huff_4_codes[(lits >> 16) & 0xFF].m_code_size + g_dyn_huff_4_codes[(lits >> 24)].m_code_size;
                        
                        if ((match_code_bits + len_extra_bits + 1) > lit_bits)
                            goto do_literals;
                    }

                    PUT_BITS_CZ(g_dyn_huff_4_codes[g_defl_len_sym[adj_match_len]].m_code, match_code_bits);
                    PUT_BITS(adj_match_len & g_bitmasks[g_defl_len_extra[adj_match_len]], len_extra_bits + 1); // up to 6 bits, +1 for the match distance Huff code which is always 0

                    src_ofs += match_len;
                }
                else
                {
do_literals:
                    PUT_BITS_CZ(g_dyn_huff_4_codes[lits & 0xFF].m_code, g_dyn_huff_4_codes[lits & 0xFF].m_code_size);
                    PUT_BITS_CZ(g_dyn_huff_4_codes[(lits >> 8) & 0xFF].m_code, g_dyn_huff_4_codes[(lits >> 8) & 0xFF].m_code_size);
                    PUT_BITS_CZ(g_dyn_huff_4_codes[(lits >> 16) & 0xFF].m_code, g_dyn_huff_4_codes[(lits >> 16) & 0xFF].m_code_size);

                    if (bit_buf_size >= 49)
                    {
                        PUT_BITS_FLUSH;
                    }

                    PUT_BITS_CZ(g_dyn_huff_4_codes[(lits >> 24)].m_code, g_dyn_huff_4_codes[(lits >> 24)].m_code_size);

                    src_ofs += 4;
                    
                    prev_lits = lits;
                }

                PUT_BITS_FLUSH;

            } // while (src_ofs < end_src_ofs)

        } // y

        assert(src_ofs == h * bpl);

        assert(bit_buf_size <= 7);

        PUT_BITS_CZ(g_dyn_huff_4_codes[256].m_code, g_dyn_huff_4_codes[256].m_code_size);

        PUT_BITS_FORCE_FLUSH;

        // Write zlib adler32 placeholder
        for (uint32_t i = 0; i < 4; i++)
        {
            if ((dst_ofs + 1) > dst_buf_size)
                return 0;
            *(uint8_t*)(pDst + dst_ofs) = 0;
            dst_ofs++;
        }

        return dst_ofs;
    }

} // namespace fpng

namespace
{
    using namespace mango;
    using namespace mango::image;
    using namespace mango::math;

    constexpr int PNG_SIMD_PADDING = 16;
    constexpr int PNG_FILTER_BYTE = 1;
    constexpr u64 PNG_HEADER_MAGIC = 0x89504e470d0a1a0a;

    enum ColorType
    {
        COLOR_TYPE_I       = 0,
        COLOR_TYPE_RGB     = 2,
        COLOR_TYPE_PALETTE = 3,
        COLOR_TYPE_IA      = 4,
        COLOR_TYPE_RGBA    = 6,
    };

    inline
    const char* get_string(ColorType type)
    {
        const char* text = "INVALID";
        switch (type)
        {
            case COLOR_TYPE_I:
                text = "I";
                break;
            case COLOR_TYPE_RGB:
                text = "RGB";
                break;
            case COLOR_TYPE_PALETTE:
                text = "PALETTE";
                break;
            case COLOR_TYPE_IA:
                text = "IA";
                break;
            case COLOR_TYPE_RGBA:
                text = "RGBA";
                break;
            default:
                break;
        }
        return text;
    }

    // ------------------------------------------------------------
    // png_strnlen
    // ------------------------------------------------------------

    // Long story short; the strnlen() is not part of std and not available
    // on all platforms or tool-chains so we have to wrap it like this. :(

#if defined(__ppc__)
    size_t png_strnlen(const char* s, size_t maxlen)
    {
        for (size_t i = 0; i < maxlen ; ++i)
        {
            if (s[i] == 0)
                return i;
        }
        return maxlen;
    }
#else
    #define png_strnlen strnlen
#endif

    // ------------------------------------------------------------
    // Filter
    // ------------------------------------------------------------

    enum FilterType : u8
    {
        FILTER_NONE    = 0,
        FILTER_SUB     = 1,
        FILTER_UP      = 2,
        FILTER_AVERAGE = 3,
        FILTER_PAETH   = 4,
    };

    using FilterFunc = void (*)(u8* scan, const u8* prev, int bytes, int bpp);

    void filter1_sub(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(prev);

        for (int x = bpp; x < bytes; ++x)
        {
            scan[x] += scan[x - bpp];
        }
    }

    void filter2_up(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        for (int x = 0; x < bytes; ++x)
        {
            scan[x] += prev[x];
        }
    }

    void filter3_average(u8* scan, const u8* prev, int bytes, int bpp)
    {
        for (int x = 0; x < bpp; ++x)
        {
            scan[x] += prev[x] / 2;
        }

        for (int x = bpp; x < bytes; ++x)
        {
            scan[x] += (prev[x] + scan[x - bpp]) / 2;
        }
    }

    void filter4_paeth(u8* scan, const u8* prev, int bytes, int bpp)
    {
        for (int x = 0; x < bpp; ++x)
        {
            scan[x] += prev[x];
        }

        for (int x = bpp; x < bytes; ++x)
        {
            int c = prev[x - bpp];
            int a = scan[x - bpp];
            int b = prev[x];

            int p = b - c;
            int q = a - c;
            int pa = std::abs(p);
            int pb = std::abs(q);
            int pc = std::abs(p + q);

            if (pb < pa)
            {
                pa = pb;
                a = b;
            }

            if (pc < pa)
            {
                a = c;
            }

            scan[x] += u8(a);
        }
    }

    void filter4_paeth_8bit(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        int c = prev[0];
        int a = (scan[0] + c) & 0xff;
        scan[0] = u8(a);

        for (int x = 1; x < bytes; ++x)
        {
            int b = prev[x];

            int p = b - c;
            int q = a - c;
            int pa = abs(p);
            int pb = abs(q);
            int pc = abs(p + q);

            if (pb < pa)
            {
                pa = pb;
                a = b;
            }

            if (pc < pa)
            {
                a = c;
            }

            c = b;
            a = (a + scan[x]) & 0xff;
            scan[x] = u8(a);
        }
    }

#if defined(MANGO_ENABLE_SSE2)

    // -----------------------------------------------------------------------------------
    // SSE2 Filters
    // -----------------------------------------------------------------------------------

    // helper functions

    inline __m128i load4(const void* p)
    {
        u32 temp = uload32(p);
        return _mm_cvtsi32_si128(temp);
    }

    inline void store4(void* p, __m128i v)
    {
        u32 temp = _mm_cvtsi128_si32(v);
        ustore32(p, temp);
    }

    inline __m128i load3(const void* p)
    {
        // NOTE: we have PNG_SIMD_PADDING for overflow guardband
        u32 temp = uload32(p);
        return _mm_cvtsi32_si128(temp);
    }

    inline void store3(void* p, __m128i v)
    {
        u32 temp = _mm_cvtsi128_si32(v);
        std::memcpy(p, &temp, 3);
    }

    inline __m128i average_sse2(__m128i a, __m128i b, __m128i d)
    {
        __m128i avg = _mm_avg_epu8(a, b);
        avg = _mm_sub_epi8(avg, _mm_and_si128(_mm_xor_si128(a, b), _mm_set1_epi8(1)));
        d = _mm_add_epi8(d, avg);
        return d;
    }

    inline int16x8 nearest_sse2(int16x8 a, int16x8 b, int16x8 c, int16x8 d)
    {
        int16x8 pa = b - c;
        int16x8 pb = a - c;
        int16x8 pc = pa + pb;
        pa = abs(pa);
        pb = abs(pb);
        pc = abs(pc);
        int16x8 smallest = min(pc, min(pa, pb));
        int16x8 nearest = select(smallest == pa, a,
                          select(smallest == pb, b, c));
        d = _mm_add_epi8(d, nearest);
        return d;
    }

    // filters

    void filter1_sub_24bit_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(prev);
        MANGO_UNREFERENCED(bpp);

        __m128i d = _mm_setzero_si128();

        for (int x = 0; x < bytes; x += 3)
        {
            d = _mm_add_epi8(load3(scan + x), d);
            store3(scan + x, d);
        }
    }

    void filter1_sub_32bit_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(prev);
        MANGO_UNREFERENCED(bpp);

        __m128i d = _mm_setzero_si128();

        while (bytes >= 16)
        {
            __m128i b = _mm_loadu_si128(reinterpret_cast<__m128i*>(scan));
            d = _mm_add_epi8(d, b);
            d = _mm_add_epi8(d, _mm_bslli_si128(b, 4));
            d = _mm_add_epi8(d, _mm_bslli_si128(b, 8));
            d = _mm_add_epi8(d, _mm_bslli_si128(b, 12));
            _mm_storeu_si128(reinterpret_cast<__m128i*>(scan), d);
            d = _mm_shuffle_epi32(d, 0xff);
            scan += 16;
            bytes -= 16;
        }

        while (bytes >= 4)
        {
            d = _mm_add_epi8(load4(scan), d);
            store4(scan, d);
            scan += 4;
            bytes -= 4;
        }
    }

    void filter2_up_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        while (bytes >= 16)
        {
            __m128i a = _mm_loadu_si128(reinterpret_cast<__m128i*>(scan));
            __m128i b = _mm_loadu_si128(reinterpret_cast<const __m128i*>(prev));
            a = _mm_add_epi8(a, b);
            _mm_storeu_si128(reinterpret_cast<__m128i*>(scan), a);
            scan += 16;
            prev += 16;
            bytes -= 16;
        }

        for (int x = 0; x < bytes; ++x)
        {
            scan[x] += prev[x];
        }
    }

    void filter3_average_24bit_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        __m128i d = _mm_setzero_si128();

        for (int x = 0; x < bytes; x += 3)
        {
            d = average_sse2(d, load3(prev + x), load3(scan + x));
            store3(scan + x, d);
        }
    }

    void filter3_average_32bit_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        __m128i d = _mm_setzero_si128();

        for (int x = 0; x < bytes; x += 4)
        {
            d = average_sse2(d, load4(prev + x), load4(scan + x));
            store4(scan + x, d);
        }
    }

    void filter4_paeth_24bit_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        int8x16 zero = 0;
        int16x8 b = 0;
        int16x8 d = 0;

        for (int x = 0; x < bytes; x += 3)
        {
            int16x8 c = b;
            int16x8 a = d;
            b = _mm_unpacklo_epi8(load3(prev + x), zero);
            d = _mm_unpacklo_epi8(load3(scan + x), zero);
            d = nearest_sse2(a, b, c, d);
            store3(scan + x, _mm_packus_epi16(d, d));
        }
    }

    void filter4_paeth_32bit_sse2(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(bpp);

        int8x16 zero = 0;
        int16x8 b = 0;
        int16x8 d = 0;

        for (int x = 0; x < bytes; x += 4)
        {
            int16x8 c = b;
            int16x8 a = d;
            b = _mm_unpacklo_epi8(load4(prev + x), zero);
            d = _mm_unpacklo_epi8(load4(scan + x), zero);
            d = nearest_sse2(a, b, c, d);
            store4(scan + x, _mm_packus_epi16(d, d));
        }
    }

#endif // MANGO_ENABLE_SSE2

#if defined(MANGO_ENABLE_SSE4_1)

    // -----------------------------------------------------------------------------------
    // SSE4.1 Filters
    // -----------------------------------------------------------------------------------

    void filter1_sub_24bit_sse41(u8* scan, const u8* prev, int bytes, int bpp)
    {
        MANGO_UNREFERENCED(prev);
        MANGO_UNREFERENCED(bpp);

        __m128i d = _mm_setzero_si128();

        __m128i shuf0 = _mm_set_epi8(-1, -1, -1, -1, 14, 13, 12, 10, 9, 8, 6, 5, 4, 2, 1, 0);
        __m128i shuf1 = _mm_set_epi8(4, 2, 1, 0, -1, -1, -1, -1, 14, 13, 12, 10, 9, 8, 6, 5);
        __m128i shuf2 = _mm_set_epi8(9, 8, 6, 5, 4, 2, 1, 0, -1, -1, -1, -1, 14, 13, 12, 10);
        __m128i shuf3 = _mm_set_epi8(14, 13, 12, 10, 9, 8, 6, 5, 4, 2, 1, 0, -1, -1, -1, -1);
        __m128i shuf4 = _mm_set_epi8(-1, 11, 10, 9, -1, 8, 7, 6, -1, 5, 4, 3, -1, 2, 1, 0);
        __m128i mask0 = _mm_set_epi32(0, -1, -1, -1);
        __m128i mask1 = _mm_set_epi32(0,  0, -1, -1);
        __m128i mask2 = _mm_set_epi32(0,  0,  0, -1);

        while (bytes >= 48)
        {
            // load
            __m128i v0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(scan + 0 * 16));
            __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(scan + 1 * 16));
            __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(scan + 2 * 16));

            // unpack
            __m128i color0 = _mm_shuffle_epi8(v0, shuf4);
            __m128i color1 = _mm_shuffle_epi8(_mm_alignr_epi8(v1, v0, 12), shuf4);
            __m128i color2 = _mm_shuffle_epi8(_mm_alignr_epi8(v2, v1,  8), shuf4);
            __m128i color3 = _mm_shuffle_epi8(_mm_alignr_epi8(v2, v2,  4), shuf4);

            d = _mm_add_epi8(d, color0);
            d = _mm_add_epi8(d, _mm_bslli_si128(color0, 4));
            d = _mm_add_epi8(d, _mm_bslli_si128(color0, 8));
            d = _mm_add_epi8(d, _mm_bslli_si128(color0, 12));
            color0 = _mm_shuffle_epi8(d, shuf0);
            d = _mm_shuffle_epi32(d, 0xff);

            d = _mm_add_epi8(d, color1);
            d = _mm_add_epi8(d, _mm_bslli_si128(color1, 4));
            d = _mm_add_epi8(d, _mm_bslli_si128(color1, 8));
            d = _mm_add_epi8(d, _mm_bslli_si128(color1, 12));
            color1 = _mm_shuffle_epi8(d, shuf1);
            d = _mm_shuffle_epi32(d, 0xff);

            d = _mm_add_epi8(d, color2);
            d = _mm_add_epi8(d, _mm_bslli_si128(color2, 4));
            d = _mm_add_epi8(d, _mm_bslli_si128(color2, 8));
            d = _mm_add_epi8(d, _mm_bslli_si128(color2, 12));
            color2 = _mm_shuffle_epi8(d, shuf2);
            d = _mm_shuffle_epi32(d, 0xff);

            d = _mm_add_epi8(d, color3);
            d = _mm_add_epi8(d, _mm_bslli_si128(color3, 4));
            d = _mm_add_epi8(d, _mm_bslli_si128(color3, 8));
            d = _mm_add_epi8(d, _mm_bslli_si128(color3, 12));
            color3 = _mm_shuffle_epi8(d, shuf3);
            d = _mm_shuffle_epi32(d, 0xff);

            // pack
            v0 = _mm_blendv_epi8(color1, color0, mask0);
            v1 = _mm_blendv_epi8(color2, color1, mask1);
            v2 = _mm_blendv_epi8(color3, color2, mask2);

            // store
            _mm_storeu_si128(reinterpret_cast<__m128i *>(scan + 0 * 16), v0);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(scan + 1 * 16), v1);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(scan + 2 * 16), v2);

            scan += 48;
            bytes -= 48;
        }

        for (int x = 0; x < bytes; x += 3)
        {
            d = _mm_add_epi8(load3(scan + x), d);
            store3(scan + x, d);
        }
    }

#endif // MANGO_ENABLE_SSE4_1

#if defined(MANGO_ENABLE_NEON)

    // -----------------------------------------------------------------------------------
    // NEON Filters
    // -----------------------------------------------------------------------------------

    // helper functions

    static inline
    uint8x16_t load12(const u8* ptr)
    {
        // NOTE: 4 byte overflow guarded by PNG_SIMD_PADDING
        return vld1q_u8(ptr);
    }

    static inline
    void store12(u8* dest, uint8x8_t v0, uint8x8_t v1)
    {
        vst1_u8(dest, v0);
        vst1_lane_u32(reinterpret_cast<u32 *>(dest + 8), vreinterpret_u32_u8(v1), 0);
    }

    static inline
    uint8x16_t splat(uint8x16_t v)
    {
#ifdef __aarch64__
        return vreinterpretq_u8_u32(vdupq_laneq_u32(vreinterpretq_u32_u8(v), 3));
#else
        return vreinterpretq_u8_u32(vdupq_n_u32(vgetq_lane_u32(vreinterpretq_u32_u8(v), 3)));
#endif
    }

    static inline
    void sub12(u8* scan, uint8x8_t& last)
    {
        uint8x16_t a = load12(scan);
        uint8x8_t a0 = vget_low_u8(a);
        uint8x8_t a1 = vget_high_u8(a);

        uint8x8_t s0 = a0;
        uint8x8_t s1 = vext_u8(a0, a1, 3);
        uint8x8_t s2 = vext_u8(a0, a1, 6);
        uint8x8_t s3 = vext_u8(a1, a1, 1);

        uint8x8_t v0 = vadd_u8(last, s0);
        uint8x8_t v1 = vadd_u8(v0, s1);
        uint8x8_t v2 = vadd_u8(v1, s2);
        uint8x8_t v3 = vadd_u8(v2, s3);

        last = v3;

        const uint8x8x4_t table =
        {
            v0, v1, v2, v3
        };

        const uint8x8_t idx0 = { 0, 1, 2, 8, 9, 10, 16, 17 };
        const uint8x8_t idx1 = { 18, 24, 25, 26, 255, 255, 255, 255 };

        a0 = vtbl4_u8(table, idx0);
        a1 = vtbl4_u8(table, idx1);

        store12(scan, a0, a1);
    }

    static inline
    void sub16(u8* scan, uint8x16_t& last)
    {
        const uint8x16_t zero = vdupq_n_u8(0);

        uint8x16_t a = vld1q_u8(scan);

        uint8x16_t d = vaddq_u8(a, last);
        d = vaddq_u8(d, vextq_u8(zero, a, 12));
        d = vaddq_u8(d, vextq_u8(zero, a, 8));
        d = vaddq_u8(d, vextq_u8(zero, a, 4));

        last = splat(d);

        vst1q_u8(scan, d);
    }

    static inline
    uint8x8_t average(uint8x8_t v, uint8x8_t a, uint8x8_t b)
    {
        return vadd_u8(vhadd_u8(v, b), a);
    }

    static inline
    void average12(u8* scan, const u8* prev, uint8x8_t& last)
    {
        // NOTE: 4 byte overflow guarded by PNG_SIMD_PADDING
        uint8x16_t a = vld1q_u8(scan);
        uint8x16_t b = vld1q_u8(prev);

        uint8x8_t a0 = vget_low_u8(a);
        uint8x8_t b0 = vget_low_u8(b);
        uint8x8_t b1 = vget_high_u8(b);
        uint8x8_t a1 = vget_high_u8(a);

        uint8x8_t s1 = vext_u8(a1, a1, 1);
        uint8x8_t t1 = vext_u8(b1, b1, 1);
        uint8x8_t a3 = vext_u8(a0, a1, 3);
        uint8x8_t b3 = vext_u8(b0, b1, 3);
        uint8x8_t a6 = vext_u8(a0, a1, 6);
        uint8x8_t b6 = vext_u8(b0, b1, 6);

        uint8x8x4_t value;
        value.val[0] = average(        last, a0, b0);
        value.val[1] = average(value.val[0], a3, b3);
        value.val[2] = average(value.val[1], a6, b6);
        value.val[3] = average(value.val[2], s1, t1);

        last = value.val[3];

        const uint8x8_t idx0 = { 0, 1, 2, 8, 9, 10, 16, 17 };
        const uint8x8_t idx1 = { 18, 24, 25, 26, 255, 255, 255, 255 };

        uint8x8_t v0 = vtbl4_u8(value, idx0);
        uint8x8_t v1 = vtbl4_u8(value, idx1);

        store12(scan, v0, v1);
    }

    static inline
    void average16(u8* scan, const u8* prev, uint8x8_t& last)
    {
        uint32x2x4_t tmp;

        tmp = vld4_lane_u32(reinterpret_cast<u32 *>(scan), tmp, 0);
        uint8x8x4_t a = *(uint8x8x4_t *) &tmp; // !!!

        tmp = vld4_lane_u32(reinterpret_cast<const u32 *>(prev), tmp, 0);
        uint8x8x4_t b = *(uint8x8x4_t *) &tmp; // !!!

        uint8x8x4_t value;
        value.val[0] = average(        last, a.val[0], b.val[0]);
        value.val[1] = average(value.val[0], a.val[1], b.val[1]);
        value.val[2] = average(value.val[1], a.val[2], b.val[2]);
        value.val[3] = average(value.val[2], a.val[3], b.val[3]);

        last = value.val[3];

        const uint8x8_t idx0 = { 0, 1, 2, 3, 8, 9, 10, 11 };
        const uint8x8_t idx1 = { 16, 17, 18, 19, 24, 25, 26, 27 };

        uint8x8_t v0 = vtbl4_u8(value, idx0);
        uint8x8_t v1 = vtbl4_u8(value, idx1);
        vst1q_u8(scan, vcombine_u8(v0, v1));
    }

    static inline
    uint8x8_t paeth(uint8x8_t a, uint8x8_t b, uint8x8_t c, uint8x8_t delta)
    {
        uint16x8_t pa = vabdl_u8(b, c);
        uint16x8_t pb = vabdl_u8(a, c);
        uint16x8_t pc = vabdq_u16(vaddl_u8(a, b), vaddl_u8(c, c));

        uint16x8_t ab = vcleq_u16(pa, pb);
        uint16x8_t ac = vcleq_u16(pa, pc);
        uint16x8_t bc = vcleq_u16(pb, pc);

        uint16x8_t abc = vandq_u16(ab, ac);

        uint8x8_t d = vmovn_u16(bc);
        uint8x8_t e = vmovn_u16(abc);

        d = vbsl_u8(d, b, c);
        e = vbsl_u8(e, a, d);

        e = vadd_u8(e, delta);

        return e;
    }

    static inline
    void paeth12(u8* scan, const u8* prev, uint8x8x4_t& value, uint8x8_t& last)
    {
        // NOTE: 4 byte overflow guarded by PNG_SIMD_PADDING
        uint8x8x2_t a = vld1_u8_x2(scan);
        uint8x8x2_t b = vld1_u8_x2(prev);

        uint8x8_t a0 = a.val[0];
        uint8x8_t b0 = b.val[0];
        uint8x8_t a1 = vext_u8(a.val[1], a.val[1], 1);
        uint8x8_t b1 = vext_u8(b.val[1], b.val[1], 1);
        uint8x8_t a3 = vext_u8(a.val[0], a.val[1], 3);
        uint8x8_t b3 = vext_u8(b.val[0], b.val[1], 3);
        uint8x8_t a6 = vext_u8(a.val[0], a.val[1], 6);
        uint8x8_t b6 = vext_u8(b.val[0], b.val[1], 6);

        value.val[0] = paeth(value.val[3], b0, last, a0);
        value.val[1] = paeth(value.val[0], b3, b0, a3);
        value.val[2] = paeth(value.val[1], b6, b3, a6);
        value.val[3] = paeth(value.val[2], b1, b6, a1);

        last = b1;

        const uint8x8_t idx0 = { 0, 1, 2, 8, 9, 10, 16, 17 };
        const uint8x8_t idx1 = { 18, 24, 25, 26, 255, 255, 255, 255 };

        uint8x8_t v0 = vtbl4_u8(value, idx0);
        uint8x8_t v1 = vtbl4_u8(value, idx1);

        store12(scan, v0, v1);
    }

    static inline
    void paeth16(u8* scan, const u8* prev, uint8x8x4_t& value, uint8x8_t& last)
    {
        uint32x2x4_t tmp;

        tmp = vld4_lane_u32(reinterpret_cast<u32 *>(scan), tmp, 0);
        uint8x8x4_t a = *(uint8x8x4_t*) &tmp; // !!!

        tmp = vld4_lane_u32(reinterpret_cast<const u32 *>(prev), tmp, 0);
        uint8x8x4_t b = *(uint8x8x4_t *) &tmp; // !!!

        value.val[0] = paeth(value.val[3], b.val[0],     last, a.val[0]);
        value.val[1] = paeth(value.val[0], b.val[1], b.val[0], a.val[1]);
        value.val[2] = paeth(value.val[1], b.val[2], b.val[1], a.val[2]);
        value.val[3] = paeth(value.val[2], b.val[3], b.val[2], a.val[3]);

        last = b.val[3];

        const uint8x8_t idx0 = { 0, 1, 2, 3, 8, 9, 10, 11 };
        const uint8x8_t idx1 = { 16, 17, 18, 19, 24, 25, 26, 27 };

        uint8x8_t v0 = vtbl4_u8(value, idx0);
        uint8x8_t v1 = vtbl4_u8(value, idx1);
        vst1q_u8(scan, vcombine_u8(v0, v1));
    }

    // filters

    void filter1_sub_24bit_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        uint8x8_t last = vdup_n_u8(0);

        while (bytes >= 12)
        {
            sub12(scan, last);
            scan += 12;
            bytes -= 12;
        }

        if (bytes > 0)
        {
            u8 temp[16];
            std::memcpy(temp, scan, bytes);

            sub12(temp, last);
            std::memcpy(scan, temp, bytes);
        }
    }

    void filter1_sub_32bit_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        uint8x16_t last = vdupq_n_u8(0);

        while (bytes >= 16)
        {
            sub16(scan, last);
            scan += 16;
            bytes -= 16;
        }

        if (bytes > 0)
        {
            u8 temp[16];
            std::memcpy(temp, scan, bytes);

            sub16(temp, last);
            std::memcpy(scan, temp, bytes);
        }
    }

    void filter2_up_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        while (bytes >= 16)
        {
            uint8x16_t a = vld1q_u8(scan);
            uint8x16_t b = vld1q_u8(prev);
            vst1q_u8(scan, vaddq_u8(a, b));
            scan += 16;
            prev += 16;
            bytes -= 16;
        }

        for (int x = 0; x < bytes; ++x)
        {
            scan[x] += prev[x];
        }
    }

    void filter3_average_24bit_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        uint8x8_t last = vdup_n_u8(0);

        while (bytes >= 12)
        {
            average12(scan, prev, last);
            scan += 12;
            prev += 12;
            bytes -= 12;
        }

        if (bytes > 0)
        {
            u8 temp[16];
            std::memcpy(temp, scan, bytes);

            average16(temp, prev, last);
            std::memcpy(scan, temp, bytes);
        }
    }

    void filter3_average_32bit_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        uint8x8_t last = vdup_n_u8(0);

        while (bytes >= 16)
        {
            average16(scan, prev, last);
            scan += 16;
            prev += 16;
            bytes -= 16;
        }

        if (bytes > 0)
        {
            u8 temp[16];
            std::memcpy(temp, scan, bytes);

            average16(temp, prev, last);
            std::memcpy(scan, temp, bytes);
        }
    }

    void filter4_paeth_24bit_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        uint8x8_t last = vdup_n_u8(0);

        uint8x8x4_t value;
        value.val[3] = vdup_n_u8(0);

        while (bytes >= 12)
        {
            paeth12(scan, prev, value, last);
            scan += 12;
            prev += 12;
            bytes -= 12;
        }

        if (bytes > 0)
        {
            u8 temp[16];
            std::memcpy(temp, scan, bytes);

            paeth12(temp, prev, value, last);
            std::memcpy(scan, temp, bytes);
        }
    }

    void filter4_paeth_32bit_neon(u8* scan, const u8* prev, int bytes, int bpp)
    {
        uint8x8_t last = vdup_n_u8(0);

        uint8x8x4_t value;
        value.val[3] = vdup_n_u8(0);

        while (bytes >= 16)
        {
            paeth16(scan, prev, value, last);
            scan += 16;
            prev += 16;
            bytes -= 16;
        }

        if (bytes > 0)
        {
            u8 temp[16];
            std::memcpy(temp, scan, bytes);

            paeth16(temp, prev, value, last);
            std::memcpy(scan, temp, bytes);
        }
    }

#endif // MANGO_ENABLE_NEON

    struct FilterDispatcher
    {
        FilterFunc sub = filter1_sub;
        FilterFunc up = filter2_up;
        FilterFunc average = filter3_average;
        FilterFunc paeth = filter4_paeth;
        int bpp = 0;

        FilterDispatcher(int bpp)
            : bpp(bpp)
        {
            u64 features = getCPUFlags();

            switch (bpp)
            {
                case 1:
                    paeth = filter4_paeth_8bit;
                    break;
                case 3:
#if defined(MANGO_ENABLE_SSE2)
                    sub = filter1_sub_24bit_sse2;
                    average = filter3_average_24bit_sse2;
                    paeth = filter4_paeth_24bit_sse2;
#endif
#if defined(MANGO_ENABLE_SSE4_1)
                    if (features & INTEL_SSE4_1)
                    {
                        sub = filter1_sub_24bit_sse41;
                    }
#endif
#if defined(MANGO_ENABLE_NEON)
                    if (features & ARM_NEON)
                    {
                        sub = filter1_sub_24bit_neon;
                        average = filter3_average_24bit_neon;
                        paeth = filter4_paeth_24bit_neon;
                    }
#endif
                    break;
                case 4:
#if defined(MANGO_ENABLE_SSE2)
                    sub = filter1_sub_32bit_sse2;
                    average = filter3_average_32bit_sse2;
                    paeth = filter4_paeth_32bit_sse2;
#endif
#if defined(MANGO_ENABLE_NEON)
                    if (features & ARM_NEON)
                    {
                        sub = filter1_sub_32bit_neon;
                        average = filter3_average_32bit_neon;
                        paeth = filter4_paeth_32bit_neon;
                    }
#endif
                    break;
            }

#if defined(MANGO_ENABLE_SSE2)
            up = filter2_up_sse2;
#endif
#if defined(MANGO_ENABLE_NEON)
            if (features & ARM_NEON)
            {
                up = filter2_up_neon;
            }
#endif

            MANGO_UNREFERENCED(features);
        }

        void operator () (u8* scan, const u8* prev, int bytes) const
        {
            FilterType method = FilterType(scan[0]);
            //printf("%d", method);

            // skip filter byte
            ++scan;
            ++prev;
            --bytes;

            switch (method)
            {
                case FILTER_NONE:
                default:
                    break;

                case FILTER_SUB:
                    sub(scan, prev, bytes, bpp);
                    break;

                case FILTER_UP:
                    up(scan, prev, bytes, bpp);
                    break;

                case FILTER_AVERAGE:
                    average(scan, prev, bytes, bpp);
                    break;

                case FILTER_PAETH:
                    paeth(scan, prev, bytes, bpp);
                    break;
            }
        }
    };

    // ------------------------------------------------------------
    // color conversion
    // ------------------------------------------------------------

    struct ColorState
    {
        using Function = void (*)(const ColorState& state, int width, u8* dst, const u8* src);

        int bits;

        // tRNS
        bool transparent_enable = false;
        u16 transparent_sample[3];
        Color transparent_color;

        Color* palette = nullptr;
    };

    void process_pal1to4_indx(const ColorState& state, int width, u8* dst, const u8* src)
    {
        const int bits = state.bits;
        const u32 mask = (1 << bits) - 1;

        u32 data = 0;
        int offset = -1;

        for (int x = 0; x < width; ++x)
        {
            if (offset < 0)
            {
                offset = 8 - bits;
                data = *src++;
            }

            u32 index = (data >> offset) & mask;
            dst[x] = index;
            offset -= bits;
        }
    }

    void process_pal1to4(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u32* dest = reinterpret_cast<u32*>(dst);

        Color* palette = state.palette;

        const int bits = state.bits;
        const u32 mask = (1 << bits) - 1;

        u32 data = 0;
        int offset = -1;

        for (int x = 0; x < width; ++x)
        {
            if (offset < 0)
            {
                offset = 8 - bits;
                data = *src++;
            }

            u32 index = (data >> offset) & mask;
            dest[x] = palette[index];
            offset -= bits;
        }
    }

    void process_pal8_indx(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        std::memcpy(dst, src, width);
    }

    void process_pal8(const ColorState& state, int width, u8* dst, const u8* src)
    {
        u32* dest = reinterpret_cast<u32*>(dst);

        Color* palette = state.palette;

        for (int x = 0; x < width; ++x)
        {
            dest[x] = palette[src[x]];
        }
    }

    void process_i1to4(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u8* dest = dst;

        const bool transparent_enable = state.transparent_enable;
        const u16 transparent_sample = state.transparent_sample[0];

        const int bits = state.bits;
        const int maxValue = (1 << bits) - 1;
        const int scale = (255 / maxValue) * 0x010101;
        const u32 mask = (1 << bits) - 1;

        u32 data = 0;
        int offset = -1;

        for (int x = 0; x < width; ++x)
        {
            if (offset < 0)
            {
                offset = 8 - bits;
                data = *src++;
            }

            int value = (data >> offset) & mask;
            *dest++ = value * scale;

            if (transparent_enable)
            {
                *dest++ = (value == transparent_sample) ? 0 : 0xff;
            }

            offset -= bits;
        }
    }

    void process_i8(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        std::memcpy(dst, src, width);
    }

    void process_i8_trns(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        const u16 transparent_sample = state.transparent_sample[0];

        for (int x = 0; x < width; ++x)
        {
            const u16 alpha = transparent_sample == src[x] ? 0 : 0xff00;
            dest[x] = alpha | src[x];
        }
    }

    void process_i16(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        for (int x = 0; x < width; ++x)
        {
            dest[x] = uload16be(src);
            src += 2;
        }
    }

    void process_i16_trns(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        const u16 transparent_sample = state.transparent_sample[0];

        for (int x = 0; x < width; ++x)
        {
            u16 gray = uload16be(src);
            u16 alpha = (transparent_sample == gray) ? 0 : 0xffff;
            dest[0] = gray;
            dest[1] = alpha;
            dest += 2;
            src += 2;
        }
    }

    void process_ia8(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        std::memcpy(dst, src, width * 2);
    }

    void process_ia16(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        for (int x = 0; x < width; ++x)
        {
            dest[0] = uload16be(src + 0);
            dest[1] = uload16be(src + 2);
            dest += 2;
            src += 4;
        }
    }

    void process_rgb8(const ColorState& state, int width, u8* dst, const u8* src)
    {
        // SIMD: SSSE3, NEON
        MANGO_UNREFERENCED(state);

        u32* dest = reinterpret_cast<u32*>(dst);

        while (width >= 4)
        {
            u32 v0 = uload32(src + 0); // R0, G0, B0, R1
            u32 v1 = uload32(src + 4); // G1, B1, R2, G2
            u32 v2 = uload32(src + 8); // B2, R3, G3, B3
            dest[0] = 0xff000000 | v0;
            dest[1] = 0xff000000 | (v0 >> 24) | (v1 << 8);
            dest[2] = 0xff000000 | (v1 >> 16) | (v2 << 16);
            dest[3] = 0xff000000 | (v2 >> 8);
            src += 12;
            dest += 4;
            width -= 4;
        }

        for (int x = 0; x < width; ++x)
        {
            dest[x] = Color(src[0], src[1], src[2], 0xff);
            src += 3;
        }
    }

    void process_rgb8_trns(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u32* dest = reinterpret_cast<u32*>(dst);

        const Color transparent_color = state.transparent_color;

        for (int x = 0; x < width; ++x)
        {
            Color color(src[0], src[1], src[2], 0xff);
            if (color == transparent_color)
            {
                color.a = 0;
            }
            dest[x] = color;
            src += 3;
        }
    }

    void process_rgb16(const ColorState& state, int width, u8* dst, const u8* src)
    {
        // SIMD: NEON
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        for (int x = 0; x < width; ++x)
        {
            dest[0] = uload16be(src + 0);
            dest[1] = uload16be(src + 2);
            dest[2] = uload16be(src + 4);
            dest[3] = 0xffff;
            dest += 4;
            src += 6;
        }
    }

    void process_rgb16_trns(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        const u16 transparent_sample0 = state.transparent_sample[0];
        const u16 transparent_sample1 = state.transparent_sample[1];
        const u16 transparent_sample2 = state.transparent_sample[2];

        for (int x = 0; x < width; ++x)
        {
            u16 red   = uload16be(src + 0);
            u16 green = uload16be(src + 2);
            u16 blue  = uload16be(src + 4);
            u16 alpha = 0xffff;
            if (transparent_sample0 == red &&
                transparent_sample1 == green &&
                transparent_sample2 == blue) alpha = 0;
            dest[0] = red;
            dest[1] = green;
            dest[2] = blue;
            dest[3] = alpha;
            dest += 4;
            src += 6;
        }
    }

    void process_rgba8(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        std::memcpy(dst, src, width * 4);
    }

    void process_rgba16(const ColorState& state, int width, u8* dst, const u8* src)
    {
        // SIMD: SSE2, SSSE3, NEON
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        for (int x = 0; x < width; ++x)
        {
            dest[0] = uload16be(src + 0);
            dest[1] = uload16be(src + 2);
            dest[2] = uload16be(src + 4);
            dest[3] = uload16be(src + 6);
            dest += 4;
            src += 8;
        }
    }

#if defined(MANGO_ENABLE_SSE2)

    void process_rgba16_sse2(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        while (width >= 4)
        {
            __m128i v0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 0));
            __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 16));
            __m128i a0 = _mm_slli_epi16(v0, 8);
            __m128i b0 = _mm_srli_epi16(v0, 8);
            __m128i a1 = _mm_slli_epi16(v1, 8);
            __m128i b1 = _mm_srli_epi16(v1, 8);
            v0 = _mm_or_si128(a0, b0);
            v1 = _mm_or_si128(a1, b1);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest + 0), v0);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest + 8), v1);
            src += 32;
            dest += 16;
            width -= 4;
        }

        for (int x = 0; x < width; ++x)
        {
            dest[0] = uload16be(src + 0);
            dest[1] = uload16be(src + 2);
            dest[2] = uload16be(src + 4);
            dest[3] = uload16be(src + 6);
            dest += 4;
            src += 8;
        }
    }

#endif // MANGO_ENABLE_SSE2

#if defined(MANGO_ENABLE_SSSE3)

    void process_rgba16_ssse3(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        __m128i mask = _mm_setr_epi8(1, 0, 3, 2, 5, 4, 7, 6, 9, 8, 11, 10, 13, 12, 15, 14);

        while (width >= 4)
        {
            __m128i v0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 0));
            __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 16));
            v0 = _mm_shuffle_epi8(v0, mask);
            v1 = _mm_shuffle_epi8(v1, mask);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest + 0), v0);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest + 8), v1);
            src += 32;
            dest += 16;
            width -= 4;
        }

        for (int x = 0; x < width; ++x)
        {
            dest[0] = uload16be(src + 0);
            dest[1] = uload16be(src + 2);
            dest[2] = uload16be(src + 4);
            dest[3] = uload16be(src + 6);
            dest += 4;
            src += 8;
        }
    }

    void process_rgb8_ssse3(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u32* dest = reinterpret_cast<u32*>(dst);

        __m128i mask = _mm_setr_epi8(0, 1, 2, -1, 3, 4, 5, -1, 6, 7, 8, -1, 9, 10, 11, -1);
        __m128i alpha = _mm_set1_epi32(0xff000000);

        while (width >= 16)
        {
            __m128i v0 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 0 * 16));
            __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 1 * 16));
            __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i *>(src + 2 * 16));
            __m128i color0 = _mm_shuffle_epi8(v0, mask);
            __m128i color1 = _mm_shuffle_epi8(_mm_alignr_epi8(v1, v0, 12), mask);
            __m128i color2 = _mm_shuffle_epi8(_mm_alignr_epi8(v2, v1,  8), mask);
            __m128i color3 = _mm_shuffle_epi8(_mm_alignr_epi8(v2, v2,  4), mask);
            color0 = _mm_or_si128(color0, alpha);
            color1 = _mm_or_si128(color1, alpha);
            color2 = _mm_or_si128(color2, alpha);
            color3 = _mm_or_si128(color3, alpha);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest +  0), color0);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest +  4), color1);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest +  8), color2);
            _mm_storeu_si128(reinterpret_cast<__m128i *>(dest + 12), color3);
            src += 48;
            dest += 16;
            width -= 16;
        }

        for (int x = 0; x < width; ++x)
        {
            dest[x] = Color(src[0], src[1], src[2], 0xff);
            src += 3;
        }
    }

#endif // MANGO_ENABLE_SSSE3

#if defined(MANGO_ENABLE_NEON)

    void process_rgb8_neon(const ColorState& state, int width, u8* dest, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        while (width >= 8)
        {
            const uint8x8x3_t rgb = vld3_u8(src);
            uint8x8x4_t rgba;
            rgba.val[0] = rgb.val[0];
            rgba.val[1] = rgb.val[1];
            rgba.val[2] = rgb.val[2];
            rgba.val[3] = vdup_n_u8(0xff);
            vst4_u8(dest, rgba);
            src += 24;
            dest += 32;
            width -= 8;
        }

        while (width-- > 0)
        {
            dest[0] = src[0];
            dest[1] = src[1];
            dest[2] = src[2];
            dest[3] = 0xff;
            src += 3;
            dest += 4;
        }
    }

    void process_rgb16_neon(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        u16* dest = reinterpret_cast<u16*>(dst);

        while (width >= 4)
        {
            const uint16x4x3_t rgb = vld3_u16(reinterpret_cast<const u16*>(src));
            uint16x4x4_t rgba;
            rgba.val[0] = vrev16_u8(rgb.val[0]);
            rgba.val[1] = vrev16_u8(rgb.val[1]);
            rgba.val[2] = vrev16_u8(rgb.val[2]);
            rgba.val[3] = vdup_n_u16(0xffff);
            vst4_u16(dest, rgba);
            src += 24;
            dest += 16;
            width -= 4;
        }

        while (width-- > 0)
        {
            dest[0] = uload16be(src + 0);
            dest[1] = uload16be(src + 2);
            dest[2] = uload16be(src + 4);
            dest[3] = 0xffff;
            src += 6;
            dest += 4;
        }
    }

    void process_rgba16_neon(const ColorState& state, int width, u8* dst, const u8* src)
    {
        MANGO_UNREFERENCED(state);

        const u16* source = reinterpret_cast<const u16*>(src);
        u16* dest = reinterpret_cast<u16*>(dst);

        while (width >= 2)
        {
            uint16x8_t a = vld1q_u16(source);
            a = vrev16q_u8(a);
            vst1q_u16(dest, a);
            source += 8;
            dest += 8;
            width -= 2;
        }

        if (width > 0)
        {
            uint16x4_t a = vld1_u16(source);
            a = vrev16_u8(a);
            vst1_u16(dest, a);
        }
    }

#endif // MANGO_ENABLE_NEON

    ColorState::Function getColorFunction(const ColorState& state, int color_type, int bit_depth)
    {
        ColorState::Function function = nullptr;
        u64 features = getCPUFlags();

        if (color_type == COLOR_TYPE_PALETTE)
        {
            if (bit_depth < 8)
            {
                if (state.palette)
                {
                    function = process_pal1to4;
                }
                else
                {
                    function = process_pal1to4_indx;
                }
            }
            else
            {
                if (state.palette)
                {
                    function = process_pal8;
                }
                else
                {
                    function = process_pal8_indx;
                }
            }
        }
        else if (color_type == COLOR_TYPE_I)
        {
            if (bit_depth < 8)
            {
                function = process_i1to4;
            }
            else if (bit_depth == 8)
            {
                if (state.transparent_enable)
                {
                    function = process_i8_trns;
                }
                else
                {
                    function = process_i8;
                }
            }
            else
            {
                if (state.transparent_enable)
                {
                    function = process_i16_trns;
                }
                else
                {
                    function = process_i16;
                }
            }
        }
        else if (color_type == COLOR_TYPE_IA)
        {
            if (bit_depth == 8)
            {
                function = process_ia8;
            }
            else
            {
                function = process_ia16;
            }
        }
        else if (color_type == COLOR_TYPE_RGB)
        {
            if (bit_depth == 8)
            {
                if (state.transparent_enable)
                {
                    function = process_rgb8_trns;
                }
                else
                {
                    function = process_rgb8;
#if defined(MANGO_ENABLE_SSSE3)
                    if (features & INTEL_SSSE3)
                    {
                        function = process_rgb8_ssse3;
                    }
#endif
#if defined(MANGO_ENABLE_NEON)
                    if (features & ARM_NEON)
                    {
                        function = process_rgb8_neon;
                    }
#endif
                }
            }
            else
            {
                if (state.transparent_enable)
                {
                    function = process_rgb16_trns;
                }
                else
                {
                    function = process_rgb16;
#if defined(MANGO_ENABLE_NEON)
                    if (features & ARM_NEON)
                    {
                        function = process_rgb16_neon;
                    }
#endif
                }
            }
        }
        else if (color_type == COLOR_TYPE_RGBA)
        {
            if (bit_depth == 8)
            {
                function = process_rgba8;
            }
            else
            {
                function = process_rgba16;
#if defined(MANGO_ENABLE_SSE2)
                if (features & INTEL_SSE2)
                {
                    function = process_rgba16_sse2;
                }
#endif
#if defined(MANGO_ENABLE_SSSE3)
                if (features & INTEL_SSSE3)
                {
                    function = process_rgba16_ssse3;
                }
#endif
#if defined(MANGO_ENABLE_NEON)
                if (features & ARM_NEON)
                {
                    function = process_rgba16_neon;
                }
#endif
            }
        }

        MANGO_UNREFERENCED(features);
        return function;
    }

    // ------------------------------------------------------------
    // AdamInterleave
    // ------------------------------------------------------------

    struct AdamInterleave
    {
        int xorig;
        int yorig;
        int xspc;
        int yspc;
        int w;
        int h;

        AdamInterleave(u32 pass, u32 width, u32 height)
        {
            assert(pass >=0 && pass < 7);

            const u32 orig = 0x01020400 >> (pass * 4);
            const u32 spc = 0x01122333 >> (pass * 4);

            xorig = (orig >> 4) & 15;
            yorig = (orig >> 0) & 15;
            xspc = (spc >> 4) & 15;
            yspc = (spc >> 0) & 15;
            w = (width - xorig + (1 << xspc) - 1) >> xspc;
            h = (height - yorig + (1 << yspc) - 1) >> yspc;
        }
    };

    // ------------------------------------------------------------
    // ParserPNG
    // ------------------------------------------------------------

    struct Chromaticity
    {
        float32x2 white;
        float32x2 red;
        float32x2 green;
        float32x2 blue;
    };

    struct Frame
    {
        enum Dispose : u8
        {
            NONE       = 0,
            BACKGROUND = 1,
            PREVIOUS   = 2,
        };

        enum Blend : u8
        {
            SOURCE = 0,
            OVER   = 1,
        };

        u32 sequence_number;
        u32 width;
        u32 height;
        u32 xoffset;
        u32 yoffset;
        u16 delay_num;
        u16 delay_den;
        u8 dispose;
        u8 blend;

        void read(BigEndianConstPointer p)
        {
            sequence_number = p.read32();
            width = p.read32();
            height = p.read32();
            xoffset = p.read32();
            yoffset = p.read32();
            delay_num = p.read16();
            delay_den = p.read16();
            dispose = p.read8();
            blend = p.read8();

            debugPrint("  Sequence: %d\n", sequence_number);
            debugPrint("  Frame: %d x %d (%d, %d)\n", width, height, xoffset, yoffset);
            debugPrint("  Time: %d / %d\n", delay_num, delay_den);
            debugPrint("  Dispose: %d\n", dispose);
            debugPrint("  Blend: %d\n", blend);
        }
    };

    class ParserPNG
    {
    protected:
        ConstMemory m_memory;
        ImageHeader m_header;

        const u8* m_pointer = nullptr;
        const u8* m_end = nullptr;
        const char* m_error = nullptr;

        Buffer m_compressed;

        ColorState m_color_state;

        // IHDR
        int m_width;
        int m_height;
        int m_color_type;
        int m_compression;
        int m_filter;
        int m_interlace;

        int m_channels;

        // CgBI
        bool m_iphoneOptimized = false; // Apple's proprietary iphone optimized pngcrush

        // PLTE
        Palette m_palette;

        // cHRM
        Chromaticity m_chromaticity;

        // gAMA
        float m_gamma = 0.0f;

        // sBIT
        u8 m_scale_bits[4];

        // sRGB
        u8 m_srgb_render_intent = -1;

        // acTL
        u32 m_number_of_frames = 0;
        u32 m_repeat_count = 0;
        u32 m_current_frame_index = 0;
        u32 m_next_frame_index = 0;

        // fcTL
        Frame m_frame;
        const u8* m_first_frame = nullptr;

        // iCCP
        Buffer m_icc;

        // iDOT
        const u8* m_idot_address = nullptr;
        size_t m_idot_offset = 0;
        u32 m_first_half_height = 0;
        u32 m_second_half_height = 0;

        // pLLD
        u32 m_parallel_height = 0;
        u8 m_parallel_flags = 0;
        std::vector<ConstMemory> m_parallel_segments;

        void read_IHDR(BigEndianConstPointer p, u32 size);
        void read_IDAT(BigEndianConstPointer p, u32 size);
        void read_PLTE(BigEndianConstPointer p, u32 size);
        void read_tRNS(BigEndianConstPointer p, u32 size);
        void read_cHRM(BigEndianConstPointer p, u32 size);
        void read_gAMA(BigEndianConstPointer p, u32 size);
        void read_sBIT(BigEndianConstPointer p, u32 size);
        void read_sRGB(BigEndianConstPointer p, u32 size);
        void read_acTL(BigEndianConstPointer p, u32 size);
        void read_fcTL(BigEndianConstPointer p, u32 size);
        void read_fdAT(BigEndianConstPointer p, u32 size);
        void read_iCCP(BigEndianConstPointer p, u32 size);
        void read_iDOT(BigEndianConstPointer p, u32 size);
        void read_pLLD(BigEndianConstPointer p, u32 size);

        void parse();

        void blend_ia8      (u8* dest, const u8* src, int width);
        void blend_ia16     (u8* dest, const u8* src, int width);
        void blend_bgra8    (u8* dest, const u8* src, int width);
        void blend_rgba16   (u8* dest, const u8* src, int width);
        void blend_indexed  (u8* dest, const u8* src, int width);

        void deinterlace1to4(u8* output, int width, int height, size_t stride, u8* buffer);
        void deinterlace8(u8* output, int width, int height, size_t stride, u8* buffer);
        void filter(u8* buffer, int bytes, int height);
        void process_range(u8* image, u8* buffer, size_t stride, int width, const FilterDispatcher& filter, int y0, int y1);
        void process(u8* dest, int width, int height, size_t stride, u8* buffer, bool multithread);

        void blend(Surface& d, Surface& s, Palette* palette);

        int getBytesPerLine(int width) const
        {
            return m_channels * ((m_color_state.bits * width + 7) / 8);
        }

        void setError(const std::string& error)
        {
            m_header.info = "[ImageDecoder.PNG] ";
            m_header.info += error;
            m_header.success = false;
        }

        int getImageBufferSize(int width, int height) const;

    public:
        ParserPNG(ConstMemory memory);
        ~ParserPNG();

        const ImageHeader& getHeader();
        ImageDecodeStatus decode(const Surface& dest, bool multithread, Palette* palette);

        ConstMemory icc()
        {
            return m_icc;
        }

    };

    // ------------------------------------------------------------
    // ParserPNG
    // ------------------------------------------------------------

    ParserPNG::ParserPNG(ConstMemory memory)
        : m_memory(memory)
        , m_end(memory.address + memory.size)
    {
        BigEndianConstPointer p = memory.address;

        // read header magic
        const u64 magic = p.read64();
        if (magic != PNG_HEADER_MAGIC)
        {
            setError("Incorrect header magic.");
            return;
        }

        // read first chunk; in standard it is IHDR, but apple has CgBI+IHDR
        u32 size = p.read32();
        u32 id = p.read32();

        if (id == u32_mask_rev('C', 'g', 'B', 'I'))
        {
            debugPrint("CgBI: reading PNG as iphone optimized");

            m_iphoneOptimized = true;
            p += size; // skip chunk data
            p += sizeof(u32); // skip crc

            // next chunk
            size = p.read32();
            id = p.read32();
        }

        if (id != u32_mask_rev('I', 'H', 'D', 'R'))
        {
            setError("Incorrect file; the IHDR chunk must come first.");
            return;
        }

        read_IHDR(p, size);
        p += size; // skip chunk data
        p += sizeof(u32); // skip crc

        // keep track of parsing position
        m_pointer = p;
    }

    ParserPNG::~ParserPNG()
    {
    }

    const ImageHeader& ParserPNG::getHeader()
    {
        if (m_header.success)
        {
            m_header.width   = m_width;
            m_header.height  = m_height;
            m_header.depth   = 0;
            m_header.levels  = 0;
            m_header.faces   = 0;
            m_header.palette = false;
            m_header.compression = TextureCompression::NONE;
            m_header.premultiplied = m_iphoneOptimized; // apple pngcrush premultiplies alpha

            // force alpha channel on when transparency is enabled
            int color_type = m_color_type;
            if (m_color_state.transparent_enable && color_type != COLOR_TYPE_PALETTE)
            {
                color_type |= 4;
            }

            // select decoding format
            int bits = m_color_state.bits <= 8 ? 8 : 16;
            switch (color_type)
            {
                case COLOR_TYPE_I:
                    m_header.format = LuminanceFormat(bits, Format::UNORM, bits, 0);
                    break;

                case COLOR_TYPE_IA:
                    m_header.format = LuminanceFormat(bits * 2, Format::UNORM, bits, bits);
                    break;

                case COLOR_TYPE_PALETTE:
                    m_header.format = Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8);
                    m_header.palette = true;
                    break;

                case COLOR_TYPE_RGB:
                case COLOR_TYPE_RGBA:
                    m_header.format = Format(bits * 4, Format::UNORM, m_iphoneOptimized ? Format::BGRA : Format::RGBA, bits, bits, bits, bits);
                    break;
            }
        }

        return m_header;
    }

    void ParserPNG::read_IHDR(BigEndianConstPointer p, u32 size)
    {
        debugPrint("[\"IHDR\"] %d bytes\n", size);

        if (size != 13)
        {
            setError("Incorrect IHDR chunk size.");
            return;
        }

        m_width = p.read32();
        m_height = p.read32();
        m_color_state.bits = p.read8();
        m_color_type = p.read8();
        m_compression = p.read8();
        m_filter = p.read8();
        m_interlace = p.read8();

        if (m_width <= 0 || m_height <= 0)
        {
            setError(makeString("Incorrect dimensions (%d x %d).", m_width, m_height));
            return;
        }

        if (!u32_is_power_of_two(m_color_state.bits))
        {
            setError(makeString("Incorrect bit depth (%d).", m_color_state.bits));
            return;
        }

        // look-ahead into the chunks to see if we have transparency information
        p += 4; // skip crc
        for ( ; p < m_end - 8; )
        {
            const u32 size = p.read32();
            const u32 id = p.read32();
            switch (id)
            {
                case u32_mask_rev('t', 'R', 'N', 'S'):
                    if (m_color_type < 4)
                    {
                        // Enable color keying only for valid color types
                        m_color_state.transparent_enable = true;
                    }
                    break;
            }
            p += (size + 4);
        }

        // bit-depth range defaults
        int minBits = 3; // log2(8) bits
        int maxBits = 4; // log2(16) bits

        switch (m_color_type)
        {
            case COLOR_TYPE_I:
                // supported: 1, 2, 4, 8, 16 bits
                minBits = 0;
                m_channels = 1;
                break;
            case COLOR_TYPE_RGB:
                // supported: 8, 16 bits
                m_channels = 3;
                break;
            case COLOR_TYPE_PALETTE:
                // supported: 1, 2, 4, 8 bits
                minBits = 0;
                maxBits = 3;
                m_channels = 1;
                break;
            case COLOR_TYPE_IA:
                // supported: 8, 16 bits
                m_channels = 2;
                break;
            case COLOR_TYPE_RGBA:
                // supported: 8, 16 bits
                m_channels = 4;
                break;
            default:
                setError(makeString("Incorrect color type (%d).", m_color_type));
                return;
        }

        const int log2bits = u32_log2(m_color_state.bits);
        if (log2bits < minBits || log2bits > maxBits)
        {
            setError(makeString("Unsupported bit depth for color type (%d).", m_color_state.bits));
            return;
        }

        // load default scaling values (override from sBIT chunk)
        for (int i = 0; i < m_channels; ++i)
        {
            m_scale_bits[i] = m_color_state.bits;
        }

        debugPrint("  Image: (%d x %d), %d bits\n", m_width, m_height, m_color_state.bits);
        debugPrint("  Color:       %s\n", get_string(ColorType(m_color_type)));
        debugPrint("  Compression: %d\n", m_compression);
        debugPrint("  Filter:      %d\n", m_filter);
        debugPrint("  Interlace:   %d\n", m_interlace);
    }

    void ParserPNG::read_IDAT(BigEndianConstPointer p, u32 size)
    {
        if (m_parallel_height)
        {
            m_parallel_segments.emplace_back(p, size);
        }
        else
        {
            if (p == m_idot_address)
            {
                m_idot_offset = m_compressed.size();
            }

            m_compressed.append(p, size);
        }
    }

    void ParserPNG::read_PLTE(BigEndianConstPointer p, u32 size)
    {
        if (size % 3)
        {
            setError("Incorrect palette size.");
            return;
        }

        if (size > 768)
        {
            setError("Too large palette.");
            return;
        }

        m_palette.size = size / 3;
        for (u32 i = 0; i < m_palette.size; ++i)
        {
            m_palette[i] = Color(p[0], p[1], p[2], 0xff);
            p += 3;
        }
    }

    void ParserPNG::read_tRNS(BigEndianConstPointer p, u32 size)
    {
        if (m_color_type == COLOR_TYPE_I)
        {
            if (size != 2)
            {
                setError("Incorrect tRNS chunk size.");
                return;
            }

            m_color_state.transparent_enable = true;
            m_color_state.transparent_sample[0] = p.read16();
        }
        else if (m_color_type == COLOR_TYPE_RGB)
        {
            if (size != 6)
            {
                setError("Incorrect tRNS chunk size.");
                return;
            }

            m_color_state.transparent_enable = true;
            m_color_state.transparent_sample[0] = p.read16();
            m_color_state.transparent_sample[1] = p.read16();
            m_color_state.transparent_sample[2] = p.read16();
            m_color_state.transparent_color = Color(m_color_state.transparent_sample[0] & 0xff,
                                                    m_color_state.transparent_sample[1] & 0xff,
                                                    m_color_state.transparent_sample[2] & 0xff, 0xff);
        }
        else if (m_color_type == COLOR_TYPE_PALETTE)
        {
            if (m_palette.size < u32(size))
            {
                setError("Incorrect alpha palette size.");
                return;
            }

            for (u32 i = 0; i < size; ++i)
            {
                m_palette[i].a = p[i];
            }
        }
        else
        {
            // From the specification, ISO/IEC 15948:2003 ;
            // "A tRNS chunk shall not appear for colour types 4 and 6, since a full alpha channel is already present in those cases."
            //
            // Of course, some software apparently writes png files with color_type=RGBA which have a tRNS chunk
            // so the following validation check is disabled because the users expect the files to "work"

            //setError("Incorrect color type for alpha palette.");
        }
    }

    void ParserPNG::read_cHRM(BigEndianConstPointer p, u32 size)
    {
        if (size != 32)
        {
            setError("Incorrect cHRM chunk size.");
            return;
        }

        const float scale = 100000.0f;
        m_chromaticity.white.x = p.read32() / scale;
        m_chromaticity.white.y = p.read32() / scale;
        m_chromaticity.red.x   = p.read32() / scale;
        m_chromaticity.red.y   = p.read32() / scale;
        m_chromaticity.green.x = p.read32() / scale;
        m_chromaticity.green.y = p.read32() / scale;
        m_chromaticity.blue.x  = p.read32() / scale;
        m_chromaticity.blue.y  = p.read32() / scale;
    }

    void ParserPNG::read_gAMA(BigEndianConstPointer p, u32 size)
    {
        if (size != 4)
        {
            setError("Incorrect gAMA chunk size.");
            return;
        }

        m_gamma = p.read32() / 100000.0f;
    }

    void ParserPNG::read_sBIT(BigEndianConstPointer p, u32 size)
    {
        if (size > 4)
        {
            setError("Incorrect sBIT chunk size.");
            return;
        }

        for (u32 i = 0; i < size; ++i)
        {
            m_scale_bits[i] = p[i];
        }
    }

    void ParserPNG::read_sRGB(BigEndianConstPointer p, u32 size)
    {
        if (size != 1)
        {
            setError("Incorrect sRGB chunk size.");
            return;
        }

        m_srgb_render_intent = p[0];
    }

    void ParserPNG::read_acTL(BigEndianConstPointer p, u32 size)
    {
        if (size != 8)
        {
            setError("Incorrect acTL chunk size.");
            return;
        }

        m_number_of_frames = p.read32();
        m_repeat_count = p.read32();

        debugPrint("  Frames: %d\n", m_number_of_frames);
        debugPrint("  Repeat: %d %s\n", m_repeat_count, m_repeat_count ? "" : "(infinite)");
    }

    void ParserPNG::read_fcTL(BigEndianConstPointer p, u32 size)
    {
        if (size != 26)
        {
            setError("Incorrect fcTL chunk size.");
            return;
        }

        if (!m_first_frame)
        {
            m_first_frame = p - 8;
        }

        m_frame.read(p);
    }

    void ParserPNG::read_fdAT(BigEndianConstPointer p, u32 size)
    {
        u32 sequence_number = p.read32();
        size -= 4;

        debugPrint("  Sequence: %d\n", sequence_number);
        MANGO_UNREFERENCED(sequence_number);

        m_compressed.append(p, size);
    }

    void ParserPNG::read_iCCP(BigEndianConstPointer p, u32 size)
    {
        /*
        Profile name:       1-79 bytes (character string)
        Null separator:     1 byte
        Compression method: 1 byte (0=deflate)
        Compressed profile: n bytes
        */
        const char* name = (const char*)&p[0];
        size_t name_len = png_strnlen(name, size);
        if (name_len == size)
        {
            debugPrint("iCCP: profile name not terminated\n");
            return;
        }

        debugPrint("  profile name '%s'\n", name);

        const u8* profile = p + name_len + 2;
        const size_t icc_bytes = size - name_len - 2;

        m_icc.reset();

        debugPrint("  decompressing icc profile %d bytes\n", int(icc_bytes));

        constexpr size_t max_profile_size = 1024 * 1024 * 2;
        Buffer buffer(max_profile_size);

        CompressionStatus status = deflate_zlib::decompress(buffer, ConstMemory(profile, icc_bytes));
        if (status)
        {
            debugPrint("  unpacked icc profile %d bytes\n", int(status.size));
            m_icc.append(buffer, status.size);
        }

        debugPrint("  icc profile %d bytes\n", int(m_icc.size()));
    }

    void ParserPNG::read_iDOT(BigEndianConstPointer p, u32 size)
    {
        if (size != 28)
        {
            setError("Incorrect iDOT chunk size.");
            return;
        }

        u32 height_divisor = p.read32();
        p += 4;
        u32 divided_height = p.read32();
        p += 4;

        if (height_divisor != 2)
        {
            setError("Unsupported height divisor.");
            return;
        }

        m_first_half_height = p.read32();
        m_second_half_height = p.read32();
        u32 idat_offset = p.read32();

        // Compute IDAT address (the offset is relative to iDOT chunk)
        BigEndianConstPointer x = p + idat_offset - 36;

        x += 4; // skip size field
        u32 id = x.read32();
        if (id == u32_mask_rev('I', 'D', 'A', 'T'))
        {
            m_idot_address = x;

            debugPrint("  First:  %d\n", m_first_half_height);
            debugPrint("  Second: %d\n", m_second_half_height);
            debugPrint("  Offset: %d\n", idat_offset);
        }

        MANGO_UNREFERENCED(divided_height);
    }

    void ParserPNG::read_pLLD(BigEndianConstPointer p, u32 size)
    {
        if (size != 5)
        {
            setError("Incorrect pLLD chunk size.");
            return;
        }

        m_parallel_height = p.read32();
        m_parallel_flags = p.read8();

        debugPrint("  Segment height: %d\n", m_parallel_height);
        debugPrint("  Flags: %x\n", m_parallel_flags);
    }

    void ParserPNG::parse()
    {
        BigEndianConstPointer p = m_pointer;

        for ( ; p < m_end - 8; )
        {
            const u32 size = p.read32();
            const u32 id = p.read32();

            const u8* ptr_next_chunk = p + size;
            ptr_next_chunk += 4; // skip crc

            debugPrint("[\"%c%c%c%c\"] %d bytes\n", (id >> 24), (id >> 16), (id >> 8), (id >> 0), size);

            // check that we won't read past end of file
            if (p + size + 4 > m_end)
            {
                setError("Corrupted file.\n");
                return;
            }

            switch (id)
            {
                case u32_mask_rev('I', 'H', 'D', 'R'):
                    setError("File can only have one IHDR chunk.");
                    break;

                case u32_mask_rev('P', 'L', 'T', 'E'):
                    read_PLTE(p, size);
                    break;

                case u32_mask_rev('t', 'R', 'N', 'S'):
                    read_tRNS(p, size);
                    break;

                case u32_mask_rev('g', 'A', 'M', 'A'):
                    read_gAMA(p, size);
                    break;

                case u32_mask_rev('s', 'B', 'I', 'T'):
                    read_sBIT(p, size);
                    break;

                case u32_mask_rev('s', 'R', 'G', 'B'):
                    read_sRGB(p, size);
                    break;

                case u32_mask_rev('c', 'H', 'R', 'M'):
                    read_cHRM(p, size);
                    break;

                case u32_mask_rev('I', 'D', 'A', 'T'):
                    read_IDAT(p, size);
                    if (m_number_of_frames > 0)
                    {
                        m_pointer = ptr_next_chunk;
                        return;
                    }
                    break;

                case u32_mask_rev('a', 'c', 'T', 'L'):
                    read_acTL(p, size);
                    break;

                case u32_mask_rev('f', 'c', 'T', 'L'):
                    read_fcTL(p, size);
                    break;

                case u32_mask_rev('f', 'd', 'A', 'T'):
                    read_fdAT(p, size);
                    m_pointer = ptr_next_chunk;
                    return;

                case u32_mask_rev('i', 'C', 'C', 'P'):
                    read_iCCP(p, size);
                    break;

                case u32_mask_rev('i', 'D', 'O', 'T'):
                    read_iDOT(p, size);
                    break;

                case u32_mask_rev('p', 'L', 'L', 'D'):
                    read_pLLD(p, size);
                    break;

                case u32_mask_rev('p', 'H', 'Y', 's'):
                case u32_mask_rev('b', 'K', 'G', 'D'):
                case u32_mask_rev('z', 'T', 'X', 't'):
                case u32_mask_rev('t', 'E', 'X', 't'):
                case u32_mask_rev('t', 'I', 'M', 'E'):
                case u32_mask_rev('i', 'T', 'X', 't'):
                    // NOTE: ignoring these chunks
                    break;

                case u32_mask_rev('I', 'E', 'N', 'D'):
                    if (m_number_of_frames > 0)
                    {
                        // reset current pointer to first animation frame (for looping)
                        ptr_next_chunk = m_first_frame;
                    }
                    else
                    {
                        // terminate parsing
                        m_pointer = m_end;
                        return;
                    }
                    break;

                default:
                    debugPrint("  # UNKNOWN: [\"%c%c%c%c\"] %d bytes\n", (id >> 24), (id >> 16), (id >> 8), (id >> 0), size);
                    break;
            }

            if (m_error)
            {
                // error occured while reading chunks
                return;
            }

            p = ptr_next_chunk;
        }
    }

    void ParserPNG::blend_ia8(u8* dest, const u8* src, int width)
    {
        for (int x = 0; x < width; ++x)
        {
            u8 alpha = src[1];
            u8 invalpha = 255 - alpha;
            dest[0] = src[0] + ((dest[0] - src[0]) * invalpha) / 255;
            dest[1] = alpha;
            src += 2;
            dest += 2;
        }
    }

    void ParserPNG::blend_ia16(u8* dest8, const u8* src8, int width)
    {
        u16* dest = reinterpret_cast<u16*>(dest8);
        const u16* src = reinterpret_cast<const u16*>(src8);
        for (int x = 0; x < width; ++x)
        {
            u16 alpha = src[1];
            u16 invalpha = 255 - alpha;
            dest[0] = src[0] + ((dest[0] - src[0]) * invalpha) / 255;
            dest[1] = alpha;
            src += 2;
            dest += 2;
        }
    }

    void ParserPNG::blend_bgra8(u8* dest, const u8* src, int width)
    {
        for (int x = 0; x < width; ++x)
        {
            u8 alpha = src[3];
            u8 invalpha = 255 - alpha;
            dest[0] = src[0] + ((dest[0] - src[0]) * invalpha) / 255;
            dest[1] = src[1] + ((dest[1] - src[1]) * invalpha) / 255;
            dest[2] = src[2] + ((dest[2] - src[2]) * invalpha) / 255;
            dest[3] = alpha;
            src += 4;
            dest += 4;
        }
    }

    void ParserPNG::blend_rgba16(u8* dest8, const u8* src8, int width)
    {
        u16* dest = reinterpret_cast<u16*>(dest8);
        const u16* src = reinterpret_cast<const u16*>(src8);
        for (int x = 0; x < width; ++x)
        {
            u16 alpha = src[3];
            u16 invalpha = 255 - alpha;
            dest[0] = src[0] + ((dest[0] - src[0]) * invalpha) / 65535;
            dest[1] = src[1] + ((dest[1] - src[1]) * invalpha) / 65535;
            dest[2] = src[2] + ((dest[2] - src[2]) * invalpha) / 65535;
            dest[3] = alpha;
            src += 4;
            dest += 4;
        }
    }

    void ParserPNG::blend_indexed(u8* dest, const u8* src, int width)
    {
        for (int x = 0; x < width; ++x)
        {
            u8 sample = src[x];
            Color color = m_palette[sample];
            if (color.a)
            {
                dest[x] = sample;
            }
        }
    }

    void ParserPNG::blend(Surface& d, Surface& s, Palette* palette)
    {
        int width = s.width;
        int height = s.height;
        if (m_frame.blend == Frame::SOURCE || !s.format.isAlpha())
        {
            d.blit(0, 0, s);
        }
        else if (m_frame.blend == Frame::OVER)
        {
            for (int y = 0; y < height; ++y)
            {
                const u8* src = s.address(0, y);
                u8* dest = d.address(0, y);

                if (palette)
                {
                    // palette extract mode: blend with the main image palette
                    blend_indexed(dest, src, width);
                }
                else
                {
                    switch (s.format.bits)
                    {
                        case 8:
                            blend_ia8(dest, src, width);
                            break;
                        case 16:
                            blend_ia16(dest, src, width);
                            break;
                        case 32:
                            blend_bgra8(dest, src, width);
                            break;
                        case 64:
                            blend_rgba16(dest, src, width);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    int ParserPNG::getImageBufferSize(int width, int height) const
    {
        int buffer_size = 0;

        if (m_interlace)
        {
            // NOTE: brute-force loop to resolve memory consumption
            for (int pass = 0; pass < 7; ++pass)
            {
                AdamInterleave adam(pass, width, height);
                if (adam.w && adam.h)
                {
                    buffer_size += (PNG_FILTER_BYTE + getBytesPerLine(adam.w)) * adam.h;
                }
            }
        }
        else
        {
            buffer_size = (PNG_FILTER_BYTE + getBytesPerLine(width)) * height;
        }

        return buffer_size;
    }

    void ParserPNG::deinterlace1to4(u8* output, int width, int height, size_t stride, u8* buffer)
    {
        const int samples = 8 / m_color_state.bits;
        const int mask = samples - 1;
        const int shift = u32_log2(samples);
        const int valueShift = u32_log2(m_color_state.bits);
        const int valueMask = (1 << m_color_state.bits) - 1;

        for (int pass = 0; pass < 7; ++pass)
        {
            AdamInterleave adam(pass, width, height);
            debugPrint("  pass: %d (%d x %d)\n", pass, adam.w, adam.h);

            const int bw = PNG_FILTER_BYTE + ((adam.w + mask) >> shift);
            filter(buffer, bw, adam.h);

            if (adam.w && adam.h)
            {
                for (int y = 0; y < adam.h; ++y)
                {
                    const int yoffset = (y << adam.yspc) + adam.yorig;
                    u8* dest = output + yoffset * stride + PNG_FILTER_BYTE;
                    u8* src = buffer + y * bw + PNG_FILTER_BYTE;

                    for (int x = 0; x < adam.w; ++x)
                    {
                        const int xoffset = (x << adam.xspc) + adam.xorig;
                        u8 v = src[x >> shift];
                        int a = (mask - (x & mask)) << valueShift;
                        int b = (mask - (xoffset & mask)) << valueShift;
                        v = ((v >> a) & valueMask) << b;
                        dest[xoffset >> shift] |= v;
                    }
                }

                // next pass
                buffer += bw * adam.h;
            }
        }
    }

    void ParserPNG::deinterlace8(u8* output, int width, int height, size_t stride, u8* buffer)
    {
        const int components = m_channels * (m_color_state.bits / 8);

        for (int pass = 0; pass < 7; ++pass)
        {
            AdamInterleave adam(pass, width, height);
            debugPrint("  pass: %d (%d x %d)\n", pass, adam.w, adam.h);

            const int bw = PNG_FILTER_BYTE + adam.w * components;
            filter(buffer, bw, adam.h);

            if (adam.w && adam.h)
            {
                const int ps = adam.w * components + PNG_FILTER_BYTE;

                for (int y = 0; y < adam.h; ++y)
                {
                    const int yoffset = (y << adam.yspc) + adam.yorig;
                    u8* dest = output + yoffset * stride + PNG_FILTER_BYTE;
                    u8* src = buffer + y * ps + PNG_FILTER_BYTE;

                    dest += adam.xorig * components;
                    const int xmax = (adam.w * components) << adam.xspc;
                    const int xstep = components << adam.xspc;

                    for (int x = 0; x < xmax; x += xstep)
                    {
                        std::memcpy(dest + x, src, components);
                        src += components;
                    }
                }

                // next pass
                buffer += bw * adam.h;
            }
        }
    }

    void ParserPNG::filter(u8* buffer, int bytes, int height)
    {
        const int bpp = (m_color_state.bits < 8) ? 1 : m_channels * m_color_state.bits / 8;
        if (bpp > 8)
            return;

        FilterDispatcher filter(bpp);

        // zero scanline
        Buffer zeros(bytes, 0);
        const u8* prev = zeros.data();

        for (int y = 0; y < height; ++y)
        {
            filter(buffer, prev, bytes);
            prev = buffer;
            buffer += bytes;
        }
    }

    void ParserPNG::process_range(u8* image, u8* buffer, size_t stride, int width, const FilterDispatcher& filter, int y0, int y1)
    {
        const int bytes_per_line = getBytesPerLine(width) + PNG_FILTER_BYTE;
        ColorState::Function convert = getColorFunction(m_color_state, m_color_type, m_color_state.bits);

        buffer += y0 * bytes_per_line;
        image += y0 * stride;

        for (int y = y0; y < y1; ++y)
        {
            // filtering
            filter(buffer, buffer - bytes_per_line, bytes_per_line);

            // color conversion
            convert(m_color_state, width, image, buffer + PNG_FILTER_BYTE);

            buffer += bytes_per_line;
            image += stride;
        }
    }

    void ParserPNG::process(u8* image, int width, int height, size_t stride, u8* buffer, bool multithread)
    {
        if (m_error)
        {
            return;
        }

        const int bytes_per_line = getBytesPerLine(width) + PNG_FILTER_BYTE;

        ColorState::Function convert = getColorFunction(m_color_state, m_color_type, m_color_state.bits);

        if (m_interlace)
        {
            Buffer temp(height * bytes_per_line, 0);

            // deinterlace does filter for each pass
            if (m_color_state.bits < 8)
            {
                deinterlace1to4(temp, width, height, bytes_per_line, buffer);
            }
            else
            {
                deinterlace8(temp, width, height, bytes_per_line, buffer);
            }

            // use de-interlaced temp buffer as processing source
            buffer = temp;

            // color conversion
            for (int y = 0; y < height; ++y)
            {
                convert(m_color_state, width, image, buffer + PNG_FILTER_BYTE);
                image += stride;
                buffer += bytes_per_line;
            }
        }
        else
        {
            const int bpp = (m_color_state.bits < 8) ? 1 : m_channels * m_color_state.bits / 8;
            if (bpp > 8)
                return;

            FilterDispatcher filter(bpp);

            int y0 = 0;

            if (multithread)
            {
                ConcurrentQueue q;

                for (int y = 0; y < height; ++y)
                {
                    u8 f = buffer[bytes_per_line * y]; // extract filter byte
                    if (f <= 1)
                    {
                        // does not use previous scanline -> may start a new range
                        if ((y - y0) > 32)
                        {
                            // enqueue current range
                            q.enqueue([=] ()
                            {
                                process_range(image, buffer, stride, width, filter, y0, y);
                            });

                            // start a new range
                            y0 = y;
                        }
                    }
                    else
                    {
                        // uses previous scanline -> must continue current range
                    }
                }
            }

            process_range(image, buffer, stride, width, filter, y0, height);
        }
    }

    ImageDecodeStatus ParserPNG::decode(const Surface& dest, bool multithread, Palette* ptr_palette)
    {
        ImageDecodeStatus status;

        m_compressed.reset();

        parse();

        if (!m_compressed.size() && m_parallel_segments.empty())
        {
            status.setError("No compressed data.");
            return status;
        }

        if (!m_header.success)
        {
            status.setError(m_header.info);
            return status;
        }

        if (ptr_palette)
        {
            // caller requests palette; give it and decode u8 indices
            *ptr_palette = m_palette;
            m_color_state.palette = nullptr;
        }
        else
        {
            // caller doesn't want palette; lookup RGBA colors from palette
            m_color_state.palette = m_palette.color;
        }

        // default: main image from "IHDR" chunk
        int width = m_width;
        int height = m_height;
        size_t stride = dest.stride;
        u8* image = dest.image;

        std::unique_ptr<u8[]> framebuffer;

        // override with animation frame
        if (m_number_of_frames > 0)
        {
            width = m_frame.width;
            height = m_frame.height;
            stride = width * dest.format.bytes();

            // decode frame into temporary buffer (for composition)
            image = new u8[stride * height];
            framebuffer.reset(image);

            // compute frame indices (for external users)
            m_current_frame_index = m_next_frame_index++;
            if (m_next_frame_index >= m_number_of_frames)
            {
                m_next_frame_index = 0;
            }
        }

        const int bytes_per_line = getBytesPerLine(width) + PNG_FILTER_BYTE;
        const int buffer_size = getImageBufferSize(width, height);

        // allocate output buffer
        Buffer temp(bytes_per_line + buffer_size + PNG_SIMD_PADDING);
        debugPrint("  buffer bytes:  %d\n", buffer_size);

        // zero scanline for filters at the beginning
        std::memset(temp, 0, bytes_per_line);

        Memory buffer(temp + bytes_per_line, buffer_size);

        if (m_parallel_height)
        {
            // ----------------------------------------------------------------------
            // pLLD decoding
            // ----------------------------------------------------------------------

            ConcurrentQueue q;

            u32 y = 0;
            auto decompress = deflate_zlib::decompress;

            for (ConstMemory memory : m_parallel_segments)
            {
                int h = std::min(m_parallel_height, m_height - y);

                Memory output;
                output.address = buffer.address + bytes_per_line * y;
                output.size = bytes_per_line * h;

                q.enqueue([=]
                {
                    CompressionStatus result = decompress(output, memory);
                    if (!result)
                    {
                        debugPrint("  %s\n", result.info.c_str());
                    }
                });

                decompress = deflate::decompress;
                y += m_parallel_height;
            }

            q.wait();
        }
        else if (m_idot_address && multithread)
        {
            // ----------------------------------------------------------------------
            // Apple iDOT decoding
            // ----------------------------------------------------------------------

            size_t top_size = bytes_per_line * m_first_half_height;

            Memory top_buffer;
            top_buffer.address = buffer.address;
            top_buffer.size = top_size;

            ConstMemory top_memory;
            top_memory.address = m_compressed.data();
            top_memory.size = m_idot_offset;

            Memory bottom_buffer;
            bottom_buffer.address = buffer.address + top_size;
            bottom_buffer.size = buffer.size - top_size;

            ConstMemory bottom_memory;
            bottom_memory.address = m_compressed.data() + m_idot_offset;
            bottom_memory.size = m_compressed.size() - m_idot_offset;

            // enqueue task into the threadpool
            FutureTask<size_t> task([=] () -> size_t
            {
                // Apple uses raw deflate format for iDOT extended IDAT chunks
                CompressionStatus result = deflate::decompress(bottom_buffer, bottom_memory);
                return result.size;
            });

            // Apple uses raw deflate format
            // png standard uses zlib frame format
            auto decompress = m_iphoneOptimized ?
                deflate::decompress :
                deflate_zlib::decompress;

            CompressionStatus result = decompress(top_buffer, top_memory);

            size_t bytes_out_top = result.size;
            size_t bytes_out_bottom = task.get(); // synchronize

            debugPrint("  output top bytes:     %d\n", int(bytes_out_top));
            debugPrint("  output bottom bytes:  %d\n", int(bytes_out_bottom));
            MANGO_UNREFERENCED(bytes_out_top);
            MANGO_UNREFERENCED(bytes_out_bottom);
        }
        else
        {
            // ----------------------------------------------------------------------
            // Default decoding
            // ----------------------------------------------------------------------

            // Apple uses raw deflate format
            // png standard uses zlib frame format
            auto decompress = m_iphoneOptimized ?
                deflate::decompress :
                deflate_zlib::decompress;

            CompressionStatus result = decompress(buffer, m_compressed);
            if (!result)
            {
                debugPrint("  %s\n", result.info.c_str());
                status.setError(result.info);
                return status;
            }

            debugPrint("  output bytes:  %d\n", int(result.size));
        }

        // process image
        process(image, width, height, stride, buffer, multithread);

        if (m_number_of_frames > 0)
        {
            Surface d(dest, m_frame.xoffset, m_frame.yoffset, width, height);
            Surface s(width, height, dest.format, stride, image);
            blend(d, s, ptr_palette);
        }

        status.current_frame_index = m_current_frame_index;
        status.next_frame_index = m_next_frame_index;

        return status;
    }

    // ------------------------------------------------------------
    // write_png()
    // ------------------------------------------------------------

    static
    void write_filter_sub(u8* dest, const u8* scan, size_t bpp, size_t bytes)
    {
        std::memcpy(dest, scan, bpp);
        bytes -= bpp;
        dest += bpp;

        for (size_t i = 0; i < bytes; ++i)
        {
            dest[i] = u8(scan[i + bpp] - scan[i]);
        }
    }

#if 0
    // NOTE: This is what the UP/AVERAGE/PAETH filters would look like if we wanted to use them.
    //       We don't want to use them because these depend on previous scanline so don't work
    //       with parallel decoder unless each segment starts with NONE or SUB filter which do not
    //       require the previous scanline. Selecting which filter is the best one is too slow so
    //       we go with the SUB filter every time. :)

    static
    void write_filter_up(u8* dest, const u8* scan, const u8* prev, size_t bytes)
    {
        for (size_t i = 0; i < bytes; ++i)
        {
            dest[i] = u8(scan[i] - prev[i]);
        }
    }

    static
    void write_filter_average(u8* dest, const u8* scan, const u8* prev, size_t bpp, size_t bytes)
    {
        for (size_t i = 0; i < bpp; ++i)
        {
            dest[i] = u8(scan[i] - (prev[i] / 2));
        }

        bytes -= bpp;
        dest += bpp;
        prev += bpp;

        for (size_t i = 0; i < bytes; ++i)
        {
            dest[i] = u8(scan[i + bpp] - ((prev[i] + scan[i]) / 2));
        }
    }

    static
    void write_filter_paeth(u8* dest, const u8* scan, const u8* prev, size_t bpp, size_t bytes)
    {
        for (size_t i = 0; i < bpp; ++i)
        {
            dest[i] = u8(scan[i] - prev[i]);
        }

        bytes -= bpp;
        dest += bpp;

        for (size_t i = 0; i < bytes; ++i)
        {
            int b = prev[i + bpp];
            int c = prev[i];
            int a = scan[i];

            int p = b - c;
            int q = a - c;

            int pa = abs(p);
            int pb = abs(q);
            int pc = abs(p + q);

            p = (pa <= pb && pa <=pc) ? a : (pb <= pc) ? b : c;

            dest[i] = u8(scan[i + bpp] - p);
        }
    }
#endif

    void write_chunk(Stream& stream, u32 chunk_id, ConstMemory memory)
    {
        BigEndianStream s(stream);

        u8 temp[4];
        ustore32be(temp, chunk_id);

        u32 crc = crc32(0, Memory(temp, 4));
        crc = crc32(crc, memory);

        u32 chunk_size = u32(memory.size);

        s.write32(chunk_size);
        s.write32(chunk_id);
        s.write(memory);
        s.write32(crc);
    }

    void write_IHDR(Stream& stream, const Surface& surface, u8 color_bits, ColorType color_type)
    {
        BufferStream buffer;
        BigEndianStream s(buffer);

        s.write32(surface.width);
        s.write32(surface.height);
        s.write8(color_bits);
        s.write8(color_type);
        s.write8(0); // compression
        s.write8(0); // filter
        s.write8(0); // interlace

        write_chunk(stream, u32_mask_rev('I', 'H', 'D', 'R'), buffer);
    }

    void write_iCCP(Stream& stream, const ImageEncodeOptions& options)
    {
        if (options.icc.size == 0)
        {
            // omit empty profile chunk
            return;
        }

        BufferStream buffer;
        BigEndianStream s(buffer);

        s.write8('-'); // profile name is 1-79 char according to spec. we dont have/need name
        s.write8(0); // profile name null separator
        s.write8(0); // compression method, 0=deflate

        size_t bound = deflate_zlib::bound(options.icc.size);
        Buffer compressed(bound);
        CompressionStatus result = deflate_zlib::compress(compressed, options.icc, options.compression);
        if (result)
        {
            buffer.write(compressed, result.size); // rest of chunk is compressed profile
            write_chunk(stream, u32_mask_rev('i', 'C', 'C', 'P'), buffer);
        }
    }

    void write_pLLD(Stream& stream, int segment_height)
    {
        BufferStream buffer;
        BigEndianStream s(buffer);

        s.write32(segment_height);
        s.write8(0x01); // parallel filtering is supported (because we always use SUB filter)

        write_chunk(stream, u32_mask_rev('p', 'L', 'L', 'D'), buffer);
    }

    void filter_range(u8* buffer, const Surface& surface, int y0, int y1)
    {
        const int bpp = surface.format.bytes();
        const int bytes_per_scan = surface.width * bpp;

        u8* image = surface.address(0, y0);

        for (int y = y0; y < y1; ++y)
        {
            *buffer++ = FILTER_SUB;
            write_filter_sub(buffer, image, bpp, bytes_per_scan);
            buffer += bytes_per_scan;
            image += surface.stride;
        }
    }

    void compress_serial_libdeflate(Stream& stream, const Surface& surface, int segment_height, const ImageEncodeOptions& options)
    {
        const int bpp = surface.format.bytes();
        const int bytes_per_scan = surface.width * bpp + 1;

        Buffer buffer(bytes_per_scan * surface.height);

        // filtering
        filter_range(buffer, surface, 0, surface.height);

        // compress
        size_t bound = deflate_zlib::bound(buffer.size());
        Buffer compressed(bound);
        size_t bytes_out = deflate_zlib::compress(compressed, buffer, options.compression);

        // write chunkdID + compressed data
        write_chunk(stream, u32_mask_rev('I', 'D', 'A', 'T'), ConstMemory(compressed, bytes_out));
    }

    void compress_parallel_zlib(Stream& stream, const Surface& surface, int segment_height, const ImageEncodeOptions& options)
    {
        const int bpp = surface.format.bytes();
        const int bytes_per_scan = surface.width * bpp + 1;

        Buffer buffer(bytes_per_scan * surface.height);

        const int N = ceil_div(surface.height, segment_height);
        const int level = math::clamp(options.compression, 0, 9);

        u32 cumulative_adler = 1;

        ConcurrentQueue q;
        TicketQueue tk;

        for (int i = 0; i < N; ++i)
        {
            int y = i * segment_height;
            int h = std::min(segment_height, surface.height - y);

            Memory source;
            source.address = buffer.data() + y * bytes_per_scan;
            source.size = h * bytes_per_scan;

            bool is_first = (i == 0);
            bool is_last = (i == N - 1);

            auto ticket = tk.acquire();

            q.enqueue([=, &surface, &stream, &cumulative_adler]
            {
                filter_range(source.address, surface, y, y + h);

                constexpr size_t SIZE = 128 * 1024;
                u8 temp[SIZE];

                z_stream strm;

                strm.zalloc = 0;
                strm.zfree = 0;
                strm.next_in = source.address;
                strm.avail_in = uInt(source.size);
                strm.next_out = temp;
                strm.avail_out = SIZE;

                ::deflateInit(&strm, level);

                Buffer compressed;

                while (strm.avail_in != 0)
                {
                    int res = ::deflate(&strm, Z_NO_FLUSH);
                    MANGO_UNREFERENCED(res); // should be Z_OK

                    if (strm.avail_out == 0)
                    {
                        compressed.append(temp, SIZE);
                        strm.next_out = temp;
                        strm.avail_out = SIZE;
                    }
                }

                compressed.append(temp, SIZE - strm.avail_out);
                strm.next_out = temp;
                strm.avail_out = SIZE;

                int flush = is_last ? Z_FINISH : Z_FULL_FLUSH;
                int res = ::deflate(&strm, flush);
                MANGO_UNREFERENCED(res); // should be Z_STREAM_END

                ::deflateEnd(&strm);

                compressed.append(temp, SIZE - strm.avail_out);

                // capture compressed memory
                Memory segment_memory = compressed.acquire();

                u32 segment_adler = strm.adler;
                u32 segment_length = u32(source.size);

                ticket.consume([=, &cumulative_adler, &stream]
                {
                    cumulative_adler = ::adler32_combine(cumulative_adler, segment_adler, segment_length);

                    Memory c = segment_memory;

                    if (!is_first)
                    {
                        // trim zlib header
                        c.address += 2;
                        c.size -= 2;
                    }

                    if (is_last)
                    {
                        // 4 last bytes is adler, overwrite it with cumulative adler
                        ustore32be(c.address + c.size - 4, cumulative_adler);
                    }

                    // write chunkdID + compressed data
                    write_chunk(stream, u32_mask_rev('I', 'D', 'A', 'T'), c);

                    // free compressed memory
                    Buffer::release(segment_memory);
                });
            });
        }

        q.wait();
        tk.wait();
    }

    void compress_parallel_fpng(Stream& stream, const Surface& surface, int segment_height, const ImageEncodeOptions& options)
    {
        const int bpp = surface.format.bytes();
        const int bytes_per_scan = surface.width * bpp + 1;

        Buffer buffer(bytes_per_scan * surface.height);

        const int N = ceil_div(surface.height, segment_height);
        const int level = math::clamp(options.compression, 0, 9);

        u32 cumulative_adler = 1;

        ConcurrentQueue q;
        TicketQueue tk;

        for (int i = 0; i < N; ++i)
        {
            int y = i * segment_height;
            int h = std::min(segment_height, surface.height - y);

            Memory source;
            source.address = buffer.data() + y * bytes_per_scan;
            source.size = h * bytes_per_scan;

            bool is_first = (i == 0);
            bool is_last = (i == N - 1);

            auto ticket = tk.acquire();

            q.enqueue([=, &surface, &stream, &cumulative_adler]
            {
                filter_range(source.address, surface, y, y + h);

                Buffer compressed(bytes_per_scan * h + 4094); // 4K "gimme a break" -factor
                u32 defl_size = fpng::pixel_deflate_dyn_4_rle_one_pass(source.address, 
                    surface.width, h, compressed.data(), compressed.size());

                // capture compressed memory
                Memory segment_memory = compressed.acquire();
                segment_memory.size = defl_size;

                u32 segment_adler = adler32(1, source);
                u32 segment_length = u32(source.size);

                ticket.consume([=, &cumulative_adler, &stream]
                {
                    cumulative_adler = ::adler32_combine(cumulative_adler, segment_adler, segment_length);

                    Memory c = segment_memory;

                    if (!is_first)
                    {
                        // trim zlib header
                        c.address += 2;
                        c.size -= 2;
                    }

                    if (is_last)
                    {
                        // 4 last bytes is adler, overwrite it with cumulative adler
                        ustore32be(c.address + c.size - 4, cumulative_adler);
                    }
                    else
                    {
                        // trim adler
                        c.size -= 4;
                    }

                    // write chunkdID + compressed data
                    write_chunk(stream, u32_mask_rev('I', 'D', 'A', 'T'), c);

                    // free compressed memory
                    Buffer::release(segment_memory);
                });
            });
        }

        q.wait();
        tk.wait();
    }

    int configure_segment(const Surface& surface, const ImageEncodeOptions& options)
    {
        int height = 0;

        if (options.parallel)
        {
            size_t scan_bytes = surface.width * surface.format.bytes();
            size_t image_bytes = surface.height * scan_bytes;

            constexpr size_t block_size = 256 * 1024;
            size_t N = image_bytes / block_size;

            if (N > 1)
            {
                height = int(surface.height / N);
            }
        }

        return height;
    }

    void write_png(Stream& stream, const Surface& surface, u8 color_bits, ColorType color_type, const ImageEncodeOptions& options)
    {
        BigEndianStream s(stream);

        int segment_height = configure_segment(surface, options);
        //int segment_height = surface.height;

        // write magic
        s.write64(PNG_HEADER_MAGIC);

        write_IHDR(stream, surface, color_bits, color_type);
        write_iCCP(stream, options);
        if (segment_height)
        {
            write_pLLD(stream, segment_height);
            //compress_parallel_zlib(stream, surface, segment_height, options);
            compress_parallel_fpng(stream, surface, segment_height, options);
        }
        else
        {
            compress_serial_libdeflate(stream, surface, segment_height, options);
        }

        // write IEND
        s.write32(0);
        s.write32(0x49454e44);
        s.write32(0xae426082);
    }

    // ------------------------------------------------------------
    // ImageDecoder
    // ------------------------------------------------------------

    struct Interface : ImageDecoderInterface
    {
        ParserPNG m_parser;

        Interface(ConstMemory memory)
            : m_parser(memory)
        {
        }

        ~Interface()
        {
        }

        ImageHeader header() override
        {
            return m_parser.getHeader();
        }

        ConstMemory icc() override
        {
            return m_parser.icc();
        }

        ImageDecodeStatus decode(const Surface& dest, const ImageDecodeOptions& options, int level, int depth, int face) override
        {
            MANGO_UNREFERENCED(level);
            MANGO_UNREFERENCED(depth);
            MANGO_UNREFERENCED(face);

            ImageDecodeStatus status;

            const ImageHeader& header = m_parser.getHeader();
            if (!header.success)
            {
                status.setError(header.info);
                return status;
            }

            bool direct = dest.format == header.format &&
                          dest.width >= header.width &&
                          dest.height >= header.height &&
                          options.palette == nullptr;

            if (direct)
            {
                // direct decoding
                status = m_parser.decode(dest, options.multithread, nullptr);
            }
            else
            {
                if (options.palette && header.palette)
                {
                    // direct decoding with palette
                    status = m_parser.decode(dest, options.multithread, options.palette);
                    direct = true;
                }
                else
                {
                    // indirect
                    Bitmap temp(header.width, header.height, header.format);
                    status = m_parser.decode(temp, options.multithread, nullptr);
                    dest.blit(0, 0, temp);
                }
            }

            status.direct = direct;

            return status;
        }
    };

    ImageDecoderInterface* createInterface(ConstMemory memory)
    {
        ImageDecoderInterface* x = new Interface(memory);
        return x;
    }

    // ------------------------------------------------------------
    // ImageEncoder
    // ------------------------------------------------------------

    ImageEncodeStatus imageEncode(Stream& stream, const Surface& surface, const ImageEncodeOptions& options)
    {
        MANGO_UNREFERENCED(options);

        ImageEncodeStatus status;

        // defaults
        u8 color_bits = 8;
        ColorType color_type = COLOR_TYPE_RGBA;
        Format format;

        // select png format
        if (surface.format.isLuminance())
        {
            if (surface.format.isAlpha())
            {
                color_type = COLOR_TYPE_IA;

                if (surface.format.size[0] > 8)
                {
                    color_bits = 16;
                    format = LuminanceFormat(32, Format::UNORM, 16, 16);
                }
                else
                {
                    color_bits = 8;
                    format = LuminanceFormat(16, Format::UNORM, 8, 8);
                }
            }
            else
            {
                color_type = COLOR_TYPE_I;

                if (surface.format.size[0] > 8)
                {
                    color_bits = 16;
                    format = LuminanceFormat(16, Format::UNORM, 16, 0);
                }
                else
                {
                    color_bits = 8;
                    format = LuminanceFormat(8, Format::UNORM, 8, 0);
                }
            }
        }
        else
        {
            // always encode alpha in non-luminance formats
            color_type = COLOR_TYPE_RGBA;

            if (surface.format.size[0] > 8)
            {
                color_bits = 16;
                format = Format(64, Format::UNORM, Format::RGBA, 16, 16, 16, 16);
            }
            else
            {
                color_bits = 8;
                format = Format(32, Format::UNORM, Format::RGBA, 8, 8, 8, 8);
            }
        }

        if (surface.format == format)
        {
            write_png(stream, surface, color_bits, color_type, options);
        }
        else
        {
            Bitmap temp(surface, format);
            write_png(stream, temp, color_bits, color_type, options);
        }

        return status;
    }

} // namespace

namespace mango::image
{

    void registerImageDecoderPNG()
    {
        registerImageDecoder(createInterface, ".png");
        registerImageEncoder(imageEncode, ".png");
    }

} // namespace mango::image
