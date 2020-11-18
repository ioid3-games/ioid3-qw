#ifndef __CONFIG_TYPES_H__
#define __CONFIG_TYPES_H__

/* these are filled in by configure or cmake*/
#define INCLUDE_INTTYPES_H INCLUDE_INTTYPES_H
#define INCLUDE_STDINT_H INCLUDE_STDINT_H
#define INCLUDE_SYS_TYPES_H INCLUDE_SYS_TYPES_H

#if INCLUDE_INTTYPES_H
#  include <inttypes.h>
#endif
#if INCLUDE_STDINT_H
#  include <stdint.h>
#endif
#if INCLUDE_SYS_TYPES_H
#  include <sys/types.h>
#endif

typedef signed short ogg_int16_t;
typedef unsigned short ogg_uint16_t;
typedef signed int ogg_int32_t;
typedef unsigned int ogg_uint32_t;
typedef signed long long ogg_int64_t;
typedef unsigned long long ogg_uint64_t;

#endif
