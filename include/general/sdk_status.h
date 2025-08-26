/**
 * @file general/sdk_status.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

/**
 * @brief Describes the return value of some SDK functions
 */
typedef enum SDKStatus {
    SDK_OK,                        ///< Success
    SDK_INVALID_ARGUMENT,          ///< Invalid argument(s) (for example, NULL pointer)
    SDK_ARGUMENT_PROCESSING_ERROR, ///< Use for example when json parsing failed
    SDK_ALLOCATION_ERROR,          ///< Used, for example, when `malloc` or `strdup` fails.
    SDK_INTERNALS_UNINITIALIZED,   ///< Used when internal structures are not initialized. For
    ///< example, you use `sdk_log` but did not call `sdk_module_setup`
    ///< in the `sdk_module_init` function.
    SDK_OTHER_ERROR ///< Used for other errors when nothing fits
} SDKStatus;
