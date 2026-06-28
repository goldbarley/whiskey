#ifndef WHIS_ATTR_H
#define WHIS_ATTR_H

#if defined(_MSC_VER)
#ifdef WHIS_LIB_STATIC
#define WHIS_EXPORT
#elif defined(WHIS_LIB_SHARED)
#define WHIS_EXPORT __declspec(dllexport)
#else
#define WHIS_EXPORT __declspec(dllimport)
#endif
#define WHIS_INLINE static __forceinline
#else
#ifdef WHIS_LIB_SHARED
#define WHIS_EXPORT __attribute__((__visibility__("default")))
#else
#define WHIS_EXPORT
#endif /* WHIS_LIB_SHARED */
#define WHIS_INLINE static inline __attribute__((__always_inline__))
#endif /* Compilers */

#endif /* WHIS_ATTR_H */
