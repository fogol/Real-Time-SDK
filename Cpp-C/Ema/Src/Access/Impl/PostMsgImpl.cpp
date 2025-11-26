/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "PostMsgImpl.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "rtr/rsslMsgDecoders.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

PostMsgImpl::PostMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _extHeader(),
 _permission()
{
	clearRsslPostMsg();
}

PostMsgImpl& PostMsgImpl::operator=( const PostMsgImpl& other )
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslPostMsg));

	rsslClearBuffer(&(getRsslPostMsg()->permData));
	rsslClearBuffer(&(getRsslPostMsg()->extendedHeader));

	// adjust RsslBuffer values of the rssl msg that are specific only to the Ack msg
	RsslPostMsg& dstMsg = *getRsslPostMsg();
	const RsslPostMsg& srcMsg = *other.getRsslPostMsg();

	// permData
	if (other.hasPermissionData())
	{
		if (! memberCopyInPlace<PostMsgImpl>(&RsslPostMsg::permData, dstMsg, srcMsg))
		{
			setPermissionData(other.getPermissionData());
		}
	}

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<PostMsgImpl>(&RsslPostMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	return *this;
}

PostMsgImpl::~PostMsgImpl()
{
}

void PostMsgImpl::applyHasMsgKey()
{
	rsslPostMsgApplyHasMsgKey(getRsslPostMsg());
}

bool PostMsgImpl::hasPostId() const
{
	return rsslPostMsgCheckHasPostId( getRsslPostMsg() ) ? true : false;
}

bool PostMsgImpl::hasPostUserRights() const
{
	return rsslPostMsgCheckHasPostUserRights( getRsslPostMsg() ) ? true : false;
}

bool PostMsgImpl::hasExtendedHeader() const
{
	return ( rsslPostMsgCheckHasExtendedHdr(getRsslPostMsg()) == RSSL_TRUE
			 && getRsslPostMsg()->extendedHeader.length != 0 )
		? true : false;
}

bool PostMsgImpl::hasSeqNum() const
{
	return rsslPostMsgCheckHasSeqNum(getRsslPostMsg()) == RSSL_TRUE
		? true : false;
}

bool PostMsgImpl::hasPermissionData() const
{
	return rsslPostMsgCheckHasPermData(getRsslPostMsg()) == RSSL_TRUE
		? true : false;
}

bool PostMsgImpl::hasPartNum() const
{
	return rsslPostMsgCheckHasPartNum(getRsslPostMsg()) == RSSL_TRUE
		? true : false;
}

UInt32 PostMsgImpl::getSeqNum() const
{
	if ( !hasSeqNum() )
	{
		EmaString temp( "Attempt to getSeqNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslPostMsg()->seqNum;
}

UInt32 PostMsgImpl::getPublisherIdUserId() const
{
	return getRsslPostMsg()->postUserInfo.postUserId;
}

UInt32 PostMsgImpl::getPublisherIdUserAddress() const
{
	return getRsslPostMsg()->postUserInfo.postUserAddr;
}

UInt32 PostMsgImpl::getPostId() const
{
	if ( !hasPostId() )
	{
		EmaString temp( "Attempt to getPostId() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslPostMsg()->postId;
}

UInt16 PostMsgImpl::getPartNum() const
{
	if ( !hasPartNum() )
	{
		EmaString temp( "Attempt to getPartNum() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslPostMsg()->partNum;
}

UInt16 PostMsgImpl::getPostUserRights() const
{
	if ( !hasPostUserRights() )
	{
		EmaString temp( "Attempt to getPostUserRights() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslPostMsg()->postUserRights;
}

bool PostMsgImpl::getSolicitAck() const
{
	return rsslPostMsgCheckAck(getRsslPostMsg()) == RSSL_TRUE
		? true : false;
}

bool PostMsgImpl::getComplete() const
{
	return rsslPostMsgCheckPostComplete(getRsslPostMsg()) == RSSL_TRUE
		? true : false;
}

const EmaBuffer& PostMsgImpl::getPermissionData() const
{
	if ( !hasPermissionData() )
	{
		EmaString temp( "Attempt to getPermissionData() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_permission.setFromInt( getRsslPostMsg()->permData.data, getRsslPostMsg()->permData.length );

	return _permission.toBuffer();
}

const EmaBuffer& PostMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslPostMsg()->extendedHeader.data, getRsslPostMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

void PostMsgImpl::clearRsslPostMsg()
{
	rsslClearPostMsg( getRsslPostMsg() );

	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;
}

void PostMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslPostMsg();
}

void PostMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
	_permissionData.release();
}

void PostMsgImpl::setExtendedHeader( const EmaBuffer& extHeader )
{
	if (extHeader.length() == 0)
	{
		rsslClearBuffer(&getRsslPostMsg()->extendedHeader);
		applyHasNoFlag(getRsslPostMsg(), RSSL_PSMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslPostMsg()->extendedHeader, extHeader))
		{
			_extHeaderData = extHeader;

			getRsslPostMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslPostMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslPostMsgApplyHasExtendedHdr( getRsslPostMsg() );
	}
}

void PostMsgImpl::setPermissionData( const EmaBuffer& permData )
{
	if (permData.length() == 0)
	{
		rsslClearBuffer(&getRsslPostMsg()->permData);
		applyHasNoFlag(getRsslPostMsg(), RSSL_PSMF_HAS_PERM_DATA);
	}
	else
	{
		if (! _arena.trySave(getRsslPostMsg()->permData, permData))
		{
			_permissionData = permData;

			getRsslPostMsg()->permData.data = (char*)_permissionData.c_buf();
			getRsslPostMsg()->permData.length = _permissionData.length();
		}

		rsslPostMsgApplyHasPermData( getRsslPostMsg() );
	}
}

void PostMsgImpl::setSeqNum( UInt32 seqNum )
{
	getRsslPostMsg()->seqNum = seqNum;
	rsslPostMsgApplyHasSeqNum( getRsslPostMsg() );
}

void PostMsgImpl::setPartNum( UInt16 partNum )
{
	getRsslPostMsg()->partNum = partNum;
	rsslPostMsgApplyHasPartNum( getRsslPostMsg() );
}

void PostMsgImpl::setPostUserRights( UInt16 postUserRights )
{
	getRsslPostMsg()->postUserRights = postUserRights;
	rsslPostMsgApplyHasPostUserRights( getRsslPostMsg() );
}

void PostMsgImpl::setPublisherId( UInt32 userId, UInt32 userAddress )
{
	getRsslPostMsg()->postUserInfo.postUserId = userId;
	getRsslPostMsg()->postUserInfo.postUserAddr = userAddress;
}

void PostMsgImpl::setPostId( UInt32 postId )
{
	getRsslPostMsg()->postId = postId;

	rsslPostMsgApplyHasPostId( getRsslPostMsg() );
}

void PostMsgImpl::setSolicitAck( bool ack )
{
	if ( ack )
		rsslPostMsgApplyAck(getRsslPostMsg());
	else
		applyHasNoFlag(getRsslPostMsg(), RSSL_PSMF_ACK);
}

void PostMsgImpl::setComplete( bool complete )
{
	if ( complete )
		rsslPostMsgApplyPostComplete(getRsslPostMsg());
	else
		applyHasNoFlag(getRsslPostMsg(), RSSL_PSMF_POST_COMPLETE);
}
