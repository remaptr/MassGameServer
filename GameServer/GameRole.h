#pragma once
#include"GameMsg.h"
#include"GameProtocol.h"
#include<random>


class GameChannel;
class GameProtocol;


class GameRole : public Irole
{
private:
	static int smRoleCount;

public:
	GameRole();
	virtual ~GameRole();

	// 通过 Irole 继承
	virtual bool Init() override;
	virtual UserData * ProcMsg(UserData & _poUserData) override;
	virtual void Fini() override;


public:
	/*创建上线时的id和姓名消息*/
	GameMsg *MakeLogonSyncPid();
	
	/*创建广播聊天消息*/
	GameMsg *MakeTalkBroadcast(std::string _talkContent);
	
	/*创建广播出生位置消息*/
	GameMsg *MakeInitPosBroadcast();
	
	/*创建广播移动后新位置消息*/
	GameMsg *MakeNewPosBroadcast();
	
	/*创建下线时的id和姓名消息*/
	GameMsg *MakeLogoffSyncPid();
	
	/*创建周围玩家位置消息*/
	GameMsg *MakeSurPlays();
	
	/*进入场景确认的消息*/
	GameMsg *MakeChangeWorldResponse(int srcId, int targetId);
	
	/*技能触发消息*/
	GameMsg *MakeSkillTrigger(pb::SkillTrigger *trigger);
	
	/*技能碰撞消息*/
	GameMsg *MakeSkillContact(pb::SkillContact *contact);


public:
void ProcNewPosition(float _x, float _y, float _z, float _v);
void ProcTalkContent(std::string content);
void ProcChangeWorld(int srcId, int targetWorldId);
void ProcSkillTrigger(pb::SkillTrigger *trigger);
void ProcSkillContact(pb::SkillContact *contact);


public:
	GameProtocol *mProtocol=nullptr;
	GameChannel *mChannel=nullptr;

    // 玩家ID
	int mPlayerId;           

	// 玩家姓名
	std::string mPlayerName;

	// 玩家坐标
	float x;
	float y;
	float z;
	// 玩家面向的方向
	float v;

	// 玩家血量
	int hp;
};

