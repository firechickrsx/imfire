/*夸平台实现 CreateEvent/SetEvent/WaitForMutiObject*/


#ifndef EVENT_EVENT_H_1
#define EVENT_EVENT_H_1
#include <condition_variable>
#include<string>
#include <memory>


#define FC_EVENT_NO_EXIST -1
#define FC_EVENT_WAIT_TIMEOUT -2
#define FC_EVENT_TIME_SET_FAIL -3
#define FC_EVENT_WAIT_0 0
#define FC_EVENT_WAIT_INFINITI 0Xffffffff

#define EVENTINSTANCE int

namespace fc{




	/*CreateEventInstance 创建一个事件实例
	//bManualReset 是否自动恢复event

	*/
	EVENTINSTANCE CreateEventInstance(bool bManualReset, bool bbInitialState, std::string sEventName);

	//关闭event
	int CloseEventInstance(EVENTINSTANCE event);

	//设置Event为激活状态
	int SetEventInstace(EVENTINSTANCE event);

	//设置Event为未激活状态
	int ResetEventInstance(EVENTINSTANCE event);

	//等待Event,阻塞
	int WaitForEvent(EVENTINSTANCE events[], int eventNUm, bool bWaitAll, long long waitTimeMs);
}

#endif
