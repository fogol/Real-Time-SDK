/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Msg.h"
#include "OmmBuffer.h"
#include "EmaString.h"
#include "ExceptionTranslator.h"
#include "MsgImpl.h"

#include "Utilities.h"

using namespace refinitiv::ema::access;

Msg::Msg() :
 _pImpl( nullptr ),
 _attrib(),
 _payload()
{
}

Msg::~Msg()
{
}

bool Msg::hasMsgKey() const
{
	return _pImpl->hasMsgKey();
}

bool Msg::hasName() const
{
	return _pImpl->hasName();
}

bool Msg::hasNameType() const
{
	return _pImpl->hasNameType();
}

bool Msg::hasServiceId() const
{
	return _pImpl->hasServiceId();
}

bool Msg::hasId() const
{
	return _pImpl->hasId();
}

bool Msg::hasFilter() const
{
	return _pImpl->hasFilter();
}

bool Msg::hasExtendedHeader() const
{
	return _pImpl->hasExtendedHeader();
}

Int32 Msg::getStreamId() const
{
	return _pImpl->getStreamId();
}

UInt16 Msg::getDomainType() const
{
	return _pImpl->getDomainType();
}

const EmaString& Msg::getName() const
{
	return _pImpl->getName();
}

UInt8 Msg::getNameType() const
{
	return _pImpl->getNameType();
}

UInt32 Msg::getServiceId() const
{
	return _pImpl->getServiceId();
}

Int32 Msg::getId() const
{
	return _pImpl->getId();
}

UInt32 Msg::getFilter() const
{
	return _pImpl->getFilter();
}

const EmaBuffer& Msg::getExtendedHeader() const
{
	return _pImpl->getExtendedHeader();
}

const Encoder& Msg::getEncoder() const
{
	_pImpl->_encoder.init();
	return _pImpl->_encoder;
}

const Attrib& Msg::getAttrib() const
{
	return _attrib;
}

const Payload& Msg::getPayload() const
{
	return _payload;
}

void Msg::setImpl( MsgImpl* pImpl )
{
	_pImpl = pImpl;
	_payload._pMsgImpl = pImpl;
	_attrib._pMsgImpl = pImpl;
}

bool Msg::hasDecoder() const
{
	return _pImpl ? true : false;
}

bool Msg::hasEncoder() const
{
	return _pImpl ? true : false;
}
