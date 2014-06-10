#ifndef __BASE_64_ENCODER_H
#define __BASE_64_ENCODER_H

#include "Export.h"

extern "C" {
/// \brief Returns how many bytes were written.
// outputData should be at least the size of inputData * 2 + 6
int Base64Encoding(const unsigned char *inputData, int dataLength, char *outputData);
}

extern "C" {
const char *Base64Map(void);
}

#endif
