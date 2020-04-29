#include "GameRole.h"

int GameRole::smRoleCount = 1;
std::default_random_engine g_rand_engine(time(nullptr));




// 随机出生点
GameRole::GameRole()
{
	this->mPlayerId = smRoleCount;
	smRoleCount++;
	//this->mPlayerName = "Player_" + std::to_string(mPlayerId);
	mPlayerName = RandomName::GetInstance().GetName();

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
	// 归还名字
	RandomName::GetInstance().ReleaseName(this->mPlayerName);
}


// 角色层处理完毕后,把消息返回给协议层
bool GameRole::Init()
{
	std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&11111111111111111111111111" << std::endl;
	std::cout << "++++++++++++++++++++测试打印+++++++++++++++++++++" << std::endl;

	// 服务器将消息返回给协议层
	auto pMsg = MakeLogonSyncPid();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	// 服务器将当前玩家的出生位置发送给当前玩家
	pMsg = MakeInitPosBroadcast();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	// 获取世界1
	mCurrentWorld = WorldManager::GetInstance().GetWorld(1);

	//将当前玩家添加到游戏世界中
	mCurrentWorld->AddPlayer(this);

	// 当前玩家把自己的出生通知发送给其他人,并将其他人的出生告知自己
	//auto players = ZinxKernel::Zinx_GetAllRole();

	// 服务器将周围玩家的信息一次性发送给自己
	pMsg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	//获取当前游戏世界中周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);


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
		// 消息发到mProtocol后,mProtocol后面的处理者是mChannel
		pMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);

		// 将他人的出生信息告知自己,这里的他人就是role, 而this才是自己
		// 服务器不再将其他玩家的位置信息 一次一个的发送给当前玩家
		//pMsg = role->MakeInitPosBroadcast();
		//ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);
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
		{
			auto pbMsg = dynamic_cast<pb::Position*>(single->m_poGameMsg);

			/*
			std::cout << "x=" << pbMsg->x() << " "
				<< "y=" << pbMsg->y() << " "
				<< "z=" << pbMsg->z() << " "
				<< "v=" << pbMsg->v() << " "
				<< std::endl;
			*/

			// 当前玩家把新的位置坐标发送给其他客户端
			this->ProcNewPosition(pbMsg->x(), pbMsg->y(), pbMsg->z(), pbMsg->v());
			break;
		}

		// 服务器在这里拿到客户端玩家的聊天信息 
		case GameSingleTLV::GAME_MSG_TALK_CONTENT:
		{
			// single里面的mpoGameMsg 存储了已经反序列化的客户端的发来的消息
			auto pbMsg = dynamic_cast<pb::Talk*>(single->m_poGameMsg);

			this->ProcTalkContent(pbMsg->content());
			break;
		}

		// 服务器在这里拿到客户端传来的原世界Id和新世界Id
		case GameSingleTLV::GAME_MSG_CHANGE_WORLD:
		{
			auto pbMsg = dynamic_cast<pb::ChangeWorldRequest*>(single->m_poGameMsg);
			this->ProcChangeWorld(pbMsg->srcid(), pbMsg->targetid());
			break;
		}

		// 服务器在这里拿到技能命中的信息
		case GameSingleTLV::GAME_MSG_SKILL_CONTACT:
		{
			auto pbMsg = dynamic_cast<pb::SkillContact*>(single->m_poGameMsg);
			cout << "SrcPid: " << pbMsg->srcpid()
				<< " TargetPid: "	<< pbMsg->targetpid()
				<< " SkillId: " << pbMsg->skillid()
				<< " BulletId: " << pbMsg->bulletid() << endl;
			this->ProcSkillContact(pbMsg);
			break;
		}
        
		// 服务器在这里拿到技能触发的信息
		case GameSingleTLV::GAME_MSG_SKILL_TRIGGER:
		{
			//客户端触发了技能
			auto pbMsg = dynamic_cast<pb::SkillTrigger*>(single->m_poGameMsg);
			this->ProcSkillTrigger(pbMsg);
			break;
		}


		}
	}

	return nullptr;
}





void GameRole::Fini()
{

	std::cout << std::endl;
	std::cout << "--------------------down------------------" << std::endl;
	std::cout << "&&&&&&&&&&&&&&&&&&&&down&&&&&&&&&&&&&&&&&&&&" << std::endl;
	std::cout << std::endl;
	//当前玩家下线的时候，这个函数被调用,在这里实现下线逻辑
	//获取所有的玩家
	//auto players = ZinxKernel::Zinx_GetAllRole();

	//获取玩家周围的所有玩家
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto &r : players)
	{
		if (r == this)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);
		//生成玩家下线消息(此处需要每一次重新创建玩家下线消息)
		auto pMsg = MakeLogoffSyncPid();

		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

	//从当前游戏世界中移除当前玩家
	mCurrentWorld->DelPlayer(this);
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



// 先序列化聊天信息,在发给所有其他在线玩家
GameMsg * GameRole::MakeTalkBroadcast(std::string _talkContent)
{
	auto pbMsg = new BroadCast;

	pbMsg->set_pid(this->mPlayerId);
	/*根据Tp不同，BroadCast消息会包含：
	  1 聊天内容（Content）
	  2 初始位置(P)
	  4 新位置P
	  */
	pbMsg->set_tp(1);
	pbMsg->set_username(this->mPlayerName);

	pbMsg->set_content(_talkContent);

	GameSingleTLV *tlv = new GameSingleTLV(GameSingleTLV::GAME_MSG_BROADCAST, pbMsg);

	GameMsg *retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(tlv);

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
// 服务器把此玩家的新位置先序列化,再发给其他所有在线玩家
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
// 这一步的作用并不是直接把玩家删除,具体怎么删除可能是客户端的实现
// 这里只是把要下线或离开当前格子的玩家的信息序列化,准备发送给其他玩家
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



// 获取周围玩家信息,放入数组中一次性发送,避免多次发送
GameMsg * GameRole::MakeSurPlays()
{
	auto pbMsg = new SyncPlayers;

	//获取周边所有的玩家
	auto players = mCurrentWorld->GetSurPlayers(this);


	for (auto r : players)
	{
		GameRole *role = dynamic_cast<GameRole*>(r);

		auto p = pbMsg->add_ps();

		p->set_pid(role->mPlayerId);
		p->set_username(role->mPlayerName);

		// 获得子对象pos
		auto pos = p->mutable_p();
		// 设置子对象
		pos->set_x(role->x);
		pos->set_y(role->y);
		pos->set_z(role->z);
		pos->set_v(role->v);
		pos->set_bloodvalue(role->hp);
	}

	GameSingleTLV *tlv = new GameSingleTLV(GameSingleTLV::GAME_MSG_SUR_PLAYER, pbMsg);

	GameMsg *retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(tlv);

	return retMsg;
}



// 为客户端准备切换世界后的姓名/位置/HP,然后序列化以备发送给客户端
GameMsg * GameRole::MakeChangeWorldResponse(int srcId, int targetId)
{

	auto pbMsg = new ChangeWorldResponse;

	pbMsg->set_pid(this->mPlayerId);
	//切换成功返回1
	pbMsg->set_changeres(1);
	pbMsg->set_srcid(srcId);
	pbMsg->set_targetid(targetId);

	//玩家初始出生点
	auto pos = pbMsg->mutable_p();
	pos->set_bloodvalue(hp);
	pos->set_x(x);
	pos->set_y(y);
	pos->set_z(z);
	pos->set_v(v);

	auto single = new GameSingleTLV(GameSingleTLV::GAME_MSG_CHANGE_WORLD_RESPONSE, pbMsg);
	auto retMsg = new GameMsg;
	retMsg->m_MsgList.push_back(single);

	return retMsg;
}



// 序列化技能消息,以待返回给客户端
GameMsg * GameRole::MakeSkillTrigger(pb::SkillTrigger * trigger)
{
	auto *pbMsg = new SkillTrigger(*trigger);

	GameSingleTLV *tlv = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_BROAD, pbMsg);

	GameMsg *retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(tlv);

	return retMsg;
}



// 技能命中序列化,准备返回给客户端
GameMsg * GameRole::MakeSkillContact(pb::SkillContact * contact)
{
	auto *pbMsg = new SkillContact(*contact);

	GameSingleTLV *tlv = new GameSingleTLV(GameSingleTLV::GAME_MSG_SKILL_CONTACT_BROAD, pbMsg);

	GameMsg *retMsg = new GameMsg;

	retMsg->m_MsgList.push_back(tlv);

	return retMsg;
}



void GameRole::ProcNewPosition(float _x, float _y, float _z, float _v)
{
	//判断玩家移动的过程中是否跨格子
	if (mCurrentWorld->GridChanged(this, _x, _y))
	{
		//获取当前玩家周围玩家
		std::list<AOI_Player*> oldList = mCurrentWorld->GetSurPlayers(this);

		//从当前格子中移除当前玩家
		mCurrentWorld->DelPlayer(this);
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->v = _v;

		//添加玩家到格子中
		mCurrentWorld->AddPlayer(this);

		//获取当前玩家周围的玩家
		std::list<AOI_Player*> newList = mCurrentWorld->GetSurPlayers(this);

		//视野消失
		this->ViewDisappear(oldList, newList);

		//视野出现
		this->ViewAppear(oldList, newList);
	}

	//当玩家移动的时候会收到移动消息
	this->x = _x;
	this->y = _y;
	this->z = _z;
	this->v = _v;

	//获取所有的玩家
	//auto players = ZinxKernel::Zinx_GetAllRole();
	//获取周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto &r : players)
	{
		if (r == this)
			continue;

		auto role = dynamic_cast<GameRole*>(r);
		//生成玩家下线消息
		auto pMsg = MakeNewPosBroadcast();

		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

}

void GameRole::ProcTalkContent(std::string content)
{
	//服务端收到客户端聊天的内容之后，应该广播给所有其它玩家
	auto players = ZinxKernel::Zinx_GetAllRole();

	for (auto r : players)
	{
	   /*  如果加上这句,自己就无法看到自己的聊天消息了
		if (r == this)
		{
			continue;
		}
		*/
		GameRole *role = dynamic_cast<GameRole*>(r);

		auto pMsg = MakeTalkBroadcast(content);

		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}
}



// 把服务器准备好的新世界消息发给客户端 
// 先发给服务器自己的GameProtocol层,再通过GameChannel发送给客户端
// srcId和targetWorldId 是客户端传来的原世界Id和新世界Id
void GameRole::ProcChangeWorld(int srcId, int targetWorldId)
{
	//当前AOI世界要进行切换
	//下线当前AOI世界
	mCurrentWorld->DelPlayer(this);

	//告诉周边玩家我下线了
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (this == r)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);
		auto pMsg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}


	//产生一个随机出生点位置
	if (1 == targetWorldId)
	{
		x = 100 + (g_rand_engine() % 20);
		y = 0;
		z = 100 + (g_rand_engine() % 20);
		v = 0;
		hp = 1000;
	}

	if (2 == targetWorldId)
	{
		//战斗场景的区间x[0, 140] z[0, 140]
		//最后控制出生点在10-130之间即可
		x = 10 + g_rand_engine() % 120;
		y = 0;
		z = 10 + g_rand_engine() % 120;
	}

	//上线新的AOI世界
	mCurrentWorld = WorldManager::GetInstance().GetWorld(targetWorldId);
	mCurrentWorld->AddPlayer(this);
	players = mCurrentWorld->GetSurPlayers(this);
	for (auto &r : players)
	{
		if (this == r)
		{
			continue;
		}

		auto role = dynamic_cast<GameRole*>(r);
		auto pMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}

	// 服务器告知当前玩家在新世界的位置等信息
	auto pMsg = MakeChangeWorldResponse(srcId, targetWorldId);
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);

	//告诉自己新的场景周边有什么玩家
	pMsg = MakeSurPlays();
	ZinxKernel::Zinx_SendOut(*pMsg, *mProtocol);
}



// 把技能消息发给其他玩家
void GameRole::ProcSkillTrigger(pb::SkillTrigger * trigger)
{
	if (trigger->pid() != this->mPlayerId)
	{
		return;
	}

	//获取周围玩家
	auto players = mCurrentWorld->GetSurPlayers(this);
	for (auto r : players)
	{
		GameRole *role = dynamic_cast<GameRole*>(r);

		if (role == this)
		{
			continue;
		}

		auto pMsg = MakeSkillTrigger(trigger);
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}
}



// 技能命中后的处理
void GameRole::ProcSkillContact(pb::SkillContact * contact)
{
	int targetId = contact->targetpid();

	//cout << "srcId: " << contact->srcpid() << " targetId: " << contact->targetpid() << endl;

	if (this->mPlayerId != contact->srcpid())
	{
		return;
	}



	GameRole *targetRole = nullptr;
	auto players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);
		if (role->mPlayerId == targetId)
		{
			targetRole = role;
			cout << "击中玩家 " << role->mPlayerId << endl;
			break;
		}
	}

	if (nullptr == targetRole)
	{
		return;
	}

	//计算受伤害的值
	int attackHp = 300 +  g_rand_engine() % 300;
	targetRole->hp -= attackHp;

	//auto tmpContactMsg = new SkillContact(*contact);

	//继续广播给所有的玩家
	auto pos = contact->mutable_contactpos();
	pos->set_bloodvalue(targetRole->hp);

	cout << "血量: " << pos->bloodvalue() << endl;

	players = mCurrentWorld->GetSurPlayers(this);

	for (auto r : players)
	{
		auto role = dynamic_cast<GameRole*>(r);

		auto pMsg = MakeSkillContact(contact);
		//发送消息给所有的玩家
		ZinxKernel::Zinx_SendOut(*pMsg, *role->mProtocol);
	}


	if (targetRole->hp <= 0)
	{
		targetRole->ProcChangeWorld(targetRole->mCurrentWorld->mWorldId, 1);
	}
}


int GameRole::GetX()
{
	return x;
}

//三维坐标中的Z相当于二维中Y
int GameRole::GetY()
{
	return z;
}



// 视野消失
void GameRole::ViewDisappear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	//思路 旧的周围玩家集合 - 新的周围玩家集合 = A
	vector<AOI_Player*> diff;

	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	//排序
	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//计算差集 oldVec - newVec
	set_difference(oldVec.begin(), oldVec.end(), newVec.begin(), newVec.end(), inserter(diff, diff.begin()));

	for (auto &it : diff)
	{
		GameRole *role = dynamic_cast<GameRole*>(it);

		auto pbMsg = MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*pbMsg, *role->mProtocol);

		pbMsg = role->MakeLogoffSyncPid();
		ZinxKernel::Zinx_SendOut(*pbMsg, *mProtocol);
	}
}


// 视野出现
void GameRole::ViewAppear(std::list<AOI_Player*>& oldList, std::list<AOI_Player*>& newList)
{
	//思路 新的周围玩家集合 - 旧的周围玩家集合 = A
	vector<AOI_Player*> diff;

	vector<AOI_Player*> oldVec(oldList.begin(), oldList.end());
	vector<AOI_Player*> newVec(newList.begin(), newList.end());

	//排序
	sort(oldVec.begin(), oldVec.end());
	sort(newVec.begin(), newVec.end());

	//计算差集 newVec - oldVec
	set_difference(newVec.begin(), newVec.end(), oldVec.begin(), oldVec.end(), inserter(diff, diff.begin()));

	for (auto &it : diff)
	{
		GameRole *role = dynamic_cast<GameRole*>(it);

		auto pbMsg = MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pbMsg, *role->mProtocol);

		pbMsg = role->MakeInitPosBroadcast();
		ZinxKernel::Zinx_SendOut(*pbMsg, *mProtocol);
	}
}
