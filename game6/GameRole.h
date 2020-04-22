#pragma once
#include "zinx.h"

class GameChannel;
class GameProtocol;


class GameRole : public Irole
{
public:
	GameRole();
	virtual ~GameRole();

	// Í¨¹ý Irole ¼Ì³Ð
	virtual bool Init() override;
	virtual UserData * ProcMsg(UserData & _poUserData) override;
	virtual void Fini() override;

public:
	GameProtocol *mProtocol;
	GameChannel *mChannel;
};

