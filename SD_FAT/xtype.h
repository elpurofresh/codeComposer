#ifndef __XTYPE_H__
#define __XTYPE_H__

#define _MCS51_ 1

#ifdef _MCS51_
    typedef unsigned char uint8;
    typedef unsigned int uint16;
    typedef unsigned long uint32;

    typedef char int8;
    typedef int int16;
    typedef long int32;

    
    typedef uint8 bit;
#else

    typedef unsigned char uint8;
    typedef unsigned short uint16;
    typedef unsigned int uint32;

    typedef char int8;
    typedef short int16;
    typedef int int32;

    typedef unsigned char bool;
#endif  
    
    
typedef uint8 uint8_t;
typedef uint16 uint16_t;
typedef uint32 uint32_t;

#endif
