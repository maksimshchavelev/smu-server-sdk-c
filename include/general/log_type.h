/**
 * @file general/log_type.h
 *
 * @license GPLv3, see LICENSE for details
 * @copyright Copyright (©) 2025, Maksim Shchavelev <maksimshchavelev@gmail.com>
 */

#pragma once

/**
 * @brief Type of logs
 * @see sdk_log
 */
typedef enum LogType {
    LOG_INFO,    ///< **White** logs
    LOG_WARNING, ///< **Yellow** logs
    LOG_ERROR    ///< **Red** logs
} LogType;
