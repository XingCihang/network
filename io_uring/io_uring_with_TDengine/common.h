// common.h
#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>

typedef struct {
    char src_ip[64];
    char dst_ip[64];
    char protocol[32];
    char device_type[32];
    int bytes;
    uint64_t ts_ms; // 毫秒时间戳
} NetLog;

#endif