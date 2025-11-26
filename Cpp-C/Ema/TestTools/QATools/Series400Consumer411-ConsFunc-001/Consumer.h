///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

#ifndef __ema_consumer_h_
#define __ema_consumer_h_

#include <queue>
#include <mutex>
#include <atomic>
#include <memory>
#include <thread>
#include <tuple>

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
	bool tryPush(T&& element)
	{
		const std::lock_guard<std::mutex> lock{ _mutex };

		if (_queue.size() < _maxQueueSize)
		{
			_queue.push(std::move(element));
			return true;
		}

		return false;
	}

	/* Puts an element into the end of queue. Waits until queue is not full.
	*/
	void push(T&& element)
	{
		while (!tryPush(std::move(element)))
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

		element = std::move(_queue.front());
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
	//API QA
	void setOmmConsumer(refinitiv::ema::access::OmmConsumer*);
	//END API QA

protected:

	void onRefreshMsg(const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

	void onUpdateMsg(const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

	void onStatusMsg(const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent&);
	//API QA
	void onAckMsg(const refinitiv::ema::access::AckMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

	void onPostMsg(const refinitiv::ema::access::PostMsg&, const refinitiv::ema::access::OmmConsumerEvent&);

	void onGenericMsg(const refinitiv::ema::access::GenericMsg&, const refinitiv::ema::access::OmmConsumerEvent&);
	//END API QA

private:

	using MsgPtr = typename std::unique_ptr<refinitiv::ema::access::Msg>;
	using MsgWrapper = typename std::tuple<MsgPtr, refinitiv::ema::access::UInt64>;

	// incoming message queue
	ConcurrentQueue<MsgWrapper> _messageQueue;

	// pools of different message type objects
	ConcurrentQueue<std::unique_ptr<refinitiv::ema::access::RefreshMsg>> _refreshMsgPool;
	ConcurrentQueue<std::unique_ptr<refinitiv::ema::access::UpdateMsg>> _updateMsgPool;
	ConcurrentQueue<std::unique_ptr<refinitiv::ema::access::StatusMsg>> _statusMsgPool;
	//API QA
	ConcurrentQueue<std::unique_ptr<refinitiv::ema::access::AckMsg>> _ackMsgPool;
	ConcurrentQueue<std::unique_ptr<refinitiv::ema::access::PostMsg>> _postMsgPool;
	ConcurrentQueue<std::unique_ptr<refinitiv::ema::access::GenericMsg>> _genericMsgPool;

	refinitiv::ema::access::OmmConsumer* _pOmmConsumer;
	//END API QA

	std::atomic<bool> _exit;
	std::thread _dispatchThread;
};

#endif // __ema_consumer_h_
