#ifndef TIMER_TIMER_2_H
#define TIMER_TIMER_2_H

#include <functional>
#include <mutex>
#include <map>
#include <thread>
#include <chrono>
#include "Event.h"
using namespace std;

struct TIMERSTRUCT{
	TIMERSTRUCT(){
		m_id = 0;
		m_afterMs = std::chrono::milliseconds(0);
		m_endTime = std::chrono::high_resolution_clock::now();
		m_bTimeOut = false;

	}
	~TIMERSTRUCT(){

	}

	int m_id;
	std::chrono::milliseconds m_afterMs;
	std::chrono::high_resolution_clock::time_point m_endTime;
	bool m_bTimeOut;
	function<void()> m_callback;

};

class Timer{
public:
	 Timer();
	 ~Timer();

	int RegTimeOutNotify(std::function<void()> timeOutcallbackFunction);
	int StartTimer();
	int StopTimer();

	struct TIMERSTRUCT GetTimeOutFunction();
	void GetTimeOutFunctionAndRun();

	int SetTimer(int timerID, int afterMs, function<void()> callBackFunction);//SetTimer的人要负责 对回调函数适当保护  返回值为当前本次事件的ID
	int KillTimer(int timerId);




private:
	void TimeOutProc();
	std::chrono::milliseconds CountLastWaitTime();
	void ActiveTimer();

	std::map<int, TIMERSTRUCT> m_timerMap;
	std::function<void()> m_timeOutNotifyCallback;
	std::chrono::high_resolution_clock::time_point m_timeBegin;//计时器的起始点
	std::thread m_timerProcThread;
	bool m_bThreadRunning;
	EVENTINSTANCE m_timeOutEvent;
	EVENTINSTANCE m_stopThreadEvent;
	EVENTINSTANCE m_TimerNewEvent;
	std::chrono::milliseconds m_lastWaitTime;//下一次触发定时器需要等待的时间

	std::mutex m_lock;
};

#endif
