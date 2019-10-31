///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：XMLReceive.cpp
// 创建者：jiangcheng
// 创建时间：2009-05-20
// 内容描述：接收应用系统下达XML格式的命令
///////////////////////////////////////////////////////////////////////////////////////////

#pragma warning(disable:4996)
#include "SMSReceiver.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/FileOperater.h"
#include "../Foundation/XmlParser.h"
#include "SysMsgSender.h"

#include <fstream>
#include <iostream>

#if defined(WIN32) || defined(__WIN32__)
#include <windows.h> 
#include <conio.h>
#include <Sddl.h>
#endif


SMSReceiver::SMSReceiver()
{
	m_vecSmsFileName.clear();
}

SMSReceiver::~SMSReceiver()
{

}

int SMSReceiver::svc()
{
	std::string taskRtnXml;
	string dirs = "C:\\SMSUp\\";

#if defined(WIN32) || defined(__WIN32__)
	string strFindPath = dirs + "*.xml";//接收xml文件的目录
	while (bFlag)
	{
		HANDLE hFile;
		WIN32_FIND_DATA  wf;
		hFile = FindFirstFile(strFindPath.c_str(), &wf);//查找文件
		if (hFile == INVALID_HANDLE_VALUE)	//没有文件
		{
			OSFunction::Sleep(0, 100);
			continue;
		}
		
		do
		{
			OSFunction::Sleep( 2 );		//等待tomcat将所有内容写入文件

			std::string curFileName = dirs + wf.cFileName;
			DWORD dwAttrs = GetFileAttributes( curFileName.c_str() ); 
			if ((dwAttrs & FILE_ATTRIBUTE_READONLY))	//如果文件属性只读，改为正常
			{ 
				SetFileAttributes(curFileName.c_str(), FILE_ATTRIBUTE_NORMAL); 
			} 

			/* 获取任务类型和监测类型 */
			XmlParser fileParser;
			fileParser.LoadFromFile( curFileName.c_str() );
			pXMLNODE rootNode = fileParser.GetRootNode();
			std::string strDvbtype;
			fileParser.GetAttrNode(rootNode, "Type", strDvbtype);
			pXMLNODE returnNode = fileParser.GetNodeFromPath("Msg/Return");
			std::string taskType;
			fileParser.GetAttrNode(returnNode, "Type", taskType);

			/* 任务类型为空，就认为是无效的xml，直接删除 */
			if (taskType == "")		
			{
				DeleteFile( curFileName.c_str() );	//删除文件 
				continue;
			}
			
			/* 转存到sms文件夹 */
			std::string smsTempFileName = "C:/sms/SMS";
			smsTempFileName += (taskType+".xml");
			fileParser.SaveAsFile( smsTempFileName.c_str() );
			DeleteFile( curFileName.c_str() );//删除文件 

			/* 插入到容器中，等待任务获取 */
			insertFileName( smsTempFileName );

		}while( FindNextFile(hFile, &wf) );

		FindClose(hFile);
	}

#endif
	return 0;
}


void SMSReceiver::insertFileName(std::string strFileName)
{
	ACE_Guard<ACE_Thread_Mutex> guard( m_visitMutex );
	
	/* 暂时不保证文件的唯一性 */
	m_vecSmsFileName.push_back( strFileName );

	return;
}



bool SMSReceiver::getSmsFileName(std::string taskName, std::string& retFileName)
{
	ACE_Guard<ACE_Thread_Mutex> guard( m_visitMutex );

	bool ret = false;
	std::vector<std::string>::iterator visitIter = m_vecSmsFileName.begin();
	for (; visitIter<m_vecSmsFileName.end(); visitIter++)
	{
		if ( (*visitIter).find(taskName) != std::string::npos )
		{
			ret = true;
			retFileName = *visitIter;
			m_vecSmsFileName.erase( visitIter );	//删除
			break;
		}
	}

	return ret;
}