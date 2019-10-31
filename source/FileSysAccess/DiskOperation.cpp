///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：DiskOperation.cpp
// 创建者：jiangcheng
// 创建时间：2009-06-18
// 内容描述：磁盘文件操作类
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
	
	mDirNameVec.clear();//gxd 20110331 多个录像存储路径
	PROPMANAGER::instance()->GetRecFileLocVec(mDirNameVec);
	mLowestSpace=StrUtil::Str2Int(PROPMANAGER::instance()->GetMaxAvailableSize());//获得录像文件所在磁盘的最小空闲值
}

long DiskOperation::CalculateFreeSpace(const std::string& dirName)
{
#if defined(WIN32) || defined(__WIN32__)
	unsigned __int64 FreeBytesAvailableToCaller = 0;
	std::string dPath = dirName.substr(0,dirName.find_first_of(":/",0));//录像文件存储的盘符
	dPath.append(":\\");
	::GetDiskFreeSpaceEx(dPath.c_str(),
		(ULARGE_INTEGER *)&FreeBytesAvailableToCaller,NULL,NULL);//获得磁盘可用空间

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
		if(DeleteRecFile(*iter))							//删除磁盘文件
			DBMANAGER::instance()->DeleteRecord(*iter);			//删除数据库

		ACE_DEBUG ((LM_DEBUG,"(%T | %t)删除[%s]!\n",(*iter).c_str()));
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
		if(DeleteRecFile(*iter))								//删除磁盘文件
			DBMANAGER::instance()->DeleteRecord(*iter);			//删除数据库

		ACE_DEBUG ((LM_DEBUG,"(%T | %t)删除[%s]!\n",(*iter).c_str()));
		OSFunction::Sleep(0,10);
	}
	bRet = true;
	return bRet;
}

bool DiskOperation::CheckDiskIsFullForRegular(int nTimeValue)
{
	bool bRet = false;

	if (nTimeValue > 0)//检测时间间隔
	{
		time_t currentTime = time(0);
		if ((currentTime - mPreTime) >= nTimeValue)
		{
			mPreTime = currentTime;								//更新上次检测时间
		}
		else
		{
			return bRet;
		}
	}

	std::string storetype = PROPMANAGER::instance()->GetStoreType();	//获取录像存储方式：分散 还是 顺序)
	
	if (storetype == "0" || storetype=="")	//顺序存储或者未定义，磁盘全满时做删除逻辑
	{
		while(1)
		{
			std::vector<std::string>::iterator diriter=mDirNameVec.begin();
			for (;diriter!=mDirNameVec.end(); ++diriter)
			{
				long dirSpace = CalculateFreeSpace(*diriter);		//检查每个路径的剩余空间
				if(dirSpace > mLowestSpace)							//某个盘符空间符合要求，则无需删除
				{
					break;
				}
				OSFunction::Sleep(0,10);
			}
			if (diriter == mDirNameVec.end())						//如果所有磁盘空间都不符合要求
			{
				DeleteOldestFileByDays(100);						//定时检查时删除100条历史记录
				continue;
			}
			break;													//如果任何一个磁盘空间符合要求则退出本次检查
		}
	}
	else if (storetype == "1")				//分散存储，每个不足空间的磁盘都需要删除
	{
		std::vector<std::string>::iterator diriter = mDirNameVec.begin();
		for (;diriter!=mDirNameVec.end(); ++diriter)
		{
			long dirSpace = CalculateFreeSpace(*diriter);		//检查每个路径的剩余空间
			if(dirSpace <= mLowestSpace)						//某个盘符空间不足
			{
				DeleteOldestFileInPath(100,*diriter);			//定时检查时删除100条历史记录
				diriter = mDirNameVec.begin();					//重新检查
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
	int CheckTimes = 50;	//检测次数
	if (deviceid == -1)		//顺序
	{
		while(1)
		{
			std::vector<std::string>::iterator diriter=mDirNameVec.begin();
			for (;diriter!=mDirNameVec.end(); ++diriter)
			{
				long dirSpace = CalculateFreeSpace(*diriter);		//检查每个路径的剩余空间
				if(dirSpace > mLowestSpace)							//某个盘符空间符合要求
				{
					validpath = *diriter;
					break;
				}
				OSFunction::Sleep(0,5);
			}
			if (diriter == mDirNameVec.end() && --CheckTimes)		//如果所有磁盘空间都不符合要求
			{
				DeleteOldestFileByDays(50);							//录制检查时删除50条历史记录
				OSFunction::Sleep(0,10);
				continue;
			}
			break;													//如果任何一个磁盘空间符合要求则退出本次检查
		}
	}
	else		//分散
	{
		std::string recPath = PROPMANAGER::instance()->GetRecPathByDevId(deviceid);		//通过通道号获取存储路径
		if ( recPath.empty() )
			return false;

		while (1)
		{
			long lDirSpace = CalculateFreeSpace(recPath);		//获取空闲空间
			if (lDirSpace > mLowestSpace)
			{
				validpath = recPath;
				break;
			}
			else if( --CheckTimes)
			{
				DeleteOldestFileInPath(50, recPath);		//删除recpath下50条历史记录
				OSFunction::Sleep(0,10);
				continue;
			}
			break;
		}	
	}
	
	return true;	//调用函数会判断validpath是否为空
}

bool DiskOperation::ProcessInprireFiles()
{
	bool bRet = false;
	std::vector<std::string> filenames;
	cout<<"开始删除频谱数据.........."<<endl;
	DBMANAGER::instance()->DeleteSpecDataExpire();		//删除过期频谱
	cout<<"结束删除频谱数据!"<<endl;
	cout<<"开始删除指标数据.........."<<endl;
	DBMANAGER::instance()->DeleteQualityDataExpire();		//删除过期指标
	cout<<"结束删除指标数据!"<<endl;
	DBMANAGER::instance()->QueryRecordExpire(filenames);		//查询过期录像录音文件
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)开始删除录音录像文件,共[%d]个!\n",(int)filenames.size()));

	for (std::vector<std::string>::iterator iter=filenames.begin(); iter!=filenames.end(); ++iter)
	{
		DeleteRecFile(*iter);									//删除过期录音录像文件
		DBMANAGER::instance()->DeleteRecord(*iter);				//删除数据库中的录音录像记录
		OSFunction::Sleep(0,500);
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)结束删除录音录像文件,共[%d]个!\n",(int)filenames.size()));

	filenames.clear();

	DBMANAGER::instance()->QueryTempFileExpire(filenames);		//查询过期临时文件
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)开始删除临时文件,共[%d]个!\n",(int)filenames.size()));

	for (std::vector<std::string>::iterator iter=filenames.begin(); iter!=filenames.end(); ++iter)
	{
		DeleteTempFile(*iter);									//删除过期临时文件
		DBMANAGER::instance()->DeleteTempFile(*iter);			//删除数据库中的临时文件记录
		OSFunction::Sleep(0,500);
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)结束删除临时文件,共[%d]个!\n",(int)filenames.size()));

	//删除过期日志文件
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)开始删除过期日志文件!\n"));
	DeleteExpireLog();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)结束删除过期日志文件!\n"));

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
			string msg = "删除录像文件出错";
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

//删除过期日志文件
bool DiskOperation::DeleteExpireLog()			
{
	string logpath = PROPMANAGER::instance()->GetLogPath();		//c:/vscttb/log/
	logpath += "*.*";

	string logExpire = PROPMANAGER::instance()->GetLogExpire();
	//通过以下方法取负值
	int expire = (0 - StrUtil::Str2Int(logExpire) );

	//计算过期日期
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
				//获取log的不同类型的文件夹名称
				string logtype = string(FindFileData.cFileName);
				string templogpath = logpath;
				templogpath.erase(15);
				//生成文件夹路径
				string DirName = templogpath + logtype + string("/")+string("*.*");

				WIN32_FIND_DATA ChildFindFileData;
				HANDLE ChildhFind;

				//获取具体日志文件
				ChildhFind = FindFirstFile(DirName.c_str(), &ChildFindFileData);
				while(ChildhFind !=INVALID_HANDLE_VALUE)
				{
					if (strcmp(ChildFindFileData.cFileName, ".") != 0 && strcmp(ChildFindFileData.cFileName, "..") != 0)
					{
						//实际文件名，格式为：xxxx-xx-xx.txt
						string FileNameTemp = string(ChildFindFileData.cFileName);

						//取出文件名中的日期信息，用于比较
						string fileDate = FileNameTemp.substr(0, 10);
						if (TimeUtil::DiffDate(logExpireDate, fileDate) > 0)	//如果过期
						{
							string tempDirName = DirName;
							tempDirName.erase( tempDirName.find_last_of("/")+1 );
							string fullFileName = tempDirName + FileNameTemp;	//deletefile函数参数必须包含实际路径

							int ret = DeleteFile( fullFileName.c_str() );	//删除
							if ( !ret )
							{
								string msg = "删除日志文件失败，文件名为" + FileNameTemp;
								SYSMSGSENDER::instance()->SendMsg(msg,UNKNOWN,VS_MSG_SYSALARM);
							}

							//写日志
							std::string msg = string("删除日志文件成功，文件名为：") + fullFileName;
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

