/**
 *	t2h api configuration header.
 *	t2h config header have follow [used difined|build system defined] preprocessor values :
 *	> T2H_EXPORT - if this values set then external api will set to export variant.
 *	> T2H_IMPORT - if this values set then external api will set to inport variant.
 *	> T2H_EXTERNAL_API_CALL_CONVERION - if this values was set to some call conversion variant then 
 *						all external api will have T2H_EXTERNAL_API_CALL_CONVERION call conversion.
 *	> T2H_INT_WORKAROUND - workaraund definition
 */
#ifndef T2H_CONFIG_H_INCLUDED
#define T2H_CONFIG_H_INCLUDED

/**
 * Set up call conversion for the current platform 
 */
#if defined(WIN32) || defined(WIN64) || defined(__CYGWIN__)
#	if !defined(T2H_EXTERNAL_API_CALL_CONVERION)
#		define EXPORT_API __stdcall
#	else
#		define EXPORT_API T2H_EXTERNAL_API_CALL_CONVERION
#	endif
#else 
#	define EXPORT_API
#endif// WINXX

/**
 * Set up export functions signature for the current platform
 */
#if defined(WIN32) || defined(WIN64) || defined(__CYGWIN__)
#	if defined(T2H_EXPORT) && !defined(T2H_IMPORT)
#		define T2H_SPEC __declspec(dllexport)
#	else
#		define T2H_SPEC __declspec(dllimport)
#	endif  
#else 
#	if __GNUC__ >= 4
#		define T2H_SPEC __attribute__ ((visibility ("default")))
#	else
#		error "Too old gcc detected, min support version is gcc 4.0"
#	endif // __GNUC__
#endif // WINXX

/**
 *	Set up for the current platform T2H_TORRENT_ID_TYPE_ and T2H_SIZE_TYPE types
 */
#if !defined(T2H_INT_WORKAROUND)
#	if defined(WIN32) || defined(WIN64) || defined(__CYGWIN__)
#		include <windows.h>
#		define T2H_SIZE_TYPE SIZE_T
#		define T2H_TORRENT_ID_TYPE_ SIZE_T
#	elif defined(__GNUC__)
#		include <stdlib.h>
#		define T2H_SIZE_TYPE size_t
#		define T2H_TORRENT_ID_TYPE_ size_t
#	endif
#else
#	define T2H_SIZE_TYPE int
#	if defined(WIN32) || defined(WIN64) || defined(__CYGWIN__)
#		include <windows.h>
#		define T2H_TORRENT_ID_TYPE_ SIZE_T
#	elif defined(__GNUG__)
#		include <stdlib.h>
#		define T2H_TORRENT_ID_TYPE_ size_t
#	endif // WINX
#endif // T2H_INT_WORKAROUND

// TODO test this cases with ObjC compiler
#if defined(__cplusplus)
#	define EXTERN_C extern "C" 
#else
# 	define EXTERN_C
#endif // __cplusplus

#define T2H_HANDLE_TYPE int
#define T2H_STD_API_(x) EXTERN_C T2H_SPEC x EXPORT_API 
#define T2H_STD_API T2H_STD_API_(void)

#endif

