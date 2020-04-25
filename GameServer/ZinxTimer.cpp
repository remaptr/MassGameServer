#include "ZinxTimer.h"

ZinxTimer::ZinxTimer()
{
}

ZinxTimer::~ZinxTimer()
{
}

bool ZinxTimer::Init()
{
	int ret = -1;
	struct itimerspec timer = { {1,0},{1,0} };

	mTimerFd = timerfd_create(CLOCK_MONOTONIC,0);
	if (-1 == mTimerFd)
	{
		perror("timerfd_create");
		return false;
	}


	ret = timerfd_settime(mTimerFd, 0, &timer, nullptr);
	if (-1 == ret)
	{
		perror("timerfd_settime");
		close(mTimerFd);
		return false;
	}

	return true;
}

bool ZinxTimer::ReadFd(std::string & _input)
{
	uint64_t count = 0;
	int ret = -1;

	ret = read(mTimerFd, &count, sizeof(count));
	if (sizeof(count) != ret)
	{
		perror("read");
		close(mTimerFd);
		return false;
	}

//	std::cout << "超时---------:" << count << std::endl;

	_input.append((char*)&count, sizeof(count));
	//_input =std::to_string(count);
	return true;
}

bool ZinxTimer::WriteFd(std::string & _output)
{
	return false;
}

void ZinxTimer::Fini()
{
	if (mTimerFd != 0)
	{
		close(mTimerFd);
		mTimerFd = -1;
	}
}

int ZinxTimer::GetFd()
{
	return mTimerFd;
}

std::string ZinxTimer::GetChannelInfo()
{
	return std::string("ZinxTimerChannel");
}

AZinxHandler * ZinxTimer::GetInputNextStage(BytesMsg & _oInput)
{
	return &ZinxTimerDeliver::GetInstance();
}
