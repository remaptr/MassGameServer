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
	// 你要弄清楚,每有一个新的客户端成功连接时,就会产生一个新的connfd,表现为产生一个新通道
	// 在这里,每产生一个新游戏通道,就意味着产生了一个新角色和新协议
	// 而且这里产生的三个对象是互相关联的
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

	//将role以及protocol 添加到Zinx中,注意是添加到list容器中,不是添加到epoll树上!
	//并且GameRole的实例只有初始化后才能添加到容器中.
	//只有GameChannel才添加到epoll树上.
	ZinxKernel::Zinx_Add_Role(*role);
	ZinxKernel::Zinx_Add_Proto(*protocol);

	return channel;
}


// 下个处理者为协议层
AZinxHandler* GameChannel::GetInputNextStage(BytesMsg & _oInput)
{
	return  mProtocol;
}
