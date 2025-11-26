/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "UpdateMsgImpl.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

#include "rtr/rsslMsgDecoders.h"

using namespace refinitiv::ema::access;

UpdateMsgImpl::UpdateMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _extHeader(),
 _permission()
{
	clearRsslUpdateMsg();
}

UpdateMsgImpl& UpdateMsgImpl::operator=( const UpdateMsgImpl& other)
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslUpdateMsg));

	rsslClearBuffer(&(getRsslUpdateMsg()->permData));
	rsslClearBuffer(&(getRsslUpdateMsg()->extendedHeader));

	// adjust RsslBuffer values of the rssl msg that are specific only to the Ack msg
	RsslUpdateMsg& dstMsg = *getRsslUpdateMsg();
	const RsslUpdateMsg& srcMsg = *other.getRsslUpdateMsg();

	// permData
	if (other.hasPermissionData())
	{
		if (! memberCopyInPlace<UpdateMsgImpl>(&RsslUpdateMsg::permData, dstMsg, srcMsg))
		{
			setPermissionData(other.getPermissionData());
		}
	}

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<UpdateMsgImpl>(&RsslUpdateMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	return *this;
}

UpdateMsgImpl::~UpdateMsgImpl()
{
}

void UpdateMsgImpl::applyHasMsgKey()
{
	rsslUpdateMsgApplyHasMsgKey(getRsslUpdateMsg());
}

bool UpdateMsgImpl::hasExtendedHeader() const
{
	return rsslUpdateMsgCheckHasExtendedHdr( getRsslUpdateMsg() ) == RSSL_TRUE
		? true : false;
}

const EmaBuffer& UpdateMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslUpdateMsg()->extendedHeader.data, getRsslUpdateMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

bool UpdateMsgImpl::hasSeqNum() const
{
	return rsslUpdateMsgCheckHasSeqNum(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

bool UpdateMsgImpl::hasPermissionData() const
{
	return rsslUpdateMsgCheckHasPermData(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

bool UpdateMsgImpl::hasConflated() const
{
	return rsslUpdateMsgCheckHasConfInfo(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

bool UpdateMsgImpl::hasPublisherId() const
{
	return rsslUpdateMsgCheckHasPostUserInfo(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

UInt8 UpdateMsgImpl::getUpdateTypeNum() const
{
	return getRsslUpdateMsg()->updateType;
}

UInt32 UpdateMsgImpl::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslUpdateMsg()->seqNum;
}

const EmaBuffer& UpdateMsgImpl::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( getRsslUpdateMsg()->permData.data, getRsslUpdateMsg()->permData.length );

	return _permission.toBuffer();
}

UInt16 UpdateMsgImpl::getConflatedTime() const
{
	if ( !hasConflated() )
	{
		EmaString temp( "Attempt to getConflatedTime() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslUpdateMsg()->conflationTime;
}

UInt16 UpdateMsgImpl::getConflatedCount() const
{
	if ( !hasConflated() )
	{
		EmaString temp( "Attempt to getConflatedCount() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslUpdateMsg()->conflationCount;
}

UInt32 UpdateMsgImpl::getPublisherIdUserId() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslUpdateMsg()->postUserInfo.postUserId;
}

UInt32 UpdateMsgImpl::getPublisherIdUserAddress() const
{
	if ( !hasPublisherId() )
	{
		EmaString temp( "Attempt to getPublisherIdUserAddress() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslUpdateMsg()->postUserInfo.postUserAddr;
}

bool UpdateMsgImpl::getDoNotCache() const
{
	return rsslUpdateMsgCheckDoNotCache(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

bool UpdateMsgImpl::getDoNotConflate() const
{
	return rsslUpdateMsgCheckDoNotConflate(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

bool UpdateMsgImpl::getDoNotRipple() const
{
	return rsslUpdateMsgCheckDoNotRipple(getRsslUpdateMsg()) == RSSL_TRUE
		? true : false;
}

void UpdateMsgImpl::clearRsslUpdateMsg()
{
	rsslClearUpdateMsg( getRsslUpdateMsg() );

	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;
}

void UpdateMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslUpdateMsg();
}

void UpdateMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
	_permissionData.release();
}

void UpdateMsgImpl::setUpdateTypeNum( UInt8 value )
{
	getRsslUpdateMsg()->updateType = value;
}

void UpdateMsgImpl::setSeqNum( UInt32 value )
{
	getRsslUpdateMsg()->seqNum = value;

	rsslUpdateMsgApplyHasSeqNum(getRsslUpdateMsg());
}

void UpdateMsgImpl::setPublisherId( UInt32 userId, UInt32 userAddress )
{
	getRsslUpdateMsg()->postUserInfo.postUserAddr = userAddress;
	getRsslUpdateMsg()->postUserInfo.postUserId = userId;

	rsslUpdateMsgApplyHasPostUserInfo(getRsslUpdateMsg());
}

void UpdateMsgImpl::setConflated( UInt16 count, UInt16 time )
{
	getRsslUpdateMsg()->conflationCount = count;
	getRsslUpdateMsg()->conflationTime = time;

	rsslUpdateMsgApplyHasConfInfo(getRsslUpdateMsg());
}

void UpdateMsgImpl::setPermissionData( const EmaBuffer& permData )
{
	if (permData.length() == 0)
	{
		rsslClearBuffer(&getRsslUpdateMsg()->permData);
		applyHasNoFlag(getRsslUpdateMsg(), RSSL_UPMF_HAS_PERM_DATA);
	}
	else
	{
		if (! _arena.trySave(getRsslUpdateMsg()->permData, permData))
		{
			_permissionData = permData;

			getRsslUpdateMsg()->permData.data = (char*)_permissionData.c_buf();
			getRsslUpdateMsg()->permData.length = _permissionData.length();
		}

		rsslUpdateMsgApplyHasPermData(getRsslUpdateMsg());
	}
}

void UpdateMsgImpl::setExtendedHeader( const EmaBuffer& extHeader )
{
	if (extHeader.length() == 0)
	{
		rsslClearBuffer(&getRsslUpdateMsg()->extendedHeader);
		applyHasNoFlag(getRsslUpdateMsg(), RSSL_UPMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslUpdateMsg()->extendedHeader, extHeader))
		{
			_extHeaderData = extHeader;

			getRsslUpdateMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslUpdateMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslUpdateMsgApplyHasExtendedHdr(getRsslUpdateMsg());
	}
}

void UpdateMsgImpl::setDoNotCache( bool value )
{
	if ( value )
		rsslUpdateMsgApplyDoNotCache(getRsslUpdateMsg());
	else
		applyHasNoFlag(getRsslUpdateMsg(), RSSL_UPMF_DO_NOT_CACHE);
}

void UpdateMsgImpl::setDoNotConflate( bool value )
{
	if ( value )
		rsslUpdateMsgApplyDoNotConflate(getRsslUpdateMsg());
	else
		applyHasNoFlag(getRsslUpdateMsg(), RSSL_UPMF_DO_NOT_CONFLATE);
}

void UpdateMsgImpl::setDoNotRipple( bool value )
{
	if ( value )
		rsslUpdateMsgApplyDoNotRipple(getRsslUpdateMsg());
	else
		applyHasNoFlag(getRsslUpdateMsg(), RSSL_UPMF_DO_NOT_RIPPLE);
}
