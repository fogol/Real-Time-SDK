/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Attrib.h"
#include "FieldList.h"
#include "ElementList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "FilterList.h"
#include "OmmOpaque.h"
#include "OmmXml.h"
#include "OmmJson.h"
#include "OmmAnsiPage.h"
#include "ReqMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "PostMsg.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "MsgImpl.h"

#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

Attrib::Attrib() :
 _pMsgImpl( nullptr )
{
}

Attrib::~Attrib()
{
}

DataType::DataTypeEnum Attrib::getDataType() const
{
	if (_pMsgImpl->hasAttrib<const MsgImpl>())
		return _pMsgImpl->getAttribData().getDataType();
	else
		return DataType::DataTypeEnum::NoDataEnum;
}

const ComplexType& Attrib::getData() const
{
	return static_cast<const ComplexType&>( _pMsgImpl->getAttribData() );
}

const ReqMsg& Attrib::getReqMsg() const
{
	if ( getDataType() != DataType::ReqMsgEnum )
	{
		EmaString temp( "Attempt to getReqMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ReqMsg&>( _pMsgImpl->getAttribData() );
}

const RefreshMsg& Attrib::getRefreshMsg() const
{
	if ( getDataType() != DataType::RefreshMsgEnum )
	{
		EmaString temp( "Attempt to getRefreshMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const RefreshMsg&>( _pMsgImpl->getAttribData() );
}

const UpdateMsg& Attrib::getUpdateMsg() const
{
	if ( getDataType() != DataType::UpdateMsgEnum )
	{
		EmaString temp( "Attempt to getUpdateMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const UpdateMsg&>( _pMsgImpl->getAttribData() );
}

const StatusMsg& Attrib::getStatusMsg() const
{
	if ( getDataType() != DataType::StatusMsgEnum )
	{
		EmaString temp( "Attempt to getStatusMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const StatusMsg&>( _pMsgImpl->getAttribData() );
}

const PostMsg& Attrib::getPostMsg() const
{
	if ( getDataType() != DataType::PostMsgEnum )
	{
		EmaString temp( "Attempt to getPostMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const PostMsg&>( _pMsgImpl->getAttribData() );
}

const AckMsg& Attrib::getAckMsg() const
{
	if ( getDataType() != DataType::AckMsgEnum )
	{
		EmaString temp( "Attempt to getAckMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const AckMsg&>( _pMsgImpl->getAttribData() );
}

const GenericMsg& Attrib::getGenericMsg() const
{
	if ( getDataType() != DataType::GenericMsgEnum )
	{
		EmaString temp( "Attempt to getGenericMsg() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const GenericMsg&>( _pMsgImpl->getAttribData() );
}

const FieldList& Attrib::getFieldList() const
{
	if ( getDataType() != DataType::FieldListEnum )
	{
		EmaString temp( "Attempt to getFieldList() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FieldList&>( _pMsgImpl->getAttribData() );
}

const ElementList& Attrib::getElementList() const
{
	if ( getDataType() != DataType::ElementListEnum )
	{
		EmaString temp( "Attempt to getElementList() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const ElementList&>( _pMsgImpl->getAttribData() );
}

const Map& Attrib::getMap() const
{
	if ( getDataType() != DataType::MapEnum )
	{
		EmaString temp( "Attempt to getMap() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Map&>( _pMsgImpl->getAttribData() );
}

const Vector& Attrib::getVector() const
{
	if ( getDataType() != DataType::VectorEnum )
	{
		EmaString temp( "Attempt to getVector() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Vector&>( _pMsgImpl->getAttribData() );
}

const Series& Attrib::getSeries() const
{
	if ( getDataType() != DataType::SeriesEnum )
	{
		EmaString temp( "Attempt to getSeries() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const Series&>( _pMsgImpl->getAttribData() );
}

const FilterList& Attrib::getFilterList() const
{
	if ( getDataType() != DataType::FilterListEnum )
	{
		EmaString temp( "Attempt to getFilterList() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const FilterList&>( _pMsgImpl->getAttribData() );
}

const OmmOpaque& Attrib::getOpaque() const
{
	if ( getDataType() != DataType::OpaqueEnum )
	{
		EmaString temp( "Attempt to getOpaque() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmOpaque&>( _pMsgImpl->getAttribData() );
}

const OmmXml& Attrib::getXml() const
{
	if ( getDataType() != DataType::XmlEnum )
	{
		EmaString temp( "Attempt to getXml() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmXml&>( _pMsgImpl->getAttribData() );
}

const OmmJson& Attrib::getJson() const
{
	if ( getDataType() != DataType::JsonEnum )
	{
		EmaString temp( "Attempt to getJson() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmJson&>( _pMsgImpl->getAttribData() );
}

const OmmAnsiPage& Attrib::getAnsiPage() const
{
	if ( getDataType() != DataType::AnsiPageEnum )
	{
		EmaString temp( "Attempt to getAnsiPage() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmAnsiPage&>( _pMsgImpl->getAttribData() );
}

const OmmError& Attrib::getError() const
{
	if ( getDataType() != DataType::ErrorEnum )
	{
		EmaString temp( "Attempt to getError() while actual dataType is " );
		temp += getDTypeAsString( getDataType() );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return static_cast<const OmmError&>( _pMsgImpl->getAttribData() );
}
