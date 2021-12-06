//系统头文件放上边
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

//自定义头文件放下边,因为g++中用了-I参数，所以这里用<>也可以
#include "ngx_func.h"     //函数声明
#include "ngx_c_conf.h"   //和配置文件处理相关的类,名字带c_表示和类有关

//静态成员赋值
CConfig *CConfig::m_instance = NULL;

//构造函数
CConfig::CConfig(){}

//析构函数
CConfig::~CConfig()
{
	std::vector<LPCConfItem>::iterator pos;
	for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
	{
		delete (*pos);
	}//end for
	m_ConfigItemList.clear();
	return;
}

//装载配置文件
bool CConfig::Load(const char *pconfName)
{
	FILE *fp;
	fp = fopen(pconfName, "r");
	if (fp == NULL)
		return false;

	//每一行配置文件缓存区
	char  linebuf[501];  //每行配置都不要太长，保持<500字符内，防止出现问题

	//走到这里，文件打开成功 
	while (!feof(fp))  //检查文件是否结束 ，没有结束则条件成立
	{
		/*char *fgets(char *str, int n, FILE *stream) 从指定的流 stream 读取一行，并把它存储在 str 所指向的字符串内。
		当读取(n - 1) 个字符时，或者读取到换行符时，或者到达文件末尾时，它会停止，具体视情况而定。*/
		if (fgets(linebuf, 500, fp) == NULL) //从文件中读数据，每次读一行，一行最多不要超过500个字符
			continue;

		if (linebuf[0] == 0)
			continue;

		//处理注释行
		if (*linebuf == ';' || *linebuf == ' ' || *linebuf == '#' || *linebuf == '\t' || *linebuf == '\n')
			continue;

	lblprocstring:
		//屁股后边若有换行，回车，空格等都截取掉
		if (strlen(linebuf) > 0)
		{
			if (linebuf[strlen(linebuf) - 1] == 10 || linebuf[strlen(linebuf) - 1] == 13 || linebuf[strlen(linebuf) - 1] == 32)
			{
				linebuf[strlen(linebuf) - 1] = 0;
				goto lblprocstring;
			}
		}
		if (linebuf[0] == 0)
			continue;
		if (*linebuf == '[') //[开头的也不处理
			continue;

		//例如“ListenPort = 5678”才能走下来；
		char *ptmp = strchr(linebuf, '=');

		if (ptmp != NULL)
		{				
			LPCConfItem p_confitem = new CConfItem;

			/*结构定义,用来读取指定行有效配置信息
			typedef struct _CConfItem
			{
				char ItemName[50];     配置的名称
				char ItemContent[500]; 配置信息数据
			}CConfItem, *LPCConfItem;*/

			memset(p_confitem, 0, sizeof(CConfItem));
			strncpy(p_confitem->ItemName, linebuf, (int)(ptmp - linebuf)); //等号左侧的拷贝到p_confitem->ItemName
			strcpy(p_confitem->ItemContent, ptmp + 1);                     //等号右侧的拷贝到p_confitem->ItemContent

			//去掉有效配置信息空格符
			Rtrim(p_confitem->ItemName);
			Ltrim(p_confitem->ItemName);
			Rtrim(p_confitem->ItemContent);
			Ltrim(p_confitem->ItemContent);

			//printf("itemname=%s | itemcontent=%s\n",p_confitem->ItemName,p_confitem->ItemContent);    
			
			//有效配置信息装入vector容器(记得内存要释放，因为这里是new出来的 )
			m_ConfigItemList.push_back(p_confitem);

		} //end if
	} //end while(!feof(fp)) 循环以载入所有配置信息于vector中

	fclose(fp); //关闭配置文件
	return true;
}

//根据ItemName获取配置信息字符串，不修改不用互斥
const char *CConfig::GetString(const char *p_itemname)
{
	std::vector<LPCConfItem>::iterator pos;
	for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
	{
		if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
			return (*pos)->ItemContent;
	}//end for
	return NULL;
}
//根据ItemName获取数字类型配置信息(只读信息,不需要互斥)
//Linux: strcasecmp不区分大小写比较字符串； Windows: stricmp区分大小写比较字符串
int CConfig::GetIntDefault(const char *p_itemname, const int def)
{
	std::vector<LPCConfItem>::iterator pos;
	for (pos = m_ConfigItemList.begin(); pos != m_ConfigItemList.end(); ++pos)
	{
		if (strcasecmp((*pos)->ItemName, p_itemname) == 0)
			return atoi((*pos)->ItemContent); //返回配置信息
	}//end for
	return def; //读取配置信息失败,返回默认信息
}
