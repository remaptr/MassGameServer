#include "RandomName.h"

#define RANDOM_FIRST "random_first.txt"
#define RANDOM_LAST "random_last.txt"


RandomName RandomName::smInstance;

//获取一个实例
RandomName & RandomName::GetInstance()
{
	// TODO: 在此处插入 return 语句
	return smInstance;
}


//构造函数
RandomName::RandomName()
{
	LoadFile();
}

//析构函数
RandomName::~RandomName()
{
}


// 加载连个文本文件
void RandomName::LoadFile()
{
	ifstream iFirst;
	ifstream iLast;

	//打开文件
	iFirst.open(RANDOM_FIRST);
	iLast.open(RANDOM_LAST);

	if (iFirst.is_open() && iLast.is_open())
	{
		string firstName;

		while (getline(iFirst, firstName))
		{
			string lastName;
			while (getline(iLast, lastName))
			{
				string finalName;

				finalName = firstName + " " + lastName;
				m_Names.push_back(finalName);
			}

			//文件指针指向文件的开头
			//重新定位文件流指针
			iLast.clear(ios::goodbit);
			iLast.seekg(ios::beg);
		}

		//设置随机种子
		srandom(time(nullptr));

		//乱序排序
		random_shuffle(m_Names.begin(), m_Names.end());

		iFirst.close();
		iLast.close();
	}
	else
	{
		cerr << "打开文件失败" << endl;
	}
}


//获取一个名字
std::string RandomName::GetName()
{
	string name;

	//从队列中获取一个名字
	name = m_Names.front();

	//从队列中移除该名字
	m_Names.pop_front();

	return name;
}

//归还名字
void RandomName::ReleaseName(std::string szName)
{
	m_Names.push_back(szName);
}
