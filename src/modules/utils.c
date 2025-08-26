/**
 * @file utils.c
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#include "modules/internals/utils.h"
#include "modules/internals/imodule.h"

// Log
void sdk_utils_log(const IModule *module, LogType log_type, const char *message) {
    sdk_imodule_get_server_core_functions(module)->abi_log(
        sdk_imodule_get_context(module), (int)log_type, message);
}

// Get ABI version
uint32_t sdk_utils_get_server_abi_version(const IModule *module) {
    return sdk_imodule_get_server_core_functions(module)->abi_get_abi_version(
        sdk_imodule_get_context(module));
}
