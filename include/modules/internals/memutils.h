/**
 * @file internals/memutils.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Writes a signed 32-bit integer to `memory` starting at offset `offset` in **Big Endian
 * order**
 *
 * @section example_usage Example usage
 * After executing this code
 * @code{.cpp}
 * write_uint32_be(mem, 0, 12)
 * @endcode
 *
 * Memory `mem` will look like:
 * ```
 * [0] : 0x00
 * [1] : 0x00
 * [2] : 0x00
 * [3] : 0x0C (12)
 * ```
 *
 * @param memory Memory
 * @param offset Offset
 * @param value Value to write
 */

static inline void write_uint32_be(void *memory, size_t offset, uint32_t value) {
    ((uint8_t *)memory)[offset] = (uint8_t)((value >> 24) & 0xFF);
    ((uint8_t *)memory)[offset + 1] = (value >> 16) & 0xFF;
    ((uint8_t *)memory)[offset + 2] = (value >> 8) & 0xFF;
    ((uint8_t *)memory)[offset + 3] = value & 0xFF;
}




/**
 * @brief Writes an unsigned 8-bit integer (a byte) to `memory`.
 * @details Endianness does not apply to a single byte, but the function is provided
 * for a consistent API.
 *
 * @section example_usage Example usage
 * After executing this code:
 * @code{.cpp}
 * write_ubyte_be(mem, 0, 255);
 * @endcode
 *
 * Memory `mem` will look like:
 * ```
 * [0] : 0xFF (255)
 * ```
 *
 * @param memory The memory buffer to write to.
 * @param offset The offset in bytes from the start of the buffer.
 * @param value The 8-bit value to write.
 */
static inline void write_ubyte_be(void *memory, size_t offset, uint8_t value) {
    ((uint8_t *)memory)[offset] = value;
}




/**
 * @brief Reads an unsigned 8-bit integer (a byte) from `memory`.
 *
 * @section example_usage Example usage
 * If memory `mem` looks like this:
 * ```
 * [0] : 0xFF (255)
 * ```
 * Then the code:
 * @code{.cpp}
 * uint8_t val = read_ubyte_be(mem, 0); // val will be 255
 * @endcode
 *
 * @param memory The memory buffer to read from.
 * @param offset The offset in bytes from the start of the buffer.
 * @return The 8-bit value read from memory.
 */
static inline uint8_t read_ubyte_be(const void *memory, size_t offset) {
    return ((uint8_t *)memory)[offset];
}




/**
 * @brief Reads an unsigned 32-bit integer from `memory` in **Big Endian** order.
 *
 * @section example_usage Example usage
 * If memory `mem` looks like this:
 * ```
 * [0] : 0x00 (MSB)
 * [1] : 0x00
 * [2] : 0x00
 * [3] : 0x0C (LSB)
 * ```
 * Then the code:
 * @code{.cpp}
 * uint32_t val = read_uint32_be(mem, 0); // val will be 12
 * @endcode
 *
 * @param memory The memory buffer to read from.
 * @param offset The offset in bytes from the start of the buffer.
 * @return The 32-bit value reconstructed from memory.
 */
static inline uint32_t read_uint32_be(const void *memory, size_t offset) {
    return ((uint32_t)(((char *)memory)[offset]) << 24) |
           ((uint32_t)(((char *)memory)[offset + 1]) << 16) |
           ((uint32_t)(((char *)memory)[offset + 2]) << 8) |
           ((uint32_t)(((char *)memory)[offset + 3]));
}
