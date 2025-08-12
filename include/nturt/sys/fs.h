/**
 * @file
 * @brief File system support.
 *
 * @author quantumspawner
 * @version 0.0.1
 * @date 2025-08-03
 * @copyright Copyright (c) 2025 NTU Racing Team
 */

#ifndef NTURT_SYS_FS_H_
#define NTURT_SYS_FS_H_

/// @brief Maximum length of a path.
#define FS_MAX_PATH_LEN 256

/* function declaration ------------------------------------------------------*/
/**
 * @brief Create a directory and all its parent directories if they do not
 * exist.
 *
 * @param path Path to the directory to create.
 * @return 0 on success, negative error code on failure.
 */
int fs_mkdir_p(const char *path);

#endif  // NTURT_SYS_FS_H_
