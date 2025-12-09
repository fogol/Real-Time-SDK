/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef _CONSUMER_THREADS_H
#define _CONSUMER_THREADS_H

#if defined(WIN32)
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#include <assert.h>

#if defined(WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#else

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <strings.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#endif

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

#include "ConsPerfConfig.h"
#include "../Common/Statistics.h"
#include "../Common/GetTime.h"
#include "../Common/AppVector.h"
#include "../Common/Mutex.h"
#include "../Common/AppUtil.h"
#include "../Common/ThreadBinding.h"
#include "LatencyCollection.h"
#include "LatencyRandomArray.h"
#include "PerfMessageData.h"

#define		BASECONSUMER_NAME "Perf_Consumer_"
#define		CONSUMER_NAME_WSJSON "Perf_Consumer_WSJSON_"
#define		CONSUMER_NAME_WSRWF "Perf_Consumer_WSRWF_"

class EmaCppConsPerf;
class ConsumerThread;
class ItemRequest;
class MessageQueueThread;

typedef perftool::common::AppVector<ItemRequest*> ItemRequestList;
typedef perftool::common::AppVector<ItemRequest*> PostGenMsgItemList;
typedef perftool::common::AppVector< TimeRecord > LatencyRecords;

class ConsumerThreadState {
public:
	ConsumerThreadState(const ConsPerfConfig&);
	void reset() {
		currentPostItemIndex = 0;
		currentGenericsItemIndex = 0;
	}

	UInt32 getCurrentTick() const { return currentTick; }
	void incrementCurrentTick() {
		if (++currentTick >= (UInt32)consConfig.ticksPerSec)
			currentTick = 0;
	}

	UInt32 getRequestPerTick() const { return requestsPerTick; }
	UInt32 getRequestPerTickRemainder() const { return requestsPerTickRemainder; }

	UInt32 getPostPerTick() const { return postsPerTick; }
	UInt32 getPostPerTickRemainder() const { return postsPerTickRemainder; }
	UInt32 getCurrentPostItemIndex() const { return currentPostItemIndex; }
	void setCurrentPostItemIndex(UInt32 index) { currentPostItemIndex = index; }

	UInt32 getGenericsPerTick() const { return genericsPerTick; }
	UInt32 getGenericsPerTickRemainder() const { return genericsPerTickRemainder; }
	UInt32 getCurrentGenericsItemIndex() const { return currentGenericsItemIndex; }
	void setCurrentGenericsItemIndex(UInt32 index) { currentGenericsItemIndex = index; }

private:
	const ConsPerfConfig& consConfig;

	UInt32	currentTick;					/* Current tick out of ticks per second. */

	// Sending Requests
	UInt32	requestsPerTick;				/* Requests per tick */
	UInt32	requestsPerTickRemainder;		/* Requests per tick (remainder) */

	// Sending PostMsg-es
	UInt32	postsPerTick;					/* Posts per tick */
	UInt32	postsPerTickRemainder;			/* Posts per tick (remainder) */
	UInt32	currentPostItemIndex;			/* Index of the current item in Posts rotating list */

	// Sending GenericMsg-es
	UInt32	genericsPerTick;				/* Number of Generic per tick */
	UInt32	genericsPerTickRemainder;		/* Generics per tick (remainder) */
	UInt32	currentGenericsItemIndex;		/* Index of the current item in Generics rotating list */
};  // class ConsumerThreadState

// Specifies PublisherId
class PublisherUserInfo
{
public:
	PublisherUserInfo();

	UInt32 userId;			// specifies publisher's user id
	UInt32 userAddress;		// specifies publisher's user address
};  // class PublisherUserInfo

class ConsumerStats
{
public:
	ConsumerStats();
	~ConsumerStats();
	PerfTimeValue	imageRetrievalStartTime;		// Time at which first item request was made.
	PerfTimeValue	imageRetrievalEndTime;			// Time at which last item refresh was received.
	PerfTimeValue	firstUpdateTime;				// Time at which first item update was received.
	PerfTimeValue	firstGenMsgSentTime;			// Time at which first generic message was sent
	PerfTimeValue	firstGenMsgRecvTime;			// Time at which first generic message was received


	CountStat		refreshCount;				// Number of item refreshes received.
	CountStat		startupUpdateCount;			// Number of item updates received during startup.
	CountStat		steadyStateUpdateCount;		// Number of item updates received during steady state.
	CountStat		requestCount;				// Number of requests sent.
	CountStat		statusCount;				// Number of item status messages received.
	CountStat		postSentCount;				// Number of posts sent.
	CountStat		postOutOfBuffersCount;		// Number of posts not sent due to lack of buffers.
	CountStat		genMsgSentCount;			// Number of generic msgs sent.
	CountStat		genMsgRecvCount;			// Number of generic msgs received.
	CountStat		latencyGenMsgSentCount;		// Number of latency generic msgs sent.
	CountStat		genMsgOutOfBuffersCount;	// Number of generic msgs not sent due to lack of buffers.
	ValueStatistics	intervalLatencyStats;		// Latency statistics (recorded by stats thread).
	ValueStatistics	intervalPostLatencyStats;	// Post latency statistics (recorded by stats thread).
	ValueStatistics	intervalGenMsgLatencyStats;	// Gen Msg latency statistics (recorded by stats thread).

	PerfTimeValue	steadyStateLatencyTime;		// Time at which steady-state latency started to calculate.
	ValueStatistics startupLatencyStats;		// Statup latency statistics.
	ValueStatistics steadyStateLatencyStats;	// Steady-state latency statistics.
	ValueStatistics overallLatencyStats;		// Overall latency statistics.
	ValueStatistics postLatencyStats;			// Posting latency statistics.
	ValueStatistics genMsgLatencyStats;			// Gen Msg latency statistics.
	bool		imageTimeRecorded;				// Stats thread sets this once it has recorded/printed
												// this consumer's image retrieval time.
};
class ServiceInfo {

public:
	ServiceInfo() : serviceId(0), serviceStateUp( false ), acceptingReq( true)  {};
	~ServiceInfo() {};
	EmaString	serviceName;
	UInt32 serviceId;
	bool	serviceStateUp;
	bool	acceptingReq;

	bool isDesiredServiceUp()
	{
		if( serviceStateUp && acceptingReq )
			return true;
		return false;
	};
	void dumpServiceInfo()
	{
		fprintf(stdout,"ServiceName: %s, ServiceId: %u, serviceStateUp: %s, acceptingReq: %s, isDesiredServiceUp: %s",
			serviceName.c_str(), serviceId,
			( serviceStateUp ? "true" : "false"), ( acceptingReq ? "true" : "false"), ( isDesiredServiceUp() ? "true" : "false"));
	}
};
// DirectoryHandler
class DirectoryClient : public refinitiv::ema::access::OmmConsumerClient
{
public :
	DirectoryClient() : pConsThread( NULL ) {};
	void decode( const refinitiv::ema::access::Map& );
	void decodeInfo( const refinitiv::ema::access::ElementList& elist, bool &gotDesiredSvc);
	void decodeState( const refinitiv::ema::access::ElementList& elist);
	void init( ConsumerThread *pConsThr );
protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	ConsumerThread *pConsThread;
};

// application defined client class for receiving and processing of item messages
class MarketPriceClient : public refinitiv::ema::access::OmmConsumerClient
{
public :
	MarketPriceClient() : pConsumerThread( NULL ) {};
	void init( ConsumerThread *pConsThr );

	static bool decodeMPUpdate(ConsumerThread* pConsumerThread,  const refinitiv::ema::access::FieldList&, UInt16 msgtype  );
	bool checkPostUserInfo() { return true; };

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent );

	void onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent );

	ConsumerThread *pConsumerThread;
};

// application defined client class for receiving and processing of item messages
class MarketByOrderClient : public refinitiv::ema::access::OmmConsumerClient
{
public :
	MarketByOrderClient() : pConsumerThread( NULL ) {};
	void init( ConsumerThread *pConsThr );

	static bool decodeMBOUpdate(ConsumerThread*, const refinitiv::ema::access::Map&, UInt16 msgtype );

	static bool decodeFldList( const FieldList& fldList, UInt16 msgtype, UInt64 &timeTracker, UInt64 &postTimeTracker,	UInt64 &genMsgTimeTracker, bool summData = false );
	bool checkPostUserInfo() { return true; };

protected :

	void onRefreshMsg( const refinitiv::ema::access::RefreshMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onUpdateMsg( const refinitiv::ema::access::UpdateMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onStatusMsg( const refinitiv::ema::access::StatusMsg&, const refinitiv::ema::access::OmmConsumerEvent& );

	void onGenericMsg( const GenericMsg& genericMsg, const OmmConsumerEvent& consumerEvent );

	void onAckMsg( const AckMsg& ackMsg, const OmmConsumerEvent& consumerEvent );

	ConsumerThread *pConsumerThread;
};

typedef enum
{
	ITEM_NOT_REQUESTED,			// Item request has not been set.
	ITEM_WAITING_FOR_REFRESH,	// Item is waiting for its solicited refresh.
	ITEM_HAS_REFRESH			// Item has received its solicited refresh.
} ItemRequestState;

typedef enum {
	ITEM_IS_STREAMING_REQ		= 0x04,		/* Provider should send updates */
	ITEM_IS_SOLICITED			= 0x10,		/* Item was requested(not published) */
	ITEM_IS_POST				= 0x20,		/* Consumer should send posts */
	ITEM_IS_GEN_MSG				= 0x40,		/* Consumer should send generic messages */
	ITEM_IS_PRIVATE				= 0x80		/* Item should be requested on private stream */
} ItemFlags;

class ItemInfo
{
public:
	ItemInfo() : handle(0), domain( refinitiv::ema::rdm::MMT_MARKET_PRICE ),
				pAppClient(NULL), itemFlags ( ITEM_IS_SOLICITED ), itemData( NULL) {};
	UInt64		handle;
	UInt16		domain;
	OmmConsumerClient	*pAppClient;
	UInt8		itemFlags;	// See ItemFlags struct
	void*		itemData; // Is it Closure???. Holds information about the item's data. This data will be different depending on the domain of the item.
};

class ItemRequest
{
public:
	ItemRequest(): position(0), requestState( ITEM_NOT_REQUESTED ), pEmaConsumer( NULL ) {};
	UInt64		position; // Link for item list.
	ItemRequestState requestState;	// Item state.
	EmaString	itemName;
	ItemInfo	itemInfo;
	OmmConsumer	*pEmaConsumer;
};


class ConsumerThread
{
	friend class EmaCppConsPerf;
	friend class MarketPriceClient;
	friend class MarketByOrderClient;
	friend class DirectoryClient;

	friend void processMpcOnRefreshMsg(const RefreshMsg&, ConsumerThread*, void*);
	friend void processMpcOnUpdateMsg(const UpdateMsg&, ConsumerThread*);
	friend void processMpcOnStatusMsg(const StatusMsg&, ConsumerThread*);
	friend void processMpcOnGenericMsg(const GenericMsg&, ConsumerThread*);

	friend void processMboOnRefreshMsg(const RefreshMsg&, ConsumerThread*, void*);
	friend void processMboOnUpdateMsg(const UpdateMsg&, ConsumerThread*);
	friend void processMboOnGenericMsg(const GenericMsg&, ConsumerThread*);

public:
	ConsumerThread( ConsPerfConfig& );

	void consumerThreadInit( ConsPerfConfig&, Int32 consIndex);
	virtual ~ConsumerThread();
	bool initialize();

	void setMessageQueueThread(MessageQueueThread* );

	void start();

	void stop();

	void run();

	bool isRunning() { return running; }
	bool isStopped() { return stopThread; }

	void setStopThread() { stopThread = true; }

	bool sendBursts();

	bool sendItemRequestBurst(UInt32 itemBurstCount);

	bool sendPostBurst(UInt32 postItemBurstCount);

	bool sendGenMsgBurst(UInt32 genMsgItemBurstCount);

protected:

	const ConsPerfConfig*   pConsPerfCfg;
	Int32				consumerThreadIndex;
	OmmConsumer			*pEmaOmmConsumer;
	ConsumerThreadState	consThreadState;

	DirectoryClient		srcClient;
	ServiceInfo			desiredService;
	bool				isDesiredServiceUp;
	UInt64				desiredServiceHandle;

	MarketPriceClient   mPriceClient;
	MarketByOrderClient mByOrderClient;

	Int32				itemListUniqueIndex;		// Index into the item list at which item
													// requests unique to this consumer start.
	Int32				itemListCount;				// Number of item requests to make.
	ConsumerStats		stats;				// Other stats, collected periodically by the main thread. */
	FILE				*statsFile;			// File for logging stats for this connection. */
	FILE				*latencyLogFile;	// File for logging latency for this connection. */

	EmaString			cpuId;				// CPU for binding Consumer thread.
	EmaString			apiThreadCpuId;		// CPU for binding EMA API internal thread when ApiDispatch mode set.
	EmaString			workerThreadCpuId;	// CPU for binding Reactor Worker thread.

	ItemRequestList		itemRequestList;
	PostGenMsgItemList	postItemList;
	PostGenMsgItemList	genMsgItemList;
	bool				stopThread;
	bool				running;

	ReqMsg				requestMsg;
	Int32				itemsRequestedCount;
	Int32				refreshCompleteCount;

	// collection of latency timestamps
	LatencyCollection		updatesLatency;
	LatencyCollection		postsLatency;
	LatencyCollection		genericsLatency;

	LatencyRandomArray*		latencyPostRandomArray;		// Determines when to send latency in PostMsg
	LatencyRandomArray*		latencyGenericRandomArray;	// Determines when to send latency in GenericMsg

	bool					testPassed;
	EmaString				failureLocation;

	static PublisherUserInfo	publisherUserInfo;

	std::thread*				_pThread;

private:

	MessageQueueThread* _pMessageQueueThread;

	void dumpConsumerItemList();

	// Post Message. Fill up payload for MarketPrice.
	void preparePostMessageMarketPrice(PostMsg& postMsg, const ItemInfo& itemInfo, PerfTimeValue latencyStartTime = 0);

	// Post Message. Fill up payload for MarketByOrder.
	void preparePostMessageMarketByOrder(PostMsg& postMsg, const ItemInfo& itemInfo, PerfTimeValue latencyStartTime = 0);

	// Generic Message. Fill up payload for MarketPrice.
	void prepareGenericMessageMarketPrice(GenericMsg& genericMsg, PerfTimeValue latencyStartTime = 0);

	// Generic Message. Fill up payload for MarketByOrder.
	void prepareGenericMessageMarketByOrder(GenericMsg& genericMsg, PerfTimeValue latencyStartTime = 0);

	void clean();
};

inline void MarketPriceClient::init( ConsumerThread *pConsThr )
{
	pConsumerThread = pConsThr;
}

inline void MarketByOrderClient::init( ConsumerThread *pConsThr )
{
	pConsumerThread = pConsThr;
}

inline void DirectoryClient::init( ConsumerThread *pConsThr )
{
	pConsThread = pConsThr;
}


class RotateAppVectorUtil {
public:
	RotateAppVectorUtil(PostGenMsgItemList& vec, refinitiv::ema::access::UInt32 startInd)
		: startIndex(startInd), currentIndex(startInd), data(vec)
	{
		if (startIndex >= data.size())
		{
			if (!data.empty())
				currentIndex = startIndex % data.size();
			else
				currentIndex = 0;
		}
	}

	refinitiv::ema::access::UInt32 getCurrentIndex() { return currentIndex; }

	const ItemRequest* getNext() {
		refinitiv::ema::access::UInt32 index = currentIndex;
		if (++currentIndex >= data.size())
		{
			currentIndex = 0;
		}
		return data[index];
	}

	const ItemRequest* operator[](refinitiv::ema::access::UInt32 index) const {
		return data[index];
	}

private:
	refinitiv::ema::access::UInt32 startIndex;
	refinitiv::ema::access::UInt32 currentIndex;

	PostGenMsgItemList& data;
};  // class RotateAppVectorUtil

/* A basic thread-safe blocking queue of fixed maximum capacity. Wraps standard queue (of
   pointers to objects).
*/
template <typename T>
class ConcurrentQueue
{
public:

	explicit ConcurrentQueue(UInt32 maxQueueSize) :
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

	std::queue<T>			_queue;     // the queue itself
	mutable std::mutex		_mutex; // used to protect _queue from data races
	UInt32					_maxQueueSize;
	volatile bool			_exit;
};

struct MsgWrapper
{
	Msg*					pMsg;
	ConsumerThread* 		pConsumerThread;
	void*					pClosure;
};

// Background thread to handle incoming messages. Runs concurrently to the ConsumerThread
// it is associated with.
//
// When a consumer receives incoming message in the callback, the message is copied into
// the messageQueue and is processed by this thread.
//
// This thread decodes the message, processes it, and reports results to the associated
// ConsumerThread.
class MessageQueueThread
{
public :

	MessageQueueThread(const ConsPerfConfig&);
	~MessageQueueThread();

	void setConsumerThread(ConsumerThread*);

	void start();
	void stop();

	void enqueue(const RefreshMsg&, ConsumerThread* , void*);
	void enqueue(const UpdateMsg&, ConsumerThread*);
	void enqueue(const StatusMsg&, ConsumerThread*);
	void enqueue(const GenericMsg&, ConsumerThread*);

private :

	ConsumerThread* _pConsThread;
	volatile bool _exit;

	std::thread* _thread;

	// the incoming message queue
	ConcurrentQueue<MsgWrapper> _messageQueue;

	// pools of message objects ready to be put into the _messageQueue by callbacks. These
	// are raw pointers into the _all*Msgs vectors
	ConcurrentQueue<RefreshMsg*> _refreshMsgPool;
	ConcurrentQueue<UpdateMsg*> _updateMsgPool;
	ConcurrentQueue<StatusMsg*> _statusMsgPool;
	ConcurrentQueue<GenericMsg*> _genericMsgPool;

	std::vector<RefreshMsg> _allRefreshMsgs;
	std::vector<UpdateMsg> _allUpdateMsgs;
	std::vector<StatusMsg> _allStatusMsgs;
	std::vector<GenericMsg> _allGenericMsgs;

	void run();

}; // class MessageQueueThread

#endif // _CONSUMER_THREADS_H
