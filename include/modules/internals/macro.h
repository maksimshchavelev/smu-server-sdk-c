/**
 * @file sdk.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

/**
 * @brief `SDK_ABI` macro makes entiny visible from outside anyway
 */
#if defined _WIN32
#define SDK_EXPORT __declspec(dllexport)
#else
#define SDK_EXPORT __attribute__((visibility("default")))
#endif
