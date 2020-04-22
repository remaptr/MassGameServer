#include "GameChannel.h"

GameChannel::GameChannel(int & _fd) :ZinxTcpData(_fd)
{
}

GameChannel::~GameChannel()
{
	if (mRole != nullptr)
	{
		ZinxKernel::Zinx_Del_Role(*mRole);
		delete mRole;
	}

	if (mProtocol != nullptr)
	{
		ZinxKernel::Zinx_Del_Proto(*mProtocol);
		delete mProtocol;
	}
}




ZinxTcpData * GameChannelFactory::CreateTcpDataChannel(int & _fd)
{
	GameChannel* channel = new GameChannel(_fd);
	GameProtocol *protocol = new GameProtocol;
	GameRole *role = new GameRole;

	//建立三者之间的关联
	channel->mProtocol = protocol;
	channel->mRole = role;

	protocol->mChannel = channel;
	protocol->mRole = role;

	role->mProtocol = protocol;
	role->mChannel = channel;

	//将role以及protocol 添加到 zinx框架中管理
	ZinxKernel::Zinx_Add_Role(*role);
	ZinxKernel::Zinx_Add_Proto(*protocol);

	return channel;
}


// 下个处理者为协议层
AZinxHandler * GameChannel::GetInputNextStage(BytesMsg & _oInput)
{
	return  mProtocol;
}
