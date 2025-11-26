/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_FixedBuffer_h
#define __refinitiv_ema_access_FixedBuffer_h

#include <cstddef>
#include <cstring>
#include <new>
#include <memory>

#include "rtr/rsslTypes.h"

#include "EmaBuffer.h"
#include "EmaString.h"

namespace refinitiv {

namespace ema {

namespace access {

/// Buffer of fixed, predefined size.
///
/// Can be used as a simple bump pointer arena or a general purpose RAII memory buffer.
///
/// Header-only implementation follows below
class FixedBuffer
{
public :

	FixedBuffer();
	~FixedBuffer();

	FixedBuffer(const FixedBuffer&) = delete;
	FixedBuffer(FixedBuffer&&) = delete;

	FixedBuffer& operator=(const FixedBuffer&) = delete;
	FixedBuffer& operator=(FixedBuffer&&) = delete;

	// Make sure that the FixedBuffer has reservation amount of bytes ready. May
	// invalidate pointers when current capacity is less than reservation. Resets current
	// pos to 0 (like clear)
	void reserve(UInt32 reservation);

	void clear();

	void release();

	char* data();

	// return true if was able to copy provided data into the current FixedBuffer, adjust
	// pointer and length for the destination RsslBuffer.
	//
	// return false if the FixedBuffer is out of free space, leave RsslBuffer unmodified
	bool trySave(RsslBuffer& dst, const char* src, UInt32 len) noexcept;
	bool trySave(RsslBuffer& dst, const RsslBuffer& src) noexcept;
	bool trySave(RsslBuffer& dst, const EmaBuffer& src) noexcept;
	bool trySave(RsslBuffer& dst, const EmaString& src) noexcept;

private :

	char* 		_pData;
	void* 		_pos;

	UInt32 		_capacity;
	size_t		_space;
};

inline FixedBuffer::FixedBuffer() :
 _pData(nullptr),
 _pos(nullptr),
 _capacity(0),
 _space(0)
{
};

inline FixedBuffer::~FixedBuffer()
{
	if (_pData != nullptr)
		delete[] _pData;
};

inline void FixedBuffer::reserve(UInt32 reservation)
{
	if (_capacity < reservation + alignof(std::max_align_t))
	{
		if (_pData)
		{
			delete[] _pData;
		}

		_capacity = reservation + alignof(std::max_align_t);
		_pData = new char[_capacity];
	}
	_pos = _pData;
	_space = _capacity;
};

inline void FixedBuffer::clear()
{
	_pos = _pData;
	_space = _capacity;
};

inline void FixedBuffer::release()
{
	if (_pData != nullptr)
	{
		delete[] _pData;

		_pData = nullptr;
		_pos = nullptr;
		_capacity = 0;
		_space = 0;
	}
};

inline char* FixedBuffer::data()
{
	return _pData;
};

inline bool FixedBuffer::trySave(RsslBuffer& dstBuffer, const char* src, UInt32 len) noexcept
{
	void *res = std::align(alignof(std::max_align_t), len, _pos, _space);
	if (res != nullptr)
	{
		dstBuffer.data = (char*)memcpy(_pos, src, len);
		dstBuffer.length = len;

		_pos = static_cast<char*>(_pos) + len;
		_space -= len;
		return true;
	}

	return false;
};

inline bool FixedBuffer::trySave(RsslBuffer& dstBuffer, const RsslBuffer& srcBuffer) noexcept
{
	return trySave(dstBuffer, srcBuffer.data, srcBuffer.length);
};

inline bool FixedBuffer::trySave(RsslBuffer& dstBuffer, const EmaBuffer& srcBuffer) noexcept
{
	return trySave(dstBuffer, srcBuffer.c_buf(), srcBuffer.length());
};

inline bool FixedBuffer::trySave(RsslBuffer& dstBuffer, const EmaString& srcString) noexcept
{
	return trySave(dstBuffer, srcString.c_str(), srcString.length());
};

}

}

}

#endif
