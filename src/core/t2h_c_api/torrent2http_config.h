#ifndef T2H_CONFIG_H_INCLUDED
#define T2H_CONFIG_H_INCLUDED

/** TODO add dllimport and dllexport for Windows palatform*/
#if defined(WIN32) || defined(WIN64)
#define EXPORT_API __stdcall
#else
#define EXPORT_API
#endif

#if defined(__cplusplus)
#define EXTERN_C extern "C" 
#else
#define EXTERN_C
#endif

#define T2H_STD_API_(x) EXTERN_C x EXPORT_API 
#define T2H_STD_API T2H_STD_API_(void)

#endif

