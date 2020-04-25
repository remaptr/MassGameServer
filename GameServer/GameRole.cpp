#include "GameRole.h"

int GameRole::smRoleCount = 0;
std::default_random_engine g_rand_engine(time(nullptr));

GameRole::GameRole()
{
	this->mPlayerId = smRoleCount;
	//std::cout << this->mPlayerId << std::endl;
	this->mPlayerName = "Player_" + std::to_string(smRoleCount);
	smRoleCount++;

	//初始位置
	x = 100 + (g_rand_engine() % 20);
	y = 0;
	z = 100 + (g_rand_engine() % 20);
	v = 0;

	//hp 满血是1000
	hp = 1000;

}

GameRole::~GameRole()
{
}


// 角色层处理完毕后,把消息返回给协议层
bool GameRole::Init()
{
	// 服务器将消息返回给协议层
	GameMsg* pMsg = MakeLogonSyncPid();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	// 服务器将当前玩家的出生位置发送给当前玩家
	pMsg = MakeInitPosBroadcast();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);


	// 当前玩家把自己的出生通知发送给其他人,并将其他人的出生告知自己
	auto players = ZinxKernel::Zinx_GetAllRole();


	// 如果是第一个玩家出生,那么for循环中的语句不会被执行,
	// 但是最后还是返回true,第一个对象被加入到zinx的list中
	for (auto &r : players)
	{
		// 实际上这一步不会发生,因为角色必须初始化完毕后,才会在players中.
		if (r == this)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);

		// 当前玩家出生的消息发送给其他玩家
		pMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);

		// 将他人的出生信息告知自己
		pMsg = role->MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);
	}

	return true;
}




UserData * GameRole::ProcMsg(UserData & _poUserData)
{
	// msg里的m_MsgList存储的是已经反序列化的protobuf对象
	GameMsg &msg = dynamic_cast<GameMsg&>(_poUserData);

	for (auto single : msg.m_MsgList)
	{
		switch (single->m_MsgType)
		{
		case GameSingleTLV::GAME_MSG_NEW_POSTION:
			auto pbMsg = dynamic_cast<pb::Position*>(single->m_poGameMsg);

			std::cout << "x=" << pbMsg->x() << " "
				<< "y=" << pbMsg->y() << " "
				<< "z=" << pbMsg->z() << " "
				<< "v=" << pbMsg->v() << " "
				<< std::endl;

			// 当前玩家把新的位置坐标发送给其他客户端
			this->ProcNewPosition(pbMsg->x(), pbMsg->y(), pbMsg->z(), pbMsg->v());
			break;
		}
	}

	return nullptr;
}





void GameRole::Fini()
{
	//当前玩家下线的时候，这个函数被调用,在这里实现下线逻辑
	//获取所有的玩家
	auto players = ZinxKernel::Zinx_GetAllRole();

	for (auto &r : players)
	{
		if (r == this)
			continue;

		auto role = dynamic_cast<GameRole*>(r);
		//生成玩家下线消息(此处需要每一次重新创建玩家下线消息)
		auto pMsg = MakeLogoffSyncPid();

		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}
}



GameMsg* GameRole::MakeLogonSyncPid()
{
	auto pbMsg = new SyncPid;

	// pbMsg用于构造GameSingleTLV的对象,用于序列化 
	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_username(this->mPlayerName);

	GameSingleTLV* single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGON_SYNCPID, pbMsg);

	GameMsg* retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}



GameMsg * GameRole::MakeInitPosBroadcast()
{
	// ID和姓名
	auto pbMsg = new BroadCast;
	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_username(this->mPlayerName);

	// 位置信息
	pbMsg->set_tp(2);
	auto pos = pbMsg->mutable_p();
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);
	pos->set_bloodvalue(hp);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}



// 玩家新位置的广播
// 注意:玩家的新位置是客户端发过来的
// 服务器把此玩家的新位置广播出去
GameMsg * GameRole::MakeNewPosBroadcast()
{
	auto pbMsg = new BroadCast;

	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_username(this->mPlayerName);

	/*根据Tp不同，BroadCast消息会包含：
	  1 聊天内容（Content）
	  2 初始位置(P)
	  4 新位置P
	*/
	pbMsg->set_tp(4);
	auto pos = pbMsg->mutable_p();
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);
	pos->set_bloodvalue(hp);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}


/*创建下线时的id和姓名消息*/
GameMsg* GameRole::MakeLogoffSyncPid()
{
	auto pbMsg = new SyncPid;

	pbMsg->set_pid(this->mPlayerId);
	pbMsg->set_username(this->mPlayerName);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_LOGOFF_SYNCPID, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}



void GameRole::ProcNewPosition(float _x, float _y, float _z, float _v)
{
      this->x=_x;
      this->y=_y;
      this->z=_z;
      this->v=_v;

	  auto players = ZinxKernel::Zinx_GetAllRole();

	  for (auto &r : players)
	  {
		  if (r == this)
		  {
			  continue;    //  直接返回for循环
		  }

		  auto role = dynamic_cast<GameRole*>(r);
		  auto pMsg = MakeNewPosBroadcast();

		  ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	  }
    
}
