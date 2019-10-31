///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����DiskOperation.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-18
// ���������������ļ�������
///////////////////////////////////////////////////////////////////////////////////////////
#include "DiskOperation.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation/FileOperater.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/OSFunction.h"
#include "../Foundation/AppLog.h"
#include "../Communications/SysMsgSender.h"


#if defined(WIN32) || defined(__WIN32__)
#include <windows.h>
#else
#include <sys/vfs.h> 
#endif
#include <time.h>  

DiskOperation::DiskOperation()
{
	Init();
}

DiskOperation::~DiskOperation()
{

}

void DiskOperation::Init()
{
	mPreTime = time(0);
	
	mDirNameVec.clear();//gxd 20110331 ���¼��洢·��
	PROPMANAGER::instance()->GetRecFileLocVec(mDirNameVec);
	mLowestSpace=StrUtil::Str2Int(PROPMANAGER::instance()->GetMaxAvailableSize());//���¼���ļ����ڴ��̵���С����ֵ
}

long DiskOperation::CalculateFreeSpace(const std::string& dirName)
{
#if defined(WIN32) || defined(__WIN32__)
	unsigned __int64 FreeBytesAvailableToCaller = 0;
	std::string dPath = dirName.substr(0,dirName.find_first_of(":/",0));//¼���ļ��洢���̷�
	dPath.append(":\\");
	::GetDiskFreeSpaceEx(dPath.c_str(),
		(ULARGE_INTEGER *)&FreeBytesAvailableToCaller,NULL,NULL);//��ô��̿��ÿռ�

	return (long)(FreeBytesAvailableToCaller/(1024*1024*1024));
#else
	long long FreeBytesAvailableToCaller = 0;
	struct statfs buf;
	memset(&buf,0,sizeof(buf));
	if (-1 != statfs(dirName.c_str(), &buf))
		FreeBytesAvailableToCaller = (long long)(buf.f_bsize * buf.f_bavail)/(1024*1024);
	return FreeBytesAvailableToCaller;
#endif
}

bool DiskOperation::DeleteOldestFileInPath(unsigned short delcount, const std::string& recPath)
{
	bool bRet = false;

	std::vector<std::string> vecfilename;
	DBMANAGER::instance()->QueryRecordTopInPath(delcount, recPath, vecfilename);

	std::vector<std::string>::iterator iter=vecfilename.begin();
	for (;iter!=vecfilename.end(); ++iter)
	{
		if(DeleteRecFile(*iter))							//ɾ�������ļ�
			DBMANAGER::instance()->DeleteRecord(*iter);			//ɾ�����ݿ�

		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ɾ��[%s]!\n",(*iter).c_str()));
		OSFunction::Sleep(0,10);
	}
	bRet = true;
	return bRet;
}


bool DiskOperation::DeleteOldestFileByDays(unsigned short delnum)
{
	bool bRet = false;	

	std::vector<std::string> filenames;
	DBMANAGER::instance()->QueryRecordTop(delnum,filenames);

	for (std::vector<std::string>::iterator iter=filenames.begin();iter!=filenames.end(); ++iter)
	{
		if(DeleteRecFile(*iter))								//ɾ�������ļ�
			DBMANAGER::instance()->DeleteRecord(*iter);			//ɾ�����ݿ�

		ACE_DEBUG ((LM_DEBUG,"(%T | %t)ɾ��[%s]!\n",(*iter).c_str()));
		OSFunction::Sleep(0,10);
	}
	bRet = true;
	return bRet;
}

bool DiskOperation::CheckDiskIsFullForRegular(int nTimeValue)
{
	bool bRet = false;

	if (nTimeValue > 0)//���ʱ����
	{
		time_t currentTime = time(0);
		if ((currentTime - mPreTime) >= nTimeValue)
		{
			mPreTime = currentTime;								//�����ϴμ��ʱ��
		}
		else
		{
			return bRet;
		}
	}

	std::string storetype = PROPMANAGER::instance()->GetStoreType();	//��ȡ¼��洢��ʽ����ɢ ���� ˳��)
	
	if (storetype == "0" || storetype=="")	//˳��洢����δ���壬����ȫ��ʱ��ɾ���߼�
	{
		while(1)
		{
			std::vector<std::string>::iterator diriter=mDirNameVec.begin();
			for (;diriter!=mDirNameVec.end(); ++diriter)
			{
				long dirSpace = CalculateFreeSpace(*diriter);		//���ÿ��·����ʣ��ռ�
				if(dirSpace > mLowestSpace)							//ĳ���̷��ռ����Ҫ��������ɾ��
				{
					break;
				}
				OSFunction::Sleep(0,10);
			}
			if (diriter == mDirNameVec.end())						//������д��̿ռ䶼������Ҫ��
			{
				DeleteOldestFileByDays(100);						//��ʱ���ʱɾ��100����ʷ��¼
				continue;
			}
			break;													//����κ�һ�����̿ռ����Ҫ�����˳����μ��
		}
	}
	else if (storetype == "1")				//��ɢ�洢��ÿ������ռ�Ĵ��̶���Ҫɾ��
	{
		std::vector<std::string>::iterator diriter = mDirNameVec.begin();
		for (;diriter!=mDirNameVec.end(); ++diriter)
		{
			long dirSpace = CalculateFreeSpace(*diriter);		//���ÿ��·����ʣ��ռ�
			if(dirSpace <= mLowestSpace)						//ĳ���̷��ռ䲻��
			{
				DeleteOldestFileInPath(100,*diriter);			//��ʱ���ʱɾ��100����ʷ��¼
				diriter = mDirNameVec.begin();					//���¼��
				OSFunction::Sleep(0,10);
				continue;		
			}
			OSFunction::Sleep(0,10);
		}									
	}
	return true;
}

bool DiskOperation::CheckDiskIsFullForRecord(std::string& validpath,int deviceid)
{
	int CheckTimes = 50;	//������
	if (deviceid == -1)		//˳��
	{
		while(1)
		{
			std::vector<std::string>::iterator diriter=mDirNameVec.begin();
			for (;diriter!=mDirNameVec.end(); ++diriter)
			{
				long dirSpace = CalculateFreeSpace(*diriter);		//���ÿ��·����ʣ��ռ�
				if(dirSpace > mLowestSpace)							//ĳ���̷��ռ����Ҫ��
				{
					validpath = *diriter;
					break;
				}
				OSFunction::Sleep(0,5);
			}
			if (diriter == mDirNameVec.end() && --CheckTimes)		//������д��̿ռ䶼������Ҫ��
			{
				DeleteOldestFileByDays(50);							//¼�Ƽ��ʱɾ��50����ʷ��¼
				OSFunction::Sleep(0,10);
				continue;
			}
			break;													//����κ�һ�����̿ռ����Ҫ�����˳����μ��
		}
	}
	else		//��ɢ
	{
		std::string recPath = PROPMANAGER::instance()->GetRecPathByDevId(deviceid);		//ͨ��ͨ���Ż�ȡ�洢·��
		if ( recPath.empty() )
			return false;

		while (1)
		{
			long lDirSpace = CalculateFreeSpace(recPath);		//��ȡ���пռ�
			if (lDirSpace > mLowestSpace)
			{
				validpath = recPath;
				break;
			}
			else if( --CheckTimes)
			{
				DeleteOldestFileInPath(50, recPath);		//ɾ��recpath��50����ʷ��¼
				OSFunction::Sleep(0,10);
				continue;
			}
			break;
		}	
	}
	
	return true;	//���ú������ж�validpath�Ƿ�Ϊ��
}

bool DiskOperation::ProcessInprireFiles()
{
	bool bRet = false;
	std::vector<std::string> filenames;
	cout<<"��ʼɾ��Ƶ������.........."<<endl;
	DBMANAGER::instance()->DeleteSpecDataExpire();		//ɾ������Ƶ��
	cout<<"����ɾ��Ƶ������!"<<endl;
	cout<<"��ʼɾ��ָ������.........."<<endl;
	DBMANAGER::instance()->DeleteQualityDataExpire();		//ɾ������ָ��
	cout<<"����ɾ��ָ������!"<<endl;
	DBMANAGER::instance()->QueryRecordExpire(filenames);		//��ѯ����¼��¼���ļ�
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ʼɾ��¼��¼���ļ�,��[%d]��!\n",(int)filenames.size()));

	for (std::vector<std::string>::iterator iter=filenames.begin(); iter!=filenames.end(); ++iter)
	{
		DeleteRecFile(*iter);									//ɾ������¼��¼���ļ�
		DBMANAGER::instance()->DeleteRecord(*iter);				//ɾ�����ݿ��е�¼��¼���¼
		OSFunction::Sleep(0,500);
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����ɾ��¼��¼���ļ�,��[%d]��!\n",(int)filenames.size()));

	filenames.clear();

	DBMANAGER::instance()->QueryTempFileExpire(filenames);		//��ѯ������ʱ�ļ�
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ʼɾ����ʱ�ļ�,��[%d]��!\n",(int)filenames.size()));

	for (std::vector<std::string>::iterator iter=filenames.begin(); iter!=filenames.end(); ++iter)
	{
		DeleteTempFile(*iter);									//ɾ��������ʱ�ļ�
		DBMANAGER::instance()->DeleteTempFile(*iter);			//ɾ�����ݿ��е���ʱ�ļ���¼
		OSFunction::Sleep(0,500);
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����ɾ����ʱ�ļ�,��[%d]��!\n",(int)filenames.size()));

	//ɾ��������־�ļ�
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ʼɾ��������־�ļ�!\n"));
	DeleteExpireLog();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)����ɾ��������־�ļ�!\n"));

	bRet = true;
	return bRet;
}

bool DiskOperation::DeleteRecFile( std::string& filename )
{
	ACE_Guard<ACE_Thread_Mutex> guard(mDelMutex);
	bool bRet = true;

#if defined(WIN32) || defined(__WIN32__)
	 if( (_access( filename.c_str(),F_OK )) == -1 )
		 return false;
#endif

	if (!filename.empty())
	{
#if defined(WIN32) || defined(__WIN32__)
		if (0== DeleteFile(filename.c_str()))
		{
			string msg = "ɾ��¼���ļ�����";
			SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);

			bRet=false;
		}
#else
		if (-1 == remove(filename.c_str()))
		{
			bRet=false;
		}
#endif
	}

	return bRet;
}


bool DiskOperation::DeleteTempFile(std::string& filename)
{
	ACE_Guard<ACE_Thread_Mutex> guard(mDelMutex);
	bool bRet = true;

#if defined(WIN32) || defined(__WIN32__)
	if( (_access( filename.c_str(),F_OK )) == -1 )
		return false;
#endif

	if (!filename.empty())
	{
#if defined(WIN32) || defined(__WIN32__)
		if (0 == DeleteFile(filename.c_str()))
		{
			bRet=false;
		}
#else
		if (-1== remove(filename.c_str()))
		{
			bRet=false;
		}
#endif
	}

	return bRet;
}

//ɾ��������־�ļ�
bool DiskOperation::DeleteExpireLog()			
{
	string logpath = PROPMANAGER::instance()->GetLogPath();		//c:/vscttb/log/
	logpath += "*.*";

	string logExpire = PROPMANAGER::instance()->GetLogExpire();
	//ͨ�����·���ȡ��ֵ
	int expire = (0 - StrUtil::Str2Int(logExpire) );

	//�����������
	string CurDateTime = TimeUtil::GetCurDateTime();
	string logExpireDateTime = TimeUtil::CalDay(CurDateTime, expire);
	string logExpireDate = TimeUtil::GetDateFromDatetime(logExpireDateTime);

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(logpath.c_str(), &FindFileData);
	while (hFind != INVALID_HANDLE_VALUE)
	{
		if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
			{
				//��ȡlog�Ĳ�ͬ���͵��ļ�������
				string logtype = string(FindFileData.cFileName);
				string templogpath = logpath;
				templogpath.erase(15);
				//�����ļ���·��
				string DirName = templogpath + logtype + string("/")+string("*.*");

				WIN32_FIND_DATA ChildFindFileData;
				HANDLE ChildhFind;

				//��ȡ������־�ļ�
				ChildhFind = FindFirstFile(DirName.c_str(), &ChildFindFileData);
				while(ChildhFind !=INVALID_HANDLE_VALUE)
				{
					if (strcmp(ChildFindFileData.cFileName, ".") != 0 && strcmp(ChildFindFileData.cFileName, "..") != 0)
					{
						//ʵ���ļ�������ʽΪ��xxxx-xx-xx.txt
						string FileNameTemp = string(ChildFindFileData.cFileName);

						//ȡ���ļ����е�������Ϣ�����ڱȽ�
						string fileDate = FileNameTemp.substr(0, 10);
						if (TimeUtil::DiffDate(logExpireDate, fileDate) > 0)	//�������
						{
							string tempDirName = DirName;
							tempDirName.erase( tempDirName.find_last_of("/")+1 );
							string fullFileName = tempDirName + FileNameTemp;	//deletefile���������������ʵ��·��

							int ret = DeleteFile( fullFileName.c_str() );	//ɾ��
							if ( !ret )
							{
								string msg = "ɾ����־�ļ�ʧ�ܣ��ļ���Ϊ" + FileNameTemp;
								SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
							}

							//д��־
							std::string msg = string("ɾ����־�ļ��ɹ����ļ���Ϊ��") + fullFileName;
							SYSMSGSENDER::instance()->SendMsg(msg);

						}
					}
					if (!FindNextFile(ChildhFind, &ChildFindFileData))
					{
						break;
					}
				}
				FindClose(ChildhFind);
			}
		}
		if (!FindNextFile(hFind, &FindFileData))
		{
			break;
		}
	}
	FindClose(hFind);

	return true;
}

