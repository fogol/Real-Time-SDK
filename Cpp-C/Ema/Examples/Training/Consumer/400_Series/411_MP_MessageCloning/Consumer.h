///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#ifndef __ema_consumer_h_
#define __ema_consumer_h_

#include <atomic>
#include <mutex>
#include <thread>
#include <tuple>
#include <queue>
#include <vector>

#include <Ema.h>

/* A basic thread-safe blocking queue of fixed maximum capacity. Wraps standard queue with
 * a thread safe lock.
*/
template <typename T>
class ConcurrentQueue
{
public:

	explicit ConcurrentQueue(refinitiv::ema::access::UInt32 maxQueueSize) :
		_maxQueueSize(maxQueueSize),
		_exit(false)
	{};

	ConcurrentQueue(const ConcurrentQueue&) = delete;

	~ConcurrentQueue()
	{
		_exit = true;
	};

	/* Returns true when queue is empty.*/
	bool empty() const
	{
		std::lock_guard<std::mutex> lock{ _mutex };

		return _queue.empty();
	}

	/* Tries to put an element into the end of queue.
	   Returns false if queue is already full, true otherwise.
	*/
	bool tryPush(const T element)
	{
		const std::lock_guard<std::mutex> lock{ _mutex };

		if (_queue.size() < _maxQueueSize)
		{
			_queue.push(element);
			return true;
		}

		return false;
	}

	/* Puts an element into the end of queue. Waits until queue is not full.
	*/
	void push(const T element)
	{
		while (!tryPush(element))
		{
			if (_exit) return;

			std::this_thread::yield();
		}
	}

	/* Tries to pop an element from the head of queue.
	   Returns nullptr when queue is empty */
	bool tryPop(T& element)
	{
		const std::lock_guard<std::mutex> lock{ _mutex };

		if (_queue.empty())
		{
			return false;
		}

		element = _queue.front();
		_queue.pop();

		return true;
	}

	/* Pops an element from the queue. Waits until there is an element to return if queue is empty. */
	T pop()
	{
		T element;
		while (!tryPop(element))
		{
			if (_exit) return {};

			std::this_thread::yield();
		}
		return element;
	}

	/* Returns the actual number of elements in the queue. */
	size_t size() const
	{
		const std::lock_guard<std::mutex> lock{ _mutex };

		return _queue.size();
	}

	/* Returns true when the number of elements in the queue has reached its capacity (MaxQueueSize). */
	bool full() const
	{
		return !(size() < _maxQueueSize);
	}

private:

	std::queue<T> _queue;     // the queue itself
	mutable std::mutex _mutex; // used to protect _queue from data races
	refinitiv::ema::access::UInt32 _maxQueueSize;
	bool _exit;
};

// application defined client class for receiving and processing of item messages
class AppClient : public refinitiv::ema::access::OmmConsumerClient
{
public:
	AppClient();

	void run();

	~AppClient();

protected:

	void onRefreshMsg(const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

	void onUpdateMsg(const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

	void onStatusMsg(const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

private:

	using MsgPtr = typename refinitiv::ema::access::Msg*;
	using MsgWrapper = typename std::tuple<MsgPtr, refinitiv::ema::access::UInt64>;

	// incoming message queue: has a pointer to the message and the corresponding handle
	ConcurrentQueue<MsgWrapper> _messageQueue;

	// pools of different message type objects
	ConcurrentQueue<refinitiv::ema::access::RefreshMsg*> _refreshMsgPool;
	ConcurrentQueue<refinitiv::ema::access::UpdateMsg*> _updateMsgPool;
	ConcurrentQueue<refinitiv::ema::access::StatusMsg*> _statusMsgPool;

	std::vector<refinitiv::ema::access::RefreshMsg> _allRefreshMsgs;
	std::vector<refinitiv::ema::access::UpdateMsg> _allUpdateMsgs;
	std::vector<refinitiv::ema::access::StatusMsg> _allStatusMsgs;

	std::atomic<bool> _exit;
	std::thread _dispatchThread;
};

#endif // __ema_consumer_h_
