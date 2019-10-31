///////////////////////////////////////////////////////////////////////////////////////////
// �ļ�����TranslateUpXMLForATV.cpp
// �����ߣ�gaoxd
// ����ʱ�䣺2010-04-09
// ������������ԭ������ִ�н����XMLת��ΪӦ��ϵͳ�����XML
///////////////////////////////////////////////////////////////////////////////////////////
#pragma warning(disable:4996)
#include "TranslateUpXMLForATV.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/PropManager.h"
#include "../Task//StreamRealtimeQueryTask.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../Foundation/PropManager.h"
#include "../DBAccess/DBManager.h"
#include "../BusinessProcess/QualityCompensationMgr.h"
#include "../Foundation/OSFunction.h"
#include "../cmdLine.h"
#include <fstream>



//����ʵʱ��ƵXML
std::string TranslateUpXMLForATV::TranslateStreamRealtimeQuery(const XMLTask* task)
{
	std::string retXml=GetXmlHeader(task,"StreamRealtimeQuery");//��׼�ķ���ͷxml
	std::string index, url;
	if (task->GetRetValue()==RUN_SUCCESS || task->GetRetValue()==PREFERENTIALTASK_USE)
	{
		index="0";
		string protocol;

		//wz_110309(�޸Ľӿ�)
		PROPMANAGER::instance()->GetStreamProtocol(ATV,protocol);
		if (protocol=="rtsp")
		{
			url="rtsp://" + PROPMANAGER::instance()->GetRtspVideoIp() + ":" + PROPMANAGER::instance()->GetRtspVideoPort() +"/"+ StrUtil::Int2Str(task->GetDeviceID());//��Ƶ���ŵ�ַ
		}
		else
		{
			url=protocol+"://" + PROPMANAGER::instance()->GetHttpVideoIp() + ":" + PROPMANAGER::instance()->GetHttpVideoPort() +"/"+ StrUtil::Int2Str(task->GetDeviceID());//��Ƶ���ŵ�ַ
		}

	}
	XmlParser retParser;
	retParser.Set_xml(retXml);

	pXMLNODE rootNode=retParser.GetRootNode();
	pXMLNODE streamNode=retParser.CreateNodePtr(rootNode,"Stream");//����Stream�ڵ�
	pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//����Stream�ڵ���ӽڵ�MediaStream�ڵ�

	retParser.SetAttrNode("Index",index,mediaStreamNode);//����MediaStream�ڵ��index����
	retParser.SetAttrNode("URL",url,mediaStreamNode);//����MediaStream�ڵ��URL����
	retParser.SaveToString(retXml);
	return retXml;
}
//����¼������XML
std::string TranslateUpXMLForATV::TranslateRecord(const XMLTask* task,const std::vector<XMLTask*>& resultVec)
{
	std::string retXML;

	if(task->GetTaskID() == "0")
	{
		retXML=GetXmlHeader(task,"SetAutoRecordChannel");
	}
	else
	{
	   retXML=GetXmlHeader(task,"TaskSet");
	}
	return retXML;
}
//�Զ�¼����Ƶ����ѯXML
std::string TranslateUpXMLForATV::TranslateAutoRecordStream(const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime)
{
	std::string retXml=GetXmlHeader(task,"StreamSimpleQuery");//��׼�ķ���ͷxml
	std::string index="0";

	XmlParser retParser;
	retParser.Set_xml(retXml);

	pXMLNODE rootNode=retParser.GetRootNode();
	pXMLNODE streamNode=retParser.CreateNodePtr(rootNode,"Stream");//����Stream�ڵ�
	startDateTime=TimeUtil::GetCurDateTime();
	endDateTime=TimeUtil::GetCurDateTime();
	retParser.SetAttrNode("StartDateTime",startDateTime,streamNode);
	retParser.SetAttrNode("EndDateTime",endDateTime,streamNode);
	pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//����Stream�ڵ���ӽڵ�MediaStream�ڵ�

	retParser.SetAttrNode("Index",index,mediaStreamNode);//����MediaStream�ڵ��index����
	retParser.SetAttrNode("URL",URL,mediaStreamNode);//����MediaStream�ڵ��URL����
	retParser.SaveToString(retXml);
	cout<<retXml<<endl;
	return retXml;
}
//���췵��XMLͷ��Ϣ
std::string TranslateUpXMLForATV::GetXmlHeader(const XMLTask* task,std::string retType)
{
	char * cheader= "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg><Return/></Msg>";

// 	string  MsgID ;
// 	DBMANAGER::instance()->GetMsgID(MsgID);
// 	int msgid=StrUtil::Str2Int(MsgID);
// 	DBMANAGER::instance()->UpdateMsgID(StrUtil::Int2Str(++msgid));
	string   Desc ="",Comment = "";
	eTaskRetStatus retValue=task->GetRetValue();//�����ִ�н��

	switch(retValue)
	{
	case RUN_SUCCESS:
		Desc="�ɹ�";
		break;
	case RUN_FAILED:
		Desc="ʧ��";
		break;
	case NOFILE_EXIST:
		Desc="�ļ�������";
		break;
	case CHANNELID_UNAVAIABLE:
		Desc="Ƶ���Ƿ�";
		break;
	case DEVICEID_UNAVAIABLE:
		Desc="ͨ���Ƿ�";
		break;
	case NODEVICEIDMATCH:
		Desc="û��ͨ��ƥ�������";
		break;
	case PREFERENTIALTASK_USE:
		Desc="�����ȼ�����ռ��ͨ��";
		break;
	case DATABASEERROR:
		Desc="�������ݿ����";
		break;
	case DEVICEERROR:
		Desc="����Ӳ������";
		break;
	default:
		Desc="ʧ��";
		break;
	}

	std::string msgType;//��Ϣ����
	if (task->GetDVBType()== ATV)
		msgType="TVMonUp";
	else
		msgType="UnKnow";

	std::string retXml="";
	XmlParser headParser(cheader);
	pXMLNODE headRootNode = headParser.GetNodeFromPath( "Msg" );//���ڵ�msg

	headParser.SetAttrNode( "Version",task->GetVersion(),headRootNode );//�汾
	string strDateTime = TimeUtil::GetCurDateTime();
	string  MsgID = OSFunction::GetXmlMsgID(strDateTime);
	headParser.SetAttrNode( "MsgID",MsgID,headRootNode );//��Ϣid����
	headParser.SetAttrNode( "Type",msgType,headRootNode );//��Ϣ����
	headParser.SetAttrNode( "DateTime",strDateTime/*TimeUtil::GetCurDateTime()*/,headRootNode );//��ǰʱ��
	headParser.SetAttrNode( "SrcCode",task->GetDstCode(),headRootNode );//������ʶ����ͨ���ӿڻ��
	headParser.SetAttrNode( "DstCode",task->GetSrcCode(),headRootNode );//Ŀ�������ʶ
	headParser.SetAttrNode( "DstURL",task->GetSrcURL(),headRootNode);//Ŀ��URL
	headParser.SetAttrNode( "ReplyID",task->GetMsgID(),headRootNode );//�ظ�����Ϣid

	pXMLNODE retNode = headParser.GetNodeFromPath("Msg/Return");
	headParser.SetAttrNode( "Type",retType,retNode );

	headParser.SetAttrNode( "Value",retValue,retNode );//return�ڵ��value����
	headParser.SetAttrNode( "Desc",Desc,retNode );//return�ڵ��Desc����
	headParser.SetAttrNode( "Comment",Comment,retNode );//return�ڵ��Comment����
	headParser.SaveToString( retXml );

	return retXml;
}

std::string TranslateUpXMLForATV::TranslateChannelScanQuery( const XMLTask* task,std::string strxml )
{
	std::string retXml=GetXmlHeader(task,"ChannelScanQuery");//��׼�ķ���ͷxml
	if (task->GetRetValue() != RUN_SUCCESS  || strxml.empty())
	{
		return retXml;
	}
	XmlParser srcParser(retXml.c_str());
	pXMLNODE root=srcParser.GetRootNode();
	pXMLNODE reportNode=srcParser.CreateNodePtr(root,"ChannelScanReport");
	pXMLNODE srcChannelScanNode=srcParser.CreateNodePtr(reportNode,"ChannelScan");
	srcParser.SetAttrNode("Index",string("0"),srcChannelScanNode);
	XmlParser rtnParser(strxml.c_str());
	pXMLNODE ChannelScanNode=rtnParser.GetNodeFromPath("Msg/ChannelScan");
	pXMLNODELIST ScanNodelist=rtnParser.GetNodeList(ChannelScanNode);
	for (int k=0;k!=rtnParser.GetChildCount(ChannelScanNode);++k)
	{
		pXMLNODE childNode=rtnParser.GetNextNode(ScanNodelist);
		pXMLNODE node=srcParser.CreateNodePtr(srcChannelScanNode,"Channel");
		string freq;
		rtnParser.GetAttrNode(childNode,"Freq",freq);
		srcParser.SetAttrNode("VideoFreq",freq,node);
	}
	srcParser.SaveToString(retXml);

	return retXml;
}

std::string TranslateUpXMLForATV::TranslateStreamRound( const XMLTask* task,std::string deviceID )
{
	std::string retXml;//���ص�xml
	std::string retType="StreamRoundQuery";//Ӳ������xml��return�ڵ�����ͺ�ֵ

	std::string headerxml=GetXmlHeader(task,retType);//��׼�ķ���ͷxml

	std::string index, url;
	if(task->GetRetValue()==5)
	{	
		XmlParser headParser;
		headParser.Set_xml(headerxml);
		pXMLNODE retNode = headParser.GetNodeFromPath("Msg/Return");
		headParser.SetAttrNode( "Type",string("StreamRoundQuery"),retNode );
		headParser.SaveToString(retXml);
		return retXml;
	}
	if (task->GetRetValue()==0)
	{
		index="0";
		string protocol;

		//wz_110309(�޸Ľӿ�)
		PROPMANAGER::instance()->GetStreamProtocol(ATV,protocol);
		if (protocol=="rtsp")
		{
			url="rtsp://" + PROPMANAGER::instance()->GetRtspVideoIp() + ":" + \
				PROPMANAGER::instance()->GetRtspVideoPort() +"/roundstream" ;//��Ƶ���ŵ�ַ
		}
		else
		{
			//wz_0217
			int virtualid = 0;
			PROPMANAGER::instance()->GetVirDeviceId(ATV, virtualid);
			url=protocol+"://" + PROPMANAGER::instance()->GetHttpVideoIp() + ":" + \
				PROPMANAGER::instance()->GetHttpVideoPort() +"/"+ StrUtil::Int2Str(virtualid);	//��Ƶ���ŵ�ַ
			//wz_0217
		}
	}
	XmlParser retParser;
	retParser.Set_xml(headerxml);

	pXMLNODE rootNode=retParser.GetRootNode();
	pXMLNODE streamNode=retParser.CreateNodePtr(rootNode,"Stream");//
	pXMLNODE mediaStreamNode=retParser.CreateNodePtr(streamNode,"MediaStream");//


	retParser.SetAttrNode("Index",index,mediaStreamNode);//����MediaStream�ڵ��index����
	retParser.SetAttrNode("URL",url,mediaStreamNode);//����MediaStream�ڵ��URL����
	retParser.SaveToString(retXml);

	return retXml;
}

std::string TranslateUpXMLForATV::TranslateChannelSet( const XMLTask* task )
{
	std::string retXml=GetXmlHeader(task,"ChannelSet");//��׼�ķ���ͷxml
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateAlarmParamSet( const XMLTask* task )
{
	std::string retXml=GetXmlHeader(task,"QualityAlarmParamSet");//��׼�ķ���ͷxml
	return retXml;
}
std::string TranslateUpXMLForATV::TranslateQualitySet( const XMLTask* task )
{
	std::string retXml=GetXmlHeader(task,"QualityReportTaskSet");//��׼�ķ���ͷxml
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateSpectrumSet(const XMLTask* task)
{
	std::string retXml=GetXmlHeader(task,"SpectrumScanTaskSet");//��׼�ķ���ͷxml
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateSpectrumQuery( const XMLTask* task,std::vector<eSpecInfo> vecSpecInfo)
{
	std::string headerxml=GetXmlHeader(task,"FreqScanQuery");
	XmlParser parser(headerxml.c_str());
	pXMLNODE rootNode = parser.GetRootNode();
	int count=vecSpecInfo.size();
	if(count>1000)
	{
		string strURL,sharepath;
		sharepath=PROPMANAGER::instance()->GetTempSharePath();
		strURL= "http://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + PROPMANAGER::instance()->GetHttpServerPort() + sharepath+"Spec_";
		time_t curtime;
		float freq;
		int status=0;
		float valu=0.0;
		string strtime=TimeUtil::DateTimeToString(time(0));
		string dstZipPath=PROPMANAGER::instance()->GetTempFileLoac()+"Spec_"+strtime+".zip";
		string srcFilePath=PROPMANAGER::instance()->GetTempFileLoac()+"Spec_"+strtime+".txt";
		strURL +=strtime+".zip";
		ofstream ofile(srcFilePath.c_str());

		for(vector<eSpecInfo>::iterator itr=vecSpecInfo.begin();itr!=vecSpecInfo.end();itr++)
		{
			freq=StrUtil::Str2Float(itr->freq);
			curtime=TimeUtil::StrToDateTime(itr->checktime);
			valu = StrUtil::Str2Float(itr->valu);
			status=StrUtil::Str2Int(itr->status);
			ofile<<curtime<<" "<<freq<<" "<<valu<<" "<<status<<" "<<endl;
		}

		ofile.close();
		OSFunction::CompressFile(dstZipPath,srcFilePath);
		pXMLNODE filenode=parser.CreateNodePtr(parser.GetRootNode(),"FreqScanReportFile");
		parser.SetAttrNode(string("URL"),strURL,filenode); //����ѹ�����ݴ�ŵ�ַ
		parser.SetAttrNode(string("Size"),(int)count,filenode); //ѹ�������ļ���С
	}
	else
	{
		pXMLNODE ReportNode =parser.CreateNodePtr(rootNode,"FreqScanReport");

		for(int i=0;i<vecSpecInfo.size();i++)
		{
			pXMLNODE childNode =parser.CreateNodePtr(ReportNode,"ScanResult");
			parser.SetAttrNode("Freq",vecSpecInfo[i].freq,childNode);
			parser.SetAttrNode("Level",vecSpecInfo[i].valu,childNode);
			parser.SetAttrNode("Status",vecSpecInfo[i].status,childNode);
			parser.SetAttrNode("CheckTime",vecSpecInfo[i].checktime,childNode);
		}
	}
	
	string retXml;
	parser.SaveToString(retXml);
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateTaskRecordStream( const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime )
{
	std::string headerxml=GetXmlHeader(task,"TaskRecordQuery");
	XmlParser parser(headerxml.c_str());
	pXMLNODE report=parser.CreateNodePtr(parser.GetRootNode(),"TaskRecordReport");
	pXMLNODE TaskRecord =parser.CreateNodePtr(report,"TaskRecord");
	parser.SetAttrNode("index",string("0"),TaskRecord);
	parser.SetAttrNode("EquCODE",string(""),TaskRecord);

	//�°���Ҫ�󷵻�channelCode
	std::string strChannelId = task->GetChannelID();
	std::string strChannelCode;
	CHANNELINFOMGR::instance()->GetChanCodeByChanId(ATV, strChannelId, strChannelCode);
	if (strChannelCode == "")
	{
		strChannelCode = strChannelId;
	}
	parser.SetAttrNode("Channelcode",strChannelCode,TaskRecord);

	parser.SetAttrNode("TaskID",task->GetTaskID(),TaskRecord);
	pXMLNODE record=parser.CreateNodePtr(TaskRecord,"record");
	parser.SetAttrNode("recordid",100,record);
	parser.SetAttrNode("Startdatetime",startDateTime,record);
	parser.SetAttrNode("Enddatetime",endDateTime,record);
	parser.SetAttrNode("size",100,record);
	parser.SetAttrNode("expiredays",PROPMANAGER::instance()->GetRecordExpiredays(task->GetDVBType()),record);
	
	parser.SetAttrNode("url",URL,record);

	string retxml;
	parser.SaveToString(retxml);
	return retxml;
}

//¼������ add by jidushan 11.04.19
std::string TranslateUpXMLForATV::TranslateTaskRecordFile( const XMLTask* task,std::string URL,std::string startDateTime,std::string endDateTime)
{
	std::string headxml = GetXmlHeader(task, "TaskRecordFileQuery");

	XmlParser UpXmlParser( headxml.c_str() );
	pXMLNODE ReportNode = UpXmlParser.CreateNodePtr(UpXmlParser.GetRootNode(), "TaskRecordFileQuery");
	pXMLNODE RecNode = UpXmlParser.CreateNodePtr(ReportNode, "TaskRecord");
	UpXmlParser.SetAttrNode("EquCode", std::string(""),RecNode);

	//�°���Ҫ�󷵻�channelCode
	std::string strChannelId = task->GetChannelID();
	std::string strChannelCode;
	CHANNELINFOMGR::instance()->GetChanCodeByChanId(ATV, strChannelId, strChannelCode);
	if (strChannelCode == "")
	{
		strChannelCode = strChannelId;
	}
	UpXmlParser.SetAttrNode("ChCode", strChannelCode,RecNode);
	UpXmlParser.SetAttrNode("TaskID", task->GetTaskID(),RecNode);

	pXMLNODE UrlNode = UpXmlParser.CreateNodePtr(RecNode, "Record");
	UpXmlParser.SetAttrNode("Number", std::string("0"), UrlNode);
	UpXmlParser.SetAttrNode("RecordID", 100, UrlNode);
	UpXmlParser.SetAttrNode("StartDateTime", startDateTime, UrlNode);
	UpXmlParser.SetAttrNode("EndDateTime", endDateTime, UrlNode);
	UpXmlParser.SetAttrNode("Size", 100000, UrlNode);
	UpXmlParser.SetAttrNode("URL", URL, UrlNode);
	UpXmlParser.SetAttrNode("ExpireDays", PROPMANAGER::instance()->GetTempFileExpire(), UrlNode);

	string retxml;
	UpXmlParser.SaveToString(retxml);
	return retxml;
}


std::string TranslateUpXMLForATV::TranslateQualityQuery( const XMLTask* task,std::map< string,vector<eQualityInfo> > FreqQualityInfo)
{
	std::string headerxml=GetXmlHeader(task,"QualityQuery");
	XmlParser parser(headerxml.c_str());
	pXMLNODE rootNode = parser.GetRootNode();

	size_t count=0;//ָ���������Ŀ������1000�����
	std::map< string,vector<eQualityInfo> >::iterator ptr=FreqQualityInfo.begin();
	for (;ptr!=FreqQualityInfo.end();++ptr)
	{
		count+=(*ptr).second.size();
	}
	if (count>1000)
	{
		string strURL,sharepath;
		sharepath=PROPMANAGER::instance()->GetTempSharePath();
		strURL= "http://" + PROPMANAGER::instance()->GetHttpServerIP() + sharepath+"Quality_";
		time_t curtime;
		//float freq;
		int type=0;
		float valu=0.0;
		string strtime=TimeUtil::DateTimeToString(time(0));
		string dstZipPath=PROPMANAGER::instance()->GetTempFileLoac()+"Quality_"+strtime+".zip";
		string srcFilePath=PROPMANAGER::instance()->GetTempFileLoac()+"Quality_"+strtime+".txt";
		strURL +=strtime+".zip";
		ofstream ofile(srcFilePath.c_str());
		std::map< string,vector<eQualityInfo> >::iterator itr=FreqQualityInfo.begin();
		for (;itr!=FreqQualityInfo.end();++itr)
		{
			for(vector<eQualityInfo>::iterator vecitr=(*itr).second.begin();vecitr!=(*itr).second.end();vecitr++)
			{
//				freq=StrUtil::Str2Float(vecitr->freq);
				std::string channelcode;
				CHANNELINFOMGR::instance()->GetChannelCodeByFreq(ATV,vecitr->freq,channelcode);

				curtime=TimeUtil::StrToDateTime(vecitr->checktime);
				type =StrUtil::Str2Int(vecitr->type);
				valu = StrUtil::Str2Float(vecitr->valu);

				ofile<<channelcode<<" "<<curtime<<" "<<type<<" "<<valu<<" "<<endl;
			}
		}
		ofile.close();
		OSFunction::CompressFile(dstZipPath,srcFilePath);
		pXMLNODE filenode=parser.CreateNodePtr(parser.GetRootNode(),"QualityReportFile");
		parser.SetAttrNode(string("URL"),strURL,filenode); //����ѹ�����ݴ�ŵ�ַ
		parser.SetAttrNode(string("Size"),(int)count,filenode); //ѹ�������ļ���С
		
	}
	else
	{
		pXMLNODE reportnode=parser.CreateNodePtr(parser.GetRootNode(),"QualityReport");
		for (std::map< string,vector<eQualityInfo> >::iterator ptr=FreqQualityInfo.begin();ptr!=FreqQualityInfo.end();++ptr)
		{
			std::map< string,vector<eQualityInfo> >  TimeQualityInfo;
			//��checktime������������ͬchecktime��eQualityInfo����ͬһvector������
			for (size_t i = 0;i<(*ptr).second.size();i++)
			{
				TimeQualityInfo[(*ptr).second.at(i).checktime].push_back((*ptr).second.at(i));
			}

			//���ڲ�ͬ��checktime����Quality�ڵ㼰QualityIndex�ڵ��б�
			for (std::map< string,vector<eQualityInfo> >::iterator in_ptr=TimeQualityInfo.begin();in_ptr!=TimeQualityInfo.end();++in_ptr)
			{
				try	
				{
					std::string channelcode;
					CHANNELINFOMGR::instance()->GetChannelCodeByFreq(ATV,(*ptr).first,channelcode);
					pXMLNODE qualitynode=parser.CreateNodePtr(reportnode,"Quality");
					parser.SetAttrNode(string("EquCode"),(*in_ptr).second[0].STD,qualitynode); //����Quality�ڵ��STD����
					parser.SetAttrNode(string("ChannelCode"),channelcode,qualitynode);  //����Quality�ڵ��Freq����
					parser.SetAttrNode(string("Freq"),(*ptr).first,qualitynode);  //����Quality�ڵ��Freq����
					parser.SetAttrNode(string("CheckTime"),(*in_ptr).first,qualitynode); //����Quality�ڵ��CheckTime����
					//��ͬSTD�� Freq�� CheckTime��ָ�����б�
					for (vector<eQualityInfo>::iterator pptr=(*in_ptr).second.begin();pptr!=(*in_ptr).second.end();++pptr)
					{
						//float Value=StrUtil::Str2Float((*pptr).valu);
						//(*pptr).valu=StrUtil::Float2Str(Value/1000);
						pXMLNODE qualityindexnode=parser.CreateNodePtr(qualitynode,"QualityIndex");
						parser.SetAttrNode(string("Type"),(*pptr).type,qualityindexnode); //����QualityIndex�ڵ��Type����
						parser.SetAttrNode(string("Desc"),(*pptr).desc,qualityindexnode); //����QualityIndex�ڵ��Desc����
						parser.SetAttrNode(string("Value"),(*pptr).valu,qualityindexnode); //����QualityIndex�ڵ��Value����
					}
				}
				catch(...)
				{
					string error = "����QualityReport�ӽڵ�����ʧ��";
					APPLOG::instance()->WriteLog( OTHER,LOG_EVENT_ERROR,error,LOG_OUTPUT_BOTH );
					return "";
				}

			}

		}
	}
	string rtnXML;
	parser.SaveToString(rtnXML);
	return  rtnXML;
}

std::string TranslateUpXMLForATV::TranslateClientInfoQuery(const XMLTask* task,std::vector<sVedioUserInfo>& vecClient)
{
	std::string retXml=GetXmlHeader(task,"ClientInfoQuery");
	XmlParser parser;
	parser.Set_xml(retXml);

	pXMLNODE rootNode=parser.GetRootNode();
	pXMLNODE newNode=parser.CreateNodePtr(rootNode,"ClientInfoQueryReport");

	//���ݿͻ��˵���Ϣ����xml
	if(vecClient.size()>0)
	{
		//����ClientInfoQueryReport�ڵ�
		pXMLNODE ClientNode = parser.GetNodeFromPath("Msg/ClientInfoQueryReport");
		size_t count=0;
	

		for(;count<vecClient.size();++count)
		{	sVedioUserInfo user=vecClient[count];
			if(user.fail_num==1)
				continue;
			pXMLNODE clientinfonode=parser.CreateNodePtr(ClientNode,"ClientInfo");
			parser.SetAttrNode(std::string("URL"),user.URL,clientinfonode);
			int num=0;
			for (;num < vecClient.size();++num)
			{
				if (user.URL==vecClient[num].URL)
				{
					//��д���ӿͻ���Ϣ
					pXMLNODE childnode=parser.CreateNodePtr(clientinfonode,"Client");
					parser.SetAttrNode(std::string("IP"),vecClient[num].IP,childnode);
					parser.SetAttrNode(std::string("UserName"),vecClient[num].Name,childnode);
					parser.SetAttrNode(std::string("Priority"),vecClient[num].Priority,childnode);
					parser.SetAttrNode(std::string("CenterCode"),vecClient[num].CenterCode,childnode);
					vecClient[num].fail_num=1;

				}
			}
		}
		
	}
	parser.SaveToString(retXml);

	return retXml;
}
std::string TranslateUpXMLForATV::TranslateStopClient( const XMLTask* task )
{
	std::string retXml=GetXmlHeader(task,"VideoClientStop");//��׼�ķ���ͷxml
	return retXml;
}
//��������ͼ����XML
std::string TranslateUpXMLForATV::TranslateRunPlanSet(const XMLTask* task)
{
	std::string retXml=GetXmlHeader(task,"RunPlanScan");//��׼�ķ���ͷxml
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateTaskSchedulerQuery(const  XMLTask* task,std::vector<sTaskInfo> &taskinfovec )
{
	std::string retXml=GetXmlHeader(task,"TaskSchedulerQuery");//��׼�ķ���ͷxml
	XmlParser parser(retXml.c_str());
	pXMLNODE rootNode = parser.GetRootNode();
	pXMLNODE ReportNode = parser.CreateNodePtr(rootNode,"TaskSchedulerReport");
	for(int i=0;i<taskinfovec.size();i++)
	{
		if (taskinfovec[i].taskname == "QualityTask")	//ָ��������ʾ
		{
			continue;
		}

		pXMLNODE TaskNode = parser.CreateNodePtr(ReportNode,"Task");
		parser.SetAttrNode("TaskID",taskinfovec[i].taskid,TaskNode);
		parser.SetAttrNode("MsgID",taskinfovec[i].msgid,TaskNode);
		parser.SetAttrNode("DateTime",taskinfovec[i].setdatetime,TaskNode);
		parser.SetAttrNode("SrcCode",task->GetDstCode(),TaskNode);
		parser.SetAttrNode("ChCode",taskinfovec[i].channelid,TaskNode);

		 pXMLNODE SchedulerNode = parser.CreateNodePtr(TaskNode,"Scheduler");
		 parser.SetAttrNode("State",string("Work"),SchedulerNode);
		 parser.SetAttrNode("StartDateTime",TimeUtil::DateTimeToStr(taskinfovec[i].startdatetime),SchedulerNode);
		 parser.SetAttrNode("EndDateTime",TimeUtil::DateTimeToStr(taskinfovec[i].enddatetime),SchedulerNode);
	}
	parser.SaveToString(retXml);
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateManualTaskSchedulerQuery(const  XMLTask* task,std::vector<sTaskInfo> &taskinfovec )
{
	std::string retXml=GetXmlHeader(task,"ManualTaskSchedulerQuery");//��׼�ķ���ͷxml
	XmlParser parser(retXml.c_str());
	pXMLNODE rootNode =parser.GetRootNode();
	pXMLNODE ReportNode =parser.CreateNodePtr(rootNode,"ManualTaskSchedulerReport");

	pXMLNODE SourceNode = parser.CreateNodePtr(ReportNode,"Resources");
	std::list<int> devicedlist;
	PROPMANAGER::instance()->GetTaskDeviceList(std::string("AutoRecord"), ATV,devicedlist);	//ֻ��ȡ¼��ͨ��
	std::list<int>::iterator ptr=devicedlist.begin();
	for(;ptr!=devicedlist.end();ptr++)
	{
		pXMLNODE DeviceNode = parser.CreateNodePtr(SourceNode,"DeviceID");
		parser.SetAttrNode("Code",StrUtil::Int2Str(*ptr),DeviceNode);
		for(int k=0;k<taskinfovec.size();k++)
		{
			if(taskinfovec[k].deviceid==*ptr)
			{
				pXMLNODE TaskNode = parser.CreateNodePtr(DeviceNode,"Task");
				parser.SetAttrNode("TaskID",taskinfovec[k].taskid,TaskNode);
				parser.SetAttrNode("MsgID",taskinfovec[k].msgid,TaskNode);
				parser.SetAttrNode("DateTime",taskinfovec[k].setdatetime,TaskNode);
				parser.SetAttrNode("SrcCode",task->GetDstCode(),TaskNode);
				parser.SetAttrNode("ChCode",taskinfovec[k].channelid,TaskNode);

				pXMLNODE SchedulerNode = parser.CreateNodePtr(TaskNode,"Scheduler");
				parser.SetAttrNode("State",string("Work"),SchedulerNode);
				parser.SetAttrNode("StartDateTime",TimeUtil::DateTimeToStr(taskinfovec[k].startdatetime),SchedulerNode);
				parser.SetAttrNode("EndDateTime",TimeUtil::DateTimeToStr(taskinfovec[k].enddatetime),SchedulerNode);

			}
		}
	}
	pXMLNODE ConflictsNode = parser.CreateNodePtr(ReportNode,"Conflicts");

	parser.SaveToString(retXml);

	return retXml;
}



std::string TranslateUpXMLForATV::TranslateChannelCheck(const XMLTask* task, const std::vector<sChannelInfo>& vecChanInfo)
{
	std::string retXml=GetXmlHeader(task,"TVChannelCheckReport");//��׼�ķ���ͷxml

	XmlParser retParser( retXml.c_str() );
	pXMLNODE rootNode = retParser.GetRootNode();
	pXMLNODE ChanReportNode = retParser.CreateNodePtr(rootNode, "TVChannelCheckReport");

	if ( vecChanInfo.empty() )
	{
		string tempXml;
		retParser.SaveToString( tempXml );
		return tempXml;
	}

	for (int i=0; i<vecChanInfo.size(); i++)
	{
		pXMLNODE channelNode = retParser.CreateNodePtr(ChanReportNode, "Channel");

		retParser.SetAttrNode("Code", vecChanInfo[i].ChannelCode, channelNode);
		retParser.SetAttrNode("Name", vecChanInfo[i].ProgramName, channelNode);
		retParser.SetAttrNode("Freq", vecChanInfo[i].Freq, channelNode);
	}
	
	string upXml = "";
	retParser.SaveToString(upXml);
	return upXml;
}

std::string TranslateUpXMLForATV::TranslateRecordCheck(const XMLTask* task, const std::map<int,sTaskInfo>& mapTaskInfo)
{
	std::string retXml=GetXmlHeader(task,"AutoRecordCheckReport");//��׼�ķ���ͷxml

	XmlParser retParser( retXml.c_str() );
	pXMLNODE rootNode = retParser.GetRootNode();
	pXMLNODE reportNode = retParser.CreateNodePtr(rootNode, "AutoRecordCheckReport");

	if ( mapTaskInfo.empty() )
	{
		string tempXml;
		retParser.SaveToString( tempXml );
		return tempXml;
	}

	std::map<int,sTaskInfo>::const_iterator taskIter = mapTaskInfo.begin();
	for (; taskIter!=mapTaskInfo.end(); taskIter++)
	{
		pXMLNODE channelNode = retParser.CreateNodePtr(reportNode, "Channel");

		string code = "";
		CHANNELINFOMGR::instance()->GetChannelCodeByFreq(ATV, (*taskIter).second.freq, code);
		retParser.SetAttrNode("Code", code, channelNode);
		retParser.SetAttrNode("Freq", (*taskIter).second.freq, channelNode);
		retParser.SetAttrNode("DeviceID", (*taskIter).first, channelNode);
		retParser.SetAttrNode("Bps", string("700000"), channelNode);
		retParser.SetAttrNode("Priority", (*taskIter).second.priority, channelNode);
	}

	string upXml = "";
	retParser.SaveToString(upXml);
	return upXml;
}


std::string TranslateUpXMLForATV::TranslateAlarmParamCheck(const XMLTask* task, const CodeAlarmParamMap& mapAlarmParam)
{
	std::string retXml=GetXmlHeader(task,"QualityAlarmParamCheckResp");//��׼�ķ���ͷxml

	XmlParser retParser( retXml.c_str() );
	pXMLNODE rootNode = retParser.GetRootNode();
	pXMLNODE checkNode = retParser.CreateNodePtr(rootNode, "QualityAlarmParamCheckResp");

	if (mapAlarmParam.empty())
	{
		string tempXml;
		retParser.SaveToString( tempXml );
		return tempXml;
	}

	CodeAlarmParamMap::const_iterator mapIter = mapAlarmParam.begin();
	for (; mapIter!=mapAlarmParam.end(); mapIter++)
	{
		pXMLNODE ChannelNode = retParser.CreateNodePtr(checkNode, "QualityAlarmParamChannel");
		retParser.SetAttrNode("ChannelCode", (*mapIter).first, ChannelNode);

		std::vector<sAlarmParam>::const_iterator alarmParamIter = (*mapIter).second.begin();
		for (; alarmParamIter!=(*mapIter).second.end(); alarmParamIter++)
		{
			//��Ƶ�������������ϱ�(�°���)
			if ((*alarmParamIter).TypeID=="50" || (*alarmParamIter).TypeID=="51"
				|| (*alarmParamIter).TypeDesc=="��Ƶ����" )
			{
				continue;
			}

			pXMLNODE alarmParamNode = retParser.CreateNodePtr(ChannelNode, "QualityAlarmParam");
			retParser.SetAttrNode("AlarmType", (*alarmParamIter).TypeID, alarmParamNode);
			retParser.SetAttrNode("Desc", (*alarmParamIter).TypeDesc, alarmParamNode);
			if (StrUtil::Str2Int(alarmParamIter->TypeID) < 10)
				retParser.SetAttrNode("DownThreshold", (*alarmParamIter).DownThreshold, alarmParamNode);
			else
				retParser.SetAttrNode("DownThreshold", (*alarmParamIter).Num, alarmParamNode);
			retParser.SetAttrNode("UpThreshold", (*alarmParamIter).UpThreshold, alarmParamNode);
			retParser.SetAttrNode("TimeThreshold", (*alarmParamIter).Duration, alarmParamNode);
		}
	}
	
	string upXml = "";
	retParser.SaveToString( upXml );
	return upXml;
}


std::string TranslateUpXMLForATV::TranslateRunPlanCheck(const XMLTask* task, std::string equCode, const RunPlanParamMap& mapRunplanParam)
{
	std::string retXml=GetXmlHeader(task,"RunPlanCheckReport");		//��׼�ķ���ͷxml

	XmlParser retParser( retXml.c_str() );
	pXMLNODE rootNode = retParser.GetRootNode();
	pXMLNODE reportNode = retParser.CreateNodePtr(rootNode, "RunPlanCheckReport");

	RunPlanParamMap::const_iterator cRunplanIter = mapRunplanParam.begin();
	for (; cRunplanIter!=mapRunplanParam.end(); cRunplanIter++)
	{
		pXMLNODE channelNode = retParser.CreateNodePtr(reportNode, "RunPlanChannel");
		retParser.SetAttrNode("EquCode", equCode, channelNode);
		retParser.SetAttrNode("ChCode", (*cRunplanIter).first, channelNode);

		RunPlanParamVec::const_iterator RunplanVecIter = (*cRunplanIter).second.begin();
		for (; RunplanVecIter!=(*cRunplanIter).second.end(); RunplanVecIter++)
		{
			//dayofweek,starttime,endtimeͬʱ��Ϊ�գ�����Ϊ��CycleWorkTime
			if ( (*RunplanVecIter).DayOfWeek!="" && (*RunplanVecIter).StartTime!="" && (*RunplanVecIter).EndTime!="" )
			{	
				pXMLNODE cycleNode = retParser.CreateNodePtr(channelNode, "CycleWorkTime");
				retParser.SetAttrNode("DayOfWeek", (*RunplanVecIter).DayOfWeek, cycleNode);
				retParser.SetAttrNode("StartTime", (*RunplanVecIter).StartTime, cycleNode);
				retParser.SetAttrNode("EndTime", (*RunplanVecIter).EndTime, cycleNode);
				retParser.SetAttrNode("ValidStartDateTime", (*RunplanVecIter).ValidStartDateTime, cycleNode);
				retParser.SetAttrNode("ValidEndDateTime", (*RunplanVecIter).ValidEndDateTime, cycleNode);
			}
			else
			{
				pXMLNODE singleNode = retParser.CreateNodePtr(channelNode, "SingleWorkTime");
				retParser.SetAttrNode("StartDateTime", (*RunplanVecIter).StartDateTime, singleNode);
				retParser.SetAttrNode("EndDateTime", (*RunplanVecIter).EndDateTime, singleNode);
			}
		}
	}

	string upXml = "";
	retParser.SaveToString( upXml );
	return upXml;
}

std::string TranslateUpXMLForATV::TranslateAlarmExpireTask(const XMLTask* task)
{
	std::string retXml=GetXmlHeader(task,"TaskAndAlarmExpire");//��׼�ķ���ͷxml
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateDeviceInfoQuery(const XMLTask* task)
{
	std::string retXml=GetXmlHeader(task,"DeviceInfoCheckResp");//��׼�ķ���ͷxml
	XmlParser paser(retXml.c_str());
	pXMLNODE ChildNode=paser.CreateNodePtr(paser.GetRootNode(),"DeviceInfoCheckResp");

	pXMLNODE InfoNode=paser.CreateNodePtr(ChildNode,"InfoList");
	paser.SetAttrNode("Manufactures",string("��ʯ"),InfoNode);
	paser.SetAttrNode("DeviceBatch",VSCTTB_DISPACH/*string("20120106")*/,InfoNode);
	paser.SetAttrNode("Softversion",VSCTTB_VERSION/*string("V1.9.1")*/,InfoNode);

	string upXml;
	paser.SaveToString(upXml);
	return upXml;
}

std::string TranslateUpXMLForATV::TranslateServerStatusSet(const XMLTask* task)
{
	std::string retXml=GetXmlHeader(task,"ServerStatusSet");//��׼�ķ���ͷxml
	return retXml;
}

std::string TranslateUpXMLForATV::TranslateTaskRecordFile(  const XMLTask* task,std::vector<sRecordFileInfo> FileInfo)
{
	std::string headxml = GetXmlHeader(task, "TaskRecordFileQuery");

	XmlParser UpXmlParser( headxml.c_str() );
	pXMLNODE ReportNode = UpXmlParser.CreateNodePtr(UpXmlParser.GetRootNode(), "TaskRecordFileQuery");
	pXMLNODE RecNode = UpXmlParser.CreateNodePtr(ReportNode, "TaskRecord");
	UpXmlParser.SetAttrNode("EquCode", std::string(""),RecNode);

	//�°���Ҫ�󷵻�channelCode
	std::string strChannelId = task->GetChannelID();
	std::string strChannelCode;
	CHANNELINFOMGR::instance()->GetChanCodeByChanId(ATV, strChannelId, strChannelCode);
	if (strChannelCode == "")
	{
		strChannelCode = strChannelId;
	}
	UpXmlParser.SetAttrNode("ChCode", strChannelCode,RecNode);

	UpXmlParser.SetAttrNode("TaskID", task->GetTaskID(),RecNode);

	for(int i=0;i<FileInfo.size();i++)
	{
		pXMLNODE UrlNode = UpXmlParser.CreateNodePtr(RecNode, "Record");
		UpXmlParser.SetAttrNode("Number", std::string("0"), UrlNode);
		UpXmlParser.SetAttrNode("RecordID", 100, UrlNode);
		UpXmlParser.SetAttrNode("StartDateTime", FileInfo[i].starttime, UrlNode);
		UpXmlParser.SetAttrNode("EndDateTime", FileInfo[i].endtime, UrlNode);
		UpXmlParser.SetAttrNode("Size", 100000, UrlNode);
		UpXmlParser.SetAttrNode("URL", FileInfo[i].url, UrlNode);
		UpXmlParser.SetAttrNode("ExpireDays", PROPMANAGER::instance()->GetTempFileExpire(), UrlNode);
	}
	string retxml;
	UpXmlParser.SaveToString(retxml);
	return retxml;
}