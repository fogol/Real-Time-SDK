/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "StatusMsg.h"
#include "StatusMsgImpl.h"

#include "EmaBufferInt.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "RdmUtilities.h"
#include "StaticDecoder.h"

#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

StatusMsg::StatusMsg() :
  Msg()
{
	setImpl( g_pool.getStatusMsgImplItem() );
}

StatusMsg::StatusMsg( UInt32 size ) :
  StatusMsg()
{
	impl()->Arena().reserve(size);
}

StatusMsg::StatusMsg( const StatusMsg& other ) :
  Msg()
{
	StatusMsgImpl* pStatusImpl = g_pool.getStatusMsgImplItem();

	*pStatusImpl = *other.impl();

	setImpl( pStatusImpl );
}

StatusMsg::StatusMsg( StatusMsg&& other) noexcept :
  Msg()
{
	setImpl( other.impl() );

	other.setImpl( nullptr );
}

StatusMsg& StatusMsg::operator=( const StatusMsg& other )
{
	if (this == &other)
		return *this;

	*impl() = *other.impl();

	return *this;
}

StatusMsg& StatusMsg::operator=( StatusMsg&& other ) noexcept
{
	if (this == &other)
		return *this;

	if ( impl() )
	{
		impl()->deallocateBuffers<StatusMsgImpl>();
		g_pool.returnItem( impl() );
	}

	setImpl( other.impl() );
	other.setImpl( nullptr );

	return *this;
}

StatusMsg::~StatusMsg()
{
	if ( impl() )
	{
		impl()->deallocateBuffers<StatusMsgImpl>();
		g_pool.returnItem( impl() );

		setImpl( nullptr );
	}
}

StatusMsg& StatusMsg::clear()
{
	EMA_ASSERT( impl() != nullptr, "Private implementation can't be missing");

	impl()->clear();

	return *this;
}

Data::DataCode StatusMsg::getCode() const
{
	return Data::NoCodeEnum;
}

DataType::DataTypeEnum StatusMsg::getDataType() const
{
	return DataType::StatusMsgEnum;
}

const EmaString& StatusMsg::toString() const
{
	return toString(0);
}

const EmaString& StatusMsg::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
	{
		return _toString.clear().append("\nDictionary is not loaded.\n");
	}

	// clone this message but with the specified dictionary instead of the original
	StatusMsg statusMsg;

	RsslBuffer& rsslBuffer = getEncoder().getEncodedBuffer();

	StaticDecoder::setRsslData( &statusMsg, &rsslBuffer,
								RSSL_DT_MSG, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION,
								dictionary._pImpl->rsslDataDictionary() );

	// now produce toString from the cloned message (with the custom dictionary)
	_toString.clear().append(statusMsg.toString());

	return _toString;
}

const EmaString& StatusMsg::toString( UInt64 indent ) const
{
	_toString.clear();

	addIndent(_toString, indent++).append("StatusMsg");
	addIndent(_toString, indent, true).append("streamId=\"").append(getStreamId()).append("\"");
	addIndent(_toString, indent, true).append("domain=\"").append(rdmDomainToString(getDomainType())).append("\"");

	if (getPrivateStream())
		addIndent(_toString, indent, true).append("PrivateStream");

	if (getClearCache())
		addIndent(_toString, indent, true).append("ClearCache");

	if (hasState())
		addIndent(_toString, indent, true).append("state=\"").append(getState().toString()).append("\"");

	if (hasItemGroup())
	{
		EmaString temp;
		hexToString(temp, getItemGroup());
		addIndent(_toString, indent, true).append("itemGroup=\"").append(temp).append("\"");
	}

	if (hasPermissionData())
	{
		EmaString temp;
		hexToString(temp, getPermissionData());
		addIndent(_toString, indent, true).append("permissionData=\"").append(temp).append("\"");
	}

	if (hasPublisherId())
	{
		addIndent(_toString, indent, true).append("publisher user address=\"").append(getPublisherIdUserAddress()).append("\"");
		addIndent(_toString, indent, true).append("publisher user id=\"").append(getPublisherIdUserId()).append("\"");
	}

	indent--;
	if (hasMsgKey())
	{
		indent++;
		if (hasName())
			addIndent(_toString, indent, true).append("name=\"").append(getName()).append("\"");

		if (hasNameType())
			addIndent(_toString, indent, true).append("nameType=\"").append(getNameType()).append("\"");

		if (hasServiceId())
			addIndent(_toString, indent, true).append("serviceId=\"").append(getServiceId()).append("\"");

		if (hasServiceName())
			addIndent(_toString, indent, true).append("serviceName=\"").append(getServiceName()).append("\"");

		if (hasFilter())
			addIndent(_toString, indent, true).append("filter=\"").append(getFilter()).append("\"");

		if (hasId())
			addIndent(_toString, indent, true).append("id=\"").append(getId()).append("\"");

		indent--;

		if ( impl()->hasAttrib<const StatusMsgImpl>() )
		{
			indent++;
			addIndent(_toString, indent, true).append("Attrib dataType=\"")
				.append( getDTypeAsString(impl()->getAttribData().getDataType()) ).append("\"\n");

			indent++;
			_toString.append(impl()->getAttribData().toString( indent ));
			indent--;

			addIndent(_toString, indent, true).append("AttribEnd");
			indent--;
		}
	}

	if (hasExtendedHeader())
	{
		indent++;
		addIndent(_toString, indent, true).append("ExtendedHeader\n");

		indent++;
		addIndent(_toString, indent);
		hexToString(_toString, getExtendedHeader());
		indent--;

		addIndent(_toString, indent, true).append("ExtendedHeaderEnd");
		indent--;
	}

	if (impl()->hasPayload())
	{
		indent++;
		addIndent(_toString, indent, true).append("Payload dataType=\"").append(getDTypeAsString(impl()->getPayloadData().getDataType())).append("\"\n");

		indent++;
		_toString.append(impl()->getPayloadData().toString(indent));
		indent--;

		addIndent(_toString, indent, true).append("PayloadEnd");
		indent--;
	}

	addIndent(_toString, indent, true).append("StatusMsgEnd\n");

	return _toString;
}

const EmaBuffer& StatusMsg::getAsHex() const
{
	return impl()->getAsHex();
}

bool StatusMsg::hasItemGroup() const
{
	return impl()->hasItemGroup();
}

bool StatusMsg::hasState() const
{
	return impl()->hasState();
}

bool StatusMsg::hasPermissionData() const
{
	return impl()->hasPermissionData();
}

bool StatusMsg::hasPublisherId() const
{
	return impl()->hasPublisherId();
}

bool StatusMsg::hasServiceName() const
{
	return impl()->hasServiceName();
}

const OmmState& StatusMsg::getState() const
{
	return impl()->getState();
}

const EmaBuffer& StatusMsg::getItemGroup() const
{
	return impl()->getItemGroup();
}

const EmaBuffer& StatusMsg::getPermissionData() const
{
	return impl()->getPermissionData();
}

UInt32 StatusMsg::getPublisherIdUserId() const
{
	return impl()->getPublisherIdUserId();
}

UInt32 StatusMsg::getPublisherIdUserAddress() const
{
	return impl()->getPublisherIdUserAddress();
}

bool StatusMsg::getClearCache() const
{
	return impl()->getClearCache();
}

bool StatusMsg::getPrivateStream() const
{
	return impl()->getPrivateStream();
}

const EmaString& StatusMsg::getServiceName() const
{
	return impl()->getServiceName();
}

Decoder& StatusMsg::getDecoder()
{
	return impl()->_decoder;
}

StatusMsg& StatusMsg::streamId( Int32 streamId )
{
	impl()->setStreamId( streamId );
	return *this;
}

StatusMsg& StatusMsg::domainType( UInt16 domainType )
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

StatusMsg& StatusMsg::name( const EmaString& name )
{
	impl()->setName( name );
	return *this;
}

StatusMsg& StatusMsg::nameType( UInt8 nameType )
{
	impl()->setNameType( nameType );
	return *this;
}

StatusMsg& StatusMsg::serviceName( const EmaString& serviceName )
{

	impl()->copyServiceName( serviceName );
	return *this;
}

StatusMsg& StatusMsg::serviceId( UInt32 serviceId )
{
	impl()->setServiceId( serviceId );
	return *this;
}

StatusMsg& StatusMsg::id( Int32 id )
{
	impl()->setId( id );
	return *this;
}

StatusMsg& StatusMsg::filter( UInt32 filter )
{
	impl()->setFilter( filter );
	return *this;
}

StatusMsg& StatusMsg::state( OmmState::StreamState streamState, OmmState::DataState dataState,
						UInt8 statusCode, const EmaString& statusText )
{
	impl()->setState( streamState, dataState, statusCode, statusText );
	return *this;
}

StatusMsg& StatusMsg::itemGroup( const EmaBuffer& itemGroup )
{
	impl()->setItemGroup( itemGroup );
	return *this;
}

StatusMsg& StatusMsg::permissionData( const EmaBuffer& permissionData )
{
	impl()->setPermissionData( permissionData );
	return *this;
}

StatusMsg& StatusMsg::publisherId( UInt32 userId, UInt32 userAddress )
{
	impl()->setPublisherId( userId, userAddress );
	return *this;
}

StatusMsg& StatusMsg::attrib( const ComplexType& data )
{
	impl()->setAttrib( data );
	return *this;
}

StatusMsg& StatusMsg::payload( const ComplexType& data )
{
	impl()->setPayload( data );
	return *this;
}

StatusMsg& StatusMsg::extendedHeader( const EmaBuffer& buffer )
{
	impl()->setExtendedHeader( buffer );
	return *this;
}

StatusMsg& StatusMsg::clearCache( bool clearCache )
{
	impl()->setClearCache( clearCache );
	return *this;
}

StatusMsg& StatusMsg::privateStream( bool privateStream )
{
	impl()->setPrivateStream( privateStream );
	return *this;
}

StatusMsgImpl* StatusMsg::impl()
{
	return static_cast<StatusMsgImpl*>(_pImpl);
}

const StatusMsgImpl* StatusMsg::impl() const
{
	return static_cast<const StatusMsgImpl*>(_pImpl);
}
