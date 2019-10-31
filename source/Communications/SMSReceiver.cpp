///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����XMLReceive.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-05-20
// ��������������Ӧ��ϵͳ�´�XML��ʽ������
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
	string strFindPath = dirs + "*.xml";//����xml�ļ���Ŀ¼
	while (bFlag)
	{
		HANDLE hFile;
		WIN32_FIND_DATA  wf;
		hFile = FindFirstFile(strFindPath.c_str(), &wf);//�����ļ�
		if (hFile == INVALID_HANDLE_VALUE)	//û���ļ�
		{
			OSFunction::Sleep(0, 100);
			continue;
		}
		
		do
		{
			OSFunction::Sleep( 2 );		//�ȴ�tomcat����������д���ļ�

			std::string curFileName = dirs + wf.cFileName;
			DWORD dwAttrs = GetFileAttributes( curFileName.c_str() ); 
			if ((dwAttrs & FILE_ATTRIBUTE_READONLY))	//����ļ�����ֻ������Ϊ����
			{ 
				SetFileAttributes(curFileName.c_str(), FILE_ATTRIBUTE_NORMAL); 
			} 

			/* ��ȡ�������ͺͼ������ */
			XmlParser fileParser;
			fileParser.LoadFromFile( curFileName.c_str() );
			pXMLNODE rootNode = fileParser.GetRootNode();
			std::string strDvbtype;
			fileParser.GetAttrNode(rootNode, "Type", strDvbtype);
			pXMLNODE returnNode = fileParser.GetNodeFromPath("Msg/Return");
			std::string taskType;
			fileParser.GetAttrNode(returnNode, "Type", taskType);

			/* ��������Ϊ�գ�����Ϊ����Ч��xml��ֱ��ɾ�� */
			if (taskType == "")		
			{
				DeleteFile( curFileName.c_str() );	//ɾ���ļ� 
				continue;
			}
			
			/* ת�浽sms�ļ��� */
			std::string smsTempFileName = "C:/sms/SMS";
			smsTempFileName += (taskType+".xml");
			fileParser.SaveAsFile( smsTempFileName.c_str() );
			DeleteFile( curFileName.c_str() );//ɾ���ļ� 

			/* ���뵽�����У��ȴ������ȡ */
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
	
	/* ��ʱ����֤�ļ���Ψһ�� */
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
			m_vecSmsFileName.erase( visitIter );	//ɾ��
			break;
		}
	}

	return ret;
}