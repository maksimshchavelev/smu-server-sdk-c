/**
 * @file sdk.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

#include "general/log_type.h"
#include "modules/internals/macro.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IModule IModule; ///< Forward declaration

/**
 * @brief Logger functions to log messages
 * @param module Not-null Pointer to `IModule`
 * @param log_type Type of log. See `LogType`
 * @param message Message to log
 */
void sdk_utils_log(const IModule *module, LogType log_type, const char *message);

/**
 * @brief Get ABI version which the server uses
 * @param module Not-null Pointer to `IModule`
 * @return `uint32_t` with ABI version
 */
uint32_t sdk_utils_get_server_abi_version(const IModule *module);

#ifdef __cplusplus
}
#endif
