
#include "Timer.h"
#include <thread>

//1代表客户未选择过
//0代表客户已选择过





Timer::Timer(){
    m_timeOutEvent = fc::CreateEventInstance(true, false, "");
    m_stopThreadEvent = fc::CreateEventInstance(true, false, "");
	m_bThreadRunning = false;
	m_timeBegin = std::chrono::high_resolution_clock::now();
}


Timer::~Timer(){
	StopTimer();
	fc::CloseEventInstance(m_timeOutEvent);
	fc::CloseEventInstance(m_stopThreadEvent);
}

int Timer::RegTimeOutNotify(std::function<void()> timeOutcallbackFunction){
	unique_lock<mutex> lock(m_lock);
	m_timeOutNotifyCallback = timeOutcallbackFunction;
	return 0;
}

int Timer::StartTimer(){
	unique_lock<mutex> lock(m_lock);
	if (m_bThreadRunning == true){
		return -1;
	}

	std::thread runThread([=]{ TimeOutProc(); });
	m_timerProcThread.swap(runThread);
	return 0;
}

int Timer::StopTimer(){
	//unique_lock<mutex> lock(m_lock); 这里不锁上，并不会有安全问题，锁上反而造成死锁。
	if (m_bThreadRunning == false){
		return -1;
	}

	fc::SetEventInstace(m_stopThreadEvent);
	if (m_timerProcThread.joinable() == true){
		m_timerProcThread.join();
	}

	return 0;
}

void Timer::TimeOutProc(){
	m_bThreadRunning = true;


	EVENTINSTANCE ev[2] = { m_timeOutEvent, m_stopThreadEvent};


	while (1){
		//查找要等待的是时间
		std::chrono::milliseconds lastWaitTime = CountLastWaitTime();
		int eventNum = fc::WaitForEvent(ev, 2, false, lastWaitTime.count());

		if (FC_EVENT_WAIT_0 == eventNum){//触发定时器
			continue;//重新计算等待时间
		}
		else if (FC_EVENT_WAIT_0 + 1== eventNum){//关闭线程
			break;//退出，结束线程
		}
		else if (FC_EVENT_WAIT_TIMEOUT == eventNum)
		{
			ActiveTimer();
		}
		else
		{
			//CLog::MakeLog("timer error");
		}
	}

	m_bThreadRunning = false;
}

void Timer::ActiveTimer(){


	std::chrono::high_resolution_clock::time_point timeNow;
	timeNow = std::chrono::high_resolution_clock::now();
	bool bNeedCallBackNotify = false;

	m_lock.lock();
	for (std::map<int, TIMERSTRUCT>::iterator it = m_timerMap.begin(); it != m_timerMap.end(); it++){
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(it->second.m_endTime - timeNow);
		if (ms.count() <= 0){
			//通知超时
			it->second.m_bTimeOut = true;
			bNeedCallBackNotify = true;
		}
	}
	m_lock.unlock();

	if (bNeedCallBackNotify == true && m_timeOutNotifyCallback == NULL){
		m_timeOutNotifyCallback();
	}
}


struct TIMERSTRUCT Timer::GetTimeOutFunction(){
	unique_lock<mutex> lock(m_lock);
	struct TIMERSTRUCT st;

	for (std::map<int,TIMERSTRUCT>::iterator it = m_timerMap.begin(); it != m_timerMap.end(); it++){
		st = it->second;
		if (st.m_bTimeOut == true){//超时了，重新设置下一次超时
			it->second.m_endTime += it->second.m_afterMs;
			it->second.m_bTimeOut = false;
			break;
		}
	}
	//更改了，让计时线程重新计算
	fc::SetEventInstace(m_timeOutEvent);
	return st;
}

void Timer::GetTimeOutFunctionAndRun(){
    	unique_lock<mutex> lock(m_lock);
	struct TIMERSTRUCT st;
	for (std::map<int,TIMERSTRUCT>::iterator it = m_timerMap.begin(); it != m_timerMap.end(); it++){
		st = it->second;
		if (st.m_bTimeOut == true){//超时了，重新设置下一次超时
			it->second.m_endTime += it->second.m_afterMs;
			it->second.m_bTimeOut = false;
			it->second.m_callback();
		}
	}
	//更改了，让计时线程重新计算
	fc::SetEventInstace(m_timeOutEvent);
}





int Timer::SetTimer(int timerID, int afterMs, function<void()> callBackFunction){//SetTimer的人要负责 对回调函数适当保护
	if (afterMs < 0 || callBackFunction == NULL){
		return -1;
	}

	unique_lock<mutex> lock(m_lock);

	struct TIMERSTRUCT st;
	st.m_id = timerID;
	st.m_afterMs = std::chrono::milliseconds(afterMs);
	st.m_endTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(afterMs);
	st.m_callback = callBackFunction;

	std::map<int, TIMERSTRUCT>::iterator it;
	it = m_timerMap.find(timerID);
	if (it  == m_timerMap.end()){
		//没找到，插入
		m_timerMap.insert(std::make_pair(st.m_id, st));
	}
	else//找到了，替换
	{
		it->second = st;
	}

	//更改了，让计时线程重新计算
	fc::SetEventInstace(m_timeOutEvent);
	return 0;
}


int Timer::KillTimer(int timerId){
	unique_lock<mutex> lock(m_lock);
	if (m_timerMap.find(timerId) == m_timerMap.end()){
		//没找到
		return -1;
	}

	m_timerMap.erase(timerId);
	//更改了，让计时线程重新计算
	fc::SetEventInstace(m_timeOutEvent);
	return 0;

}

std::chrono::milliseconds Timer::CountLastWaitTime(){
	unique_lock<mutex> lock(m_lock);
	std::chrono::milliseconds nextWaitTime(1000); //初始的等待时间
	struct TIMERSTRUCT st;

	std::chrono::high_resolution_clock::time_point timeNow;
	timeNow = std::chrono::high_resolution_clock::now();

	//遍历整个map，找出下一次触发定时器还有多少时间
	for (std::map<int, TIMERSTRUCT>::iterator it = m_timerMap.begin(); it != m_timerMap.end(); it++){
		st = it->second;
		if (st.m_bTimeOut == false){
			std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(st.m_endTime - timeNow);
			if (ms.count() <= 0){
				nextWaitTime = std::chrono::milliseconds(0);
				break;
			}
			if (ms <= nextWaitTime){//寻找最小时间
				nextWaitTime = ms;
			}

		}
	}
	return nextWaitTime;
}


