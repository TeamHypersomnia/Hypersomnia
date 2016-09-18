/*
* Copyright (c) 2006-2009 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include <Box2D/Common/b2BlockAllocator.h>
#include <limits.h>
#include <memory.h>
#include <stddef.h>

b2BlockAllocator::b2BlockAllocator()
{
}

b2BlockAllocator::~b2BlockAllocator()
{
	Clear();
}

void* b2BlockAllocator::Allocate(int32 size)
{
	auto new_ptr = b2Alloc(size);
	allocations.insert(new_ptr);

	return new_ptr;
}

void b2BlockAllocator::Free(void* p, int32 size)
{
	b2Free(p);
	allocations.erase(p);
}

void b2BlockAllocator::Clear()
{
	for (auto p : allocations) {
		b2Free(p);
	}

	allocations.clear();
}