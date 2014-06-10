#ifndef __RAKNET_TIME_H
#define __RAKNET_TIME_H

#include "NativeTypes.h"
#include "RakNetDefines.h"

namespace RakNet {

// Define __GET_TIME_64BIT if you want to use large types for GetTime (takes more bandwidth when you transmit time though!)
// You would want to do this if your system is going to run long enough to overflow the millisecond counter (over a month)
#if __GET_TIME_64BIT==1
typedef uint64_t Time;
typedef uint32_t TimeMS;
typedef uint64_t TimeUS;
#else
typedef uint32_t Time;
typedef uint32_t TimeMS;
typedef uint64_t TimeUS;
#endif

} // namespace RakNet

#endif
