#include "GameProtocol.h"
using namespace std;

GameProtocol::GameProtocol()
{
}

GameProtocol::~GameProtocol()
{

}



UserData * GameProtocol::raw2request(std::string _szInput)
{
	int msgLen = 0;
	int msgId = 0;
	string msgContent;

	mLastBuf.append(_szInput);

	/// 如果数据小于8字节,说明包头不全,前半截放在mLastBuf中存放,
	/// 待下次数据来,完成拼凑.
	while (mLastBuf.size() >= 8)
	{
		msgLen = mLastBuf[0] |
			mLastBuf[1] << 8 |
			mLastBuf[2] << 16 |
			mLastBuf[3] << 24;

		msgId = mLastBuf[4] |
			mLastBuf[5] << 8 |
			mLastBuf[6] << 16 |
			mLastBuf[7] << 24;

		/// 说明是完整的包或粘包
		if (mLastBuf.size() >= msgLen + 8)
		{
			/// 拿到消息内容 8是下标索引,msgLen是字符个数
			msgContent = mLastBuf.substr(8, msgLen);
            /// 清除mLastBuf,清除后,再次进入while循环,判断mLastBuf中的数据是否处理完毕
			mLastBuf.erase(0, 8 + msgLen);

			cout << "msgLen::" << msgLen << " " << "msgId::" << msgId << " " << "msgContent::" << msgContent << endl;
		}
		
		else
		{
			/// 如果丢包了,则前半截先放在mLastBuf中存储,
			/// 等下次发来消息时即可完成拼凑,解析数据
			cout << "丢包" << endl;
			break;
		}
	}

	return nullptr;
}

std::string * GameProtocol::response2raw(UserData & _oUserData)
{
	return nullptr;
}

Irole* GameProtocol::GetMsgProcessor(UserDataMsg & _oUserDataMsg)
{
	return mRole;
}

Ichannel* GameProtocol::GetMsgSender(BytesMsg & _oBytes)
{
	return mChannel;
}
