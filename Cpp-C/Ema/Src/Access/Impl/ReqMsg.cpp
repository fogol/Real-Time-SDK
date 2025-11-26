/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ReqMsg.h"
#include "ReqMsgImpl.h"

#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

const EmaString TickByTickName( "TickByTick" );
const EmaString JustInTimeConflatedRateName( "JustInTimeConflatedrate" );
const EmaString BestConflatedRateName( "BestConflatedRate" );
const EmaString BestRateName( "BestRate" );
EmaString UnknownReqMsgQosRateName;

const EmaString RealTimeName( "RealTime" );
const EmaString BestDelayedTimelinessName( "BestDelayedTimeliness" );
const EmaString BestTimelinessName( "BestTimeliness" );
EmaString UnknownReqMsgQosTimelinessName;

ReqMsg::ReqMsg() :
 Msg(),
 _toString()
{
	setImpl( g_pool.getReqMsgImplItem() );
}

ReqMsg::ReqMsg(UInt32 size) :
 ReqMsg()
{
	impl()->Arena().reserve(size);
}

ReqMsg::ReqMsg(const ReqMsg& other) :
 Msg(),
 _toString()
{
	setImpl( g_pool.getReqMsgImplItem() );
	*impl() = *other.impl();
}

ReqMsg::ReqMsg( ReqMsg&& other ) noexcept :
 Msg(),
 _toString()
{
	setImpl( other.impl() );
	other.setImpl( nullptr );
}

ReqMsg& ReqMsg::operator=( const ReqMsg& other )
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

ReqMsg& ReqMsg::operator=( ReqMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if ( _pImpl )
	{
		impl()->deallocateBuffers<ReqMsgImpl>();
		g_pool.returnItem( impl() );
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

ReqMsg::~ReqMsg()
{
	if ( impl() )
	{
		impl()->deallocateBuffers<ReqMsgImpl>();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

const EmaString& ReqMsg::getRateAsString() const
{
	switch ( getQosRate() )
	{
	case TickByTickEnum :
		return TickByTickName;
	case JustInTimeConflatedEnum :
		return JustInTimeConflatedRateName;
	case BestConflatedRateEnum :
		return BestConflatedRateName;
	case BestRateEnum :
		return BestRateName;
	default :
		return UnknownReqMsgQosRateName.set( "Rate on ReqMsg. Value = " ).append( (UInt64)getQosRate() );
	}
}

const EmaString& ReqMsg::getTimelinessAsString() const
{
	switch ( getQosTimeliness() )
	{
	case RealTimeEnum :
		return RealTimeName;
	case BestDelayedTimelinessEnum :
		return BestDelayedTimelinessName;
	case BestTimelinessEnum :
		return BestTimelinessName;
	default :
		return UnknownReqMsgQosTimelinessName.set( "QosTimeliness on ReqMsg. Value = " ).append( (UInt64)getQosTimeliness() );
	}
}

ReqMsg& ReqMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();

	return *this;
}

Data::DataCode ReqMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum ReqMsg::getDataType() const
{
	return DataType::ReqMsgEnum;
}

const EmaString& ReqMsg::toString() const
{
	return toString( 0 );
}

const EmaString& ReqMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	ReqMsg reqMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &reqMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary() );

	_toString.clear().append(reqMsg.toString());

	return _toString;
}

const EmaString& ReqMsg::toString(  UInt64 indent ) const
{
	_toString.clear();

	addIndent( _toString, indent++ ).append( "ReqMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );

	if ( getPrivateStream() )
		addIndent( _toString, indent, true ).append( "PrivateStream" );

	if ( getInitialImage() )
		addIndent( _toString, indent, true ).append( "InitialImage" );

	if ( getInterestAfterRefresh() )
		addIndent( _toString, indent, true ).append( "InterestAfterRefresh" );

	if ( getPause() )
		addIndent( _toString, indent, true ).append( "Pause" );

	if ( getConflatedInUpdates() )
		addIndent( _toString, indent, true ).append( "ConflatedInUpdates" );

	if ( hasPriority() )
		addIndent( _toString, indent, true )
			.append( "priorityClass=\"" ).append( getPriorityClass() ).append( "\"" )
			.append( " priorityCount=\"" ).append( getPriorityCount() ).append( "\"" );

	if ( hasQos() )
		addIndent( _toString, indent, true )
			.append( "qosRate=\"" ).append( getRateAsString() ).append( "\"" )
			.append( "qosTimeliness=\"" ).append( getTimelinessAsString() ).append( "\"" );

	indent--;
	if ( hasMsgKey() )
	{
		indent++;
		if ( hasName() )
			addIndent( _toString, indent, true ).append( "name=\"" ).append( getName() ).append( "\"" );

		if ( hasNameType() )
			addIndent( _toString, indent, true ).append( "nameType=\"" ).append( getNameType() ).append( "\"" );

		if ( hasServiceId() )
			addIndent( _toString, indent, true ).append( "serviceId=\"" ).append( getServiceId() ).append( "\"" );

		if ( hasServiceName() )
			addIndent( _toString, indent, true ).append( "serviceName=\"" ).append( getServiceName() ).append( "\"" );

		if ( hasFilter() )
			addIndent( _toString, indent, true ).append( "filter=\"" ).append( getFilter() ).append( "\"" );

		if ( hasId() )
			addIndent( _toString, indent, true ).append( "id=\"" ).append( getId() ).append( "\"" );

		indent--;
		if ( impl()->hasAttrib<const ReqMsgImpl>() )
		{
			indent++;
			addIndent( _toString, indent, true ).append( "Attrib dataType=\"" ).append( getDTypeAsString( impl()->getAttribData().getDataType() ) ).append( "\"\n" );

			indent++;
			_toString.append( impl()->getAttribData().toString( indent ) );
			indent--;

			addIndent( _toString, indent, true ).append( "AttribEnd" );
			indent--;
		}
	}

	if ( hasExtendedHeader() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "ExtendedHeader\n" );

		indent++;

		addIndent( _toString, indent );
		hexToString( _toString, getExtendedHeader() );

		indent--;

		addIndent( _toString, indent, true ).append( "ExtendedHeaderEnd" );
		indent--;
	}

	if ( impl()->hasPayload() )
	{
		indent++;
		addIndent( _toString, indent, true ).append( "Payload dataType=\"" ).append( getDTypeAsString( impl()->getPayloadData().getDataType() ) ).append( "\"\n" );

		indent++;
		_toString.append( impl()->getPayloadData().toString( indent ) );
		indent--;

		addIndent( _toString, indent, true ).append( "PayloadEnd" );
		indent--;
	}

	addIndent( _toString, indent, true ).append( "ReqMsgEnd\n" );

	return _toString;
}

const EmaBuffer& ReqMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool ReqMsg::hasPriority() const
{
	return impl()->hasPriority();
}

bool ReqMsg::hasQos() const
{
	return impl()->hasQos();
}

bool ReqMsg::hasView() const
{
	return impl()->hasView();
}

bool ReqMsg::hasBatch() const
{
	return impl()->hasBatch();
}

bool ReqMsg::hasServiceName() const
{
	return impl()->hasServiceName();
}

UInt8 ReqMsg::getPriorityClass() const
{
	return impl()->getPriorityClass();
}

UInt16 ReqMsg::getPriorityCount() const
{
	return impl()->getPriorityCount();
}

UInt32 ReqMsg::getQosTimeliness() const
{
	return impl()->getTimeliness();
}

UInt32 ReqMsg::getQosRate() const
{
	return impl()->getRate();
}

bool ReqMsg::getInitialImage() const
{
	return impl()->getInitialImage();
}

bool ReqMsg::getInterestAfterRefresh() const
{
	return impl()->getInterestAfterRefresh();
}

bool ReqMsg::getConflatedInUpdates() const
{
	return impl()->getConflatedInUpdates();
}

bool ReqMsg::getPause() const
{
	return impl()->getPause();
}

bool ReqMsg::getPrivateStream() const
{
	return impl()->getPrivateStream();
}

const EmaString& ReqMsg::getServiceName() const
{
	return impl()->getServiceName();
}

Decoder& ReqMsg::getDecoder()
{
	return impl()->_decoder;
}

ReqMsg& ReqMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

ReqMsg& ReqMsg::nameType( UInt8 nameType )
{

	impl()->setNameType( nameType );
	return *this;
}

ReqMsg& ReqMsg::serviceName( const EmaString& serviceName )
{
	if (hasServiceId() || impl()->hasServiceListName())
	{
		EmaString text( "Attempt to set serviceName while service id or service list name is already set." );
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
		return *this;
	}

	impl()->copyServiceName( serviceName );
	return *this;
}

ReqMsg& ReqMsg::serviceListName(const EmaString& serviceListName)
{
	if (hasServiceId() || hasServiceName())
	{
		EmaString text("Attempt to set serviceListName while service id or service name is already set.");
		throwIueException(text, OmmInvalidUsageException::InvalidOperationEnum);
		return *this;
	}

	impl()->setServiceListName(serviceListName);
	return *this;
}

ReqMsg& ReqMsg::serviceId( UInt32 serviceId )
{
	if ( hasServiceName() || impl()->hasServiceListName() )
	{
		EmaString text( "Attempt to set serviceId while service name or service list name is already set." );
		throwIueException( text, OmmInvalidUsageException::InvalidOperationEnum );
		return *this;
	}

	impl()->setServiceId( serviceId );
	return *this;
}

ReqMsg& ReqMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

ReqMsg& ReqMsg::filter( UInt32 filter )
{

	impl()->setFilter( filter );
	return *this;
}

ReqMsg& ReqMsg::streamId( Int32 streamId )
{

	impl()->setStreamId( streamId );
	return *this;
}

ReqMsg& ReqMsg::domainType( UInt16 domainType )
{
	if ( domainType > 255 )
	{
		EmaString temp( "Passed in DomainType is out of range." );
		throwDtuException( domainType, temp );
		return *this;
	}

	impl()->setDomainType( (UInt8)domainType );
	return *this;
}

ReqMsg& ReqMsg::priority( UInt8 priorityClass, UInt16 priorityCount )
{
	impl()->setPriority( priorityClass, priorityCount );
	return *this;
}

ReqMsg& ReqMsg::qos( UInt32 timeliness, UInt32 rate )
{
	impl()->setQos( timeliness, rate );
	return *this;
}

ReqMsg& ReqMsg::attrib( const ComplexType& data )
{

	impl()->setAttrib( data );
	return *this;
}

ReqMsg& ReqMsg::payload( const ComplexType& data )
{

	impl()->setPayload( data );
	return *this;
}

ReqMsg& ReqMsg::extendedHeader( const EmaBuffer& Buffer )
{
	impl()->setExtendedHeader( Buffer );
	return *this;
}

ReqMsg& ReqMsg::initialImage( bool initialImage )
{

	impl()->setInitialImage( initialImage );
	return *this;
}

ReqMsg& ReqMsg::interestAfterRefresh( bool interestAfterRefresh )
{
	impl()->setInterestAfterRefresh( interestAfterRefresh );
	return *this;
}

ReqMsg& ReqMsg::pause( bool pause )
{
	impl()->setPause( pause );
	return *this;
}

ReqMsg& ReqMsg::conflatedInUpdates( bool conflatedInUpdates )
{
	impl()->setConflatedInUpdates( conflatedInUpdates );
	return *this;
}

ReqMsg& ReqMsg::privateStream( bool privateStream )
{
	impl()->setPrivateStream( privateStream );
	return *this;
}

ReqMsgImpl* ReqMsg::impl()
{
	return static_cast<ReqMsgImpl*>(_pImpl);
}

const ReqMsgImpl* ReqMsg::impl() const
{
	return static_cast<const ReqMsgImpl*>(_pImpl);
}
