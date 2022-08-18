/**
 * Copyright (c) 2011-2022 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_SYSTEM_INTRINSICS_XCPU_FUNCTIONAL_256_HPP
#define LIBBITCOIN_SYSTEM_INTRINSICS_XCPU_FUNCTIONAL_256_HPP

#include <bitcoin/system/define.hpp>
#include <bitcoin/system/intrinsics/xcpu/defines.hpp>

// shl_/shr_ are undefined for 8 bit.
// All others are at most AVX2.

namespace libbitcoin {
namespace system {

#if defined(HAVE_AVX2)

using xint256_t = __m256i;

namespace f {

/// bitwise primitives
/// ---------------------------------------------------------------------------

// AVX2
INLINE xint256_t and_(xint256_t a, xint256_t b) NOEXCEPT
{
    return mm256_and_si256(a, b);
}

// AVX2
INLINE xint256_t or_(xint256_t a, xint256_t b) NOEXCEPT
{
    return mm256_or_si256(a, b);
}

// AVX2
INLINE xint256_t xor_(xint256_t a, xint256_t b) NOEXCEPT
{
    return mm256_xor_si256(a, b);
}

// AVX2
INLINE xint256_t not_(xint256_t a) NOEXCEPT
{
    return xor_(a, mm256_set1_epi64x(-1));
}

/// vector primitives
/// ---------------------------------------------------------------------------

template <auto B, auto S>
INLINE xint256_t shr(xint256_t a) NOEXCEPT
{
    // Undefined
    static_assert(S != bits<uint8_t>);
    ////if constexpr (S == bits<uint8_t>)
    ////    return mm256_srli_epi8(a, B);

    // AVX2
    if constexpr (S == bits<uint16_t>)
        return mm256_srli_epi16(a, B);
    else if constexpr (S == bits<uint32_t>)
        return mm256_srli_epi32(a, B);
    else if constexpr (S == bits<uint64_t>)
        return mm256_srli_epi64(a, B);
}

template <auto B, auto S>
INLINE xint256_t shl(xint256_t a) NOEXCEPT
{
    // Undefined
    static_assert(S != bits<uint8_t>);
    ////if constexpr (S == bits<uint8_t>)
    ////    return mm256_slli_epi8(a, B);

    // AVX2
    if constexpr (S == bits<uint16_t>)
        return mm256_slli_epi16(a, B);
    else if constexpr (S == bits<uint32_t>)
        return mm256_slli_epi32(a, B);
    else if constexpr (S == bits<uint64_t>)
        return mm256_slli_epi64(a, B);
}

template <auto B, auto S>
INLINE xint256_t ror(xint256_t a) NOEXCEPT
{
    return or_(shr<B, S>(a), shl<S - B, S>(a));
}

template <auto B, auto S>
INLINE xint256_t rol(xint256_t a) NOEXCEPT
{
    return or_(shl<B, S>(a), shr<S - B, S>(a));
}

// AVX2
template <auto S>
INLINE xint256_t add(xint256_t a, xint256_t b) NOEXCEPT
{
    if constexpr (S == bits<uint8_t>)
        return mm256_add_epi8(a, b);
    else if constexpr (S == bits<uint16_t>)
        return mm256_add_epi16(a, b);
    else if constexpr (S == bits<uint32_t>)
        return mm256_add_epi32(a, b);
    else if constexpr (S == bits<uint64_t>)
        return mm256_add_epi64(a, b);
}

// AVX
template <auto K, auto S>
INLINE xint256_t addc(xint256_t a) NOEXCEPT
{
    if constexpr (S == bits<uint8_t>)
        return add<S>(a, mm256_set1_epi8(K));
    else if constexpr (S == bits<uint16_t>)
        return add<S>(a, mm256_set1_epi16(K));
    else if constexpr (S == bits<uint32_t>)
        return add<S>(a, mm256_set1_epi32(K));
    else if constexpr (S == bits<uint64_t>)
        return add<S>(a, mm256_set1_epi64x(K));
}

} // namespace f

/// broadcast/get/set
/// ---------------------------------------------------------------------------

// AVX
template <typename Word, if_integral_integer<Word> = true>
INLINE xint256_t broadcast(Word a) NOEXCEPT
{
    // set1 broadcasts integer to all elements.
    if constexpr (is_same_type<Word, uint8_t>)
        return mm256_set1_epi8(a);
    if constexpr (is_same_type<Word, uint16_t>)
        return mm256_set1_epi16(a);
    if constexpr (is_same_type<Word, uint32_t>)
        return mm256_set1_epi32(a);
    if constexpr (is_same_type<Word, uint64_t>)
        return mm256_set1_epi64x(a);
}

// Lane zero is lowest order word.
template <typename Word, auto Lane, if_integral_integer<Word> = true>
INLINE Word get(xint256_t a) NOEXCEPT
{
    // mm256_extract_epi64 defined as no-op on 32 bit builds (must exclude).
    ////static_assert(!build_x32 && is_same_type<Word, uint64_t>);

    // AVX2
    if constexpr (is_same_type<Word, uint8_t>)
        return mm256_extract_epi8(a, Lane);
    else if constexpr (is_same_type<Word, uint16_t>)
        return mm256_extract_epi16(a, Lane);

    // AVX
    else if constexpr (is_same_type<Word, uint32_t>)
        return mm256_extract_epi32(a, Lane);
    else if constexpr (is_same_type<Word, uint64_t>)
        return mm256_extract_epi64(a, Lane);
}

// AVX
// Low order word to the left.
template <typename Word, if_same<Word, xint256_t> = true>
INLINE xint256_t set(
    uint64_t x01 = 0, uint64_t x02 = 0,
    uint64_t x03 = 0, uint64_t x04 = 0) NOEXCEPT
{
    // Low order word to the right.
    return mm256_set_epi64x(
        x04, x03, x02, x01);
}

// AVX
template <typename Word, if_same<Word, xint256_t> = true>
INLINE xint256_t set(
    uint32_t x01 = 0, uint32_t x02 = 0,
    uint32_t x03 = 0, uint32_t x04 = 0,
    uint32_t x05 = 0, uint32_t x06 = 0,
    uint32_t x07 = 0, uint32_t x08 = 0) NOEXCEPT
{
    return mm256_set_epi32(
        x08, x07, x06, x05, x04, x03, x02, x01);
}

// AVX
template <typename Word, if_same<Word, xint256_t> = true>
INLINE xint256_t set(
    uint16_t x01 = 0, uint16_t x02 = 0,
    uint16_t x03 = 0, uint16_t x04 = 0,
    uint16_t x05 = 0, uint16_t x06 = 0,
    uint16_t x07 = 0, uint16_t x08 = 0,
    uint16_t x09 = 0, uint16_t x10 = 0,
    uint16_t x11 = 0, uint16_t x12 = 0,
    uint16_t x13 = 0, uint16_t x14 = 0,
    uint16_t x15 = 0, uint16_t x16 = 0) NOEXCEPT
{
    return mm256_set_epi16(
        x16, x15, x14, x13, x12, x11, x10, x09,
        x08, x07, x06, x05, x04, x03, x02, x01);
}

// AVX
template <typename Word, if_same<Word, xint256_t> = true>
INLINE xint256_t set(
    uint8_t x01 = 0, uint8_t x02 = 0,
    uint8_t x03 = 0, uint8_t x04 = 0,
    uint8_t x05 = 0, uint8_t x06 = 0,
    uint8_t x07 = 0, uint8_t x08 = 0,
    uint8_t x09 = 0, uint8_t x10 = 0,
    uint8_t x11 = 0, uint8_t x12 = 0,
    uint8_t x13 = 0, uint8_t x14 = 0,
    uint8_t x15 = 0, uint8_t x16 = 0,
    uint8_t x17 = 0, uint8_t x18 = 0,
    uint8_t x19 = 0, uint8_t x20 = 0,
    uint8_t x21 = 0, uint8_t x22 = 0,
    uint8_t x23 = 0, uint8_t x24 = 0,
    uint8_t x25 = 0, uint8_t x26 = 0,
    uint8_t x27 = 0, uint8_t x28 = 0,
    uint8_t x29 = 0, uint8_t x30 = 0,
    uint8_t x31 = 0, uint8_t x32 = 0) NOEXCEPT
{
    return mm256_set_epi8(
        x32, x31, x30, x29, x28, x27, x26, x25,
        x24, x23, x22, x21, x20, x19, x18, x17,
        x16, x15, x14, x13, x12, x11, x10, x09,
        x08, x07, x06, x05, x04, x03, x02, x01);
}

/// pack/unpack
/// ---------------------------------------------------------------------------

////// TODO: auto pack<Word>(const uint8_t*).
////INLINE auto unpack(xint256_t a) NOEXCEPT
////{
////    std_array<uint8_t, sizeof(xint256_t)> bytes{};
////    mm256_storeu_si256(pointer_cast<xint256_t>(&bytes.front()), a);
////    return bytes;
////}

/// endianness
/// ---------------------------------------------------------------------------

// AVX2
BC_PUSH_WARNING(NO_ARRAY_INDEXING)
INLINE xint256_t byteswap(xint256_t a) NOEXCEPT
{
    static const auto mask = set<xint256_t>(
        0x08090a0b0c0d0e0f_u64, 0x08090a0b0c0d0e0f_u64,
        0x08090a0b0c0d0e0f_u64, 0x08090a0b0c0d0e0f_u64);

    return mm256_shuffle_epi8(a, mask);
}
BC_POP_WARNING()

#else

// Symbol is defined but not usable.
struct xint256_t : xmock_t {};

#endif // HAVE_AVX2

} // namespace system
} // namespace libbitcoin

#endif

////INLINE xint256_t align(const bytes256& word) NOEXCEPT
////{
////    return mm256_loadu_epi32(pointer_cast<const xint256_t>(word.data()));
////}
////
////INLINE bytes256 unalign(xint256_t value) NOEXCEPT
////{
////    bytes256 word{};
////    mm256_storeu_epi32(pointer_cast<xint256_t>(word.data()), value);
////    return word;
////    ////return *pointer_cast<bytes256>(&value);
////}
////
////INLINE xint256_t native_to_big_endian(xint256_t value) NOEXCEPT
////{
////    return *pointer_cast<xint256_t>(unalign(byteswap(value)).data());
////}
////
////INLINE xint256_t native_from_big_endian(xint256_t value) NOEXCEPT
////{
////    return byteswap(align(*pointer_cast<bytes256>(&value)));
////}
