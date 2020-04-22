#include"zinx.h"
#include"StdoutChannel.h"
#include"ZinxTimer.h"
#include"ZinxTimerDeliver.h"
#include"ZinxTCP.h"
#include"GameChannel.h"


#if 0
class TimeProc :public TimerOutProc
{
public:
	// 通过 TimerOutProc 继承
	virtual void Proc() override
	{
		std::cout << "----任务处理函数----" << std::endl;
		std::cout << std::endl;
	}

	virtual int GetTimerSec() override
	{
		return 3;
	}

};
#endif


int main()
{
	StdoutChannel* out=new StdoutChannel;
	ZinxTimer* time=new ZinxTimer;

	//TimeProc proc;
//	ZinxTimerDeliver::GetInstance().RegisterProcObject(proc);

	ZinxTCPListen* tcp = new ZinxTCPListen(10086, new GameChannelFactory);

	ZinxKernel::ZinxKernelInit();

	ZinxKernel::Zinx_Add_Channel(*out);
	ZinxKernel::Zinx_Add_Channel(*time);
	ZinxKernel::Zinx_Add_Channel(*tcp);

	ZinxKernel::Zinx_Run();

	ZinxKernel::ZinxKernelFini();
}