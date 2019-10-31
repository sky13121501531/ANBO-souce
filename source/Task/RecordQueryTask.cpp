///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����RecordQueryTask.cpp
// �����ߣ�jiangcheng
// ����ʱ�䣺2009-06-22
// �����������Զ�������¼���ѯ������������
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
	//����¼�����ͣ��Զ�¼���ļ��鿴������¼���ļ��鿴���Զ�¼������Ƶ����ѯ������¼������Ƶ����ѯ
	XmlParser parser;
	parser.Set_xml(strXML);
	pXMLNODE recordTypeNode=parser.GetNodeFirstChild(parser.GetRootNode());
	std::string strRecordType=parser.GetNodeName(recordTypeNode);//¼������

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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]¼���ѯ����ִ�� !\n",DeviceID));
	SetRunning();

	XmlParser parser( strStandardXML.c_str());	

	string RequireFreq ;
	//��ȡ��Ҫ�Ĳ�ѯ����
	pXMLNODE childNode = parser.GetNodeFirstChild(parser.GetRootNode());
	pXMLNODE recordQueryTaskNode = parser.GetNodeFirstChild(childNode);
	parser.GetAttrNode( recordQueryTaskNode,"DeviceID",RequireDeviceID );//DeviceID����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"OrgNetID",RequireOrgNetID );//OrgNetID����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"TsID",RequireTsID );//TsID����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"ServiceID",RequireServiceID );//ServiceID����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"VideoPID",RequireVideoPID );//ServiceID����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"AudioPID",RequireAudioPID );//ServiceID����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"TaskID",TaskID );//Taskid����ֵ
	parser.GetAttrNode(recordQueryTaskNode,"Code",RequireChannelCode);
	parser.GetAttrNode(recordQueryTaskNode,"Freq",RequireFreq);
	parser.GetAttrNode( recordQueryTaskNode,"StartDateTime",StartDateTime );//StartDateTime����ֵ
	parser.GetAttrNode( recordQueryTaskNode,"EndDateTime",EndDateTime );//EndDateTime����ֵ

	//��Ƶ�鿴��ز���
	PROPMANAGER::instance()->GetStreamProtocol(DVBType,VideoStreamProtocol);
	PROPMANAGER::instance()->GetFileProtocol(DVBType,VideoFileProtocol);
	PROPMANAGER::instance()->GetFileUrlType(DVBType,VideoFileurltype);
	PROPMANAGER::instance()->GetFileOffset(DVBType,FileOffset);

	//�ļ�������ز���
	PROPMANAGER::instance()->GetRecDownProtocol(DVBType,DownFileProtocol);
	PROPMANAGER::instance()->GetRecDownUrlType(DVBType,DownFileurltype);
	PROPMANAGER::instance()->GetRecDownOffset(DVBType,DownOffset);

	StartDateTime = TimeUtil::DateTimeToStr(TimeUtil::StrToDateTime(StartDateTime)-2);
	EndDateTime = TimeUtil::DateTimeToStr(TimeUtil::StrToDateTime(EndDateTime)-1);

	bool ret=true;
	RetValue=RUN_SUCCESS;

	if (RecordType==AUTORECORDQUERY && StartDateTime=="" && EndDateTime=="")			//�Զ�¼������Ƶ�鿴(ʵʱ)
	{
		if (RequireDeviceID == "")		//ģ��ģ��ʹ��ChannelCode����¼��ʵʱ�鿴
		{

			int tempID = -1;

			//�°��������ֻ�·���freq��û�·�channelCode����ֱ��ͨ��freqȥ��ȡ�Զ�¼��ͨ����
			if (!((DVBType==ATV ||DVBType==RADIO||DVBType==CTV) && RequireFreq!="" && RequireChannelCode==""))
			{
				CHANNELINFOMGR::instance()->GetFreqByChannelCode(DVBType,RequireChannelCode,RequireFreq);
			}
							
			BUSINESSLAYOUTMGR::instance()->GetAutoRecordDeviceIdByFreq(DVBType,RequireFreq,tempID);

			RequireDeviceID = StrUtil::Int2Str(tempID);	
		}
		//���ɷ���URL
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
	else					//�Զ�/���� ¼������Ƶ�鿴/����
	{
		//�������ݿ�
		std::vector<sRecordInfo> vecRecordInfo;
		switch(RecordType)
		{
		case AUTORECORDQUERY:		//�Զ�¼������Ƶ�鿴
		case AUTORECORDFILEQUERY:	//�Զ�¼������Ƶ����
			{
				string RequireChannelID;

				//�°��������ֻ�·���freq��û�·�channelCode���Ͱ�freq����channelIdȥ�����ݿ�
				if ((DVBType==ATV ||DVBType==RADIO ||DVBType==CTV) && RequireFreq!="" && RequireChannelCode=="")
				{
					RequireChannelID = RequireFreq;
				}
				else	//�����������ά��ԭ״
				{
					CHANNELINFOMGR::instance()->GetChannelID(DVBType,RequireOrgNetID,RequireTsID,RequireServiceID, \
						RequireVideoPID,RequireAudioPID,RequireChannelCode,RequireChannelID );//���ChannelCode
					if(RequireChannelID.size()==0&&RequireChannelCode.size()>0)
					{
						RequireChannelID=RequireChannelCode;
					}
				}

				if (RequireChannelID.empty())
				{
					ret = DBMANAGER::instance()->QueryRecordByDeviceID(DVBType,"0",RequireDeviceID,vecRecordInfo,StartDateTime,EndDateTime);    //��ѯ���ݿ�õ�¼���ļ��ĵ�ַ
				}
				else
				{
					ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,"0",RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); //��ѯ���ݿ�õ�¼���ļ��ĵ�ַ
					if(vecRecordInfo.size()==0)
					{
						ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,"888888",RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); //��ѯ������Ƶ¼��
					}
					if(vecRecordInfo.size()==0)
					{
						ret = DBMANAGER::instance()->QueryRecordByChannelID(DVBType,"666666",RequireChannelID,vecRecordInfo,StartDateTime,EndDateTime); //��ѯ�㲥��Ƶ¼��
					}

				}
				break;
			}
		case TASKRECORDQUERY:		//����¼������Ƶ�鿴
		case TASKRECORDFILEQUERY:	//����¼������Ƶ����
			{
				string RequireChannelID;
				//�°��������ֻ�·���freq��û�·�channelCode���Ͱ�freq����channelIdȥ�����ݿ�
				if ((DVBType==ATV ||DVBType==RADIO ||DVBType==CTV) && RequireFreq!="" && RequireChannelCode=="")
				{
					RequireChannelID = RequireFreq;
				}
				else	//�����������ά��ԭ״
				{
					CHANNELINFOMGR::instance()->GetChannelID(DVBType,RequireOrgNetID,RequireTsID,RequireServiceID, \
						RequireVideoPID,RequireAudioPID,RequireChannelCode,RequireChannelID );//���ChannelCode
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
		//��ѯ�������
		cout<<"vecRecordInfo.size:"<<vecRecordInfo.size()<<endl;

		if(vecRecordInfo.size()<=5)
		{
			VideoFileurltype = "file";
		}
		if (vecRecordInfo.size() == 0)		//δ�鵽����
		{
			SetRetValue(NOFILE_EXIST);
			switch(RecordType)
			{
			case AUTORECORDQUERY:			//�Զ�¼������Ƶ�鿴
				{
					SendXML(TranslateUpXML::TranslateAutoRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case AUTORECORDFILEQUERY:		//�Զ�¼������Ƶ����
				{
					SendXML(TranslateUpXML::TranslateAutoRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case TASKRECORDQUERY:			//����¼������Ƶ�鿴
				{
					SendXML(TranslateUpXML::TranslateTaskRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));
					break;
				}
			case TASKRECORDFILEQUERY:		//����¼������Ƶ����
				{
					SendXML(TranslateUpXML::TranslateTaskRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));
					break;
				}
			default:
				break;
			}
		}
		else //��Ч���ݴ���
		{
			//��ȡ��ѯ����е�channelId(�°���Ҫ������¼���ѯʱ���ص�xml��channelcode��Ϊ��)
			if (ChannelID == "")	//�޸Ļ����е�channelId
			{
				ChannelID = vecRecordInfo[0].channelID;
			}

			bool IsAudio = (DVBType==RADIO || DVBType==AM);		//����Ƶ�ļ���ʶ
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
			RecordFileSpliter(vecRecordInfo,IsAudio);	//����vecRecordInfo�е��ļ�ͷβ,�����Ч���ļ�����
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

			//�����ѯ��ʼ������ʱ��
			QueryStartDateTime = vecRecordInfo[0].starttime;
			QueryEndDateTime =vecRecordInfo[vecRecordInfo.size()-1].endtime;

			//������ʱ�ļ���
			string TempFileName = TimeUtil::DateTimeToString(time(0)) + GetMsgID();		//��ʱ�ļ���

			//����URL
			int HttpServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpServerPort());//С��1ΪϵͳIIS����Ŀ¼û�ж˿ںŲ���apache����Ŀ¼
			switch(RecordType)
			{
			case AUTORECORDQUERY:		//�Զ�¼������Ƶ�鿴
			case TASKRECORDQUERY:		//����¼������Ƶ�鿴
				{
					if (VideoFileurltype=="file")		//����ļ�ƴ��
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
					else if (VideoFileurltype!="file")	//�ļ��б�
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
			case AUTORECORDFILEQUERY:	//�Զ�¼������Ƶ����
			case TASKRECORDFILEQUERY:	//����¼������Ƶ����
				{
					if (DownFileurltype=="file")	//����ļ�ƴ��
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
					else if (DownFileurltype!="file")	//�ļ��б�
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
			//����������ʱ�ļ�
			switch(RecordType)
			{
			case AUTORECORDQUERY:		//�Զ�¼������Ƶ�鿴
			case TASKRECORDQUERY:		//����¼������Ƶ�鿴
				{
					if (VideoFileurltype == "file")			//����ļ�ƴ��
					{
						CreateDownFile(TempFileName,vecRecordInfo);
					}
					else if (VideoFileurltype != "file")	//�ļ��б�
					{
						CreatePlayListFile(VideoFileurltype,TempFileName,vecRecordInfo,VideoFileProtocol);
					}
					break;
				}
			case AUTORECORDFILEQUERY:	//�Զ�¼������Ƶ����
			case TASKRECORDFILEQUERY:	//����¼������Ƶ����
				{
					if (DownFileurltype == "file")			//����ļ�ƴ��
					{
						CreateDownFile(TempFileName,vecRecordInfo);
					}
					else if (DownFileurltype == "list")	//�б��ļ� ֻ����°���¼������
					{
						CreateListURL(vecRecordInfo,FileInfo,DownFileProtocol);
					}
					else  //m3u�ļ�����wpl�ļ�
					{
						CreatePlayListFile(DownFileurltype,TempFileName,vecRecordInfo,DownFileProtocol);
					}
					break;
				}
			default:
				break;
			}
			//���ͻظ�xml
			switch(RecordType)
			{
			case AUTORECORDQUERY:			//�Զ�¼������Ƶ�鿴
				{
					SendXML(TranslateUpXML::TranslateAutoRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case AUTORECORDFILEQUERY:		//�Զ�¼������Ƶ����
				{
					SendXML(TranslateUpXML::TranslateAutoRecordFile(this,RequireURL,QueryStartDateTime,QueryEndDateTime));	
					break;
				}
			case TASKRECORDQUERY:			//����¼������Ƶ�鿴
				{
					SendXML(TranslateUpXML::TranslateTaskRecordStream(this,RequireURL,QueryStartDateTime,QueryEndDateTime));
					break;
				}
			case TASKRECORDFILEQUERY:		//����¼������Ƶ����
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
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)ͨ��[%d]¼���ѯ����ֹͣ !\n",DeviceID));
}

std::string RecordQueryTask::GetTaskName()
{
	return "¼���ѯ����";
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
	std::vector<sRecordInfo>::iterator ptr = vecFile.begin();//�ļ�ͷ
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
			int HttpServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpServerPort());//С��1ΪϵͳIIS����Ŀ¼û�ж˿ںŲ���apache����Ŀ¼
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

	string FilePath = PROPMANAGER::instance()->GetTempFileLoac() + tempfilename;	//��ʱ�ļ�·��
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ѯ���񴴽��ļ�[%s]!\n",FilePath.c_str()));

	if(listtype == "m3u")
	{
		WPLFile::CreateVLCFile(FilePath,filenamevec);//����m3u�ļ�
	}
	else if (listtype == "wpl")
	{
		WPLFile::CreateWPLFile(FilePath,filenamevec);//����wpl�ļ�
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
			DeleteFile(vecFile[0].filename.c_str());//ɾ���ļ�
		}
	}

	//��ʱ�ļ����
	DBMANAGER::instance()->AddTempFile(FilePath,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());

	return true;
}


bool RecordQueryTask::CreateDownFile(std::string tempfilename,std::vector<sRecordInfo> vecFile)
{
	if (tempfilename.empty()||vecFile.empty())
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ѯ�����ļ������� !\n"));
		return false;
	}
	string FileName=PROPMANAGER::instance()->GetTempFileLoac() + tempfilename;

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ѯ���񴴽��ļ�[%s]��ʼ !\n",FileName.c_str()));

	size_t pos;
	while((pos=FileName.find("/"))!=string::npos)
	{
		FileName.replace(pos,1,"\\");
	}
	string cmdstr="copy /B ";
	//���������ļ�

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
	//��ʱ�ļ����
	DBMANAGER::instance()->AddTempFile(FileName,TimeUtil::GetCurDateTime(),PROPMANAGER::instance()->GetTempFileExpire());

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)��ѯ���񴴽��ļ�[%s]���� !\n",FileName.c_str()));
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
			int HttpServerPort = StrUtil::Str2Int(PROPMANAGER::instance()->GetHttpServerPort());//С��1ΪϵͳIIS����Ŀ¼û�ж˿ںŲ���apache����Ŀ¼
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



