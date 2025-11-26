/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ReqMsgImpl.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmQos.h"
#include "ReqMsg.h"
#include "OmmInvalidUsageException.h"

#include "rtr/rsslMsgDecoders.h"
#include "rtr/rsslArray.h"
#include "rtr/rsslElementList.h"
#include "rtr/rsslIterators.h"
#include "rtr/rsslPrimitiveDecoders.h"

using namespace refinitiv::ema::access;

ReqMsgImpl::ReqMsgImpl() :
 MsgImpl(),
 _decoder(this),
 _extHeader(),
 _domainTypeSet(false)
{
	clearRsslRequestMsg();
}

ReqMsgImpl& ReqMsgImpl::operator=( const ReqMsgImpl& other)
{
	if (this == &other)
		return *this;

	resetRsslInt();

	copyMsgBaseFrom(other, sizeof(RsslRequestMsg));

	rsslClearBuffer(&(getRsslRequestMsg()->extendedHeader));

	// adjust RsslBuffer values of the rssl msg that are specific only to the Ack msg
	RsslRequestMsg& dstMsg = *getRsslRequestMsg();
	const RsslRequestMsg& srcMsg = *other.getRsslRequestMsg();

	// extendedHeader
	if (other.hasExtendedHeader())
	{
		if (! memberCopyInPlace<ReqMsgImpl>(&RsslRequestMsg::extendedHeader, dstMsg, srcMsg))
		{
			setExtendedHeader(other.getExtendedHeader());
		}
	}

	return *this;
}

ReqMsgImpl::~ReqMsgImpl()
{
}

void ReqMsgImpl::applyHasMsgKey()
{
	// note: msgKey is *required* for an RsslRequestMsg (which implies it is always present)
}

bool ReqMsgImpl::hasExtendedHeader() const
{
	return rsslRequestMsgCheckHasExtendedHdr(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

const EmaBuffer& ReqMsgImpl::getExtendedHeader() const
{
	if ( !hasExtendedHeader() )
	{
		EmaString temp( "Attempt to getExtendedHeader() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	_extHeader.setFromInt( getRsslRequestMsg()->extendedHeader.data, getRsslRequestMsg()->extendedHeader.length );

	return _extHeader.toBuffer();
}

bool ReqMsgImpl::hasPriority() const
{
	return rsslRequestMsgCheckHasPriority(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

bool ReqMsgImpl::hasQos() const
{
	return rsslRequestMsgCheckHasQos(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

bool ReqMsgImpl::hasView() const
{
	return rsslRequestMsgCheckHasView(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

bool ReqMsgImpl::hasBatch() const
{
	return rsslRequestMsgCheckHasBatch(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

UInt8 ReqMsgImpl::getPriorityClass() const
{
	if ( !hasPriority() )
	{
		EmaString temp( "Attempt to getPriorityClass() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslRequestMsg()->priorityClass;
}

UInt16 ReqMsgImpl::getPriorityCount() const
{
	if ( !hasPriority() )
	{
		EmaString temp( "Attempt to getPriorityCount() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return getRsslRequestMsg()->priorityCount;
}

UInt32 ReqMsgImpl::getTimeliness() const
{
	if ( !hasQos() )
	{
		EmaString temp( "Attempt to getTimeliness() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	UInt32 timeliness = ReqMsg::BestTimelinessEnum;

	if ( rsslRequestMsgCheckHasWorstQos(getRsslRequestMsg()) )
	{
		if ( getRsslRequestMsg()->qos.timeliness == getRsslRequestMsg()->worstQos.timeliness )
		{
			switch ( getRsslRequestMsg()->qos.timeliness )
			{
			case RSSL_QOS_TIME_REALTIME:
				timeliness = ReqMsg::RealTimeEnum;
				break;
			case RSSL_QOS_TIME_DELAYED_UNKNOWN:
				timeliness = ReqMsg::BestDelayedTimelinessEnum;
				break;
			case RSSL_QOS_TIME_DELAYED:
				if ( getRsslRequestMsg()->qos.timeInfo == getRsslRequestMsg()->worstQos.timeInfo )
					timeliness = getRsslRequestMsg()->qos.timeInfo;
				else
					timeliness = ReqMsg::BestDelayedTimelinessEnum;
				break;
			}
		}
		else
		{
			if ( getRsslRequestMsg()->qos.timeliness == RSSL_QOS_TIME_REALTIME )
				timeliness = ReqMsg::BestRateEnum;
			else
				timeliness = ReqMsg::BestDelayedTimelinessEnum;
 		}
	}
	else
	{
		switch ( getRsslRequestMsg()->qos.timeliness )
		{
			case RSSL_QOS_TIME_REALTIME:
				timeliness = ReqMsg::RealTimeEnum;
				break;
			case RSSL_QOS_TIME_DELAYED_UNKNOWN:
				timeliness = ReqMsg::BestDelayedTimelinessEnum;
				break;
			case RSSL_QOS_TIME_DELAYED:
					timeliness = getRsslRequestMsg()->qos.timeInfo;
				break;
		}
	}

	return timeliness;
}

UInt32 ReqMsgImpl::getRate() const
{
	if ( !hasQos() )
	{
		EmaString temp( "Attempt to getRate() while it is NOT set." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	UInt32 rate = ReqMsg::BestRateEnum;

	if ( getRsslRequestMsg()->flags & RSSL_RQMF_HAS_WORST_QOS )
	{
		if ( getRsslRequestMsg()->qos.rate == getRsslRequestMsg()->worstQos.rate )
		{
			switch ( getRsslRequestMsg()->qos.rate )
			{
			case RSSL_QOS_RATE_TICK_BY_TICK:
				rate = ReqMsg::TickByTickEnum;
				break;
			case RSSL_QOS_RATE_JIT_CONFLATED:
				rate = ReqMsg::JustInTimeConflatedEnum;
				break;
			case RSSL_QOS_RATE_TIME_CONFLATED:
				if ( getRsslRequestMsg()->qos.rateInfo == getRsslRequestMsg()->worstQos.rateInfo )
					rate = getRsslRequestMsg()->qos.rateInfo;
				else
					rate = ReqMsg::BestConflatedRateEnum;
				break;
			}
		}
		else
		{
			if ( getRsslRequestMsg()->qos.rate == RSSL_QOS_RATE_TICK_BY_TICK )
				rate = ReqMsg::BestRateEnum;
			else
				rate = ReqMsg::BestConflatedRateEnum;
		}
	}
	else
	{
		switch ( getRsslRequestMsg()->qos.rate )
		{
		case RSSL_QOS_RATE_TICK_BY_TICK:
			rate = ReqMsg::TickByTickEnum;
			break;
		case RSSL_QOS_RATE_JIT_CONFLATED:
			rate = ReqMsg::JustInTimeConflatedEnum;
			break;
		case RSSL_QOS_RATE_TIME_CONFLATED:
			rate = getRsslRequestMsg()->qos.rateInfo;
		}
	}

	return rate;
}

bool ReqMsgImpl::getInitialImage() const
{
	return rsslRequestMsgCheckNoRefresh(getRsslRequestMsg()) == RSSL_TRUE
		? false : true;
}

bool ReqMsgImpl::getInterestAfterRefresh() const
{
	return rsslRequestMsgCheckStreaming(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

bool ReqMsgImpl::getConflatedInUpdates() const
{
	return rsslRequestMsgCheckConfInfoInUpdates(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

bool ReqMsgImpl::getPause() const
{
	return rsslRequestMsgCheckPause(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

bool ReqMsgImpl::getPrivateStream() const
{
	return rsslRequestMsgCheckPrivateStream(getRsslRequestMsg()) == RSSL_TRUE
		? true : false;
}

void ReqMsgImpl::adjustPayload()
{
	if ( getRsslMsg()->msgBase.containerType == RSSL_DT_ELEMENT_LIST )
	{
		checkBatchView( &getRsslRequestMsg()->msgBase.encDataBody );
	}
}

void ReqMsgImpl::clearRsslRequestMsg()
{
	rsslClearRequestMsg( getRsslRequestMsg() );
	rsslClearQos( &getRsslRequestMsg()->qos );

	getRsslMsg()->msgBase.domainType = RSSL_DMT_MARKET_PRICE;
	getRsslMsg()->msgBase.containerType = RSSL_DT_NO_DATA;

	getRsslRequestMsg()->flags = RSSL_RQMF_STREAMING;
}

void ReqMsgImpl::clear()
{
	clearBaseMsgImpl();
	clearRsslRequestMsg();

	_domainTypeSet = false;
}

void ReqMsgImpl::releaseMsgBuffers()
{
	_extHeaderData.release();
}

void ReqMsgImpl::setPriority( UInt8 priorityClass, UInt16 priorityCount )
{
	getRsslRequestMsg()->priorityClass = priorityClass;
	getRsslRequestMsg()->priorityCount = priorityCount;
	rsslRequestMsgApplyHasPriority( getRsslRequestMsg() );
}

void ReqMsgImpl::setExtendedHeader( const EmaBuffer& extHeader )
{
	if (extHeader.length() == 0)
	{
		rsslClearBuffer(&getRsslRequestMsg()->extendedHeader);
		applyHasNoFlag(getRsslRequestMsg(), RSSL_RQMF_HAS_EXTENDED_HEADER);
	}
	else
	{
		if (! _arena.trySave(getRsslRequestMsg()->extendedHeader, extHeader))
		{
			_extHeaderData = extHeader;

			getRsslRequestMsg()->extendedHeader.data = (char*)_extHeaderData.c_buf();
			getRsslRequestMsg()->extendedHeader.length = _extHeaderData.length();
		}

		rsslRequestMsgApplyHasExtendedHdr( getRsslRequestMsg() );
	}
}

void ReqMsgImpl::setQos( UInt32 timeliness, UInt32 rate )
{
	rsslClearQos( &getRsslRequestMsg()->qos );
	rsslClearQos( &getRsslRequestMsg()->worstQos );

	getRsslRequestMsg()->qos.dynamic = RSSL_FALSE;
	getRsslRequestMsg()->worstQos.dynamic = RSSL_FALSE;
	getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_QOS;

	switch ( rate )
	{
	case ReqMsg::TickByTickEnum :
		getRsslRequestMsg()->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		getRsslRequestMsg()->worstQos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		break;
	case ReqMsg::JustInTimeConflatedEnum :
		getRsslRequestMsg()->qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		getRsslRequestMsg()->worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		break;
	case ReqMsg::BestConflatedRateEnum :
		getRsslRequestMsg()->qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		getRsslRequestMsg()->qos.rateInfo = 1;
		getRsslRequestMsg()->worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	case ReqMsg::BestRateEnum :
		getRsslRequestMsg()->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		getRsslRequestMsg()->worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	default :
		if ( rate <= 65535 )
		{
			getRsslRequestMsg()->qos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
			getRsslRequestMsg()->qos.rateInfo = rate;
			getRsslRequestMsg()->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
			getRsslRequestMsg()->worstQos.rateInfo = rate;
		}
		else
		{
			getRsslRequestMsg()->qos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
			getRsslRequestMsg()->worstQos.rate = RSSL_QOS_RATE_JIT_CONFLATED;
		}
		break;
	}

	switch ( timeliness )
	{
	case ReqMsg::RealTimeEnum :
		getRsslRequestMsg()->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		getRsslRequestMsg()->worstQos.timeliness = RSSL_QOS_TIME_REALTIME;
		break;
	case ReqMsg::BestDelayedTimelinessEnum :
		getRsslRequestMsg()->qos.timeliness = RSSL_QOS_TIME_DELAYED;
		getRsslRequestMsg()->qos.timeInfo = 1;
		getRsslRequestMsg()->worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
		getRsslRequestMsg()->worstQos.timeInfo = 65535;
		getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	case ReqMsg::BestTimelinessEnum :
		getRsslRequestMsg()->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		getRsslRequestMsg()->worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
		getRsslRequestMsg()->worstQos.timeInfo = 65535;
		getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_WORST_QOS;
		break;
	default :
		if ( timeliness <= 65535 )
		{
			getRsslRequestMsg()->qos.timeliness = RSSL_QOS_TIME_DELAYED;
			getRsslRequestMsg()->qos.timeInfo = timeliness;
			getRsslRequestMsg()->worstQos.timeliness = RSSL_QOS_TIME_DELAYED;
			getRsslRequestMsg()->worstQos.timeInfo = timeliness;
		}
		else
		{
			getRsslRequestMsg()->qos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
			getRsslRequestMsg()->worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		}
	}

	if (getRsslRequestMsg()->flags & RSSL_RQMF_HAS_WORST_QOS)
	{
		getRsslRequestMsg()->qos.dynamic = RSSL_TRUE;
		getRsslRequestMsg()->worstQos.dynamic = RSSL_TRUE;
	}
}

void ReqMsgImpl::setInitialImage( bool initialImage )
{
	if ( initialImage )
		applyHasNoFlag(getRsslRequestMsg(), RSSL_RQMF_NO_REFRESH);
	else
		rsslRequestMsgApplyNoRefresh(getRsslRequestMsg());
}

void ReqMsgImpl::setInterestAfterRefresh( bool interestAfterRefresh )
{
	if ( interestAfterRefresh )
		rsslRequestMsgApplyStreaming(getRsslRequestMsg());
	else
		applyHasNoFlag(getRsslRequestMsg(), RSSL_RQMF_STREAMING);
}

void ReqMsgImpl::setPause( bool pause )
{
	if ( pause )
		rsslRequestMsgApplyPause(getRsslRequestMsg());
	else
		applyHasNoFlag(getRsslRequestMsg(), RSSL_RQMF_PAUSE);
}

void ReqMsgImpl::setConflatedInUpdates( bool conflatedInUpdates )
{
	if ( conflatedInUpdates )
		rsslRequestMsgApplyConfInfoInUpdates(getRsslRequestMsg());
	else
		applyHasNoFlag(getRsslRequestMsg(), RSSL_RQMF_CONF_INFO_IN_UPDATES);
}

void ReqMsgImpl::setPrivateStream( bool privateStream )
{
	if ( privateStream )
		rsslRequestMsgApplyPrivateStream(getRsslRequestMsg());
	else
		applyHasNoFlag(getRsslRequestMsg(), RSSL_RQMF_PRIVATE_STREAM);
}

void ReqMsgImpl::checkBatchView( RsslBuffer* pRsslBuffer )
{
	RsslElementList	rsslElementList;
	RsslElementEntry rsslElementEntry;
	RsslDecodeIterator decodeIter;

	rsslClearDecodeIterator( &decodeIter );
	rsslClearElementEntry( &rsslElementEntry );
	rsslClearElementList( &rsslElementList );

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIter, pRsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to set iterator buffer in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp, retCode );
		return;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to set iterator version in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp, retCode );
		return;
	}

	retCode = rsslDecodeElementList( &decodeIter, &rsslElementList, 0 );

	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to decode ElementList in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp, retCode );
		return;
	}

	while ( true )
	{
		retCode = rsslDecodeElementEntry( &decodeIter, &rsslElementEntry );

		switch ( retCode )
		{
		case RSSL_RET_END_OF_CONTAINER :
				return;

		case RSSL_RET_SUCCESS :

			if ( rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_VIEW_DATA ) && rsslElementEntry.dataType == RSSL_DT_ARRAY )
				getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_VIEW;

			else if ( rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST ) && rsslElementEntry.dataType == RSSL_DT_ARRAY )
			{
				RsslArray rsslArray;
				rsslClearArray( &rsslArray );
				rsslArray.encData = rsslElementEntry.encData;

				if ( rsslDecodeArray( &decodeIter, &rsslArray ) >= RSSL_RET_SUCCESS )
				{
					if ( rsslArray.primitiveType == RSSL_DT_ASCII_STRING )
					{
						RsslBuffer rsslBuffer;

						while ( rsslDecodeArrayEntry( &decodeIter, &rsslBuffer ) != RSSL_RET_END_OF_CONTAINER )
							getRsslRequestMsg()->flags |= RSSL_RQMF_HAS_BATCH;
					}
				}
			}
			break;

		case RSSL_RET_INCOMPLETE_DATA :
		case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		default :
			{
				EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to decode ElementEntry. Internal error " );
				temp.append( rsslRetCodeToString( retCode ) );
				throwIueException( temp, retCode );
			}
			return;
		}
	}
}

UInt32 ReqMsgImpl::getBatchItemList(EmaVector<EmaString>* nameVector) const
{
	if ( !( getRsslRequestMsg()->flags & RSSL_RQMF_HAS_BATCH ) ) return 0;

	RsslElementList	rsslElementList;
	RsslElementEntry rsslElementEntry;
	RsslDecodeIterator decodeIter;

	rsslClearDecodeIterator( &decodeIter );
	rsslClearElementEntry( &rsslElementEntry );
	rsslClearElementList( &rsslElementList );

	RsslBuffer tempRsslBuffer = getRsslRequestMsg()->msgBase.encDataBody;

	RsslRet retCode = rsslSetDecodeIteratorBuffer( &decodeIter, &tempRsslBuffer );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to set iterator buffer in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp, retCode );
		return 0;
	}

	retCode = rsslSetDecodeIteratorRWFVersion( &decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
	if ( RSSL_RET_SUCCESS != retCode )
	{
		EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to set iterator version in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp, retCode );
		return 0;
	}

	retCode = rsslDecodeElementList( &decodeIter, &rsslElementList, 0 );

	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to decode ElementList in ReqMsg::payload(). Internal error " );
		temp.append( rsslRetCodeToString( retCode ) );
		throwIueException( temp, retCode );
		return 0;
	}

	while ( true )
	{
		retCode = rsslDecodeElementEntry( &decodeIter, &rsslElementEntry );

		switch ( retCode )
		{
		case RSSL_RET_END_OF_CONTAINER :
			return 0;

		case RSSL_RET_SUCCESS :
			{
				UInt32 batchListSize = 0;

				if ( rsslBufferIsEqual( &rsslElementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST ) && rsslElementEntry.dataType == RSSL_DT_ARRAY )
				{
					RsslArray rsslArray;
					rsslClearArray( &rsslArray );
					rsslArray.encData = rsslElementEntry.encData;

					if ( rsslDecodeArray( &decodeIter, &rsslArray ) >= RSSL_RET_SUCCESS )
					{
						if ( rsslArray.primitiveType == RSSL_DT_ASCII_STRING )
						{
							RsslBuffer rsslBuffer;
							while ( rsslDecodeArrayEntry( &decodeIter, &rsslBuffer ) != RSSL_RET_END_OF_CONTAINER )
							{
								if (nameVector != NULL)
									nameVector->push_back(EmaString(rsslBuffer.data, rsslBuffer.length));

								batchListSize++;
						}
					}

					return batchListSize;
				}
			}
			break;
			}
		case RSSL_RET_INCOMPLETE_DATA :
		case RSSL_RET_UNSUPPORTED_DATA_TYPE :
		default :
			{
				EmaString temp( "ReqMsgImpl::checkBatchView(): Failed to decode ElementEntry. Internal error " );
				temp.append( rsslRetCodeToString( retCode ) );
				throwIueException( temp, retCode );
			}
			return 0;
		}
	}

	return 0;
}


bool ReqMsgImpl::getViewPayload(EmaBuffer& payloadBuffer) const
{
	if (!hasView()) return 0;

	payloadBuffer.clear();

	RsslElementList	rsslElementList;
	RsslElementEntry rsslElementEntry;
	RsslDecodeIterator decodeIter;
	RsslEncodeIterator encodeIter;
	RsslElementList encElementList;

	bool foundViewType = false;
	bool foundViewData = false;

	rsslClearEncodeIterator(&encodeIter);
	rsslClearElementList(&encElementList);

	rsslClearDecodeIterator(&decodeIter);
	rsslClearElementEntry(&rsslElementEntry);
	rsslClearElementList(&rsslElementList);

	RsslBuffer tempRsslBuffer = getRsslRequestMsg()->msgBase.encDataBody;

	FixedBuffer tmpBufferData;
	tmpBufferData.reserve(tempRsslBuffer.length + 8);

	char* tmpBuffer = tmpBufferData.data();

	RsslBuffer tempEncBuffer;
	tempEncBuffer.data = tmpBuffer;
	// Add in a small buffer to make sure that the encoding can be handled.
	tempEncBuffer.length = tempRsslBuffer.length + 8;

	RsslRet retCode = rsslSetDecodeIteratorBuffer(&decodeIter, &tempRsslBuffer);
	if (RSSL_RET_SUCCESS != retCode)
	{
		EmaString temp("ReqMsgImpl::getViewPayload(): Failed to set iterator buffer in ReqMsg::payload(). Internal error ");
		temp.append(rsslRetCodeToString(retCode));
		throwIueException(temp, retCode);
		return false;
	}

	retCode = rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	if (RSSL_RET_SUCCESS != retCode)
	{
		EmaString temp("ReqMsgImpl::getViewPayload(): Failed to set iterator version in ReqMsg::payload(). Internal error ");
		temp.append(rsslRetCodeToString(retCode));
		throwIueException(temp, retCode);
		return false;
	}

	retCode = rsslSetEncodeIteratorBuffer(&encodeIter, &tempEncBuffer);
	if (RSSL_RET_SUCCESS != retCode)
	{
		EmaString temp("ReqMsgImpl::getViewPayload(): Failed to set iterator buffer in ReqMsg::payload(). Internal error ");
		temp.append(rsslRetCodeToString(retCode));
		throwIueException(temp, retCode);
		return false;
	}

	retCode = rsslSetEncodeIteratorRWFVersion(&encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	if (RSSL_RET_SUCCESS != retCode)
	{
		EmaString temp("ReqMsgImpl::getViewPayload(): Failed to set iterator version in ReqMsg::payload(). Internal error ");
		temp.append(rsslRetCodeToString(retCode));
		throwIueException(temp, retCode);
		return false;
	}

	retCode = rsslDecodeElementList(&decodeIter, &rsslElementList, 0);

	if (retCode != RSSL_RET_SUCCESS)
	{
		EmaString temp("ReqMsgImpl::getViewPayload(): Failed to decode ElementList in ReqMsg::payload(). Internal error ");
		temp.append(rsslRetCodeToString(retCode));
		throwIueException(temp, retCode);
		return false;
	}

	encElementList.flags = RSSL_ELF_HAS_STANDARD_DATA;

	retCode = rsslEncodeElementListInit(&encodeIter, &encElementList, NULL, 0);

	if (retCode != RSSL_RET_SUCCESS)
	{
		EmaString temp("ReqMsgImpl::getViewPayload(): Failed to encode ElementList in ReqMsg::payload(). Internal error ");
		temp.append(rsslRetCodeToString(retCode));
		throwIueException(temp, retCode);
		return false;
	}

	while (true)
	{
		retCode = rsslDecodeElementEntry(&decodeIter, &rsslElementEntry);

		switch (retCode)
		{
			case RSSL_RET_END_OF_CONTAINER:
			{
				if (foundViewType && foundViewData)
				{
					retCode = rsslEncodeElementListComplete(&encodeIter, RSSL_TRUE);

					if (retCode != RSSL_RET_SUCCESS)
					{
						EmaString temp("ReqMsgImpl::getViewPayload(): Failed to encode ElementEntry in ReqMsg::payload(). Internal error ");
						temp.append(rsslRetCodeToString(retCode));
						throwIueException(temp, retCode);
						return false;
					}

					tempEncBuffer.length = rsslGetEncodedBufferLength(&encodeIter);

					payloadBuffer.setFrom(tempEncBuffer.data, tempEncBuffer.length);

					return true;
				}

				return false;
			}

			case RSSL_RET_SUCCESS:
			{
				if (rsslBufferIsEqual(&rsslElementEntry.name, &RSSL_ENAME_VIEW_TYPE) && rsslElementEntry.dataType == RSSL_DT_UINT)
				{
					// There's a view type, so just re-encode the element entry as-is into the new list.
					retCode = rsslEncodeElementEntry(&encodeIter, &rsslElementEntry, NULL);
					foundViewType = true;
					if (retCode != RSSL_RET_SUCCESS)
					{
						EmaString temp("ReqMsgImpl::getViewPayload(): Failed to encode ElementEntry in ReqMsg::payload(). Internal error ");
						temp.append(rsslRetCodeToString(retCode));
						throwIueException(temp, retCode);
						return false;
					}
				}

				if (rsslBufferIsEqual(&rsslElementEntry.name, &RSSL_ENAME_VIEW_DATA) && rsslElementEntry.dataType == RSSL_DT_ARRAY)
				{
					// There's a view data array, so just re-encode the element entry as-is into the new list.
					retCode = rsslEncodeElementEntry(&encodeIter, &rsslElementEntry, NULL);
					foundViewData = true;
					if (retCode != RSSL_RET_SUCCESS)
					{
						EmaString temp("ReqMsgImpl::getViewPayload(): Failed to encode ElementEntry in ReqMsg::payload(). Internal error ");
						temp.append(rsslRetCodeToString(retCode));
						throwIueException(temp, retCode);
						return false;
					}
				}
				break;
			}

			case RSSL_RET_INCOMPLETE_DATA:
			case RSSL_RET_UNSUPPORTED_DATA_TYPE:
			default:
			{
				EmaString temp("ReqMsgImpl::getViewPayload(): Failed to decode ElementEntry. Internal error ");
				temp.append(rsslRetCodeToString(retCode));
				throwIueException(temp, retCode);
				return false;
			}
		}
	}

	return false;
}


bool ReqMsgImpl::isDomainTypeSet() const
{
	return _domainTypeSet;
}
