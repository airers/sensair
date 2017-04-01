#ifndef _DATATYPES_H_
#define _DATATYPES_H_

typedef union {
    char bytes[4];
    long data;
} long_u;

typedef union {
    char bytes[4];
    float data;
} float_u;

typedef union {
    char bytes[2];
    uint16_t data;
} uint16_u;


#endif
