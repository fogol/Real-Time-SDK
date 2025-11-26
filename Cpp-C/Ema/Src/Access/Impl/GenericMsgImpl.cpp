/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "GenericMsgImpl.h"

#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

GenericMsgImpl::GenericMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _extHeader(),
 _extHeaderData(),
 _permission(),
 _permissionData()
{
	clearRsslGenericMsg();
}

GenericMsgImpl& GenericMsgImpl::operator=(const GenericMsgImpl& other)
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslGenericMsg));

	rsslClearBuffer(&(getRsslGenericMsg()->permData));
	rsslClearBuffer(&(getRsslGenericMsg()->extendedHeader));

	// adjust RsslBuffer values of the rssl msg that are specific only to the Ack msg
	RsslGenericMsg& dstMsg = *getRsslGenericMsg();
	const RsslGenericMsg& srcMsg = *other.getRsslGenericMsg();

	// permData
	if (other.hasPermissionData())
	{
		if (! memberCopyInPlace<GenericMsgImpl>(&RsslGenericMsg::permData, dstMsg, srcMsg))
		{
			setPermissionData(other.getPermissionData());
		}
	}

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<GenericMsgImpl>(&RsslGenericMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	return *this;
}

GenericMsgImpl::~GenericMsgImpl()
{
}

void GenericMsgImpl::applyHasMsgKey()
{
	rsslGenericMsgApplyHasMsgKey( getRsslGenericMsg() );
}

bool GenericMsgImpl::hasSeqNum() const
{
	return rsslGenericMsgCheckHasSeqNum( getRsslGenericMsg() ) == RSSL_TRUE
			? true : false;
}

bool GenericMsgImpl::hasSecondarySeqNum() const
{
	return rsslGenericMsgCheckHasSecondarySeqNum(getRsslGenericMsg()) == RSSL_TRUE
			? true : false;
}

bool GenericMsgImpl::hasPermissionData() const
{
	return rsslGenericMsgCheckHasPermData(getRsslGenericMsg()) == RSSL_TRUE
			? true : false;
}

bool GenericMsgImpl::hasPartNum() const
{
	return rsslGenericMsgCheckHasPartNum(getRsslGenericMsg()) == RSSL_TRUE
			? true : false;
}

UInt32 GenericMsgImpl::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslGenericMsg()->seqNum;
}

UInt32 GenericMsgImpl::getSecondarySeqNum() const
{
	if ( !hasSecondarySeqNum() )
	{
		EmaString temp( "Attempt to getSecondarySeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslGenericMsg()->secondarySeqNum;
}

UInt16 GenericMsgImpl::getPartNum() const
{
	if ( !hasPartNum() )
	{
		EmaString temp( "Attempt to getPartNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslGenericMsg()->partNum;
}

bool GenericMsgImpl::getComplete() const
{
	return rsslGenericMsgCheckMessageComplete(getRsslGenericMsg()) == RSSL_TRUE
			? true : false;
}

const EmaBuffer& GenericMsgImpl::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( getRsslGenericMsg()->permData.data, getRsslGenericMsg()->permData.length );

	return _permission.toBuffer();
}

void GenericMsgImpl::clearRsslGenericMsg()
{
	rsslClearGenericMsg( getRsslGenericMsg() );

	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;
}

void GenericMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslGenericMsg();
}

void GenericMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
	_permissionData.release();
}

bool GenericMsgImpl::hasExtendedHeader() const
{
	return rsslGenericMsgCheckHasExtendedHdr( getRsslGenericMsg() ) == RSSL_TRUE
			? true : false;
}

const EmaBuffer& GenericMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslGenericMsg()->extendedHeader.data, getRsslGenericMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

void GenericMsgImpl::setExtendedHeader( const EmaBuffer& extHeader )
{
	if (extHeader.length() == 0)
	{
		rsslClearBuffer(&getRsslGenericMsg()->extendedHeader);
		applyHasNoFlag(getRsslGenericMsg(), RSSL_GNMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslGenericMsg()->extendedHeader, extHeader))
		{
			_extHeaderData = extHeader;

			getRsslGenericMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslGenericMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslGenericMsgApplyHasExtendedHdr( getRsslGenericMsg() );
	}
}

void GenericMsgImpl::setPermissionData( const EmaBuffer& permData )
{
	if (permData.length() == 0)
	{
		rsslClearBuffer(&getRsslGenericMsg()->permData);
		applyHasNoFlag(getRsslGenericMsg(), RSSL_GNMF_HAS_PERM_DATA);
	}
	else
	{
		if (! _arena.trySave(getRsslGenericMsg()->permData, permData))
		{
			_permissionData = permData;

			getRsslGenericMsg()->permData.data = (char*)_permissionData.c_buf();
			getRsslGenericMsg()->permData.length = _permissionData.length();

		}

		rsslGenericMsgApplyHasPermData( getRsslGenericMsg() );
	}
}

void GenericMsgImpl::setSeqNum( UInt32 seqNum )
{
	getRsslGenericMsg()->seqNum = seqNum;

	rsslGenericMsgApplyHasSeqNum( getRsslGenericMsg() );
}

void GenericMsgImpl::setSecondarySeqNum( UInt32 seqNum )
{
	getRsslGenericMsg()->secondarySeqNum = seqNum;

	rsslGenericMsgApplyHasSecondarySeqNum( getRsslGenericMsg() );
}

void GenericMsgImpl::setPartNum( UInt16 partNum )
{
	getRsslGenericMsg()->partNum = partNum;

	rsslGenericMsgApplyHasPartNum( getRsslGenericMsg() );
}

void GenericMsgImpl::setComplete( bool complete )
{
	if ( complete )
		rsslGenericMsgApplyMessageComplete(getRsslGenericMsg());
	else
		applyHasNoFlag(getRsslGenericMsg(), RSSL_GNMF_MESSAGE_COMPLETE);
}

void GenericMsgImpl::setProviderDriven( bool providerDriven )
{
	if (providerDriven)
		rsslGenericMsgApplyProviderDriven(getRsslGenericMsg());
	else
		applyHasNoFlag(getRsslGenericMsg(), RSSL_GNMF_PROVIDER_DRIVEN);
}
