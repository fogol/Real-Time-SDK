/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_MsgEncoder_h
#define __refinitiv_ema_access_MsgEncoder_h

#include "rtr/rsslMsg.h"

#include "Encoder.h"

namespace refinitiv {

namespace ema {

namespace access {

class MsgImpl;

/// Encodes associated _pMsgImpl into the message buffer
class MsgEncoder final : public Encoder
{
public :

	MsgEncoder(MsgImpl* pMsgImpl);
	virtual ~MsgEncoder();

	void release() override { releaseEncIterator(); };

	void endEncodingEntry() const override {};

	RsslBuffer& getEncodedBuffer() const override;

	void init() { acquireEncIterator(); };

	void clear() override { clearEncIterator(); };

	bool isComplete() const override { return true; };

private :

	MsgImpl* _pMsgImpl;
};

}

}

}

#endif //__refinitiv_ema_access_MsgEncoder_h
