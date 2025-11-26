/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015,2019-2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_GlobalPool_h
#define __refinitiv_ema_access_GlobalPool_h

#include "OmmArrayDecoder.h"
#include "ElementListDecoder.h"
#include "FieldListDecoder.h"
#include "FilterListDecoder.h"
#include "MapDecoder.h"
#include "SeriesDecoder.h"
#include "VectorDecoder.h"

#include "AckMsgImpl.h"
#include "GenericMsgImpl.h"
#include "PostMsgImpl.h"
#include "ReqMsgImpl.h"
#include "RefreshMsgImpl.h"
#include "StatusMsgImpl.h"
#include "UpdateMsgImpl.h"

#include "OmmArrayEncoder.h"
#include "ElementListEncoder.h"
#include "FieldListEncoder.h"
#include "VectorEncoder.h"
#include "SeriesEncoder.h"
#include "FilterListEncoder.h"
#include "MapEncoder.h"
#include "OmmAnsiPageEncoder.h"
#include "OmmOpaqueEncoder.h"
#include "OmmXmlEncoder.h"
#include "OmmJsonEncoder.h"

#include "ElementListSetDef.h"
#include "FieldListSetDef.h"

namespace refinitiv {

namespace ema {

namespace access {

#define DO_ITEM_FUNCS(item, pool)\
item * get##item##Item()\
{\
	return pool.getItem();\
}\
void returnItem(item * pEncoder)\
{\
	if (_isFinalState)\
		delete pEncoder;\
	else\
		pool.returnItem(pEncoder);\
}

class GlobalPool
{
public :

	GlobalPool();
	virtual ~GlobalPool();

	// When the global pool is in final state
	// then clients must stop all the operations with it
	static void setFinalState() {
		_isFinalState = true;
	}

	DO_ITEM_FUNCS(ElementListSetDef, _elementListSetDefPool)
	DO_ITEM_FUNCS(FieldListSetDef, _fieldListSetDefPool)

	DO_ITEM_FUNCS(EncodeIterator, _encodeIteratorPool)
	DO_ITEM_FUNCS(OmmArrayEncoder, _arrayEncoderPool)
	DO_ITEM_FUNCS(ElementListEncoder, _elementListEncoderPool)
	DO_ITEM_FUNCS(FieldListEncoder, _fieldListEncoderPool)
	DO_ITEM_FUNCS(MapEncoder, _mapEncoderPool)
	DO_ITEM_FUNCS(VectorEncoder, _vectorEncoderPool)
	DO_ITEM_FUNCS(SeriesEncoder, _seriesEncoderPool)
	DO_ITEM_FUNCS(FilterListEncoder, _filterListEncoderPool)
	DO_ITEM_FUNCS(OmmAnsiPageEncoder, _ommAnsiPageEncoderPool)
	DO_ITEM_FUNCS(OmmOpaqueEncoder, _ommOpaqueEncoderPool)
	DO_ITEM_FUNCS(OmmXmlEncoder, _ommXmlEncoderPool)
	DO_ITEM_FUNCS(OmmJsonEncoder, _ommJsonEncoderPool)

	DO_ITEM_FUNCS(AckMsgImpl, _ackMsgImplPool)
	DO_ITEM_FUNCS(GenericMsgImpl, _genericMsgImplPool)
	DO_ITEM_FUNCS(PostMsgImpl, _postMsgImplPool)
	DO_ITEM_FUNCS(ReqMsgImpl, _reqMsgImplPool)
	DO_ITEM_FUNCS(RefreshMsgImpl, _refreshMsgImplPool)
	DO_ITEM_FUNCS(StatusMsgImpl, _statusMsgImplPool)
	DO_ITEM_FUNCS(UpdateMsgImpl, _updateMsgImplPool)

	DO_ITEM_FUNCS(OmmArrayDecoder, _arrayDecoderPool)
	DO_ITEM_FUNCS(ElementListDecoder, _elementListDecoderPool)
	DO_ITEM_FUNCS(FieldListDecoder, _fieldListDecoderPool)
	DO_ITEM_FUNCS(FilterListDecoder, _filterListDecoderPool)
	DO_ITEM_FUNCS(MapDecoder, _mapDecoderPool)
	DO_ITEM_FUNCS(VectorDecoder, _vectorDecoderPool)
	DO_ITEM_FUNCS(SeriesDecoder, _seriesDecoderPool)

private:

	static bool					_isFinalState;  // indicates that the global pool is destroyed, clients must stop all operations with it

	ElementListSetDefPool		_elementListSetDefPool;
	FieldListSetDefPool			_fieldListSetDefPool;

	EncodeIteratorPool			_encodeIteratorPool;
	OmmArrayEncoderPool			_arrayEncoderPool;
	ElementListEncoderPool		_elementListEncoderPool;
	FieldListEncoderPool		_fieldListEncoderPool;
	MapEncoderPool				_mapEncoderPool;
	VectorEncoderPool			_vectorEncoderPool;
	SeriesEncoderPool			_seriesEncoderPool;
	FilterListEncoderPool		_filterListEncoderPool;
	OmmAnsiPageEncoderPool		_ommAnsiPageEncoderPool;
	OmmOpaqueEncoderPool		_ommOpaqueEncoderPool;
	OmmXmlEncoderPool			_ommXmlEncoderPool;
	OmmJsonEncoderPool			_ommJsonEncoderPool;

	AckMsgImplPool			_ackMsgImplPool;
	GenericMsgImplPool		_genericMsgImplPool;
	PostMsgImplPool			_postMsgImplPool;
	ReqMsgImplPool			_reqMsgImplPool;
	RefreshMsgImplPool		_refreshMsgImplPool;
	StatusMsgImplPool			_statusMsgImplPool;
	UpdateMsgImplPool		_updateMsgImplPool;

	OmmArrayDecoderPool			_arrayDecoderPool;
	ElementListDecoderPool		_elementListDecoderPool;
	FieldListDecoderPool		_fieldListDecoderPool;
	FilterListDecoderPool		_filterListDecoderPool;
	MapDecoderPool				_mapDecoderPool;
	VectorDecoderPool			_vectorDecoderPool;
	SeriesDecoderPool			_seriesDecoderPool;
};

#undef DO_ITEM_FUNCS

}

}

}

extern refinitiv::ema::access::GlobalPool g_pool;

#endif // __refinitiv_ema_access_GlobalPool_h
