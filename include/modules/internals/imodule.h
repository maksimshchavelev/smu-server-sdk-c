/**
 * @file modules/internals/imodule.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include "abi.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A pseudo-base "class" for a module. Stores unique internal data for that module.
 */
typedef struct IModule {
    ABI_MODULE_CONTEXT   context;   ///< Context of module
    ABI_MODULE_MDTP_DATA mdtp_data; ///< Temporary buffer for MDTP data
    ABI_MODULE_FUNCTIONS vtable;    ///< Methods of module. See `ABI_MODULE_FUNCTIONS`
} IModule;

#ifdef __cplusplus
}
#endif
