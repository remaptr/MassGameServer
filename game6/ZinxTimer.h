#pragma once
#include "zinx.h"
#include<sys/timerfd.h>
#include<stdint.h>
#include"ZinxTimerDeliver.h"

class ZinxTimer :	public Ichannel
{
public:
	ZinxTimer();
	~ZinxTimer();
public:
	// Í¨¹ý Ichannel ¼Ì³Ð
	virtual bool Init() override;
	virtual bool ReadFd(std::string & _input) override;
	virtual bool WriteFd(std::string & _output) override;
	virtual void Fini() override;
	virtual int GetFd() override;
	virtual std::string GetChannelInfo() override;
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput) override;

private:
	int mTimerFd = -1;
};

