#pragma once
#include "zinx.h"
#include <vector>
#include <list>



class TimerOutProc
{
public:
	virtual void Proc() = 0;
	virtual int GetTimerSec() = 0;
	virtual ~TimerOutProc() {};
};



struct WheelNode
{
	int round;
	TimerOutProc* pProc = nullptr;
};



class ZinxTimerDeliver :public AZinxHandler
{
private:
	static ZinxTimerDeliver m_Single;
	int m_Cur_Index;
	std::vector<std::list<WheelNode>> m_TimerWheel;


public:
	ZinxTimerDeliver();
	static ZinxTimerDeliver& GetInstance();
	bool RegisterProcObject(TimerOutProc& _proc);
	bool UnRegisterProcObject(TimerOutProc& _proc);
	// Í¨¹ý AZinxHandler ¼Ì³Ð
	virtual IZinxMsg * InternalHandle(IZinxMsg & _oInput) override;
	virtual AZinxHandler * GetNextHandler(IZinxMsg & _oNextMsg) override;
};

