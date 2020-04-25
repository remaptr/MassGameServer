#include "StdoutChannel.h"

bool StdoutChannel::Init()
{
	return true;
}

bool StdoutChannel::ReadFd(std::string & _input)
{
	return false;
}

bool StdoutChannel::WriteFd(std::string & _output)
{
	std::cout << "StdoutChannel::WriteFd----:" << _output << std::endl;
	return true;
}

void StdoutChannel::Fini()
{
}

int StdoutChannel::GetFd()
{
	return STDOUT_FILENO;
}

std::string StdoutChannel::GetChannelInfo()
{
	return std::string("StdoutChannel");
}

AZinxHandler * StdoutChannel::GetInputNextStage(BytesMsg & _oInput)
{
	return nullptr;
}
