#include "GameMsg.h"


GameMsg::GameMsg()
{
}

GameMsg::~GameMsg()
{
	for (auto it : m_MsgList)
	{
		if (it != nullptr)
		{
			delete it;
		}
	}

}


GameSingleTLV::GameSingleTLV(GameMsgType _Type, google::protobuf::Message * _poGameMsg)
{
	// 在这里把消息类型赋值
	m_MsgType = _Type;
	m_poGameMsg = _poGameMsg;
}



// 创建对象的同时直接反序列化
GameSingleTLV::GameSingleTLV(GameMsgType _Type, std::string _szInputData)
{
	//需要反序列化的构造函数 _type是消息ID,_szInputData是protobuf的序列化二进制数据

	m_MsgType = _Type;

	switch (_Type)
	{
		//同步玩家ID和名字
	case GAME_MSG_LOGON_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

	case GAME_MSG_TALK_CONTENT:
		m_poGameMsg = new Talk;
		break;

	case GAME_MSG_NEW_POSTION:
		m_poGameMsg = new Position;
		break;

	case GAME_MSG_SKILL_TRIGGER:
		m_poGameMsg = new SkillTrigger;
		break;

	case GAME_MSG_SKILL_CONTACT:
		m_poGameMsg = new SkillContact;
		break;

	case GAME_MSG_CHANGE_WORLD:
		m_poGameMsg = new ChangeWorldRequest;
		break;

	case GAME_MSG_BROADCAST:
		m_poGameMsg = new BroadCast;
		break;

	case GAME_MSG_LOGOFF_SYNCPID:
		m_poGameMsg = new SyncPid;
		break;

	case GAME_MSG_SUR_PLAYER:
		m_poGameMsg = new SyncPlayers;
		break;

	case GAME_MSG_SKILL_BROAD:
		m_poGameMsg = new SkillTrigger;
		break;

	case GAME_MSG_SKILL_CONTACT_BROAD:
		m_poGameMsg = new SkillContact;
		break;

	case GAME_MSG_CHANGE_WORLD_RESPONSE:
		m_poGameMsg = new ChangeWorldResponse;
		break;

	default:
		break;
	}

	//反序列化
	m_poGameMsg->ParseFromString(_szInputData);
}



GameSingleTLV::~GameSingleTLV()
{
	if (m_poGameMsg != nullptr)
	{
		delete m_poGameMsg;
		m_poGameMsg = nullptr;
	}
}



std::string GameSingleTLV::Serialize()
{
	if (m_poGameMsg != nullptr)
	{
		std::string str;
		m_poGameMsg->SerializeToString(&str);
		return str;
	}

	return std::string();
}

