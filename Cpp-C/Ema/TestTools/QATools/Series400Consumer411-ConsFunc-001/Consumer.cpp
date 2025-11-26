///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#include <chrono>
#include <iostream>
#include <thread>
#include <cstring>

#include <stdlib.h>
#include <stdio.h>

#include "Consumer.h"

using namespace refinitiv::ema::access;

constexpr UInt32 DEFAULT_QUEUE_SIZE = 32;
constexpr UInt32 DEFAULT_MESSAGE_SIZE = 16384;

//API QA
bool testGenericMsg = false;
bool testPostMsg = false;
//END API QA

template<class MsgType, UInt32 messageSize = DEFAULT_MESSAGE_SIZE>
void populatePool(ConcurrentQueue<std::unique_ptr<MsgType>>& pool)
{
	while (! pool.full())
	{
		pool.push(std::unique_ptr<MsgType>(new MsgType(messageSize)));
	}
}

AppClient::AppClient() :
	_messageQueue(DEFAULT_QUEUE_SIZE),
	_refreshMsgPool(DEFAULT_QUEUE_SIZE),
	_updateMsgPool(DEFAULT_QUEUE_SIZE),
	_statusMsgPool(DEFAULT_QUEUE_SIZE),
	_ackMsgPool(DEFAULT_QUEUE_SIZE),
	_postMsgPool(DEFAULT_QUEUE_SIZE),
	_genericMsgPool(DEFAULT_QUEUE_SIZE)
{
	populatePool<RefreshMsg>(_refreshMsgPool);
	populatePool<UpdateMsg>(_updateMsgPool);
	populatePool<StatusMsg>(_statusMsgPool);
	//API QA
	populatePool<AckMsg>(_ackMsgPool);
	populatePool<PostMsg>(_postMsgPool);
	populatePool<GenericMsg>(_genericMsgPool);
	//END API QA

	_exit.store(false);

	_dispatchThread = std::thread{ &AppClient::run, this };
}

AppClient::~AppClient()
{
	_exit.store(true);

	if (_dispatchThread.joinable())
		_dispatchThread.join();
}

void AppClient::onRefreshMsg(const RefreshMsg& refreshMsg, const OmmConsumerEvent& evt)
{
	std::unique_ptr<RefreshMsg> msg = _refreshMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = refreshMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ std::move(msg), evt.getHandle() });
}

void AppClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& evt)
{
	std::unique_ptr<UpdateMsg> msg = _updateMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = updateMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ std::move(msg), evt.getHandle() });
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& evt)
{
	std::unique_ptr<StatusMsg> msg = _statusMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = statusMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ std::move(msg), evt.getHandle() });
}

//API QA
void AppClient::onAckMsg(const AckMsg& ackMsg, const OmmConsumerEvent& evt)
{
	std::unique_ptr<AckMsg> msg = _ackMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = ackMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ std::move(msg), evt.getHandle() });
}

void AppClient::onPostMsg(const PostMsg& postMsg, const OmmConsumerEvent& evt)
{
	std::unique_ptr<PostMsg> msg = _postMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = postMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ std::move(msg), evt.getHandle() });
}

void AppClient::onGenericMsg(const GenericMsg& genericMsg, const OmmConsumerEvent& evt)
{
	std::unique_ptr<GenericMsg> msg = _genericMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = genericMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ std::move(msg), evt.getHandle() });
}

void AppClient::setOmmConsumer(OmmConsumer* consumer)
{
	_pOmmConsumer = consumer;
}
//END API QA

void AppClient::run()
{
	MsgWrapper queuedMessage;

	while (!_exit.load())
	{
		//API QA
		int i = 0;
		//END API QA
		if (_messageQueue.tryPop(queuedMessage))
		{
			std::cout << "Item Name: " << (std::get<0>(queuedMessage)->hasName()
										   ? std::get<0>(queuedMessage)->getName() : "<not set>")
				<< std::endl;

			switch (std::get<0>(queuedMessage)->getDataType())
			{
			case DataType::RefreshMsgEnum:
			{
				std::unique_ptr<RefreshMsg> refreshMsg{ static_cast<RefreshMsg*>(std::get<0>(queuedMessage).release()) };

				std::cout << "Service Name: " << (refreshMsg->hasServiceName() ? refreshMsg->getServiceName() : "<not set>") << std::endl;
				std::cout << "Item State: " << refreshMsg->getState() << std::endl;

				if (DataType::FieldListEnum == refreshMsg->getPayload().getDataType())
					std::cout << refreshMsg->getPayload().getFieldList().toString() << std::endl;

				//API QA
				if (testGenericMsg)
				{
					// submit a generic message when stream becomes open / ok
					if (refreshMsg->getState().getStreamState() == OmmState::OpenEnum &&
						refreshMsg->getState().getDataState() == OmmState::OkEnum)
					{
						_pOmmConsumer->submit(GenericMsg()
							.domainType(200).name("genericMsg").payload(ElementList().addInt("value", ++i).complete()),
							std::get<1>(queuedMessage));
					}
				}

				if (testPostMsg)
				{
					// submit an on stream post message when item is open ok
					if (refreshMsg->getState().getStreamState() == OmmState::OpenEnum &&
						refreshMsg->getState().getDataState() == OmmState::OkEnum)
					{
						_pOmmConsumer->submit(PostMsg()
							.postId(++i).serviceId(1).name("LSEG.L").solicitAck(true)
							.payload(UpdateMsg()
								.payload(FieldList()
									.addReal(25, 80, OmmReal::ExponentPos1Enum)
									.complete()))
							.complete(),
							std::get<1>(queuedMessage));
					}
				}
				//END API QA

				refreshMsg->clear();
				_refreshMsgPool.push(std::move(refreshMsg));

				break;
			}

			case DataType::UpdateMsgEnum:
			{
				std::unique_ptr<UpdateMsg> updateMsg{ static_cast<UpdateMsg*>(std::get<0>(queuedMessage).release()) };

				std::cout << "Service Name: " << (updateMsg->hasServiceName() ? updateMsg->getServiceName() : "<not set>") << std::endl;

				if (DataType::FieldListEnum == updateMsg->getPayload().getDataType())
					std::cout << updateMsg->getPayload().getFieldList().toString() << std::endl;

				updateMsg->clear();
				_updateMsgPool.push(std::move(updateMsg));

				break;
			}

			case DataType::StatusMsgEnum:
			{
				std::unique_ptr<StatusMsg> statusMsg{ static_cast<StatusMsg*>(std::get<0>(queuedMessage).release()) };

				std::cout << "Service Name: " << (statusMsg->hasServiceName() ? statusMsg->getServiceName() : "<not set>") << std::endl;

				if (statusMsg->hasState())
					std::cout << "Item State: " << statusMsg->getState() << std::endl;

				statusMsg->clear();
				_statusMsgPool.push(std::move(statusMsg));

				break;
			}
			//API QA
			case DataType::AckMsgEnum:
			{
				std::unique_ptr<AckMsg> ackMsg{ static_cast<AckMsg*>(std::get<0>(queuedMessage).release()) };

				std::cout << "Ack Item Name: " << (ackMsg->hasName() ? ackMsg->getName() : "<not set>") << std::endl;

				ackMsg->clear();
				_ackMsgPool.push(std::move(ackMsg));

				break;
			}
			case DataType::PostMsgEnum:
			{
				std::unique_ptr<PostMsg> postMsg{ static_cast<PostMsg*>(std::get<0>(queuedMessage).release()) };

				std::cout << "Service Name: " << (postMsg->hasServiceName() ? postMsg->getServiceName() : "<not set>") << std::endl;
				std::cout << "Item Name: " << (postMsg->hasName() ? postMsg->getName() : "<not set>") << std::endl;

				postMsg->clear();
				_postMsgPool.push(std::move(postMsg));

				break;
			}
			case DataType::GenericMsgEnum:
			{
				std::unique_ptr<GenericMsg> genericMsg{ static_cast<GenericMsg*>(std::get<0>(queuedMessage).release()) };

				std::cout << "Generic Msg Item Name: " << (genericMsg->hasName() ? genericMsg->getName() : "<not set>") << std::endl;

				genericMsg->clear();
				_genericMsgPool.push(std::move(genericMsg));

				break;
			}
			//END API QA
			default:
				break;
			}
		}
		else // no messages incoming yet
		{
			std::this_thread::yield();
		}
	}
}
//API QA
void printHelp()
{
	std::cout << std::endl << "Options:\n" << " -?\tShows this usage" << std::endl
		<< " -testGenericMsg for testing GenericMsg" << std::endl
		<< " -testPostMsg for testing PostMsg and AckMsg" << std::endl;
}
//END API QA

int main(int argc, char* argv[])
{
	try
	{
		//API QA
		for (int i = 1; i < argc; i++)
		{
			if (strcmp(argv[i], "-?") == 0)
			{
				printHelp();
				return 0;
			}

			else if (strcmp(argv[i], "-testGenericMsg") == 0)
			{
				std::cout << "Generic Msg on-stream post test: ON" << std::endl;
				testGenericMsg = true;
			}
			else if (strcmp(argv[i], "-testPostMsg") == 0)
			{
				std::cout << "Post Msg on-stream post test: ON" << std::endl;
				testPostMsg = true;
			}

		}
		//END API QA

		AppClient client;
		OmmConsumer consumer(OmmConsumerConfig().host("localhost:14002").username("user"));
		client.setOmmConsumer(&consumer);

		//API QA
		if (testGenericMsg)
		{
			consumer.registerClient(ReqMsg().domainType(200).serviceName("DIRECT_FEED").name("LSEG.L"), client);
		}
		else
		{
			consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("LSEG.L"), client);
		}
		//END API QA

		std::this_thread::sleep_for(std::chrono::seconds(60));	// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch (const OmmException& excp)
	{
		std::cout << excp << std::endl;
	}
	return 0;
}
