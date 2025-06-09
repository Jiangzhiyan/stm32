/* CLion type definitions for better IDE support */
#ifndef __CLION_TYPES_H
#define __CLION_TYPES_H

#ifdef __JETBRAINS_IDE__
/* Help CLion understand ARM types */
#ifndef _STDINT_H
typedef signed char         int8_t;
typedef short               int16_t; 
typedef long                int32_t;
typedef long long           int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long       uint32_t;
typedef unsigned long long  uint64_t;
#endif
#endif

#endif /* __CLION_TYPES_H */