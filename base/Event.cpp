
#include "Event.h"
#include <mutex>
#include <list>
#include <chrono>


using namespace std;

namespace fc{
   	
//event实例的结构体定义
	struct EVENTINSTANCESTRUCT{
	public:
		std::string m_eventName;
		int m_eventId;
		bool m_bMunalReset;
		bool m_bActive;
		bool m_bClose;
		EVENTINSTANCESTRUCT(){
			m_bActive = false;
			m_bMunalReset = false;
			m_bClose = false;
		};
	};

	//全局变量
	//因为程序的各类析构的顺序不固定，析构时可能调用EVENT相关函数。如果这里的全局变量首先被析构，那么就会导致异常。 所以这里故意使用纯指针，那就不会析构了
	list< std::shared_ptr<EVENTINSTANCESTRUCT> >*  event_regEventList = new list< std::shared_ptr<EVENTINSTANCESTRUCT> >;
	int* event_structUseId = new int(20000);//EVENTINSTANCE从20000开始
	mutex* event_localMutex = new mutex;//本地锁，其实只是为了匹配CV的定义
	condition_variable* event_cv =new condition_variable;
	//end




	//
	EVENTINSTANCE CreateEventInstance(bool bManualReset, bool bbInitialState, std::string sEventName){
		unique_lock<mutex> lock(*event_localMutex);
		if (sEventName != ""){
			for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
				if ((*it)->m_eventName == "sEventName"){
					return (*it)->m_eventId;
				}
			}
		}


		shared_ptr<struct EVENTINSTANCESTRUCT> ptr(new EVENTINSTANCESTRUCT);
		ptr->m_bActive = bbInitialState;
		ptr->m_bMunalReset = bManualReset;
		ptr->m_eventId = (*event_structUseId)++;
		ptr->m_eventName = sEventName;
		event_regEventList->push_back(ptr);
		return ptr->m_eventId;
	}

	int SetEventInstace(EVENTINSTANCE event){
		unique_lock<mutex> lock(*event_localMutex);

		bool bEventExist = false;

		for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
			if ((*it)->m_eventId == event){
				bEventExist = true;
				(*it)->m_bActive = true;
				event_cv->notify_all();
				break;
			}
		}
		return bEventExist?0:FC_EVENT_NO_EXIST;
	}

	int ResetEventInstance(EVENTINSTANCE event){
		unique_lock<mutex> lock(*event_localMutex);

		bool bEventExist = false;

		for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
			if ((*it)->m_eventId == event){
				bEventExist = true;
				(*it)->m_bActive = false;
			}
		}

		return bEventExist ? 0 : FC_EVENT_NO_EXIST;
	}



	int CloseEventInstance(EVENTINSTANCE event){
		unique_lock<mutex> lock(*event_localMutex);

		bool bEventExist = false;

		for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
			if ((*it)->m_eventId == event){
				bEventExist = true;
				(*it)->m_bActive = true;
				event_cv->notify_all();//如果有WaitForEvent正在wait的情况，先触发一下，让他可以打开阻塞
				(*it)->m_bClose = true;
				event_regEventList->erase(it);
				(*event_structUseId)--;
				break;
			}
		}

		return bEventExist ? 0 : FC_EVENT_NO_EXIST;
	}


	int WaitForEvent(EVENTINSTANCE events[], int eventNum, bool bWaitAll, long long waitTimeMs){
		int eventActiveNum = 0;

		if (events == NULL || waitTimeMs < -1){
			return FC_EVENT_TIME_SET_FAIL;
		}

		unique_lock<mutex> lock(*event_localMutex);

		while (1){
			//检查event的实例是否真实存在   因为上一次循环中wait_for可能曾经出锁了，所以要重新检查
			int  eventExistCount = 0;
			for (int i = 0; i < eventNum; i++){
				for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
					if ((*it)->m_eventId == events[i] && (*it)->m_bClose == false){
						eventExistCount++;
					}
				}
			}
			if (eventExistCount != eventNum){
				return FC_EVENT_NO_EXIST;
			}


			int eventActiveCount = 0;
			//先检查一下是否有已经触发的消息
			for (int i = 0; i < eventNum; i++){
				for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
					if ((*it)->m_eventId == events[i] && (*it)->m_bActive == true){
						eventActiveCount++;
						eventActiveNum = i;
						if (bWaitAll == false){
							//bWaitAll =false 将事件reset一下
							if ((*it)->m_bMunalReset == true){
								(*it)->m_bActive = false;
							}
							return eventActiveNum;
						}
					}
				}
			}


			if (bWaitAll == true){
				if (eventActiveCount == eventNum){//所有信号都等到了
					for (int i = 0; i < eventNum; i++){
						//bWaitAll =true 在返回前，把所有的事件reset一下
						for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
							if ((*it)->m_eventId == events[i] && (*it)->m_bActive == true){
								if ((*it)->m_bMunalReset == true){
									(*it)->m_bActive = false;
								}
							}
						}
					}
					return FC_EVENT_WAIT_0;
				}
				else//仍有信号需要等待
				{
					//跳到wait_for
				}
			}
			else if (bWaitAll == false)
			{
					//跳到wait_for
			}



			std::chrono::high_resolution_clock::time_point  startWaitTime = std::chrono::high_resolution_clock::now();
			std::cv_status st = event_cv->wait_for(lock, std::chrono::milliseconds(waitTimeMs));
			if (st == std::cv_status::timeout){
				if (bWaitAll == true){
					std::chrono::high_resolution_clock::duration d = std::chrono::high_resolution_clock::now() - startWaitTime;
					std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(d);
					if (waitTimeMs - ms.count() > 0){
						waitTimeMs = waitTimeMs - ms.count();
						continue;
					}
					else
					{
						return FC_EVENT_WAIT_TIMEOUT;
					}
				}
				else//超时时间返回
				{
					return FC_EVENT_WAIT_TIMEOUT;
				}
			}
			else
			{
				continue;
				//信号来了，检查是否要触发
			}
		}
	}




























	/*
	int WaitForEvent(EVENTINSTANCE events[], int eventNum, bool bWaitAll,long long waitTimeMs){
		int eventActiveNum = 0;

		if (events == NULL || waitTimeMs < -1){
			return FC_EVENT_TIME_SET_FAIL;
		}

		unique_lock<mutex> lock(*event_localMutex);

		//检查event的实例是否真实存在
		int  eventExistCount = 0;
		for (int i = 0; i < eventNum; i++){
			for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
				if ((*it)->m_eventId == events[i] && (*it)->m_bClose == false){
					eventExistCount++;
				}
			}

		}
		if (eventExistCount != eventNum){
			return FC_EVENT_NO_EXIST;
		}





		while (1){
			int eventActiveCount = 0;
			std::chrono::high_resolution_clock::time_point  startWaitTime = std::chrono::high_resolution_clock::now();


			//登陆信号触发
			std::cv_status st = event_cv->wait_for(lock, std::chrono::milliseconds(waitTimeMs));
			if (st == std::cv_status::timeout){
				return FC_EVENT_WAIT_TIMEOUT;
			}



			//因为锁lock可能已经被换手，所以要重新检查event的实例是否真实存在
			eventExistCount = 0;
			for (int i = 0; i < eventNum; i++){
				for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
					if ((*it)->m_eventId == events[i] && (*it)->m_bClose == false){
						eventExistCount++;
					}
				}
			}
			if (eventExistCount != eventNum){
				return FC_EVENT_NO_EXIST;
			}


			//没有超时，有信号来了
			for (int i = 0; i < eventNum; i++){
				for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
					if ((*it)->m_eventId == events[i] && (*it)->m_bActive == true){
						eventActiveCount++;
						eventActiveNum = i;
						if (bWaitAll == true){
							continue;
						}
						else if (bWaitAll ==false)
						{
							//bWaitAll =false 在返回前，把对应的事件reset一下
							if ((*it)->m_bMunalReset == true){
								(*it)->m_bActive = false;
								eventActiveNum = i;
							}
							break;
						}
					}
				}
			}


			if (bWaitAll == true){

				if (eventActiveCount == eventNum){//所有信号都等到了
					for (int i = 0; i < eventNum; i++){
						//bWaitAll =true 在返回前，把所有的事件reset一下
						for (auto it = event_regEventList->begin(); it != event_regEventList->end(); it++){
							if ((*it)->m_eventId == events[i] && (*it)->m_bActive == true){
								if ((*it)->m_bMunalReset == true){
									(*it)->m_bActive = false;
								}
							}
						}
					}
					return FC_EVENT_WAIT_0;
				}
				else//继续等待
				{
					std::chrono::high_resolution_clock::duration d= std::chrono::high_resolution_clock::now() - startWaitTime;
					std::chrono::milliseconds ms= std::chrono::duration_cast<std::chrono::milliseconds>(d);
					if (waitTimeMs - ms.count() > 0){
						waitTimeMs = waitTimeMs - ms.count();
						continue;
					}
					else
					{
						return FC_EVENT_WAIT_TIMEOUT;
					}
				}
			}
			else if (bWaitAll ==false)
			{
				return eventActiveNum;
			}


		}
	}
	*/



}//namespace fc

