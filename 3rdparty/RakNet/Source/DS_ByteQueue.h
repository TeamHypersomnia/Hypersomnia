/// \file DS_ByteQueue.h
/// \internal
/// \brief Byte queue
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#ifndef __BYTE_QUEUE_H
#define __BYTE_QUEUE_H

#include "RakMemoryOverride.h"
#include "Export.h"

/// The namespace DataStructures was only added to avoid compiler errors for commonly named data structures
/// As these data structures are stand-alone, you can use them outside of RakNet for your own projects if you wish.
namespace DataStructures
{
	class ByteQueue
	{
	public:
		ByteQueue();
		~ByteQueue();
		void WriteBytes(const char *in, unsigned length, const char *file, unsigned int line);
		bool ReadBytes(char *out, unsigned maxLengthToRead, bool peek);
		unsigned GetBytesWritten(void) const;
		char* PeekContiguousBytes(unsigned int *outLength) const;
		void IncrementReadOffset(unsigned length);
		void DecrementReadOffset(unsigned length);
		void Clear(const char *file, unsigned int line);
		void Print(void);

	protected:
		char *data;
		unsigned readOffset, writeOffset, lengthAllocated;
	};
}

#endif
