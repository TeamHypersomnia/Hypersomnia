#pragma once
#include <stdint.h>
#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif
uint32_t updateCRC32(unsigned char ch, uint32_t crc);
uint32_t crc32buf(char *buf, size_t len);
#if defined(__cplusplus)
}
#endif
