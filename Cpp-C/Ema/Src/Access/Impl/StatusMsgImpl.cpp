/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "EmaBufferInt.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

#include "StatusMsgImpl.h"
#include "StatusMsg.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString(DataType::DataTypeEnum dType);

StatusMsgImpl::StatusMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _stateSet(false),
 _extHeader(),
 _extHeaderData(),
 _permission(),
 _permissionData(),
 _itemGroup(),
 _itemGroupData(),
 _state(),
 _statusTextData()
{
	clearRsslStatusMsg();
}

StatusMsgImpl& StatusMsgImpl::operator=(const StatusMsgImpl& other)
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslStatusMsg));

	rsslClearBuffer(&(getRsslStatusMsg()->state.text));
	rsslClearBuffer(&(getRsslStatusMsg()->groupId));
	rsslClearBuffer(&(getRsslStatusMsg()->permData));
	rsslClearBuffer(&(getRsslStatusMsg()->extendedHeader));
	rsslClearBuffer(&(getRsslStatusMsg()->reqMsgKey.name));
	rsslClearBuffer(&(getRsslStatusMsg()->reqMsgKey.encAttrib));

	// adjust RsslBuffer values of the RsslStatusMsg that are specific only to the statusMsg
	RsslStatusMsg& dstMsg = *getRsslStatusMsg();
	const RsslStatusMsg& srcMsg = *other.getRsslStatusMsg();

	// state.text
	if (other.hasState() && srcMsg.state.text.length > 0)
	{
		if (! memberCopyInPlace( dstMsg.state.text, srcMsg.state.text,
								 dstMsg.msgBase.encMsgBuffer, srcMsg.msgBase.encMsgBuffer ) )
		{
			setStatusText(srcMsg.state.text.data, srcMsg.state.text.length);
		}
	}

	// groupId
	if (other.hasItemGroup())
	{
		if (! memberCopyInPlace<StatusMsgImpl>(&RsslStatusMsg::groupId, dstMsg, srcMsg))
		{
			setItemGroup(other.getItemGroup());
		}
	}

	// permData
	if (other.hasPermissionData())
	{
		if (! memberCopyInPlace<StatusMsgImpl>(&RsslStatusMsg::permData, dstMsg, srcMsg))
		{
			setPermissionData(other.getPermissionData());
		}
	}

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<StatusMsgImpl>(&RsslStatusMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	// reqMsgKey is not used in Ema
	applyHasNoFlag(getRsslStatusMsg(), RSSL_STMF_HAS_REQ_MSG_KEY);

	return *this;
}

StatusMsgImpl::~StatusMsgImpl()
{
}

void StatusMsgImpl::applyHasMsgKey()
{
	rsslStatusMsgApplyHasMsgKey(getRsslStatusMsg());
}

void StatusMsgImpl::clearRsslStatusMsg()
{
	rsslClearStatusMsg( getRsslStatusMsg() );

	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;

	resetRsslInt();
}

void StatusMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslStatusMsg();
}

void StatusMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
	_permissionData.release();
	_itemGroupData.release();
}

Data::DataCode StatusMsgImpl::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum StatusMsgImpl::getDataType() const
{
	return DataType::StatusMsgEnum;
}

bool StatusMsgImpl::hasItemGroup() const
{
	return rsslStatusMsgCheckHasGroupId(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

bool StatusMsgImpl::hasState() const
{
	return rsslStatusMsgCheckHasState(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

bool StatusMsgImpl::hasPublisherId() const
{
	return rsslStatusMsgCheckHasPostUserInfo(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

const OmmState& StatusMsgImpl::getState() const
{
	if (!hasState())
	{
		EmaString temp("Attempt to getState() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	if (! _stateSet)
	{
		StaticDecoder::setRsslData(&_state, &getRsslStatusMsg()->state);
		_stateSet = true;
	}

	return static_cast<const OmmState&>(static_cast<const Data&>(_state));
}

const EmaBuffer& StatusMsgImpl::getItemGroup() const
{
	if (!hasItemGroup())
	{
		EmaString temp("Attempt to getItemGroup() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_itemGroup.setFromInt(getRsslStatusMsg()->groupId.data, getRsslStatusMsg()->groupId.length);

	return _itemGroup.toBuffer();
}

bool StatusMsgImpl::hasPermissionData() const
{
	return rsslStatusMsgCheckHasPermData(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

const EmaBuffer& StatusMsgImpl::getPermissionData() const
{
	if (!hasPermissionData())
	{
		EmaString temp("Attempt to getPermissionData() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	_permission.setFromInt(getRsslStatusMsg()->permData.data, getRsslStatusMsg()->permData.length);

	return _permission.toBuffer();
}

UInt32 StatusMsgImpl::getPublisherIdUserId() const
{
	if (!hasPublisherId())
	{
		EmaString temp("Attempt to getPublisherIdUserId() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return getRsslStatusMsg()->postUserInfo.postUserId;
}

UInt32 StatusMsgImpl::getPublisherIdUserAddress() const
{
	if (!hasPublisherId())
	{
		EmaString temp("Attempt to getPublisherIdUserAddress() while it is NOT set.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}

	return getRsslStatusMsg()->postUserInfo.postUserAddr;
}

bool StatusMsgImpl::getClearCache() const
{
	return rsslStatusMsgCheckClearCache(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

bool StatusMsgImpl::getPrivateStream() const
{
	return rsslStatusMsgCheckPrivateStream(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

void StatusMsgImpl::setState(OmmState::StreamState streamState, OmmState::DataState dataState,
	UInt8 statusCode, const EmaString& statusText)
{
	getRsslStatusMsg()->state.streamState = streamState;
	getRsslStatusMsg()->state.dataState = dataState;
	getRsslStatusMsg()->state.code = statusCode;

	setStatusText(statusText.c_str(), statusText.length());

	rsslStatusMsgApplyHasState(getRsslStatusMsg());

	_stateSet = false;
}

void StatusMsgImpl::setStatusText(const char* str, UInt32 len)
{
	if (! _arena.trySave(getRsslStatusMsg()->state.text, str, len))
	{
		_statusTextData.set(str, len);

		getRsslStatusMsg()->state.text.data = (char*)_statusTextData.c_str();
		getRsslStatusMsg()->state.text.length = _statusTextData.length();
	}
	_stateSet = false;
}

void StatusMsgImpl::setItemGroup(const EmaBuffer& itemGroup)
{
	if (itemGroup.length() == 0)
	{
		rsslClearBuffer(&getRsslStatusMsg()->groupId);
		applyHasNoFlag(getRsslStatusMsg(), RSSL_STMF_HAS_GROUP_ID);
	}
	else
	{
		if (! _arena.trySave(getRsslStatusMsg()->groupId, itemGroup))
		{
			_itemGroupData = itemGroup;

			getRsslStatusMsg()->groupId.data = (char*)_itemGroupData.c_buf();
			getRsslStatusMsg()->groupId.length = _itemGroupData.length();
		}

		rsslStatusMsgApplyHasGroupId(getRsslStatusMsg());
	}
}

void StatusMsgImpl::setPermissionData(const EmaBuffer& permData)
{
	if (permData.length() == 0)
	{
		rsslClearBuffer(&getRsslStatusMsg()->permData);
		applyHasNoFlag(getRsslStatusMsg(), RSSL_STMF_HAS_PERM_DATA);
	}
	else
	{
		if (! _arena.trySave(getRsslStatusMsg()->permData, permData))
		{
			_permissionData = permData;

			getRsslStatusMsg()->permData.data = (char*)_permissionData.c_buf();
			getRsslStatusMsg()->permData.length = _permissionData.length();
		}

		rsslStatusMsgApplyHasPermData(getRsslStatusMsg());
	}
}

void StatusMsgImpl::setPublisherId(UInt32 userId, UInt32 userAddress)
{
	getRsslStatusMsg()->postUserInfo.postUserAddr = userAddress;
	getRsslStatusMsg()->postUserInfo.postUserId = userId;

	rsslStatusMsgApplyHasPostUserInfo(getRsslStatusMsg());
}

bool StatusMsgImpl::hasExtendedHeader() const
{
	return rsslStatusMsgCheckHasExtendedHdr(getRsslStatusMsg()) == RSSL_TRUE
		? true : false;
}

const EmaBuffer& StatusMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslStatusMsg()->extendedHeader.data, getRsslStatusMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

void StatusMsgImpl::setExtendedHeader(const EmaBuffer& buffer)
{
	if (buffer.length() == 0)
	{
		rsslClearBuffer(&getRsslStatusMsg()->extendedHeader);
		applyHasNoFlag(getRsslStatusMsg(), RSSL_STMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslStatusMsg()->extendedHeader, buffer))
		{
			_extHeaderData = buffer;

			getRsslStatusMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslStatusMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslStatusMsgApplyHasExtendedHdr(getRsslStatusMsg());
	}
}

void StatusMsgImpl::setClearCache(bool clearCache)
{
	if ( clearCache )
		rsslStatusMsgApplyClearCache(getRsslStatusMsg());
	else
		applyHasNoFlag(getRsslStatusMsg(), RSSL_STMF_CLEAR_CACHE);
}

void StatusMsgImpl::setPrivateStream(bool privateStream)
{
	if ( privateStream )
		rsslStatusMsgApplyPrivateStream(getRsslStatusMsg());
	else
		applyHasNoFlag(getRsslStatusMsg(), RSSL_STMF_PRIVATE_STREAM);
}
