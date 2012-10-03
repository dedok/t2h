#ifndef T2H_CONFIG_H_INCLUDED
#define T2H_CONFIG_H_INCLUDED

#if defined(WIN32) || defined(WIN64) || defined (__CYGWIN__)
#	define EXPORT_API __stdcall
#else
#	define EXPORT_API
#endif // WINXX

/* TODO add more size_t type tests for windows platforms */
#if defined(WIN32) || defined(WIN64)
#include <windows.h>
#define T2H_SIZE_TYPE SIZE_T
#	if defined(T2H_EXPORT) && !defined(T2H_IMPORT)
#		define T2H_SPEC __declspec(dllexport)
#	else
#		define T2H_SPEC __declspec(dllimport)
#	endif // WINXX 
#else 
#include <stdlib.h>
#define T2H_SIZE_TYPE size_t
#	if __GNUC__ >= 4
#		define T2H_SPEC __attribute__ ((visibility ("default")))
#	else
#		error "Too old gcc detected, min support version is gcc 4.0"
#	endif // __GNUC__
#endif

#if defined(__cplusplus)
#	define EXTERN_C extern "C" 
#else
# 	define EXTERN_C
#endif // __cplusplus

#define T2H_STD_API_(x) EXTERN_C T2H_SPEC x EXPORT_API 
#define T2H_STD_API T2H_STD_API_(void)

#endif

