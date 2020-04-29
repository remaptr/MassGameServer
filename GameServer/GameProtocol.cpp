#include "GameProtocol.h"
using namespace std;

GameProtocol::GameProtocol()
{
}

GameProtocol::~GameProtocol()
{

}




// 解决粘包 丢包问题, 反序列化并储存到容器中
UserData * GameProtocol::raw2request(std::string _szInput)
{
	int msgLen = 0;
	int msgId = 0;
	string msgContent;

	mLastBuf.append(_szInput);

	GameMsg *retMsg = nullptr;

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

			// 在这里反序列化的目的是啥呢? 是为了在角色层直接拿到解析后的数据.
			// 在这里拿到客户端的消息内容,并储存到反序列化protobuf对象中
	 	GameSingleTLV* tlv = new GameSingleTLV((GameSingleTLV::GameMsgType)msgId, msgContent);
	    
		if (nullptr == retMsg)
		{
			retMsg = new GameMsg;
	    }

		retMsg->m_MsgList.push_back(tlv);
		}
		
		else
		{
			/// 如果丢包了,则前半截先放在mLastBuf中存储,
			/// 等下次发来消息时即可完成拼凑,解析数据
			cout << "丢包" << endl;
			break;
		}
	}

	return retMsg;
}


// 这是服务器把消息做完处理以后准备把数据返回给客户端
// 接收来自角色层的消息,序列化数据,准备返回给客户端
// 实际上,在客户端,也会有相同的操作.这里可以解答一部分你关于客户端的疑问.
std::string * GameProtocol::response2raw(UserData & _oUserData)
{
	GameMsg &msg = dynamic_cast<GameMsg&>(_oUserData);


	string *retBuf = new string;

	for (auto &single : msg.m_MsgList)
	{
		// protobufString是存储序列化后得到的字符串
		auto protobufString=single->Serialize();

		// 果然,这里和我猜测的一样,至于数据的长度,是由序列化后的数据决定的
		int msgLen = protobufString.size();
        // 消息类型
		int msgId = single->m_MsgType;
        
		retBuf->push_back((char)(msgLen & 0xff));
		retBuf->push_back((char)(msgLen >> 8  & 0xff));
		retBuf->push_back((char)(msgLen >> 16 & 0xff));
		retBuf->push_back((char)(msgLen >> 24 & 0xff));

		retBuf->push_back((char)(msgId & 0xff));
		retBuf->push_back((char)(msgId >> 8  & 0xff));
		retBuf->push_back((char)(msgId >> 16 & 0xff));
		retBuf->push_back((char)(msgId >> 24 & 0xff));

		retBuf->append(protobufString);

	   	// return retBuf;
	}
	// 要把所有消息一次性发出?
	return retBuf;
}

Irole* GameProtocol::GetMsgProcessor(UserDataMsg & _oUserDataMsg)
{
	return mRole;
}

Ichannel* GameProtocol::GetMsgSender(BytesMsg & _oBytes)
{
	return mChannel;
}
