///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：RecordQueryTask.cpp
// 创建者：jiangcheng
// 创建时间：2009-06-22
// 内容描述：自动、任务录像查询、下载任务类
///////////////////////////////////////////////////////////////////////////////////////////
#include "RecordQueryTask.h"
#include "TranslateDownXML.h"
#include "ace/Log_Msg.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/PropManager.h"
#include "TranslateUpXML.h"
#include "../DBAccess/DBManager.h"
#include "../BusinessProcess/BusinessLayoutMgr.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/OSFunction.h"
#include "../FileSysAccess/WPLFile.h"
#include <vector>
#include <string>
#include <fstream>
using namespace std;
#include "../Foundation/StrUtil.h"
RecordQueryTask::RecordQueryTask() : DeviceIndependentTask()
{

}

RecordQueryTask::RecordQueryTask(std::string strXML) : DeviceIndependentTask(strXML)
{
	//解析录像类型：自动录像文件查看，任务录像文件查看，自动录像音视频流查询，任务录像音视频流查询
	XmlParser parser;
	parser.Set_xml(strXML);
	pXMLNODE recordTypeNode=parser.GetNodeFirstChild(parser.GetRootNode());
	std::string strRecordType=parser.GetNodeName(recordTypeNode);//录像类型

	if (strRecordType == "AutoRecordQuery")
	{
		RecordType = AUTORECORDQUERY;
	}
	else if (strRecordType == "AutoRecordFileQuery")
	{
		RecordType = AUTORECORDFILEQUERY;
	}
	else if (strRecordType == "TaskRecordQuery")
	{
		RecordType = TASKRECORDQUERY;
	}
	else if (strRecordType == "TaskRecordFileQuery")
	{
		RecordType = TASKRECORDFILEQUERY;
	}
}

RecordQueryTask::~RecordQueryTask()
{

}

void RecordQueryTask::Run()
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]录像查询任务执行 !\n",DeviceID));
	SetRunning();

	XmlParser parser( strStandardXML.c_str());	

	string RequireFreq ;
	//获取必要的查询参数
	pXMLNODE childNode = parser.GetNodeFirstChild(parser.GetRootNode());
	pXMLNODE recordQueryTaskNode = parser.GetNodeFirstChild(childNode);
	parser.GetAttrNode( recordQueryTaskNode,"DeviceID",RequireDeviceID );//DeviceID属性值
	parser.GetAttrNode( recordQueryTaskNode,"OrgNetID",RequireOrgNetID );//OrgNetID属性值
	parser.GetAttrNode( recordQueryTaskNode,"TsID",RequireTsID );//TsID属性值
	parser.GetAttrNode( recordQueryTaskNode,"ServiceID",RequireServiceID );//ServiceID属性值
	parser.GetAttrNode( recordQueryTaskNode,"VideoPID",RequireVideoPID );//ServiceID属性值
	parser.GetAttrNode( recordQueryTaskNode,"AudioPID",RequireAudioPID );//ServiceID属性值
	parser.GetAttrNode( recordQueryTaskNode,"TaskID",TaskID );//Taskid属性值
	parser.GetAttrNode(recordQueryTaskNode,"Code",RequireChannelCode);
	parser.GetAttrNode(recordQueryTaskNode,"Freq",RequireFreq);
	parser.GetAttrNode( recordQueryTaskNode,"StartDateTime",StartDateTime );//StartDateTime属性值
	parser.GetAttrNode( recordQueryTaskNode,"EndDateTime",EndDateTime );//EndDateTime属性值

	//视频查看相关参数
	PROPMANAGER::instance()->GetStreamProtocol(DVBType,VideoStreamProtocol);
	PROPMANAGER::instance()->GetFileProtocol(DVBType,VideoFileProtocol);
	PROPMANAGER::instance()->GetFileUrlType(DVBType,VideoFileurltype);
	PROPMANAGER::instance()->GetFileOffset(DVBType,FileOffset);

	//文件下载相关参数
	PROPMANAGER::instance()->GetRecDownProtocol(DVBType,DownFileProtocol);
	PROPMANAGER::instance()->GetRecDownUrlType(DVBType,DownFileurltype);
	PROPMANAGER::instance()->GetRecDownOffset(DVBType,DownOffset);

	StartDateTime = TimeUtil::DateTimeToStr(TimeUtil::StrToDateTime(StartDateTime)-2);
	EndDateTime = TimeUtil::DateTimeToStr(TimeUtil::StrToDateTime(EndDateTime)-1);

	bool ret=true;
	RetValue=RUN_SUCCESS;

	if (RecordType==AUTORECORDQUERY && StartDateTime=="" && EndDateTime=="")			//自动录像音视频查看(实时)
	{
		if (RequireDeviceID == "")		//模拟模块使用ChannelCode进行录像实时查看
		{

			int tempID = -1;

			//新安播：如果只下发了freq，没下发channelCode，就直接通过freq去获取自动录像通道号
			if (!((DVBType==ATV ||DVBType==RADIO||DVBType==CTV) && RequireFreq!="" && RequireChannelCode==""))
			{
				CHANNELINFOMGR::instance()->GetFreqByChannelCode(DVBType,RequireChannelCode,RequireFreq);
			}
							
			BUSINESSLAYOUTMGR::instance()->GetAutoRecordDeviceIdByFreq(DVBType,RequireFreq,tempID);

			RequireDeviceID = StrUtil::Int2Str(tempID);	
		}
		//生成返回URL
		if (VideoStreamProtocol == "rtsp")
		{
			RequireURL = VideoStreamProtocol + "://" + PROPMANAGER::instance()->GetRtspVideoIp() + ":" + \
				PROPMANAGER::instance()->GetRtspVideoPort()+"/"+RequireDeviceID;
		}
		else
		{
			RequireURL = VideoStreamProtocol + "://" + PROPMANAGER::instance()->GetHttpVideoIp() + ":" + \
				PROPMANAGER::instance()->GetHttpVideoPort()+"/"+RequireDeviceID;
		}

		SendXML(TranslateUpXML::TranslateAutoRecordStream(this,RequireURL,string(""),string("")));	
	}
	else					//自动/任务 录像音视频查看/下载
	{
		//查找数据库
		std::vector<sRecordInfo> vecRecordInfo;
		switch(RecordType)
		{
		case AUTORECORDQUERY:		//自动录像音视频查看
		case AUTORECORDFILEQUERY:	//自动录像音视频下载
			{
				string RequireChannelID;

				//新安播：如果只下发了freq，没下发channelCode，就把freq当做channelId去查数据库
				if ((DVBType==ATV ||DVBType==RADIO ||DVBType==CTV) && RequireFreq!="" && RequireChannelCode=="")
				{
					RequireChannelID = RequireFreq;
				}
				else	//其他情况还是维持原状
				{
					CHANNELINFOMGR::instance()->GetChannelID(DVBType,RequireOrgNetID,RequireTsID,RequireServiceID, \
						RequireVideoPID,RequireAudioPID,RequireChannelCode,RequireChannelID );//获得ChannelCode
					if(RequireChannelID.size()==0&&RequireChannelCode.size()>0)
					{
						RequireChannelID=RequireChannelCode;
					}
				}

				if (RequireChannelID.empty())
				{
					ret = DBMANAGER::instance()->QueryRecordByDeviceID(DVBType,"0",RequireDeviceID,vecRecordInfo,StartDateTime,EndDateTime);    //查询数据库得到录像文件的地址
				}
				else
				{
					ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,"0",RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); //查询数据库得到录像文件的地址
					if(vecRecordInfo.size()==0)
					{
						ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,"888888",RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); //查询电视新频录像
					}
					if(vecRecordInfo.size()==0)
					{
						ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,"666666",RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); //查询广播新频录像
					}

				}
				break;
			}
		case TASKRECORDQUERY:		//任务录像音视频查看
		case TASKRECORDFILEQUERY:	//任务录像音视频下载
			{
				string RequireChannelID;
				//新安播：如果只下发了freq，没下发channelCode，就把freq当做channelId去查数据库
				if ((DVBType==ATV ||DVBType==RADIO ||DVBType==CTV) && RequireFreq!="" && RequireChannelCode=="")
				{
					RequireChannelID = RequireFreq;
				}
				else	//其他情况还是维持原状
				{
					CHANNELINFOMGR::instance()->GetChannelID(DVBType,RequireOrgNetID,RequireTsID,RequireServiceID, \
						RequireVideoPID,RequireAudioPID,RequireChannelCode,RequireChannelID );//获得ChannelCode
				}
				if(RequireChannelID.empty())
				{
					ret=DBMANAGER::instance()->QueryRecordByTaskid(DVBType,TaskID,vecRecordInfo,StartDateTime,EndDateTime);
				}
				else
				{
					ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,TaskID,RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); 
				}
			}
		default:
			break;
		}
		//查询结果处理
		cout<<"vecRecordInfo.size:"<<vecRecordInfo.size()<<endl;

		if(vecRecordInfo.size()<=5)
		{
			VideoFileurltype = "file";
		}
		if (vecRecordInfo.size() == 0)		//未查到数据
		{
			SetRetValue(NOFILE_EXIST);
			switch(RecordType)
			{
			case AUTORECORDQUERY:			//自动录像音视频查看
				{
					SendXML(TranslateUpXML::TranslateAutoRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case AUTORECORDFILEQUERY:		//自动录像音视频下载
				{
					SendXML(TranslateUpXML::TranslateAutoRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case TASKRECORDQUERY:			//任务录像音视频查看
				{
					SendXML(TranslateUpXML::TranslateTaskRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));
					break;
				}
			case TASKRECORDFILEQUERY:		//任务录像音视频下载
				{
					SendXML(TranslateUpXML::TranslateTaskRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));
					break;
				}
			default:
				break;
			}
		}
		else //有效数据处理
		{
			//获取查询结果中的channelId(新安播要求任务录像查询时返回的xml中channelcode不为空)
			if (ChannelID == "")	//修改基类中的channelId
			{
				ChannelID = vecRecordInfo[0].channelID;
			}

			bool IsAudio = (DVBType==RADIO || DVBType==AM);		//纯音频文件标识
			string tempfils;
			string tempfile;
			if(vecRecordInfo.size()==1)
			{
				tempfils=vecRecordInfo[0].filename;
			}
			else if(vecRecordInfo.size()==2)
			{
				tempfils=vecRecordInfo[0].filename;
				tempfile=vecRecordInfo[1].filename;
			}
			RecordFileSpliter(vecRecordInfo,IsAudio);	//处理vecRecordInfo中的文件头尾,获得有效的文件集合
			if(vecRecordInfo.size()==1)
			{
				if(vecRecordInfo[0].filename.size()==0)
				{
					vecRecordInfo[0].filename=tempfils;
				}
			}
			else if(vecRecordInfo.size()==2)
			{
				if(vecRecordInfo[0].filename.size()==0)
				{
					vecRecordInfo[0].filename=tempfils;
				}
				if(vecRecordInfo[1].filename.size()==0)
				{
					vecRecordInfo[1].filename=tempfile;
				}
			}

			//处理查询开始、结束时间
			QueryStartDateTime = vecRecordInfo[0].starttime;
			QueryEndDateTime =vecRecordInfo[vecRecordInfo.size()-1].endtime;

			//生成临时文件名
			string TempFileName = TimeUtil::DateTimeToString(time(0)) + GetMsgID();		//临时文件名

			//生成URL
			int HttpServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpServerPort());//小于1为系统IIS发布目录没有端口号不是apache发布目录
			switch(RecordType)
			{
			case AUTORECORDQUERY:		//自动录像音视频查看
			case TASKRECORDQUERY:		//任务录像音视频查看
				{
					if (VideoFileurltype=="file")		//多个文件拼接
					{
						TempFileName +=std::string(".ts");
						if (VideoStreamProtocol == "rtsp")
						{
							RequireURL = VideoStreamProtocol + "://" + PROPMANAGER::instance()->GetRtspVideoIp() + ":" + \
								PROPMANAGER::instance()->GetRtspVideoPort() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
						}
						else
						{
							if(HttpServerPort < 1)
							{
								RequireURL = VideoStreamProtocol + "://" + PROPMANAGER::instance()->GetHttpServerIP() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
							}
							else
							{
								RequireURL = VideoStreamProtocol + "://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
									PROPMANAGER::instance()->GetHttpServerPort() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
							}
						}
					}
					else if (VideoFileurltype!="file")	//文件列表
					{
						TempFileName += std::string(".") + VideoFileurltype;
						if(HttpServerPort < 1)
						{
							RequireURL = "http://" + PROPMANAGER::instance()->GetHttpServerIP() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
						}
						else
						{
							RequireURL = "http://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
								PROPMANAGER::instance()->GetHttpServerPort() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
						}
					}
					break;
				}
			case AUTORECORDFILEQUERY:	//自动录像音视频下载
			case TASKRECORDFILEQUERY:	//任务录像音视频下载
				{
					if (DownFileurltype=="file")	//多个文件拼接
					{
						TempFileName +=std::string(".ts");
						if (DownFileProtocol == "rtsp")
						{
							RequireURL = DownFileProtocol + "://" + PROPMANAGER::instance()->GetRtspVideoIp() + ":" + \
							PROPMANAGER::instance()->GetRtspVideoPort() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
						}
						else
						{
							if(HttpServerPort < 1)
							{
								RequireURL = DownFileProtocol + "://" + PROPMANAGER::instance()->GetHttpServerIP() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
							}
							else
							{
								RequireURL = DownFileProtocol + "://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
									PROPMANAGER::instance()->GetHttpServerPort() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
							}
						}
					}
					else if (DownFileurltype!="file")	//文件列表
					{
						TempFileName += std::string(".") + DownFileurltype;
						if(HttpServerPort < 1)
						{
							RequireURL = "http://" + PROPMANAGER::instance()->GetHttpServerIP() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
						}
						else
						{
							RequireURL = "http://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
								PROPMANAGER::instance()->GetHttpServerPort() + PROPMANAGER::instance()->GetTempSharePath() + TempFileName;
						}
					}				
					break;
				}
			default:
				break;
			}
           std::vector<sRecordFileInfo> FileInfo;
			//后续生成临时文件
			switch(RecordType)
			{
			case AUTORECORDQUERY:		//自动录像音视频查看
			case TASKRECORDQUERY:		//任务录像音视频查看
				{
					if (VideoFileurltype == "file")			//多个文件拼接
					{
						CreateDownFile(TempFileName,vecRecordInfo);
					}
					else if (VideoFileurltype != "file")	//文件列表
					{
						CreatePlayListFile(VideoFileurltype,TempFileName,vecRecordInfo,VideoFileProtocol);
					}
					break;
				}
			case AUTORECORDFILEQUERY:	//自动录像音视频下载
			case TASKRECORDFILEQUERY:	//任务录像音视频下载
				{
					if (DownFileurltype == "file")			//多个文件拼接
					{
						CreateDownFile(TempFileName,vecRecordInfo);
					}
					else if (DownFileurltype == "list")	//列表文件 只针对新安播录像下载
					{
						CreateListURL(vecRecordInfo,FileInfo,DownFileProtocol);
					}
					else  //m3u文件或者wpl文件
					{
						CreatePlayListFile(DownFileurltype,TempFileName,vecRecordInfo,DownFileProtocol);
					}
					break;
				}
			default:
				break;
			}
			//发送回复xml
			switch(RecordType)
			{
			case AUTORECORDQUERY:			//自动录像音视频查看
				{
					SendXML(TranslateUpXML::TranslateAutoRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case AUTORECORDFILEQUERY:		//自动录像音视频下载
				{
					SendXML(TranslateUpXML::TranslateAutoRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case TASKRECORDQUERY:			//任务录像音视频查看
				{
					SendXML(TranslateUpXML::TranslateTaskRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));
					break;
				}
			case TASKRECORDFILEQUERY:		//任务录像音视频下载
				{
					if(DownFileurltype == "list")
					{
						SendXML(TranslateUpXML::TranslateTaskRecordFile(this,FileInfo));
					}
					else
					{
						SendXML(TranslateUpXML::TranslateTaskRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));				
					}				
					break;
				}
			default:
				break;
			}

		}
	}
	SetFinised();
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)通道[%d]录像查询任务停止 !\n",DeviceID));
}

std::string RecordQueryTask::GetTaskName()
{
	return "录像查询任务";
}

std::string RecordQueryTask::GetObjectName()
{
	return std::string("RecordQueryTask");
}

bool RecordQueryTask::CreatePlayListFile(std::string listtype,std::string tempfilename,std::vector<sRecordInfo> vecFile,std::string Protocol)
{
	if (tempfilename.empty() || tempfilename=="" || vecFile.size()==0)
		return false;
	string strTimestr = "";
	float MaxTime = 0.00;
	std::vector<std::string> filenamevec;
	std::vector<sRecordInfo>::iterator ptr = vecFile.begin();//文件头
	for (;ptr!=vecFile.end();++ptr)
	{
		std::string tempfilename = (*ptr).filename;
		size_t pos = tempfilename.find_last_of("/");
		if (pos ==std::string::npos)
		{
			continue;
		}
		std::string dirpath = tempfilename.substr(0,pos+1);
		std::string purename = tempfilename.substr(pos+1);

		std::string sharepath;
		PROPMANAGER::instance()->GetSharePathByLoc(dirpath,sharepath);

		std::string sharefilename = sharepath + purename;
		std::string InnerRequireURL;

		if (Protocol == "rtsp")
		{
			InnerRequireURL = Protocol + "://" + PROPMANAGER::instance()->GetRtspVideoIp() + sharefilename;
		}
		else
		{
			int HttpServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpServerPort());//小于1为系统IIS发布目录没有端口号不是apache发布目录
			if(HttpServerPort < 1)
			{
				InnerRequireURL = Protocol + "://" + PROPMANAGER::instance()->GetHttpServerIP() + sharefilename;
			}
			else
			{
				InnerRequireURL = Protocol + "://" + PROPMANAGER::instance()->GetHttpServerIP()+ ":" + PROPMANAGER::instance()->GetHttpServerPort() + sharefilename;
			}
		}
		strTimestr += sharefilename;
		strTimestr +=",";
		float tempmaxTime = (float)TimeUtil::DiffSecond(ptr->endtime,ptr->starttime);
		if(tempmaxTime>MaxTime)
		{
			MaxTime = tempmaxTime;
		}
		string strRecordTime = StrUtil::Float2Str(tempmaxTime);
		strTimestr += strRecordTime;
		strTimestr += ";";
		filenamevec.push_back(InnerRequireURL);
	}

	string FilePath = PROPMANAGER::instance()->GetTempFileLoac() + tempfilename;	//临时文件路径
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)查询任务创建文件[%s]!\n",FilePath.c_str()));

	if(listtype == "m3u")
	{
		WPLFile::CreateVLCFile(FilePath,filenamevec);//创建m3u文件
	}
	else if (listtype == "wpl")
	{
		WPLFile::CreateWPLFile(FilePath,filenamevec);//创建wpl文件
	}
	else if(listtype == "m3u8")
	{
		WPLFile::CreatVLCTsFile(vecFile[0].filename,"10");
		int pos = vecFile[0].filename.find(".ts");
		string strM3u8 = "";
		if(pos!=-1)
		{
			strM3u8 = vecFile[0].filename.substr(0,pos) + string(".m3u8");
		}
		strTimestr = string("MAX,") + StrUtil::Float2Str(MaxTime) + string(";") + strTimestr;
		if(WPLFile::CreateVLCFileM3U8(strM3u8,filenamevec,strTimestr))
		{
			DeleteFile(vecFile[0].filename.c_str());//删除文件
		}
	}

	//临时文件入库
	DBMANAGER::instance()->AddTempFile(FilePath,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());

	return true;
}


bool RecordQueryTask::CreateDownFile(std::string tempfilename,std::vector<sRecordInfo> vecFile)
{
	if (tempfilename.empty()||vecFile.empty())
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)查询任务文件不存在 !\n"));
		return false;
	}
	string FileName=PROPMANAGER::instance()->GetTempFileLoac() + tempfilename;

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)查询任务创建文件[%s]开始 !\n",FileName.c_str()));

	size_t pos;
	while((pos=FileName.find("/"))!=string::npos)
	{
		FileName.replace(pos,1,"\\");
	}
	string cmdstr="copy /B ";
	//遍历所有文件

	for (std::vector<sRecordInfo>::iterator ptr = vecFile.begin();ptr!= vecFile.end();++ptr)
	{
		string tempfile=(*ptr).filename;
		if(tempfile=="")
			continue;
		while((pos=tempfile.find("/"))!=string::npos)
		{
			tempfile.replace(pos,1,"\\");
		}
		cmdstr+=tempfile+"+";
	}
	pos = cmdstr.find_last_of('+');
	if (pos != std::string::npos)
	{
		cmdstr.replace(pos,1," ");
		cmdstr+=FileName;
		system(cmdstr.c_str());
	}
	//临时文件入库
	DBMANAGER::instance()->AddTempFile(FileName,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)查询任务创建文件[%s]结束 !\n",FileName.c_str()));
	return true;
}
bool RecordQueryTask::RecordFileSpliter(vector < sRecordInfo>& fileVector,bool IsAudio)
{
	if(StartDateTime==""&&EndDateTime=="")
		return true;
	if (fileVector.size()==1)
	{
		string newfilename;
		time_t starttime=TimeUtil::StrToDateTime(StartDateTime);
		starttime = starttime ;
		time_t endtime= TimeUtil::StrToDateTime(EndDateTime);
		newfilename=WPLFile::TimeFileSplit(fileVector[0].filename,starttime,endtime);
		
		fileVector[0].filename=newfilename;
		fileVector[0].starttime=StartDateTime;
		fileVector[0].endtime=EndDateTime;
		
		if(newfilename!="")
			DBMANAGER::instance()->AddTempFile(newfilename,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());
	}
	else
	{
		string newfilename;
		time_t starttime=TimeUtil::StrToDateTime(StartDateTime);
		starttime = starttime;
		time_t endtime= TimeUtil::StrToDateTime(EndDateTime);

		time_t tempend=TimeUtil::StrToDateTime(fileVector[0].endtime);
		time_t tempstart=TimeUtil::StrToDateTime(fileVector[fileVector.size()-1].starttime);
		newfilename=WPLFile::TimeFileSplit(fileVector[0].filename,starttime,tempend,1);
	
		fileVector[0].filename=newfilename;
		fileVector[0].starttime=StartDateTime;
		
		if(newfilename!="")
			DBMANAGER::instance()->AddTempFile(newfilename,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());

		newfilename=WPLFile::TimeFileSplit(fileVector[fileVector.size()-1].filename,tempstart,endtime,0);
	
		fileVector[fileVector.size()-1].filename=newfilename;
		fileVector[fileVector.size()-1].endtime=EndDateTime;
		if(newfilename!="")
			DBMANAGER::instance()->AddTempFile(newfilename,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());
	}
	return true;
}


bool RecordQueryTask::CreateListURL(std::vector<sRecordInfo> vecFile,std::vector<sRecordFileInfo> &FileInfo,std::string Protocol)
{
	if (vecFile.size()==0)
		return false;

	for (std::vector<sRecordInfo>::iterator ptr=vecFile.begin();ptr!=vecFile.end();++ptr)
	{
		sRecordFileInfo tempInfo;
		std::string tempfilename = (*ptr).filename;
		size_t pos = tempfilename.find_last_of("/");
		if (pos ==std::string::npos)
		{
			continue;
		}
		std::string dirpath = tempfilename.substr(0,pos+1);
		std::string purename = tempfilename.substr(pos+1);

		std::string sharepath;
		PROPMANAGER::instance()->GetSharePathByLoc(dirpath,sharepath);

		std::string sharefilename = sharepath + purename;
		std::string InnerRequireURL;

		if (Protocol == "rtsp")
		{
			InnerRequireURL = Protocol + "://" + PROPMANAGER::instance()->GetRtspVideoIp() + sharefilename;
		}
		else
		{
			int HttpServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpServerPort());//小于1为系统IIS发布目录没有端口号不是apache发布目录
			if(HttpServerPort < 1)
			{
				InnerRequireURL = Protocol + "://" + PROPMANAGER::instance()->GetHttpServerIP() + sharefilename;
			}
			else
			{
				InnerRequireURL = Protocol + "://" + PROPMANAGER::instance()->GetHttpServerIP()+ ":" + PROPMANAGER::instance()->GetHttpServerPort() + sharefilename;
			}
		}
		tempInfo.url=InnerRequireURL;
		tempInfo.starttime=(*ptr).starttime;
		tempInfo.endtime = (*ptr).endtime;
		FileInfo.push_back(tempInfo);
	}



	return true;
}



