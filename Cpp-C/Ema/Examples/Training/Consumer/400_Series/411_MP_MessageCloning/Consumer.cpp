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

template<typename MsgType>
void populatePool(ConcurrentQueue<MsgType*>& pool, std::vector<MsgType>& allMsgs, UInt32 messageSize = DEFAULT_MESSAGE_SIZE)
{
	while (allMsgs.size() < allMsgs.capacity())
	{
		allMsgs.emplace_back(messageSize);
		pool.push(&allMsgs.back());
	}
}

AppClient::AppClient() :
  _messageQueue(DEFAULT_QUEUE_SIZE),
  _refreshMsgPool(DEFAULT_QUEUE_SIZE),
  _updateMsgPool(DEFAULT_QUEUE_SIZE),
  _statusMsgPool(DEFAULT_QUEUE_SIZE),
  _allRefreshMsgs(),
  _allUpdateMsgs(),
  _allStatusMsgs()
{
	_allRefreshMsgs.reserve(DEFAULT_QUEUE_SIZE);
	_allUpdateMsgs.reserve(DEFAULT_QUEUE_SIZE);
	_allStatusMsgs.reserve(DEFAULT_QUEUE_SIZE);

	populatePool<RefreshMsg>(_refreshMsgPool, _allRefreshMsgs);
	populatePool<UpdateMsg>(_updateMsgPool, _allUpdateMsgs);
	populatePool<StatusMsg>(_statusMsgPool, _allStatusMsgs);

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
	RefreshMsg* msg = _refreshMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = refreshMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ msg, evt.getHandle() });
}

void AppClient::onUpdateMsg(const UpdateMsg& updateMsg, const OmmConsumerEvent& evt)
{
	UpdateMsg* msg = _updateMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = updateMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ msg, evt.getHandle() });
}

void AppClient::onStatusMsg(const StatusMsg& statusMsg, const OmmConsumerEvent& evt)
{
	StatusMsg* msg = _statusMsgPool.pop();

	// Copy incoming message into the memory pooled instance
	*msg = statusMsg;

	// and store it in the incoming message queue
	_messageQueue.push({ msg, evt.getHandle() });
}

void AppClient::run()
{
	MsgWrapper queuedMessage;

	while (!_exit.load())
	{
		if (_messageQueue.tryPop(queuedMessage))
		{
			std::cout << "Item Name: " << (std::get<0>(queuedMessage)->hasName()
										   ? std::get<0>(queuedMessage)->getName() : "<not set>")
				<< std::endl;

			switch (std::get<0>(queuedMessage)->getDataType())
			{
			case DataType::RefreshMsgEnum:
			{
				RefreshMsg* refreshMsg = static_cast<RefreshMsg*>(std::get<0>(queuedMessage));

				std::cout << "Service Name: " << (refreshMsg->hasServiceName() ? refreshMsg->getServiceName() : "<not set>") << std::endl;
				std::cout << "Item State: " << refreshMsg->getState() << std::endl;

				if (DataType::FieldListEnum == refreshMsg->getPayload().getDataType())
					std::cout << refreshMsg->getPayload().getFieldList().toString() << std::endl;

				refreshMsg->clear();
				_refreshMsgPool.push(refreshMsg);

				break;
			}

			case DataType::UpdateMsgEnum:
			{
				UpdateMsg* updateMsg = static_cast<UpdateMsg*>(std::get<0>(queuedMessage));

				std::cout << "Service Name: " << (updateMsg->hasServiceName() ? updateMsg->getServiceName() : "<not set>") << std::endl;

				if (DataType::FieldListEnum == updateMsg->getPayload().getDataType())
					std::cout << updateMsg->getPayload().getFieldList().toString() << std::endl;

				updateMsg->clear();
				_updateMsgPool.push(updateMsg);

				break;
			}

			case DataType::StatusMsgEnum:
			{
				StatusMsg* statusMsg = static_cast<StatusMsg*>(std::get<0>(queuedMessage));

				std::cout << "Service Name: " << (statusMsg->hasServiceName() ? statusMsg->getServiceName() : "<not set>") << std::endl;

				if (statusMsg->hasState())
					std::cout << "Item State: " << statusMsg->getState() << std::endl;

				statusMsg->clear();
				_statusMsgPool.push(statusMsg);

				break;
			}
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

int main(int, char**)
{
	try
	{
		AppClient client;
		OmmConsumer consumer(OmmConsumerConfig().host("localhost:14002").username("user"));

		consumer.registerClient(ReqMsg().serviceName("DIRECT_FEED").name("LSEG.L"), client);

		std::this_thread::sleep_for(std::chrono::seconds(60));	// API calls onRefreshMsg(), onUpdateMsg(), or onStatusMsg()
	}
	catch (const OmmException& excp)
	{
		std::cout << excp << std::endl;
	}
	return 0;
}
