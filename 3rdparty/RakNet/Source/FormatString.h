/// \file FormatString.h
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#ifndef __FORMAT_STRING_H
#define __FORMAT_STRING_H

#include "Export.h"

extern "C" {
char * FormatString(const char *format, ...);
}
// Threadsafe
extern "C" {
char * FormatStringTS(char *output, const char *format, ...);
}


#endif

