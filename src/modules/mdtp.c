/**
 * @file modules/mdtp.c
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "modules/internals/mdtp.h"
#include "modules/internals/abi.h"
#include "modules/internals/memutils.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static ABI_MODULE_MDTP_DATA mdtp_data = {0}; ///< MDTP buffer


// Forward declaration begin
static void *mdtp_make_value(const char *value_name, const char *value, const char *value_units);
static void  mdtp_free_value(void *value_node);
static void *mdtp_make_container(const char *name, void *first, ...);
static void  mdtp_free_container(void *container_node);
static ABI_MODULE_MDTP_DATA *mdtp_make_root(void *first, ...);
static uint32_t              mdtp_get_nodes_size(const void *first, ...);
static uint32_t              mdtp_get_nodes_size_va(const void *first, va_list args);
// Forward declaration end



// Make value node
static void *mdtp_make_value(const char *value_name, const char *value, const char *value_units) {
    // From MDTP v1 specification:

    // [node type]: 1 unsigned byte (1 because node is value)
    // [node name length]: unsigned int32
    // [name of node...]: array of char
    // [units length]: unsigned int32
    // [units...]: array of char
    // [value length]: unsigned int32
    // [value...]: array of char

    if (value_name == NULL || value == NULL || value_units == NULL) {

        return NULL;
    }

    size_t buffer_size = 1 + sizeof(uint32_t) + strlen(value_name) + sizeof(uint32_t) +
                         strlen(value_units) + sizeof(uint32_t) + strlen(value);
    size_t offset = 0; // Current position

    void *buffer = calloc(1, buffer_size); // Use calloc to fill memory by zeroes

    // Allocation error
    if (buffer == NULL) {
        return NULL;
    }

    size_t value_name_length = strlen(value_name);
    size_t value_units_length = strlen(value_units);
    size_t value_length = strlen(value);

    // Write node type
    write_ubyte_be(buffer, offset, 1);
    ++offset;

    // Write node name length
    write_uint32_be(buffer, offset, (uint32_t)value_name_length);
    offset += 4;

    // Write node name
    memcpy((char *)buffer + offset, value_name, value_name_length);
    offset += value_name_length;

    // Write units length
    write_uint32_be(buffer, offset, (uint32_t)value_units_length);
    offset += 4;

    // Write units
    memcpy((char *)buffer + offset, value_units, value_units_length);
    offset += value_units_length;

    // Write value length
    write_uint32_be(buffer, offset, (uint32_t)value_length);
    offset += 4;

    // Write value
    memcpy((char *)buffer + offset, value, value_length);

    return buffer;
}


// Free value node
static void mdtp_free_value(void *value_node) {
    // If not value node
    if (read_ubyte_be(value_node, 0) != 1) {
        return;
    }

    free(value_node);
}


// Make container node
static void *mdtp_make_container(const char *name, void *first, ...) {
    if (name == NULL || first == NULL) {
        return NULL;
    }

    va_list args;
    va_list args_copy;
    va_start(args, first);
    va_copy(args_copy, args);

    void    *ptr = first;
    void    *buffer;
    size_t   offset = 0;
    size_t   name_length = strlen(name);
    uint32_t payload_size = mdtp_get_nodes_size_va(first, args_copy);

    uint32_t size = 1 /* node type */ + 4 /* name length */ + (uint32_t)name_length /* name */ +
                    4 /* payload size */ + payload_size /* payload */;
    buffer = calloc(1, size); // calloc to fill memory by zeroes

    if (buffer == NULL) {
        va_end(args);
        va_end(args_copy);
        return NULL;
    }

    // Fill buffer
    // Write node type
    write_ubyte_be(buffer, offset, 0 /* container */);
    ++offset;

    // Write name length
    write_uint32_be(buffer, offset, (uint32_t)name_length);
    offset += 4;

    // Write name
    memcpy((char *)buffer + offset, name, name_length);
    offset += name_length;

    // Write payload size
    write_uint32_be(buffer, offset, payload_size);
    offset += 4;

    // Write payload
    while (ptr != NULL) {
        uint32_t node_size = mdtp_get_nodes_size(ptr, NULL);
        memcpy((char *)buffer + offset, ptr, node_size);

        // If node type is value
        if (read_ubyte_be(ptr, 0) == 1) {
            mdtp_free_value(ptr);
        }
        // If node type is container
        else if (read_ubyte_be(ptr, 0) == 0) {
            mdtp_free_container(ptr);
        }

        offset += node_size;

        ptr = va_arg(args, void *);
    }

    va_end(args);
    va_end(args_copy);

    return buffer;
}


// Free container node
static void mdtp_free_container(void *container_node) {
    // If not container node
    if (read_ubyte_be(container_node, 0) != 0) {
        return;
    }

    free(container_node);
}


// Make root node
static ABI_MODULE_MDTP_DATA *mdtp_make_root(void *first, ...) {
    if (first == NULL) {
        return NULL;
    }

    va_list args;
    va_list args_copy;
    va_start(args, first);
    va_copy(args_copy, args);

    void    *ptr = first;
    size_t   offset = 0;
    uint32_t payload_size = mdtp_get_nodes_size_va(first, args_copy);

    uint32_t size = 1 /* version of MDTP */ + 4 /* payload size */ + payload_size /* payload */;
    void    *buffer = calloc(1, size); // calloc to fill memory by zeroes

    if (buffer == NULL) {
        va_end(args);
        va_end(args_copy);
        return NULL;
    }

    // Success allocation
    free((void *)mdtp_data.data);
    mdtp_data.data = buffer;
    mdtp_data.size = size;

    // Fill buffer
    // Write MDTP version
    write_ubyte_be((void *)mdtp_data.data, offset, MDTP_VERSION);
    ++offset;

    // Write payload size
    write_uint32_be((void *)mdtp_data.data, offset, payload_size);
    offset += 4;

    // Write payload
    while (ptr != NULL) {
        uint32_t node_size = mdtp_get_nodes_size(ptr, NULL);
        memcpy((char *)mdtp_data.data + offset, ptr, node_size);

        // If node type is value
        if (read_ubyte_be(ptr, 0) == 1) {
            mdtp_free_value(ptr);
        }
        // If node type is container
        else if (read_ubyte_be(ptr, 0) == 0) {
            mdtp_free_container(ptr);
        }

        offset += node_size;

        ptr = va_arg(args, void *);
    }

    va_end(args);
    va_end(args_copy);

    return &mdtp_data;
}


// Get node size
static uint32_t mdtp_get_nodes_size(const void *first, ...) {
    va_list args;
    va_start(args, first);
    uint32_t size = mdtp_get_nodes_size_va(first, args);
    va_end(args);
    return size;
}


// Get nodes size via va_list
static uint32_t mdtp_get_nodes_size_va(const void *first, va_list args) {
    if (first == NULL) {
        return 0u;
    }

    uint64_t total = 0; // accumulate in 64-bit to avoid intermediate overflow

    for (const void *p = first; p != NULL; p = va_arg(args, void *)) {
        const uint8_t *b = (const uint8_t *)p;

        // Read node type (first byte)
        uint8_t type = b[0];

        if (type == 1) {
            /* VALUE NODE:
             * [1 type] [4 name_len] [name] [4 units_len] [units] [4 value_len] [value]
             */
            size_t off = 1;

            uint32_t name_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)name_len;

            uint32_t units_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)units_len;

            uint32_t value_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)value_len;

            total += off;

        } else if (type == 0) {
            /* CONTAINER NODE:
             * [1 type] [4 name_len] [name] [4 payload_size] [payload...]
             */
            size_t off = 1;

            uint32_t name_len = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)name_len;

            uint32_t payload_size = read_uint32_be((void *)b, off);
            off += 4;
            off += (size_t)payload_size;

            total += off;

        } else {
            // Unknown type: ignore (adds 0).
        }
    }

    // Clamp to UINT32_MAX if sum exceeded 32-bit range
    return (total > 0xFFFFFFFFu) ? 0xFFFFFFFFu : (uint32_t)total;
}


// Make MDTP_UTILS
MDTP_UTILS mdtp_utils_init(void) {
    return (MDTP_UTILS){.make_value = mdtp_make_value,
                        .make_container = mdtp_make_container,
                        .make_root = mdtp_make_root,
                        .free_value = mdtp_free_value,
                        .free_container = mdtp_free_container};
}


// Destroy resources
void mdtp_utils_destroy(void) {
    free((void *)mdtp_data.data);
    mdtp_data.data = NULL;
    mdtp_data.size = 0;
}
