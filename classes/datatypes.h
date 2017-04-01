#ifndef _DATATYPES_H_
#define _DATATYPES_H_

#define C_COLON  58
#define C_COMMA  44
#define C_SP   32
#define C_A   65
#define C_B   66
#define C_C   67
#define C_D   68
#define C_E   69
#define C_F   70
#define C_G   71
#define C_H   72
#define C_I   73
#define C_J   74
#define C_K   75
#define C_L   76
#define C_M   77
#define C_N   78
#define C_O   79
#define C_P   80
#define C_Q   81
#define C_R   82
#define C_S   83
#define C_T   84
#define C_U   85
#define C_V   86
#define C_W   87
#define C_X   88
#define C_Y   89
#define C_Z   90
#define C_LR  10

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
