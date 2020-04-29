#pragma once
#include<deque>
#include<string>
#include<fstream>
#include<iostream>
#include<algorithm>

using namespace std;

class RandomName
{
public:
	static RandomName &GetInstance();
	RandomName();
	virtual ~RandomName();

	void LoadFile();
	std::string GetName();
	void ReleaseName(std::string szName);

private:
	static RandomName smInstance;
	std::deque<std::string> m_Names;
};

