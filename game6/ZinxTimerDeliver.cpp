#include "ZinxTimerDeliver.h"


ZinxTimerDeliver ZinxTimerDeliver::m_Single;
#define ZINX_TIMER_WHEEL_SIZE 8

// m_Cur_Index赋值为0
ZinxTimerDeliver::ZinxTimerDeliver() :m_Cur_Index(0)
{
	m_TimerWheel.resize(ZINX_TIMER_WHEEL_SIZE);
}

ZinxTimerDeliver & ZinxTimerDeliver::GetInstance()
{
	// TODO: 在此处插入 return 语句
	return m_Single;
}

bool ZinxTimerDeliver::RegisterProcObject(TimerOutProc & _proc)
{
	int timespec = _proc.GetTimerSec();

	if (timespec < 1)
	{
		return false;
	}

	// 刻度数 即vector的索引  m_Cur_Icdex初始值为0
	int index = (m_Cur_Index + timespec) % m_TimerWheel.size();

	// 更新当前节点!!!!!!!!!!!这一步应该有的呀!!!!!!!!!!!
	// 原来这一步放在了内部处理函数中
	// m_Cur_Index = index;

	// 圈数
	int round = timespec / m_TimerWheel.size();

	// 时间轮子的具体节点赋值
	WheelNode node;
	node.pProc = &_proc;
	node.round = round;

	//  把节点添加到对应刻度上 节点包括:任务指针,圈数
	m_TimerWheel[index].push_back(node);

	return true;
}


bool ZinxTimerDeliver::UnRegisterProcObject(TimerOutProc & _proc)
{
	// 建立一个对m_TimerWheel的引用 i
	for (auto &i : m_TimerWheel)
	{
		for (auto it = i.begin(); it != i.end();)
		{
			if (it->pProc == &_proc)
			{
				it = i.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	return true;
}


IZinxMsg * ZinxTimerDeliver::InternalHandle(IZinxMsg & _oInput)
{
	uint64_t count;
	std::list<TimerOutProc*> registerList;

	BytesMsg& oBytes = dynamic_cast<BytesMsg&>(_oInput);
	oBytes.szData.copy((char*)&count, sizeof(count));
	//std::cout << "count=" << count << std::endl;

	for (int i = 0; i < count; ++i)
	{
		//std::cout << "i=" << i << std::endl;

		m_Cur_Index = (m_Cur_Index + 1) % m_TimerWheel.size();

		// 遍历当前刻度中的任务
		for (auto it = m_TimerWheel[m_Cur_Index].begin(); it != m_TimerWheel[m_Cur_Index].end();)
		{
			it->round--;
			if (it->round < 0)
			{
				// 调用任务函数
				it->pProc->Proc();
			    // 调用后加入到list中待重新注册
				registerList.push_back(it->pProc);
				// 调用后从当前刻度中删除
				 it=m_TimerWheel[m_Cur_Index].erase(it);

				 // 在这里重新注册
				 for (auto it :registerList )
				 {
				   RegisterProcObject(*it);
				 }
			}

			else
			{
				it++;
			}
		}
	}
	return nullptr;
}

AZinxHandler * ZinxTimerDeliver::GetNextHandler(IZinxMsg & _oNextMsg)
{
	return nullptr;
}
