#ifndef T2H_H_INCLUDED
#define T2H_H_INCLUDED

#include "t2h_config.h"

/** Hidden t2h handle type */
typedef void * t2h_handle_t;

/**
 * Create and init t2h handle.
 *
 * @param $config 
 *	JSON config for valid initialization.
 * @param $coroutine
 *	Optional argument. Pointer to couroutine struct(for details see type defenition of t2h_completions).
 * 
 * @return
 * 	pointer to valid object, otherwise NULL.
 */
T2H_STD_API_(t2h_handle_t) t2h_init(char const * config);

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
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	
 */
T2H_STD_API_(int) t2h_add_torrent(t2h_handle_t handle, char const * path);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	
 */
T2H_STD_API_(int) t2h_add_torrent_url(t2h_handle_t handle, char const * url);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *
 */
T2H_STD_API_(char *) t2h_get_torrent_files(t2h_handle_t handle, int torrent_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_start_download(t2h_handle_t handle, int torrent_id, int file_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_paused_download(t2h_handle_t handle, int torrent_id, int file_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_resume_download(t2h_handle_t handle, int torrent_id, int file_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_delete_torrent(t2h_handle_t handle, int torrent_id);

/**
 * 
 *
 * @param $handle
 *	Handle to valid t2h object.
 *
 * @return
 *	Nothing
 */
T2H_STD_API t2h_stop_download(t2h_handle_t handle, int torrent_id, int file_id);

#endif

