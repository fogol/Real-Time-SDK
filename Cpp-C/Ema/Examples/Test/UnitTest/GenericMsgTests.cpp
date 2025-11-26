/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|        Copyright (C) 2019-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include <array>

#include "TestUtilities.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

TEST(GenericMsgTests, testGenericMsgwithRefreshMsg)
{

	RsslDataDictionary dictionary;

	const EmaString genericMsgString =
		"GenericMsg\n"
		"    streamId=\"0\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"    Payload dataType=\"RefreshMsg\"\n"
		"        RefreshMsg\n"
		"            streamId=\"0\"\n"
		"            domain=\"MarketPrice Domain\"\n"
		"            state=\"Open / Ok / None / 'Status Text'\"\n"
		"            itemGroup=\"00 00\"\n"
		"            Attrib dataType=\"ElementList\"\n"
		"                ElementList\n"
		"                    ElementEntry name=\"Int\" dataType=\"Int\" value=\"1234\"\n"
		"                    ElementEntry name=\"Ascii\" dataType=\"Ascii\" value=\"Ascii\"\n"
		"                ElementListEnd\n"
		"\n"
		"            AttribEnd\n"
		"            Payload dataType=\"ElementList\"\n"
		"                ElementList\n"
		"                    ElementEntry name=\"Int\" dataType=\"Int\" value=\"1234\"\n"
		"                    ElementEntry name=\"Ascii\" dataType=\"Ascii\" value=\"Ascii\"\n"
		"                ElementListEnd\n"
		"\n"
		"            PayloadEnd\n"
		"        RefreshMsgEnd\n"
		"\n"
		"    PayloadEnd\n"
		"GenericMsgEnd\n";

	const EmaString genericMsgEmptyString =
		"GenericMsg\n"
		"    streamId=\"0\"\n"
		"    domain=\"MarketPrice Domain\"\n"
		"GenericMsgEnd\n";

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	DataDictionary emaDataDictionary, emaDataDictionaryEmpty;

	try {
		emaDataDictionary.loadFieldDictionary( "RDMFieldDictionaryTest" );
		emaDataDictionary.loadEnumTypeDictionary( "enumtypeTest.def" );
	}
	catch ( const OmmException& ) {
		ASSERT_TRUE( false ) << "DataDictionary::loadFieldDictionary() failed to load dictionary information";
	}
	try
	{
		ElementList eList;

		eList.addInt( EmaString( "Int" ), 1234 )
		.addAscii( EmaString( "Ascii" ), "Ascii" )
		.complete();

		RefreshMsg refreshMsg;

		refreshMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		refreshMsg.attrib( eList );
		refreshMsg.payload( eList );

		StaticDecoder::setData( &refreshMsg, &dictionary );

		GenericMsg genMsg, genMsgEmpty;
		genMsg.payload( refreshMsg );

		EXPECT_FALSE( genMsg.hasExtendedHeader() );

		EXPECT_EQ( genMsg.toString(), genericMsgString) << "GenericMsg.toString() == toString() method can be used for just encoded object when the dictionary was previously loaded. Use toString(dictionary) for just encoded object.";

		EXPECT_EQ( genMsg.toString( emaDataDictionaryEmpty ), "\nDictionary is not loaded.\n" ) << "GenericMsg.toString() == Dictionary is not loaded.";

		EXPECT_STREQ( genMsgEmpty.toString(emaDataDictionary), genericMsgEmptyString ) << "GenericMsg.toString() == genericMsgEmptyString";

		EXPECT_FALSE( genMsg.hasExtendedHeader() );

		EXPECT_STREQ( genMsg.toString( emaDataDictionary ), genericMsgString) << "GenericMsg.toString() == genericMsgString";

		StaticDecoder::setData( &genMsg, &dictionary );
		EXPECT_FALSE( genMsg.hasExtendedHeader() );

		GenericMsg genMsgClone( genMsg );
		genMsgClone.clear();

		EXPECT_FALSE( genMsg.hasExtendedHeader() );

		EXPECT_STREQ( genMsgClone.toString( emaDataDictionary ), genericMsgEmptyString ) << "GenericMsg.toString() == genericMsgEmptyString";

		EXPECT_STREQ( genMsg.toString(), genericMsgString ) << "GenericMsg.toString() == genericMsgString";
	}
	catch ( const OmmException& )
	{

		EXPECT_FALSE( true ) << "RefreshMsg as Payload of GenericMsg - exception NOT expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgWithOpaque)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer opaqueValue;
		opaqueValue.data = ( char* )"482wfshfsrf2";
		opaqueValue.length = static_cast<rtrUInt32>( strlen( opaqueValue.data ) );

		encodeNonRWFData( &rsslBuf, &opaqueValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_OPAQUE;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::OpaqueEnum ) << "GenericMsg::getPayload().getDataType() == DataType::OpaqueEnum" ;

		EmaBuffer compareTo( opaqueValue.data, opaqueValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getOpaque().getBuffer(), compareTo ) << "GenericMsg::getPayload().getOpaque().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with Opaque payload - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgWithXml)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer xmlValue;
		xmlValue.data = ( char* )"<consumerList><consumer><name dataType=\"Ascii\" value=\"Consumer_1\"/></consumer></consumerList>";
		xmlValue.length = static_cast<rtrUInt32>( strlen( xmlValue.data ) );

		encodeNonRWFData( &rsslBuf, &xmlValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_XML;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::XmlEnum ) << "GenericMsg::getPayload().getDataType() == DataType::XmlEnum" ;

		EmaBuffer compareTo( xmlValue.data, xmlValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getXml().getBuffer(), compareTo ) << "GenericMsg::getPayload().getXml().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with Xml payload - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgWithJson)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[200];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 200;

		RsslBuffer jsonValue;
		jsonValue.data = ( char* )"{\"consumerList\":{\"consumer\":{\"name\":\"\",\"dataType\":\"Ascii\",\"value\":\"Consumer_1\"}}}";
		jsonValue.length = static_cast<rtrUInt32>( strlen( jsonValue.data ) );

		encodeNonRWFData( &rsslBuf, &jsonValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_JSON;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::JsonEnum ) << "GenericMsg::getPayload().getDataType() == DataType::JsonEnum" ;

		EmaBuffer compareTo( jsonValue.data, jsonValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getJson().getBuffer(), compareTo ) << "GenericMsg::getPayload().getJson().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with Json payload - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgWithAnsiPage)
{

	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RsslGenericMsg rsslGenericMsg;
		rsslClearGenericMsg( &rsslGenericMsg );

		RsslMsgKey msgKey;
		rsslClearMsgKey( &msgKey );

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ( "ABCDEF" );
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName( &msgKey );

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType( &msgKey );

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId( &msgKey );

		msgKey.identifier = 4;
		rsslMsgKeyApplyHasIdentifier( &msgKey );

		rsslGenericMsg.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey( &rsslGenericMsg );

		char buffer[100];
		RsslBuffer rsslBuf;
		rsslBuf.data = buffer;
		rsslBuf.length = 100;

		RsslBuffer ansiPageValue;
		ansiPageValue.data = ( char* )"$&@^@FRHFSORFEQ(*YQ)(E#QRY";
		ansiPageValue.length = static_cast<rtrUInt32>( strlen( ansiPageValue.data ) );

		encodeNonRWFData( &rsslBuf, &ansiPageValue );

		rsslGenericMsg.msgBase.encDataBody = rsslBuf;
		rsslGenericMsg.msgBase.containerType = RSSL_DT_ANSI_PAGE;

		GenericMsg genericMsg;

		StaticDecoder::setRsslData( &genericMsg, ( RsslMsg* )&rsslGenericMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		EXPECT_EQ( genericMsg.getPayload().getDataType(), DataType::AnsiPageEnum ) << "GenericMsg::getPayload().getDataType() == DataType::AnsiPageEnum" ;

		EmaBuffer compareTo( ansiPageValue.data, ansiPageValue.length );
		EXPECT_STREQ( genericMsg.getPayload().getAnsiPage().getBuffer(), compareTo ) << "GenericMsg::getPayload().getAnsiPage().getBuffer()" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Decode with AnsiPage payload - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgFieldListEncodeDecode)
{
	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( flEnc );
		genMsg.payload( flEnc );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;


		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::FieldListEnum" ;

		//get FieldList that is in the attrib of genMsg
		const FieldList& flAttrib = genMsg.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getLoad().getDataType() == DataType::FieldListEnum" ;

		//get FieldList that is in the payload of genMsg
		const FieldList& flPayload = genMsg.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}


		EXPECT_TRUE( true ) << "GenericMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( elEnc );
		genMsg.payload( elEnc );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ElementListEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ElementListEnum" ;

		//get ElementList that is in the attrib of genMsg
		const ElementList& elAttrib = genMsg.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ElementListEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ElementListEnum" ;

		//get ElementList that is in the payload of genMsg
		const ElementList& elPayload = genMsg.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}


		EXPECT_TRUE( true ) << "GenericMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( mapEnc );
		genMsg.payload( mapEnc );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::MapEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::MapEnum" ;

		//get Map that is in the attrib of genMsg
		const Map& mapAttrib = genMsg.getAttrib().getMap();
		//decode mapAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::MapEnum ) << "GenericMsg::getLoad().getDataType() == DataType::MapEnum" ;

		//get Map that is in the payload of genMsg
		const Map& mapPayload = genMsg.getPayload().getMap();
		//decode mapPayload
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}


		EXPECT_TRUE( true ) << "GenericMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgReqMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( flEnc );
		requestMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the attrib of genMsg
		const ReqMsg& reqMsgInAttrib = genMsg.getAttrib().getReqMsg();
		//get attrib FieldList from reqMsgInAttrib
		const FieldList& flAttrib = reqMsgInAttrib.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		//get payload FieldList from reqMsgInAttrib
		const FieldList& flPayload = reqMsgInAttrib.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the payload of genMsg
		const ReqMsg& reqMsgInPayload = genMsg.getPayload().getReqMsg();
		//get attrib FieldList from reqMsgInPayload
		const FieldList& flAttrib2 = reqMsgInPayload.getAttrib().getFieldList();
		//decode flAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib2 );
		}

		//get payload FieldList from reqMsgInPayload
		const FieldList& flPayload2 = reqMsgInPayload.getPayload().getFieldList();
		//decode flPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


TEST(GenericMsgTests, testGenericMsgReqMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( elEnc );
		requestMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;


		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the attrib of genMsg
		const ReqMsg& reqMsgInAttrib = genMsg.getAttrib().getReqMsg();
		//get attrib ElementList from reqMsgInAttrib
		const ElementList& elAttrib = reqMsgInAttrib.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		//get payload ElementList from reqMsgInAttrib
		const ElementList& elPayload = reqMsgInAttrib.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the payload of genMsg
		const ReqMsg& reqMsgInPayload = genMsg.getPayload().getReqMsg();
		//get attrib ElementList from reqMsgInPayload
		const ElementList& elAttrib2 = reqMsgInPayload.getAttrib().getElementList();
		//decode elAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib2 );
		}

		//get payload ElementList from reqMsgInPayload
		const ElementList& elPayload2 = reqMsgInPayload.getPayload().getElementList();
		//decode elPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgReqMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		Map mapEnc;
		EmaEncodeMapAll( mapEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( mapEnc );
		requestMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the attrib of genMsg
		const ReqMsg& reqMsgInAttrib = genMsg.getAttrib().getReqMsg();
		//get attrib Map from reqMsgInAttrib
		const Map& mapAttrib = reqMsgInAttrib.getAttrib().getMap();
		//decode mapAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		//get payload Map from reqMsgInAttrib
		const Map& mapPayload = reqMsgInAttrib.getPayload().getMap();
		//decode mapPayload
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::ReqMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::ReqMsgEnum" ;

		//get ReqMsg that is in the payload of genMsg
		const ReqMsg& reqMsgInPayload = genMsg.getPayload().getReqMsg();
		//get attrib Map from reqMsgInPayload
		const Map& mapAttrib2 = reqMsgInPayload.getAttrib().getMap();
		//decode mapAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib2 );
		}

		//get payload Map from reqMsgInPayload
		const Map& mapPayload2 = reqMsgInPayload.getPayload().getMap();
		//decode mapPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgFieldListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( flEnc );
		responseMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the attrib of genMsg
		const RefreshMsg& respMsgInAttrib = genMsg.getAttrib().getRefreshMsg();
		//get attrib FieldList from respMsgInAttrib
		const FieldList& flAttrib = respMsgInAttrib.getAttrib().getFieldList();
		//decode flAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib );
		}

		//get payload FieldList from respMsgInAttrib
		const FieldList& flPayload = respMsgInAttrib.getPayload().getFieldList();
		//decode flPayload
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getLoad().getDataType() == Data::RespMsgEnum" ;

		//get RespMsg that is in the payload of genMsg
		const RefreshMsg& respMsgInPayload = genMsg.getPayload().getRefreshMsg();
		//get attrib FieldList from respMsgInPayload
		const FieldList& flAttrib2 = respMsgInPayload.getAttrib().getFieldList();
		//decode flAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flAttrib2 );
		}

		//get payload FieldList from respMsgInPayload
		const FieldList& flPayload2 = respMsgInPayload.getPayload().getFieldList();
		//decode flPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeFieldListAll");
		  EmaDecodeFieldListAll( flPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg RespMsg FieldList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RespMsg FieldList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgElementListEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( elEnc );
		responseMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the attrib of genMsg
		const RefreshMsg& respMsgInAttrib = genMsg.getAttrib().getRefreshMsg();
		//get attrib ElementList from respMsgInAttrib
		const ElementList& elAttrib = respMsgInAttrib.getAttrib().getElementList();
		//decode elAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib );
		}

		//get payload ElementList from respMsgInAttrib
		const ElementList& elPayload = respMsgInAttrib.getPayload().getElementList();
		//decode elPayload
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getLoad().getDataType() == Data::RespMsgEnum" ;

		//get RespMsg that is in the payload of genMsg
		const RefreshMsg& respMsgInPayload = genMsg.getPayload().getRefreshMsg();
		//get attrib ElementList from respMsgInPayload
		const ElementList& elAttrib2 = respMsgInPayload.getAttrib().getElementList();
		//decode elAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elAttrib2 );
		}

		//get payload ElementList from respMsgInPayload
		const ElementList& elPayload2 = respMsgInPayload.getPayload().getElementList();
		//decode elPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeElementListAll");
		  EmaDecodeElementListAll( elPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg RespMsg ElementList Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RespMsg ElementList Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgMapEncodeDecode)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( mapEnc );
		responseMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );

		EXPECT_TRUE( genMsg.hasMsgKey() ) << "GenericMsg::hasMsgKey() == true" ;

		EXPECT_EQ( genMsg.getStreamId(), 3 ) << "GenericMsg::getStreamId()" ;

		EXPECT_EQ( genMsg.getDomainType(), MMT_MARKET_PRICE) << "GenericMsg::getDomainType()" ;

		EXPECT_TRUE( genMsg.hasName() ) << "GenericMsg::hasName() == true" ;
		EXPECT_STREQ( genMsg.getName(), "GENERICMSG" ) << "GenericMsg::getName()" ;

		EXPECT_TRUE( genMsg.hasNameType() ) << "GenericMsg::hasNameType() == true" ;
		EXPECT_EQ( genMsg.getNameType(), 1 ) << "GenericMsg::getNameType()" ;

		EXPECT_TRUE( genMsg.hasServiceId() ) << "GenericMsg::hasServiceId() == true" ;
		EXPECT_EQ( genMsg.getServiceId(), 2 ) << "GenericMsg::getServiceId()" ;

		EXPECT_TRUE( genMsg.hasId() ) << "GenericMsg::hasId() == true" ;
		EXPECT_EQ( genMsg.getId(), 4 ) << "GenericMsg::getId()" ;

		EXPECT_TRUE( genMsg.hasFilter() ) << "GenericMsg::hasFilter() == true" ;
		EXPECT_EQ( genMsg.getFilter(), 8 ) << "GenericMsg::getFilter()" ;

		EXPECT_TRUE( genMsg.hasExtendedHeader() ) << "GenericMsg::hasExtendedHeader() == true" ;
		EXPECT_STREQ( genMsg.getExtendedHeader(), headerG ) << "GenericMsg::getExtendedHeader()" ;

		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getAttrib().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the attrib of genMsg
		const RefreshMsg& respMsgInAttrib = genMsg.getAttrib().getRefreshMsg();
		//get attrib Map from respMsgInAttrib
		const Map& mapAttrib = respMsgInAttrib.getAttrib().getMap();
		//decode mapAttrib
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib );
		}

		//get payload Map from respMsgInAttrib
		const Map& mapPayload = respMsgInAttrib.getPayload().getMap();
		//decode mapPayload
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload );
		}

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::RefreshMsgEnum ) << "GenericMsg::getLoad().getDataType() == DataType::RefreshMsgEnum" ;

		//get RespMsg that is in the payload of genMsg
		const RefreshMsg& respMsgInPayload = genMsg.getPayload().getRefreshMsg();
		//get attrib Map from respMsgInPayload
		const Map& mapAttrib2 = respMsgInPayload.getAttrib().getMap();
		//decode mapAttrib2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapAttrib2 );
		}

		//get payload Map from respMsgInPayload
		const Map& mapPayload2 = respMsgInPayload.getPayload().getMap();
		//decode mapPayload2
		{
		  SCOPED_TRACE("calling EmaDecodeMapAll");
		  EmaDecodeMapAll( mapPayload2 );
		}


		EXPECT_TRUE( true ) << "GenericMsg RespMsg Map Encode and Decode - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RespMsg Map Encode and Decode - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}


//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgReqMsgFieldListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( flEnc );
		requestMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg FieldList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg FieldList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgReqMsgElementListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;


		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( elEnc );
		requestMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg ElementList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg ElementList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgReqMsgMapDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		ReqMsg requestMsg;


		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		requestMsg.streamId( 1 );
		requestMsg.name( name );
		requestMsg.nameType( 1 );
		requestMsg.serviceId( 2 );
		requestMsg.id( 4 );
		requestMsg.filter( 8 );
		requestMsg.extendedHeader( extendedHeader );
		requestMsg.attrib( mapEnc );
		requestMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( requestMsg );
		genMsg.payload( requestMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg ReqMsg Map Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg ReqMsg Map Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

//encoding by EMA and decoding by EMA
TEST(GenericMsgTests, testGenericMsgRefreshMsgFieldListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;
		FieldList flEnc;
		EmaEncodeFieldListAll( flEnc );
		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( flEnc );
		responseMsg.payload( flEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg RefreshMsg FieldList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RefreshMsg FieldList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgRefreshMsgElementListDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		ElementList elEnc;
		EmaEncodeElementListAll( elEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( elEnc );
		responseMsg.payload( elEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg RefreshMsg ElementList Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RefreshMsg ElementList Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgRefreshMsgMapDecodetoString)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		RefreshMsg responseMsg;


		Map mapEnc;
		EmaEncodeMapAll( mapEnc );

		EmaBuffer itemGroup( "29", 2 );
		EmaString name( "TRI.N" );
		EmaBuffer extendedHeader( "extendedHeader", 6 );
		responseMsg.streamId( 1 );
		responseMsg.state( OmmState::OpenEnum, OmmState::OkEnum, OmmState::NoneEnum, "Status Text" );
		responseMsg.itemGroup( itemGroup );
		responseMsg.name( name );
		responseMsg.nameType( 1 );
		responseMsg.serviceId( 2 );
		responseMsg.id( 4 );
		responseMsg.filter( 8 );
		responseMsg.extendedHeader( extendedHeader );
		responseMsg.attrib( mapEnc );
		responseMsg.payload( mapEnc );

		GenericMsg genMsg;
		EmaString nameG( "GENERICMSG" );
		EmaBuffer headerG( "headerG", 7 );
		genMsg.streamId( 3 );
		genMsg.name( nameG );
		genMsg.nameType( 1 );
		genMsg.serviceId( 2 );
		genMsg.id( 4 );
		genMsg.filter( 8 );
		genMsg.extendedHeader( headerG );
		genMsg.attrib( responseMsg );
		genMsg.payload( responseMsg );


		//Now do EMA decoding of GenericMsg
		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_TRUE( true ) << "GenericMsg RefreshMsg Map Decode toString() - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg RefreshMsg Map Decode toString() - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgHybrid)
{

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary;

	ASSERT_TRUE(loadDictionaryFromFile( &dictionary )) << "Failed to load dictionary";

	try
	{
		// encoding order:  UINT, REAL, INT, DATE, TIME, DATETIME, ElementList, UINT

		RsslFieldList rsslFL;
		RsslEncodeIterator iter;

		rsslClearFieldList( &rsslFL );
		rsslClearEncodeIterator( &iter );

		std::array<char, 1000> rsslBufData;
		RsslBuffer rsslBuf;
		rsslBuf.length = (UInt32)rsslBufData.size();
		rsslBuf.data = ( char* )rsslBufData.data();

		rsslSetEncodeIteratorRWFVersion( &iter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
		rsslSetEncodeIteratorBuffer( &iter, &rsslBuf );
		rsslFL.flags = RSSL_FLF_HAS_STANDARD_DATA | RSSL_FLF_HAS_FIELD_LIST_INFO;
		rsslFL.dictionaryId = dictionary.info_DictionaryId;
		rsslFL.fieldListNum = 65;

		rsslEncodeFieldListInit( &iter, &rsslFL, 0, 0 );

		RsslFieldEntry rsslFEntry;

		// first entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		RsslUInt64 uint64 = 64;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		// second entry
		rsslFEntry.dataType = RSSL_DT_REAL;
		rsslFEntry.fieldId = 6;		// TRDPRC_1  + REAL
		RsslReal real;
		real.isBlank = RSSL_FALSE;
		real.hint = RSSL_RH_EXPONENT_2;
		real.value = 11;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&real );

		// third entry
		rsslFEntry.dataType = RSSL_DT_INT;
		rsslFEntry.fieldId = -2;		// INTEGER + INT
		RsslInt64 int64 = 32;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&int64 );

		// fourth entry
		rsslFEntry.dataType = RSSL_DT_DATE;
		rsslFEntry.fieldId = 16;		// TRADE_DATE + DATE
		RsslDate date;
		date.day = 7;
		date.month = 11;
		date.year = 1999;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&date );

		// fifth entry
		rsslFEntry.dataType = RSSL_DT_TIME;
		rsslFEntry.fieldId = 18;		// TRDTIM_1 + TIME
		RsslTime time;
		time.hour = 02;
		time.minute = 03;
		time.second = 04;
		time.millisecond = 005;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&time );

		// sixth entry
		rsslFEntry.dataType = RSSL_DT_DATETIME;
		rsslFEntry.fieldId = -3;		// TRADE_DATE + DATE
		RsslDateTime dateTime;
		dateTime.date.day = 7;
		dateTime.date.month = 11;
		dateTime.date.year = 1999;
		dateTime.time.hour = 01;
		dateTime.time.minute = 02;
		dateTime.time.second = 03;
		dateTime.time.millisecond = 000;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&dateTime );

		// seventh entry (ElementList)
		rsslFEntry.fieldId = -15;		// MY_ELEMENTLIST
		RsslElementList elementList = RSSL_INIT_ELEMENT_LIST;
		RsslElementEntry rsslEEntry;
		rsslEncodeFieldEntryInit( &iter, &rsslFEntry, 1000 );
		elementList.flags = RSSL_ELF_HAS_STANDARD_DATA | RSSL_ELF_HAS_ELEMENT_LIST_INFO;
		elementList.elementListNum = 5;
		rsslEncodeElementListInit( &iter, &elementList, 0, 0 );
		rsslEEntry.name.data = ( char* )"Element - UInt";
		rsslEEntry.name.length = 14;
		rsslEEntry.dataType = RSSL_DT_UINT;
		uint64 = 641;
		rsslEncodeElementEntry( &iter, &rsslEEntry, &uint64 );
		rsslEncodeElementListComplete( &iter, RSSL_TRUE );
		rsslEncodeFieldEntryComplete( &iter, RSSL_TRUE );

		// eigth entry
		rsslFEntry.dataType = RSSL_DT_UINT;
		rsslFEntry.fieldId = 1;		// PROD_PERM + UINT
		uint64 = 642;
		rsslEncodeFieldEntry( &iter, &rsslFEntry, ( void* )&uint64 );

		rsslEncodeFieldListComplete( &iter, RSSL_TRUE );

		// Convert RsslFieldList into EMA's FieldList
		FieldList fl;
		StaticDecoder::setRsslData( &fl, &rsslBuf, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary );

		// set field list as attrib and payload of GenericMsg
		GenericMsg genMsg;

		genMsg.attrib( fl );
		genMsg.payload( fl );

		StaticDecoder::setData( &genMsg, &dictionary );


		EXPECT_EQ( genMsg.getAttrib().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getattrib()::getDataType()" ;

		EXPECT_EQ( genMsg.getPayload().getDataType(), DataType::FieldListEnum ) << "GenericMsg::getPayload()::getDataType()" ;

		EXPECT_TRUE( true ) << "GenericMsg Hybrid Usage - exception not expected" ;
	}
	catch ( const OmmException& )
	{
		EXPECT_FALSE( true ) << "GenericMsg Hybrid Usage - exception not expected" ;
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgError)
{

	{
		try
		{
			GenericMsg msg;

			ElementList attrib;

			msg.attrib( attrib );

			EXPECT_FALSE( true ) << "GenericMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "GenericMsg::attrib( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			GenericMsg msg;

			RefreshMsg attrib;

			msg.attrib( attrib );

			EXPECT_TRUE( true ) << "GenericMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception not expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "GenericMsg::attrib( RefreshMsg ) where RefreshMsg is empty - exception not expected" ;
		}
	}

	{
		try
		{
			GenericMsg msg;

			ElementList load;

			msg.payload( load );

			EXPECT_FALSE( true ) << "GenericMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_TRUE( true ) << "GenericMsg::payload( Elementlist ) where ElementList is empty - exception expected" ;
		}
	}

	{
		try
		{
			GenericMsg msg;

			RefreshMsg load;

			msg.payload( load );

			EXPECT_TRUE( true ) << "GenericMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception not expected" ;
		}
		catch ( const OmmException& )
		{
			EXPECT_FALSE( true ) << "GenericMsg::payload( RefreshMsg ) where RefreshMsg is empty - exception not expected" ;
		}
	}

}

TEST(GenericMsgTests, testGenericMsgtoString)
{

	RsslDataDictionary dictionary;

	loadDictionaryFromFile(&dictionary);

	try
	{
		RsslGenericMsg generic;

		rsslClearGenericMsg(&generic);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType(&msgKey);

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId(&msgKey);

		std::array<char, 1000> rsslBufData;
		RsslBuffer rsslBuf;
		rsslBuf.length = (UInt32)rsslBufData.size();
		rsslBuf.data = ( char* )rsslBufData.data();

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		generic.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey(&generic);

		generic.msgBase.encDataBody = rsslBuf;
		generic.msgBase.containerType = RSSL_DT_FIELD_LIST;
		generic.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;

		RsslEncodeIterator encIter;

		rsslClearEncodeIterator(&encIter);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		int retval = 0;

		std::array<char, 2048> msgBufData;
		RsslBuffer msgBuf;
		msgBuf.length = (UInt32)msgBufData.size();
		msgBuf.data = (char*)msgBufData.data();

		/* set the buffer on an RsslEncodeIterator */
		if ((retval = rsslSetEncodeIteratorBuffer(&encIter, &msgBuf)) < RSSL_RET_SUCCESS)
		{
			//rsslReleaseBuffer(msgBuf, &error);
			EXPECT_FALSE(true) << "rsslSetEncodeIteratorBuffer() failed with return code: " << retval << std::endl;
		}

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&generic);

		RsslMsg genericDecode;
		RsslDecodeIterator decodeIter;

		rsslClearDecodeIterator(&decodeIter);

		// Set the RWF version to decode with this iterator
		rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		// Associates the RsslDecodeIterator with the RsslBuffer from which to decode.
		if ((retval = rsslSetDecodeIteratorBuffer(&decodeIter, &msgBuf)) != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslSetDecodeIteratorBuffer() failed with return code: " << retval << std::endl;
		}

		// decode contents into the RsslMsg structure
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&genericDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << std::endl;
		}

		GenericMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&genericDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		EXPECT_TRUE(true) << "GenericMsg toString Decode - exception not expected";
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "GenericMsg toString Decode - exception not expected";
	}

	rsslDeleteDataDictionary( &dictionary );
}

TEST(GenericMsgTests, testGenericMsgEditClone)
{

	RsslDataDictionary dictionary;

	loadDictionaryFromFile(&dictionary);

	try
	{
		RsslGenericMsg generic;

		rsslClearGenericMsg(&generic);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*> ("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType(&msgKey);

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId(&msgKey);

		std::array<char, 1000> rsslBufData;
		RsslBuffer rsslBuf;
		rsslBuf.length = (UInt32)rsslBufData.size();
		rsslBuf.data = ( char* )rsslBufData.data();

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		generic.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey(&generic);

		generic.msgBase.encDataBody = rsslBuf;
		generic.msgBase.containerType = RSSL_DT_FIELD_LIST;
		generic.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;

		RsslEncodeIterator encIter;

		rsslClearEncodeIterator(&encIter);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		int retval = 0;

		std::array<char, 2048> msgBufData;
		RsslBuffer msgBuf;
		msgBuf.length = (UInt32)msgBufData.size();
		msgBuf.data = (char*)msgBufData.data();

		/* set the buffer on an RsslEncodeIterator */
		if ((retval = rsslSetEncodeIteratorBuffer(&encIter, &msgBuf)) < RSSL_RET_SUCCESS)
		{
			//rsslReleaseBuffer(msgBuf, &error);
			EXPECT_FALSE(true) << "rsslSetEncodeIteratorBuffer() failed with return code: " << retval << std::endl;
		}

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&generic);

		RsslMsg genericDecode;
		RsslDecodeIterator decodeIter;

		rsslClearDecodeIterator(&decodeIter);

		// Set the RWF version to decode with this iterator
		rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		// Associates the RsslDecodeIterator with the RsslBuffer from which to decode.
		if ((retval = rsslSetDecodeIteratorBuffer(&decodeIter, &msgBuf)) != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslSetDecodeIteratorBuffer() failed with return code: " << retval << std::endl;
		}

		// decode contents into the RsslMsg structure
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&genericDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << std::endl;
		}

		GenericMsg respMsg;

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&genericDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		// Clone message
		GenericMsg cloneGenericMsg(respMsg);

		EXPECT_TRUE(cloneGenericMsg.getDomainType() == respMsg.getDomainType()) << "Compare domainType";
		EXPECT_TRUE(cloneGenericMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_TRUE(cloneGenericMsg.hasMsgKey() == respMsg.hasMsgKey()) << "Compare hasMsgKey";

		EXPECT_STREQ(respMsg.toString(), cloneGenericMsg.toString()) << "Check equal toString()";

		// Edit message
		cloneGenericMsg.streamId(10);

		StaticDecoder::setData(&cloneGenericMsg, &dictionary);

		EXPECT_FALSE(cloneGenericMsg.getStreamId() == respMsg.getStreamId()) << "Compare streamId";
		EXPECT_STRNE(respMsg.toString(), cloneGenericMsg.toString()) << "Check not equal toString()";
		EXPECT_TRUE(true) << "GenericMsg Edit Clone Success";
	}
	catch (const OmmException&)
	{
		EXPECT_FALSE(true) << "GenericMsg Edit Clone - exception not expected";
	}

	rsslDeleteDataDictionary(&dictionary);
}

TEST(GenericMsgTests, testGenericMsgCloneMsgKeyPermissionData)
{
	RsslDataDictionary dictionary;

	char permissionData[] = "permission access to important data";
	const RsslUInt32 permissionDataLen = sizeof(permissionData) / sizeof(char);

	RsslUInt32 seqNum;

	RsslUInt16 flagsTest[] = {
		RSSL_GNMF_NONE,
		RSSL_GNMF_HAS_MSG_KEY,
		RSSL_GNMF_HAS_PERM_DATA,
		RSSL_GNMF_HAS_SEQ_NUM,
		RSSL_GNMF_HAS_PART_NUM,
		RSSL_GNMF_HAS_MSG_KEY | RSSL_GNMF_HAS_PERM_DATA | RSSL_GNMF_HAS_SEQ_NUM | RSSL_GNMF_HAS_PART_NUM,
	};
	const size_t nFlags = sizeof(flagsTest) / sizeof(RsslUInt16);
	RsslUInt16 flag;

	char rsslBufferData[1000];
	char msgBufData[2048];

	RsslEncodeIterator encIter;
	RsslDecodeIterator decodeIter;

	loadDictionaryFromFile(&dictionary);

	for (RsslUInt32 i = 0; i < nFlags; ++i)
	{
		flag = flagsTest[i];
		try
		{
			RsslGenericMsg generic;
			rsslClearGenericMsg(&generic);

			generic.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;
			generic.msgBase.streamId = 3;
			generic.flags = flag;

			RsslMsgKey msgKey;
			rsslClearMsgKey(&msgKey);

			RsslBuffer rsslBuf = RSSL_INIT_BUFFER;

			if (flag & RSSL_GNMF_HAS_MSG_KEY)
			{
				RsslBuffer nameBuffer;
				nameBuffer.data = const_cast<char*> ("ABCDEF");
				nameBuffer.length = 6;

				msgKey.name = nameBuffer;
				rsslMsgKeyApplyHasName(&msgKey);

				msgKey.nameType = 1;
				rsslMsgKeyApplyHasNameType(&msgKey);

				msgKey.serviceId = 2;
				rsslMsgKeyApplyHasServiceId(&msgKey);

				rsslBuf.length = sizeof(rsslBufferData) / sizeof(char);
				rsslBuf.data = rsslBufferData;

				EmaString inText;
				encodeFieldList(rsslBuf, inText);

				msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
				msgKey.encAttrib = rsslBuf;
				rsslMsgKeyApplyHasAttrib(&msgKey);

				generic.msgBase.msgKey = msgKey;
				rsslGenericMsgApplyHasMsgKey(&generic);

				generic.msgBase.encDataBody = rsslBuf;
				generic.msgBase.containerType = RSSL_DT_FIELD_LIST;
			}
			else
			{
				generic.msgBase.encDataBody.data = 0;
				generic.msgBase.encDataBody.length = 0;
				generic.msgBase.containerType = RSSL_DT_NO_DATA;
			}

			/* Add Permission Info */
			if (flag & RSSL_GNMF_HAS_PERM_DATA)
			{
				generic.permData.length = permissionDataLen;
				generic.permData.data = permissionData;
			}

			/* Add Item Sequence Number */
			if (flag & RSSL_GNMF_HAS_SEQ_NUM)
			{
				seqNum = i;
				generic.seqNum = seqNum;
			}

			/* Add partNum Info */
			if (flag & RSSL_GNMF_HAS_PART_NUM)
			{
				generic.partNum = seqNum + 100;
			}

			RsslBuffer msgBuf;
			msgBuf.length = sizeof(msgBufData) / sizeof(char);
			msgBuf.data = msgBufData;

			RsslMsg genericDecode;
			GenericMsg respMsg;

			prepareMsgToCopy(encIter, msgBuf, (RsslMsg*)&generic, decodeIter, (RsslMsg*)&genericDecode, respMsg, dictionary);

			// Clone message
			GenericMsg cloneGenericMsg(respMsg);

			EXPECT_EQ(cloneGenericMsg.getDomainType(), respMsg.getDomainType()) << "Compare domainType";
			EXPECT_EQ(cloneGenericMsg.getDomainType(), RSSL_DMT_MARKET_BY_PRICE) << "Compare domainType: should be equal to " << RSSL_DMT_MARKET_BY_PRICE;

			EXPECT_EQ(cloneGenericMsg.getStreamId(), respMsg.getStreamId()) << "Compare streamId";
			EXPECT_EQ(cloneGenericMsg.getStreamId(), 3) << "Compare streamId: should be equal to 3";

			EXPECT_EQ(cloneGenericMsg.hasMsgKey(), respMsg.hasMsgKey()) << "Compare hasMsgKey";
			EXPECT_EQ(cloneGenericMsg.hasName(), respMsg.hasName()) << "Compare hasName";
			EXPECT_EQ(cloneGenericMsg.hasNameType(), respMsg.hasNameType()) << "Compare hasNameType";
			EXPECT_EQ(cloneGenericMsg.hasServiceId(), respMsg.hasServiceId()) << "Compare hasServiceId";
			EXPECT_EQ(cloneGenericMsg.hasId(), respMsg.hasId()) << "Compare hasId";
			EXPECT_EQ(cloneGenericMsg.hasFilter(), respMsg.hasFilter()) << "Compare hasFilter";
			EXPECT_EQ(cloneGenericMsg.hasExtendedHeader(), respMsg.hasExtendedHeader()) << "Compare hasExtendedHeader";

			EXPECT_EQ(cloneGenericMsg.hasSeqNum(), respMsg.hasSeqNum()) << "Compare hasSeqNum";
			EXPECT_EQ(cloneGenericMsg.hasPermissionData(), respMsg.hasPermissionData()) << "Compare hasPermissionData";
			EXPECT_EQ(cloneGenericMsg.hasSecondarySeqNum(), respMsg.hasSecondarySeqNum()) << "Compare hasSecondarySeqNum";
			EXPECT_EQ(cloneGenericMsg.hasPartNum(), respMsg.hasPartNum()) << "Compare hasPartNum";

			EXPECT_STREQ(cloneGenericMsg.toString(), respMsg.toString()) << "Check equal toString()";

			EXPECT_EQ(cloneGenericMsg.hasMsgKey(), (flag & RSSL_GNMF_HAS_MSG_KEY) > 0) << "Compare hasMsgKey: " << (flag & RSSL_GNMF_HAS_MSG_KEY);
			if (cloneGenericMsg.hasMsgKey() && respMsg.hasMsgKey())
			{
				if (cloneGenericMsg.hasServiceId())
				{
					EXPECT_EQ(cloneGenericMsg.getServiceId(), respMsg.getServiceId()) << "Compare serviceId";
				}
				if (cloneGenericMsg.hasName())
				{
					EXPECT_EQ(cloneGenericMsg.getName(), respMsg.getName()) << "Compare name";
				}
				if (cloneGenericMsg.hasNameType())
				{
					EXPECT_EQ(cloneGenericMsg.getNameType(), respMsg.getNameType()) << "Compare nameType";
				}
				if (cloneGenericMsg.hasId())
				{
					EXPECT_EQ(cloneGenericMsg.getId(), respMsg.getId()) << "Compare id";
				}
				if (cloneGenericMsg.hasFilter())
				{
					EXPECT_EQ(cloneGenericMsg.getFilter(), respMsg.getFilter()) << "Compare filter";
				}
			}

			EXPECT_EQ(cloneGenericMsg.hasSeqNum(), (flag & RSSL_GNMF_HAS_SEQ_NUM) > 0) << "Compare hasSeqNum: " << (flag & RSSL_GNMF_HAS_SEQ_NUM);
			if (cloneGenericMsg.hasSeqNum() && respMsg.hasSeqNum())
			{
				EXPECT_EQ(cloneGenericMsg.getSeqNum(), respMsg.getSeqNum()) << "Compare SeqNum";
				EXPECT_EQ(seqNum, cloneGenericMsg.getSeqNum()) << "Compare SeqNum: " << seqNum;
			}

			EXPECT_EQ(cloneGenericMsg.hasPermissionData(), (flag & RSSL_GNMF_HAS_PERM_DATA) > 0) << "Compare hasPermissionData: " << (flag & RSSL_GNMF_HAS_PERM_DATA);
			if (cloneGenericMsg.hasPermissionData() && respMsg.hasPermissionData())
			{
				const EmaBuffer& permDataOrig = cloneGenericMsg.getPermissionData();
				const EmaBuffer& permDataCopy = respMsg.getPermissionData();
				EmaBuffer permData(permissionData, permissionDataLen);

				EXPECT_EQ(permDataOrig.length(), permDataCopy.length()) << "Compare length of EmaBuffer Permission Data";
				EXPECT_EQ(permDataOrig.length(), permissionDataLen) << "Compare length of EmaBuffer Permission Data: " << permissionDataLen;
				EXPECT_EQ(permDataOrig, permDataCopy) << "Compare EmaBuffer Permission Data";
				EXPECT_EQ(permData, permDataCopy) << "Compare EmaBuffer Permission Data: " << permissionData;
			}

			EXPECT_EQ(cloneGenericMsg.hasPartNum(), (flag & RSSL_GNMF_HAS_PART_NUM) > 0) << "Compare hasPartNum: " << (flag & RSSL_GNMF_HAS_PART_NUM);
			if (cloneGenericMsg.hasPartNum() && respMsg.hasPartNum())
			{
				EXPECT_EQ(cloneGenericMsg.getPartNum(), respMsg.getPartNum()) << "Compare getPartNum";
				EXPECT_EQ(cloneGenericMsg.getPartNum(), seqNum + 100) << "Compare getPartNum: " << (seqNum + 100);
			}

			EXPECT_TRUE(true) << "GenericMsg Clone Success";
		}
		catch (const OmmException&)
		{
			EXPECT_FALSE(true) << "GenericMsg Clone - exception not expected";
		}
	}
	rsslDeleteDataDictionary(&dictionary);
}

// holds the memory (buffers, dictionary, etc.) needed to encode and decode a message
// releases all resources upon destruction
struct GenericMsg_forClone
{
	RsslGenericMsg generic{};
	RsslMsg genericDecode{};
	RsslBuffer msgBuf;
	std::array<char, 2048> msgBufData;
	std::array<char, 1024> rsslBufData;
	EmaString toString{};

	// load dictionary for decoding of the field list
	RsslDataDictionary dictionary{};

	GenericMsg_forClone() {
		EXPECT_TRUE(loadDictionaryFromFile(&dictionary)) << "Failed to load dictionary";
		msgBuf.length = (UInt32)msgBufData.size();
		msgBuf.data = (char*)msgBufData.data();
	}

	~GenericMsg_forClone() {
		rsslDeleteDataDictionary(&dictionary);
	}

	void encode_forClone(GenericMsg& respMsg)
	{
		rsslClearGenericMsg(&generic);

		RsslMsgKey msgKey;

		rsslClearMsgKey(&msgKey);

		RsslBuffer nameBuffer;
		nameBuffer.data = const_cast<char*>("ABCDEF");
		nameBuffer.length = 6;

		msgKey.name = nameBuffer;
		rsslMsgKeyApplyHasName(&msgKey);

		msgKey.nameType = 1;
		rsslMsgKeyApplyHasNameType(&msgKey);

		msgKey.serviceId = 2;
		rsslMsgKeyApplyHasServiceId(&msgKey);

		RsslBuffer rsslBuf;
		rsslBuf.length = (UInt32)rsslBufData.size();
		rsslBuf.data = (char*)rsslBufData.data();

		EmaString inText;
		encodeFieldList(rsslBuf, inText);

		msgKey.attribContainerType = RSSL_DT_FIELD_LIST;
		msgKey.encAttrib = rsslBuf;
		rsslMsgKeyApplyHasAttrib(&msgKey);

		generic.msgBase.msgKey = msgKey;
		rsslGenericMsgApplyHasMsgKey(&generic);

		generic.msgBase.encDataBody = rsslBuf;
		generic.msgBase.containerType = RSSL_DT_FIELD_LIST;
		generic.msgBase.domainType = RSSL_DMT_MARKET_BY_PRICE;

		RsslEncodeIterator encIter;

		rsslClearEncodeIterator(&encIter);

		/* set version information of the connection on the encode iterator so proper versioning can be performed */
		rsslSetEncodeIteratorRWFVersion(&encIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
		int retval = 0;

		/* set the buffer on an RsslEncodeIterator */
		if ((retval = rsslSetEncodeIteratorBuffer(&encIter, &msgBuf)) < RSSL_RET_SUCCESS)
		{
			//rsslReleaseBuffer(msgBuf, &error);
			EXPECT_FALSE(true) << "rsslSetEncodeIteratorBuffer() failed with return code: " << retval << std::endl;
		}

		retval = rsslEncodeMsg(&encIter, (RsslMsg*)&generic);

		RsslDecodeIterator decodeIter;

		rsslClearDecodeIterator(&decodeIter);

		// Set the RWF version to decode with this iterator
		rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);

		// Associates the RsslDecodeIterator with the RsslBuffer from which to decode.
		if ((retval = rsslSetDecodeIteratorBuffer(&decodeIter, &msgBuf)) != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslSetDecodeIteratorBuffer() failed with return code: " << retval << std::endl;
		}

		// decode contents into the RsslMsg structure
		retval = rsslDecodeMsg(&decodeIter, (RsslMsg*)&genericDecode);
		if (retval != RSSL_RET_SUCCESS)
		{
			EXPECT_FALSE(true) << "rsslDecodeMsg() failed with return code: " << retval << std::endl;
		}

		StaticDecoder::setRsslData(&respMsg, (RsslMsg*)&genericDecode, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &dictionary);

		toString = respMsg.toString();
	}

	bool check_afterClone(const GenericMsg& msg)
	{
		EXPECT_EQ(msg.getDomainType(), RSSL_DMT_MARKET_BY_PRICE) << "Compare domainType";
		EXPECT_EQ(msg.getStreamId(), 0) << "Compare streamId";
		EXPECT_EQ(msg.getServiceId(), 2) << "Compare serviceId";
		EXPECT_TRUE(msg.hasMsgKey()) << "Compare hasMsgKey";
		EXPECT_STREQ(msg.getName(), "ABCDEF") << "Compare getName";
		EXPECT_STREQ(msg.toString(), toString) << "Check equal toString()";
		return true;
	}

	GenericMsg generateMessage() {
		GenericMsg msg;
		encode_forClone(msg);
		return msg;
	}
};

TEST(GenericMsgTests, testGenericMsgClone_CopyConstruct)
{
	GenericMsg_forClone encoder;

	const GenericMsg respMsg = encoder.generateMessage();

	// Clone message
	const GenericMsg cloneGenericMsg{respMsg};

	EXPECT_TRUE(encoder.check_afterClone(cloneGenericMsg)) << "GenericMsg Clone Success";
}

TEST(GenericMsgTests, testGenericMsgClone_CopyAssign)
{
	GenericMsg_forClone encoder;
	const GenericMsg respMsg = encoder.generateMessage();

	GenericMsg cloneGenericMsg;

	// Clone message via copy assignment
	cloneGenericMsg = respMsg;

	EXPECT_TRUE(encoder.check_afterClone(cloneGenericMsg)) << "GenericMsg Clone Success";
}

TEST(GenericMsgTests, testGenericMsgClone_CopyAssignReserved)
{
	GenericMsg_forClone encoder;
	const GenericMsg _tmp = encoder.generateMessage();

	for (const UInt32 reservation : { 0, 1, 3, 15, 1024 })
	{
		GenericMsg srcMsg;

		// use the RsslMsg because its buffers point outside of the encMsgBuffer
		StaticDecoder::setRsslData(&srcMsg, (RsslMsg*)&encoder.generic, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &encoder.dictionary);

		GenericMsg cloneGenericMsg{reservation};

		// Clone message via copy assignment
		cloneGenericMsg = srcMsg;

		EXPECT_TRUE(encoder.check_afterClone(cloneGenericMsg)) << "GenericMsg Clone Success";
	}
}

TEST(GenericMsgTests, testGenericMsgClone_MoveConstruct)
{
	GenericMsg_forClone encoder;
	GenericMsg msg = encoder.generateMessage();

	// Steal message via move construct
	GenericMsg cloneGenericMsg{std::move(msg)};

	EXPECT_TRUE(encoder.check_afterClone(cloneGenericMsg)) << "GenericMsg Clone Success";
}

TEST(GenericMsgTests, testGenericMsgClone_MoveAssign)
{
	GenericMsg_forClone encoder;
	GenericMsg respMsg = encoder.generateMessage();

	GenericMsg cloneGenericMsg;

	// Steal message via move assign
	cloneGenericMsg = std::move(respMsg);

	EXPECT_TRUE(encoder.check_afterClone(cloneGenericMsg)) << "GenericMsg Clone Success";
}
