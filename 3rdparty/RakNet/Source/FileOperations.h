/// \file FileOperations.h
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_FileOperations==1

#ifndef __FILE_OPERATIONS_H
#define __FILE_OPERATIONS_H

#include "Export.h"

bool RAK_DLL_EXPORT WriteFileWithDirectories( const char *path, char *data, unsigned dataLength );
bool RAK_DLL_EXPORT IsSlash(unsigned char c);
void RAK_DLL_EXPORT AddSlash( char *input );
void RAK_DLL_EXPORT QuoteIfSpaces(char *str);
bool RAK_DLL_EXPORT DirectoryExists(const char *directory);
unsigned int RAK_DLL_EXPORT GetFileLength(const char *path);

#endif

#endif // _RAKNET_SUPPORT_FileOperations
