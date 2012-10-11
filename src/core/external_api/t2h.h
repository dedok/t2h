#ifndef T2H_H_INCLUDED
#define T2H_H_INCLUDED

#include "t2h_config.h"
#include "core_version.hpp"

/**
 * Types and definitios
 */

#define T2H_VERSION CORE_VERSION_HEX
#define T2H_VERSION_STRING CORE_VERSION_STRING

#define INVALID_T2H_HANDLE -1 
#define INVALID_TORRENT_ID -1

typedef T2H_HANDLE_TYPE t2h_handle;
typedef t2h_handle t2h_handle_t;

/**
 * Create and init t2h handle.
 *
 * @param $config 
 *	JSON config for valid initialization.
 * @return
 * 	handle object, otherwise INVALID_T2H_HANDLE.
 */
T2H_STD_API_(t2h_handle_t) t2h_init(char const * config);

/**
 * Create and init t2h handle.
 *
 * @param $config 
 *	file path to JSON config for valid initialization.
 * @return
 * 	handle object, otherwise INVALID_T2H_HANDLE.
 */
T2H_STD_API_(t2h_handle_t) t2h_init_2(char const * config_file_path);

/**
 * Close t2h service.
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_close(t2h_handle_t handle);

/**
 * Bloking wait for t2h service end of work.
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */ 
T2H_STD_API t2h_wait(t2h_handle_t handle);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	
 */
T2H_STD_API_(T2H_SIZE_TYPE) t2h_add_torrent(t2h_handle_t handle, char const * path);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	
 */
T2H_STD_API_(T2H_SIZE_TYPE) t2h_add_torrent_url(t2h_handle_t handle, char const * url);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *
 */
T2H_STD_API_(char *) t2h_get_torrent_files(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API_(char *) t2h_start_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_paused_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_resume_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id, int file_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_delete_torrent(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_stop_download(t2h_handle_t handle, T2H_SIZE_TYPE torrent_id);

#endif

