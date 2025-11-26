/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GlobalPool.h"
#include "libxml/parser.h"

using namespace refinitiv::ema::access;

GlobalPool g_pool;

bool GlobalPool::_isFinalState = false;

GlobalPool::GlobalPool()
{
	xmlInitParser();
}

GlobalPool::~GlobalPool()
{
	_isFinalState = true;  // the global pool is being destroyed

	bool needToClear = true;

	while ( needToClear )
	{
		needToClear = false;

		if ( _refreshMsgImplPool.count() )
			_refreshMsgImplPool.clear(), needToClear = true;

		if ( _statusMsgImplPool.count() )
			_statusMsgImplPool.clear(), needToClear = true;

		if ( _updateMsgImplPool.count() )
			_updateMsgImplPool.clear(), needToClear = true;

		if ( _reqMsgImplPool.count() )
			_reqMsgImplPool.clear(), needToClear = true;

		if ( _postMsgImplPool.count() )
			_postMsgImplPool.clear(), needToClear = true;

		if ( _genericMsgImplPool.count() )
			_genericMsgImplPool.clear(), needToClear = true;

		if ( _ackMsgImplPool.count() )
			_ackMsgImplPool.clear(), needToClear = true;

		if ( _seriesDecoderPool.count() )
			_seriesDecoderPool.clear(), needToClear = true;

		if ( _vectorDecoderPool.count() )
			_vectorDecoderPool.clear(), needToClear = true;

		if ( _mapDecoderPool.count() )
			_mapDecoderPool.clear(), needToClear = true;

		if ( _filterListDecoderPool.count() )
			_filterListDecoderPool.clear(), needToClear = true;

		if ( _fieldListDecoderPool.count() )
			_fieldListDecoderPool.clear(), needToClear = true;

		if ( _elementListDecoderPool.count() )
			_elementListDecoderPool.clear(), needToClear = true;

		if ( _arrayDecoderPool.count() )
			_arrayDecoderPool.clear(), needToClear = true;

		if ( _fieldListEncoderPool.count() )
			_fieldListEncoderPool.clear(), needToClear = true;

		if ( _elementListEncoderPool.count() )
			_elementListEncoderPool.clear(), needToClear = true;

		if ( _arrayEncoderPool.count() )
			_arrayEncoderPool.clear(), needToClear = true;

		if ( _encodeIteratorPool.count() )
			_encodeIteratorPool.clear(), needToClear = true;

		if ( _fieldListSetDefPool.count() )
			_fieldListSetDefPool.clear(), needToClear = true;

		if ( _elementListSetDefPool.count() )
			_elementListSetDefPool.clear(), needToClear = true;
	}

	xmlCleanupParser();
}
