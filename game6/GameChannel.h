#pragma once
#include "ZinxTCP.h"
#include"GameProtocol.h"
#include"GameRole.h"




class GameChannelFactory :public IZinxTcpConnFact
{
public:
	// 通过 IZinxTcpConnFact 继承
	virtual ZinxTcpData * CreateTcpDataChannel(int & _fd) override;
};


class GameChannel :public ZinxTcpData
{
public:

	GameChannel(int& _fd);
	virtual ~GameChannel();

	// 通过 ZinxTcpData 继承
	virtual AZinxHandler * GetInputNextStage(BytesMsg & _oInput) override;

public:
	GameProtocol *mProtocol;
	GameRole *mRole;
};

