/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "UpdateMsg.h"
#include "UpdateMsgImpl.h"
#include "EmaBufferInt.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

UpdateMsg::UpdateMsg() :
 Msg(),
 _toString()
{
	setImpl( g_pool.getUpdateMsgImplItem() );
}

UpdateMsg::UpdateMsg(UInt32 size) :
 UpdateMsg()
{
	impl()->Arena().reserve(size);
}

UpdateMsg::UpdateMsg(const UpdateMsg& other) :
 Msg(),
 _toString()
{
	setImpl( g_pool.getUpdateMsgImplItem() );
	*impl() = *other.impl();
}

UpdateMsg::UpdateMsg( UpdateMsg&& other ) noexcept :
 Msg(),
 _toString()
{
	setImpl( other.impl() );
	other.setImpl( nullptr );
}

UpdateMsg& UpdateMsg::operator=( const UpdateMsg& other )
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

UpdateMsg& UpdateMsg::operator=( UpdateMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if ( impl() )
	{
		impl()->deallocateBuffers<UpdateMsgImpl>();
		g_pool.returnItem( impl() );
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

UpdateMsg::~UpdateMsg()
{
	if ( impl() )
	{
		impl()->deallocateBuffers<UpdateMsgImpl>();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

UpdateMsg& UpdateMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();

	return *this;
}

Data::DataCode UpdateMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum UpdateMsg::getDataType() const
{
	return DataType::UpdateMsgEnum;
}

const EmaString& UpdateMsg::toString() const
{
	return toString( 0 );
}

const EmaString& UpdateMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	UpdateMsg updateMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &updateMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary() );

	_toString.clear().append(updateMsg.toString());

	return _toString;
}

const EmaString& UpdateMsg::toString( UInt64 indent ) const
{
	_toString.clear();

	addIndent( _toString, indent++ ).append( "UpdateMsg" );
	addIndent( _toString, indent, true ).append( "streamId=\"" ).append( getStreamId() ).append( "\"" );
	addIndent( _toString, indent, true ).append( "domain=\"" ).append( rdmDomainToString( getDomainType() ) ).append( "\"" );
	addIndent( _toString, indent, true ).append( "updateTypeNum=\"" ).append( getUpdateTypeNum() ).append( "\"" );

	if ( getDoNotCache() )
		addIndent( _toString, indent, true ).append( "DoNotCache" );

	if ( getDoNotConflate() )
		addIndent( _toString, indent, true ).append( "DoNotConflate" );

	if ( getDoNotRipple() )
		addIndent( _toString, indent, true ).append( "DoNotRipple" );

	if ( hasPermissionData() )
	{
		EmaString temp;
		hexToString( temp, getPermissionData() );
		addIndent( _toString, indent, true ).append( "permissionData=\"" ).append( temp ).append( "\"" );
	}

	if ( hasConflated() )
	{
		addIndent( _toString, indent, true ).append( "conflatedCount=\"" ).append( getConflatedCount() ).append( "\"" );
		addIndent( _toString, indent, true ).append( "conflatedTime=\"" ).append( getConflatedTime() ).append( "\"" );
	}

	if ( hasSeqNum() )
		addIndent( _toString, indent, true ).append( "seqNum=\"" ).append( getSeqNum() ).append( "\"" );

	if ( hasPublisherId() )
	{
		addIndent( _toString, indent, true ).append( "publisher user address=\"" ).append( getPublisherIdUserAddress() ).append( "\"" );
		addIndent( _toString, indent, true ).append( "publisher user id=\"" ).append( getPublisherIdUserId() ).append( "\"" );
	}

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

		if ( impl()->hasAttrib<const UpdateMsgImpl>() )
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

	addIndent( _toString, indent, true ).append( "UpdateMsgEnd\n" );

	return _toString;
}

const EmaBuffer& UpdateMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool UpdateMsg::hasSeqNum() const
{
	return impl()->hasSeqNum();
}

bool UpdateMsg::hasPermissionData() const
{
	return impl()->hasPermissionData();
}

bool UpdateMsg::hasConflated() const
{
	return impl()->hasConflated();
}

bool UpdateMsg::hasPublisherId() const
{
	return impl()->hasPublisherId();
}

bool UpdateMsg::hasServiceName() const
{
	return impl()->hasServiceName();
}

UInt8 UpdateMsg::getUpdateTypeNum() const
{
	return impl()->getUpdateTypeNum();
}

UInt32 UpdateMsg::getSeqNum() const
{
	return impl()->getSeqNum();
}

const EmaBuffer& UpdateMsg::getPermissionData() const
{
	return impl()->getPermissionData();
}

UInt16 UpdateMsg::getConflatedTime() const
{
	return impl()->getConflatedTime();
}

UInt16 UpdateMsg::getConflatedCount() const
{
	return impl()->getConflatedCount();
}

UInt32 UpdateMsg::getPublisherIdUserId() const
{
	return impl()->getPublisherIdUserId();
}

UInt32 UpdateMsg::getPublisherIdUserAddress() const
{
	return impl()->getPublisherIdUserAddress();
}

bool UpdateMsg::getDoNotCache() const
{
	return impl()->getDoNotCache();
}

bool UpdateMsg::getDoNotConflate() const
{
	return impl()->getDoNotConflate();
}

bool UpdateMsg::getDoNotRipple() const
{
	return impl()->getDoNotRipple();
}

const EmaString& UpdateMsg::getServiceName() const
{
	return impl()->getServiceName();
}

Decoder& UpdateMsg::getDecoder()
{
	if ( !impl() )
		setImpl( g_pool.getUpdateMsgImplItem() );

	return impl()->_decoder;
}

UpdateMsg& UpdateMsg::streamId( Int32 streamId )
{
	impl()->setStreamId( streamId );
	return *this;
}

UpdateMsg& UpdateMsg::domainType( UInt16 domainType )
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

UpdateMsg& UpdateMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

UpdateMsg& UpdateMsg::nameType( UInt8 nameType )
{
	impl()->setNameType( nameType );
	return *this;
}

UpdateMsg& UpdateMsg::serviceName( const EmaString& serviceName )
{
	impl()->copyServiceName( serviceName );
	return *this;
}

UpdateMsg& UpdateMsg::serviceId( UInt32 serviceId )
{
	impl()->setServiceId( serviceId );
	return *this;
}

UpdateMsg& UpdateMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

UpdateMsg& UpdateMsg::filter( UInt32 filter )
{
	impl()->setFilter( filter );
	return *this;
}

UpdateMsg& UpdateMsg::updateTypeNum( UInt8 updateTypeNum )
{
	impl()->setUpdateTypeNum( updateTypeNum );
	return *this;
}

UpdateMsg& UpdateMsg::seqNum( UInt32 seqNum )
{
	impl()->setSeqNum( seqNum );
	return *this;
}

UpdateMsg& UpdateMsg::permissionData( const EmaBuffer& permissionData )
{
	impl()->setPermissionData( permissionData );
	return *this;
}

UpdateMsg& UpdateMsg::conflated( UInt16 count, UInt16 time )
{
	impl()->setConflated( count, time );
	return *this;
}

UpdateMsg& UpdateMsg::publisherId( UInt32 userId, UInt32 userAddress )
{
	impl()->setPublisherId( userId, userAddress );
	return *this;
}

UpdateMsg& UpdateMsg::attrib( const ComplexType& data )
{
	impl()->setAttrib( data );
	return *this;
}

UpdateMsg& UpdateMsg::payload( const ComplexType& data )
{
	impl()->setPayload( data );
	return *this;
}

UpdateMsg& UpdateMsg::extendedHeader( const EmaBuffer& buffer )
{
	impl()->setExtendedHeader( buffer );
	return *this;
}

UpdateMsg& UpdateMsg::doNotCache( bool doNotCache )
{
	impl()->setDoNotCache( doNotCache );
	return *this;
}

UpdateMsg& UpdateMsg::doNotConflate( bool doNotConflate )
{
	impl()->setDoNotConflate( doNotConflate );
	return *this;
}

UpdateMsg& UpdateMsg::doNotRipple( bool doNotRipple )
{
	impl()->setDoNotRipple( doNotRipple );
	return *this;
}

UpdateMsgImpl* UpdateMsg::impl()
{
	return static_cast<UpdateMsgImpl*>(_pImpl);
}

const UpdateMsgImpl* UpdateMsg::impl() const
{
	return static_cast<const UpdateMsgImpl*>(_pImpl);
}
