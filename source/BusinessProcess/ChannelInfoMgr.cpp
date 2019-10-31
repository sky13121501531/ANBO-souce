#include "ChannelInfoMgr.h"
#include "DownXMLProcess.h"
#include "../Communications/CommunicationMgr.h"
#include "../DBAccess/DBManager.h"
#include "../Foundation//XmlParser.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/OSFunction.h"
using namespace std;

ChannelInfoMgr::ChannelInfoMgr()
{
	InitChannelInfo();//��ʼ��
}

ChannelInfoMgr::~ChannelInfoMgr()
{
}

bool ChannelInfoMgr::InitChannelInfo()
{
	vector<string> vecChannelXml;
	if (DBMANAGER::instance()->QueryAllChannel(vecChannelXml) == false)//�����ݿ��л���������͵�xml
	{
		OSFunction::ExitProcess("ϵͳ��ʼ��(Ƶ����)ʧ��");
	}

	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	for (vector<string>::iterator ptr = vecChannelXml.begin();ptr!=vecChannelXml.end();++ptr)
	{
		GetChannelInfoFromDBXML(*ptr);//�������ݿ��е�Ƶ����Ϣxml���ɶ�Ӧ�Ľṹ����Ϣ
	}
	return true;
}

bool ChannelInfoMgr::UpdateChannelInfo(eDVBType type, std::string channelInfo) 
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
    GetChannelInfoFromReportXML(channelInfo);//����ɨ����Ƶ��xml��Ϣ����Ƶ���ṹ����Ϣ
	return  UpdateDB(type); 
}

bool ChannelInfoMgr::UpdateChannelInfo(eDVBType type, std::vector<sChannelInfo> info )
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	mChannelInfoMap[type].clear();
	for(int i=0;i!=info.size();++i)
	{
		UpdateChannelInfoMap(type,info[i]);
	}
	UpdateDB(type);  
	return true;
}
bool ChannelInfoMgr::GetAudioID(enumDVBType dvbtype,const string orgNetID,const string tsID,const string serviceID,string& audioPID)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	//�������� ����id ts��id ��serviceid�����Ƶid
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).OrgNetID == orgNetID && (*ptr).TsID == tsID && (*ptr).ServiceID == serviceID)
		{
			audioPID = (*ptr).AudioPID;
			return true;
		}
	}
	return false;
}
bool ChannelInfoMgr::GetVideoID(enumDVBType dvbtype,const string orgNetID,const string tsID,const string serviceID,string& videoPID)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	//�������� ����id ts��id ��serviceid�����Ƶid
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).OrgNetID == orgNetID && (*ptr).TsID == tsID && (*ptr).ServiceID == serviceID)
		{
			videoPID = (*ptr).VideoPID;
			return true;
		}
	}
	return false;
}
bool ChannelInfoMgr::GetFreq(enumDVBType dvbtype,const string orgNetID,const string tsID,const string serviceID,string& freq)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	//�������� ����id ts��id ��serviceid���Ƶ��
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).OrgNetID == orgNetID && (*ptr).TsID == tsID && (*ptr).ServiceID == serviceID)
		{
			freq = (*ptr).Freq;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetProName(enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID,const std::string Freq,std::string& proname)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	//�������� ����id ts��id ��serviceid���Ƶ��
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).OrgNetID == orgNetID && (*ptr).TsID == tsID && (*ptr).ServiceID == serviceID&& (*ptr).Freq == Freq)
		{
			proname = (*ptr).ProgramName;
			return true;
		}
	}
	return false;
}


bool ChannelInfoMgr::GetChannelID(enumDVBType dvbtype,const string orgNetID,const string tsID,const string serviceID,const std::string videoPID,const std::string audioPID,std::string chcode,string& channelID)
{
	//�������������Ϊ�գ��ͷ��ؿյ�channelId������ģ��������
	if (orgNetID=="" && tsID=="" && serviceID=="" && videoPID=="" && audioPID=="" && chcode=="")
	{
		channelID = "";
		return false;
	}

	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	//�������� ����id ts��id ��serviceid���Ƶ����ʶ
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).OrgNetID == orgNetID && (*ptr).TsID == tsID && (*ptr).ServiceID == serviceID/*&&ptr->VideoPID==videoPID&&ptr->AudioPID==audioPID*/&&ptr->ChannelCode==chcode)
		{
			channelID = (*ptr).ChannelID;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetChannelInfoFromReportXML(string reportxml)
{
	string strdvbtype;
	eDVBType dvbType;//����������
	string typePath="Msg";//������͵�xml·��
	string reportPath="Msg/ChannelScanReport";//Ƶ����Ϣ·��
	XmlParser xmlparser;
	if (reportxml.length()==0)
	{
		return false;
	}
	xmlparser.Set_xml(reportxml);

	//��ü������
	pXMLNODE typeNode=xmlparser.GetNodeFromPath((char*)typePath.c_str());
	xmlparser.GetAttrNode(typeNode,"DVBType",strdvbtype);

	dvbType = OSFunction::GetEnumDVBType(strdvbtype);
	
	try//���Ƶ����Ϣ
	{
		if (CTTB==dvbType||DVBC==dvbType||DVBS==dvbType||THREED==dvbType)
		{
			pXMLNODE curNode=xmlparser.GetNodeFromPath((char*)reportPath.c_str());
			pXMLNODELIST nodeList=xmlparser.GetNodeList(curNode);
			for (int i=0;i<nodeList->Size();++i)
			{
				curNode=xmlparser.GetNextNode(nodeList);
				sChannelInfo channel;
				//���xmlͷ���������Ϣ
				xmlparser.GetAttrNode(curNode,"STD",channel.STD);
				xmlparser.GetAttrNode(curNode,"Freq",channel.Freq);
				xmlparser.GetAttrNode(curNode,"SymbolRate",channel.SymbolRate);
				xmlparser.GetAttrNode(curNode,"QAM",channel.QAM);
				xmlparser.GetAttrNode(curNode,"HeaderMode",channel.HeaderMode);
				xmlparser.GetAttrNode(curNode,"CarrierNumber",channel.CarrierNumber);
				xmlparser.GetAttrNode(curNode,"EncodeEfficiency",channel.EncodeEfficiency);
				xmlparser.GetAttrNode(curNode,"InterwovenMode",channel.InterwovenMode);
				xmlparser.GetAttrNode(curNode,"DoublePilot",channel.DoublePilot);
				xmlparser.GetAttrNode(curNode,"PnPhase",channel.PnPhase);
				xmlparser.GetAttrNode(curNode,"OrgNetID",channel.OrgNetID);
				xmlparser.GetAttrNode(curNode,"TsID",channel.TsID);

				pXMLNODELIST newlist=xmlparser.GetNodeList(curNode);
				for (int j=0;j<newlist->Size();++j)
				{
					//��ý�Ŀ��Ϣ
					pXMLNODE node=xmlparser.GetNextNode(newlist);
					xmlparser.GetAttrNode(node,"PmtPid",channel.PmtPID);
					xmlparser.GetAttrNode(node,"PcrPid",channel.PcrPID);
					xmlparser.GetAttrNode(node,"Program",channel.ProgramName);
					xmlparser.GetAttrNode(node,"ServiceID",channel.ServiceID);
					xmlparser.GetAttrNode(node,"VideoPID",channel.VideoPID);
					xmlparser.GetAttrNode(node,"AudioPID",channel.AudioPID);
					xmlparser.GetAttrNode(node,"Encryption",channel.Encryption);
					xmlparser.GetAttrNode(node,"HDTV",channel.HDTV);

					UpdateChannelInfoMap(dvbType,channel);
					channel.ChannelID = "";
				}
			}
			return true;
		}
		else if (ATV==dvbType || RADIO==dvbType || AM==dvbType || CTV==dvbType)
		{
			pXMLNODE curNode=xmlparser.GetNodeFromPath("Msg/ChannelScan");
			pXMLNODELIST nodeList=xmlparser.GetNodeList(curNode);
			for (int i=0;i<xmlparser.GetChildCount(curNode);++i)
			{
				pXMLNODE node=xmlparser.GetNextNode(nodeList);
				string freq,chcode,programname;
				sChannelInfo channelinfo;
				
				xmlparser.GetAttrNode(node,"Freq",freq);
				xmlparser.GetAttrNode(node,"CHCode",chcode);
				xmlparser.GetAttrNode(node,"CHName",programname);
				channelinfo.Freq=freq;
				channelinfo.ChannelID=freq;
				channelinfo.ChannelCode=chcode;
				channelinfo.ProgramName=programname;
				UpdateChannelInfoMap(dvbType,channelinfo);
			}
			return true;
		}
		
	}
	catch(...){
		return false;
	}
	return false;
}
bool ChannelInfoMgr::GetChannelInfoFromDBXML(string dbxml)
{
	string strdvbtype;
	eDVBType dvbType;//����������
	string typePath="Msg/DVBType";//������͵�xml·��
	XmlParser xmlparser;
	if (dbxml.length()==0)
	{
		return false;
	}
	xmlparser.Set_xml(dbxml);
	try
	{

		//��ü������
		pXMLNODE typeNode=xmlparser.GetNodeFromPath((char*)typePath.c_str());
		xmlparser.GetAttrNode(typeNode,"Type",strdvbtype);
		dvbType = OSFunction::GetEnumDVBType(strdvbtype);
		
		if (CTTB==dvbType||DVBC==dvbType||DVBS==dvbType||THREED==dvbType)
		{
			//���Ƶ����Ϣ
			pXMLNODE curNode=xmlparser.GetNodeFromPath("Msg");
			pXMLNODELIST nodeList=xmlparser.GetNodeList(curNode);

			for (int i=0;i<nodeList->Size();++i)
			{
				curNode=xmlparser.GetNextNode(nodeList);
				if (xmlparser.GetNodeName(curNode)=="ChannelScan")
				{
					sChannelInfo channel;
					//���xmlͷ��Ϣ
					xmlparser.GetAttrNode(curNode,"STD",channel.STD);
					xmlparser.GetAttrNode(curNode,"Freq",channel.Freq);
					xmlparser.GetAttrNode(curNode,"SymbolRate",channel.SymbolRate);
					xmlparser.GetAttrNode(curNode,"QAM",channel.QAM);
					xmlparser.GetAttrNode(curNode,"HeaderMode",channel.HeaderMode);
					xmlparser.GetAttrNode(curNode,"CarrierNumber",channel.CarrierNumber);
					xmlparser.GetAttrNode(curNode,"EncodeEfficiency",channel.EncodeEfficiency);
					xmlparser.GetAttrNode(curNode,"InterwovenMode",channel.InterwovenMode);
					xmlparser.GetAttrNode(curNode,"DoublePilot",channel.DoublePilot);
					xmlparser.GetAttrNode(curNode,"PnPhase",channel.PnPhase);
					xmlparser.GetAttrNode(curNode,"OrgNetID",channel.OrgNetID);
					xmlparser.GetAttrNode(curNode,"TsID",channel.TsID);

					pXMLNODELIST newlist=xmlparser.GetNodeList(curNode);//�����ӽڵ����
					for (int j=0;j<newlist->Size();++j)
					{
						pXMLNODE node=xmlparser.GetNextNode(newlist);
						//���xml�еĽ�Ŀ��Ϣ
						xmlparser.GetAttrNode(node,"ChannelID",channel.ChannelID);
						xmlparser.GetAttrNode(node,"PmtPid",channel.PmtPID);
						xmlparser.GetAttrNode(node,"PcrPid",channel.PcrPID);
						xmlparser.GetAttrNode(node,"Program",channel.ProgramName);
						xmlparser.GetAttrNode(node,"ServiceID",channel.ServiceID);
						xmlparser.GetAttrNode(node,"VideoPID",channel.VideoPID);
						xmlparser.GetAttrNode(node,"AudioPID",channel.AudioPID);
						xmlparser.GetAttrNode(node,"Encryption",channel.Encryption);
						xmlparser.GetAttrNode(node,"HDTV",channel.HDTV);
						UpdateChannelInfoMap(dvbType,channel);//����Ƶ����Ϣ
					}
				}
			}
			return true;
		}
		else if (ATV==dvbType || RADIO==dvbType || AM==dvbType || CTV==dvbType)
		{
			sChannelInfo channelinfo;
			pXMLNODE ChannelScan=xmlparser.GetNodeFromPath("Msg/Channel");
		
			string freq,chcode,programname;
			
			xmlparser.GetAttrNode(ChannelScan,"Freq",freq);
			xmlparser.GetAttrNode(ChannelScan,"CHCode",chcode);
			xmlparser.GetAttrNode(ChannelScan,"CHName",programname);
			channelinfo.ChannelID=freq;
			channelinfo.Freq=freq;
			channelinfo.ChannelCode=chcode;
			channelinfo.ProgramName=programname;
			UpdateChannelInfoMap(dvbType,channelinfo);//����Ƶ����Ϣ
		}
		
	}
	catch(...)
	{
		return false;
	}
	return false;
}

bool ChannelInfoMgr::UpdateChannelInfoMap(const eDVBType& dvbtype,sChannelInfo& channelinfo)
{
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr) == channelinfo)//�������Ƶ����Ϣ���(���ܴ��������ֶβ�ͬ���������ݸ���)
		{
			if (dvbtype == CTTB||dvbtype==DVBC||dvbtype==DVBS||dvbtype==THREED)
			{
				(*ptr).ChannelID = (*ptr).OrgNetID + (*ptr).TsID + (*ptr).ServiceID+(*ptr).VideoPID+(*ptr).AudioPID;
			}
			channelinfo.ChannelID = (*ptr).ChannelID;
			(*ptr) = channelinfo;
			break;
		}
	}
	if (ptr == mChannelInfoMap[dvbtype].end())//û���������
	{
		if (dvbtype == CTTB||dvbtype==DVBC||dvbtype==DVBS||dvbtype==THREED)
		{
			channelinfo.ChannelID = channelinfo.OrgNetID + channelinfo.TsID + channelinfo.ServiceID+channelinfo.VideoPID+channelinfo.AudioPID;//����Ƶ��id������Ϊԭʼ����id��ts��id��serviceid�����
		}
		mChannelInfoMap[dvbtype].push_back(channelinfo);//���뵽�ڴ�
	}
	return true;
}

bool ChannelInfoMgr::CreatDBXML(const eDVBType& dvbtype,sChannelInfo& channel,string& xml)
{
	XmlParser xmlparser;
	xml ="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\" ?> ";//xmlͷ
	xml += "<Msg></Msg>";

	xmlparser.Set_xml(xml);
	xmlparser.CreateNode("Msg","DVBType");

	pXMLNODE node=xmlparser.GetNodeFromPath("Msg/DVBType");
	string type = OSFunction::GetStrDVBType(dvbtype);
	
	try
	{
		xmlparser.SetAttrNode("Type",type,node);
		//����Ƶ���ṹ����Ϣ����xml�������
		//����xmlͷ���������Ϣ
		
		if ("CTTB"==type||"DVBC"==type||"DVBS"==type||"THREED"==type)
		{
			pXMLNODE ChannelScanNode=xmlparser.CreateNodePtr("Msg","ChannelScan");
			xmlparser.SetAttrNode("STD",channel.STD,ChannelScanNode);
			xmlparser.SetAttrNode("Freq",channel.Freq,ChannelScanNode);
			xmlparser.SetAttrNode("SymbolRate",channel.SymbolRate,ChannelScanNode);
			xmlparser.SetAttrNode("QAM",channel.QAM,ChannelScanNode);
			xmlparser.SetAttrNode("HeaderMode",channel.HeaderMode,ChannelScanNode);
			xmlparser.SetAttrNode("CarrierNumber",channel.CarrierNumber,ChannelScanNode);
			xmlparser.SetAttrNode("EncodeEfficiency",channel.EncodeEfficiency,ChannelScanNode);
			xmlparser.SetAttrNode("InterwovenMode",channel.InterwovenMode,ChannelScanNode);
			xmlparser.SetAttrNode("DoublePilot",channel.DoublePilot,ChannelScanNode);
			xmlparser.SetAttrNode("PnPhase",channel.PnPhase,ChannelScanNode);
			xmlparser.SetAttrNode("OrgNetID",channel.OrgNetID,ChannelScanNode);
			xmlparser.SetAttrNode("TsID",channel.TsID,ChannelScanNode);

			pXMLNODE ChannelNode=xmlparser.CreateNodePtr(ChannelScanNode,"Channel");
			//����xmlƵ����Ϣ
			xmlparser.SetAttrNode("PmtPid",channel.PmtPID,ChannelNode);
			xmlparser.SetAttrNode("PcrPid",channel.PcrPID,ChannelNode);
			xmlparser.SetAttrNode("ChannelID",channel.ChannelID,ChannelNode);
			xmlparser.SetAttrNode("Program",channel.ProgramName,ChannelNode);
			xmlparser.SetAttrNode("ServiceID",channel.ServiceID,ChannelNode);
			xmlparser.SetAttrNode("VideoPID",channel.VideoPID,ChannelNode);
			xmlparser.SetAttrNode("AudioPID",channel.AudioPID,ChannelNode);
			xmlparser.SetAttrNode("Encryption",channel.Encryption,ChannelNode);
			xmlparser.SetAttrNode("HDTV",channel.HDTV,ChannelNode);
		}
		else
		{
			pXMLNODE channelnode=xmlparser.CreateNodePtr("Msg","Channel");
			xmlparser.SetAttrNode("Freq",channel.Freq,channelnode);
			xmlparser.SetAttrNode("ChannelID",channel.ChannelID,channelnode);
			xmlparser.SetAttrNode("CHCode",channel.ChannelCode,channelnode);
			xmlparser.SetAttrNode("CHName",channel.ProgramName,channelnode);
		}
		
		xmlparser.Get_xml(xml);

		return true;
	}
	catch (...)
	{
		return false;
	}
	
}

bool ChannelInfoMgr::UpdateDB(eDVBType dvbtype) 
{
	ChannelInfoMap::iterator ptr = mChannelInfoMap.find(dvbtype);  
	if (ptr == mChannelInfoMap.end())
		return true;
	DBMANAGER::instance()->DeleteChannelByType(dvbtype);
	for (vector<sChannelInfo>::iterator in_ptr = ptr->second.begin();in_ptr!=ptr->second.end();++in_ptr)
	{
		string dbchannelxml = "";
		CreatDBXML(ptr->first,*in_ptr,dbchannelxml);//�������xml
		DBMANAGER::instance()->AddChannel(ptr->first,in_ptr->ChannelID,dbchannelxml);//��ӵ����ݿ�
	}
	return true;
}

bool ChannelInfoMgr::UpdateChannelByID(eDVBType type ,std::vector<std::string> channelvec )
{
	std::vector<std::string> vecDBChannelID;
	DBMANAGER::instance()->QueryChannelID( type,vecDBChannelID );//�������ͻ�����е�Ƶ��id
	bool ret=true;
	vector< string > ::iterator ptr = vecDBChannelID.begin();
	for ( ; ptr!= vecDBChannelID.end(); ++ptr )
	{
		vector< string > ::iterator in_ptr = channelvec.begin();
		for ( ;in_ptr!=channelvec.end();++in_ptr )
		{
			if (( * ptr ) == (* in_ptr))			
				break;
		}
		if ( in_ptr == channelvec.end())
		{
			//�������ݿ�
			ret=DBMANAGER::instance()->DeleteChannel(type,(*ptr));
			if (!ret)
			{
				break;
			}
			ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
			//�����ڴ���Ϣ
			vector<sChannelInfo>::iterator ptrch = mChannelInfoMap[type].begin();
			for (;ptrch!= mChannelInfoMap[type].end();++ptrch)
			{
				if ((*ptrch).ChannelID == (*ptr))
				{
					mChannelInfoMap[type].erase(ptrch);
					break;
				}
			}
		}
	}
	return ret;
}

bool ChannelInfoMgr::GetOrgNetID( enumDVBType dvbtype,const std::string freq,const std::string audiopid,const std::string videopid,const std::string serviceid,std::string& orgnetid )
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).Freq == freq && (*ptr).AudioPID == audiopid && (*ptr).VideoPID == videopid && (*ptr).ServiceID == serviceid)
		{
			orgnetid = (*ptr).OrgNetID;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetTsID( enumDVBType dvbtype,const std::string freq,const std::string audiopid,const std::string videopid,const std::string serviceid,std::string& tsid )
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).Freq == freq && (*ptr).AudioPID == audiopid && (*ptr).VideoPID == videopid && (*ptr).ServiceID == serviceid)
		{
			tsid = (*ptr).TsID;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::IsHDTV( enumDVBType dvbtype,const std::string orgNetID,const std::string tsID,const std::string serviceID )
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).OrgNetID == orgNetID && (*ptr).TsID == tsID && (*ptr).ServiceID == serviceID)
		{
			if ((*ptr).HDTV=="1")
				return true;
			else
				return false;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetChannelInfo( enumDVBType dvbtype,const std::string channelid,std::string& vpid,std::string& apid,std::string& serviceid,std::string& prgname )
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).ChannelID == channelid)
		{
			vpid=(*ptr).VideoPID;
			apid=(*ptr).AudioPID;
			serviceid=(*ptr).ServiceID;
			prgname=(*ptr).ProgramName;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetFreqByChannelCode( enumDVBType dvbtype,std::string code,std::string&freq )
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).ChannelCode == code)
		{
			freq=ptr->Freq;
			return true;
		}
	}
	return false;
}
bool ChannelInfoMgr::GetProNameByFreq(enumDVBType dvbtype,std::string freq,std::string&name)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).Freq == freq)
		{
			name=ptr->ProgramName;
			return true;
		}
	}
	return false;
}
bool ChannelInfoMgr::GetChannelCodeByFreq(enumDVBType dvbtype,std::string freq,std::string&channelcode)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).Freq == freq)
		{
			channelcode=ptr->ChannelCode;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetSymbolRateQAMByFreq(enumDVBType dvbtype,std::string freq,std::string& symbolrate,std::string& qam)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).Freq == freq)
		{
			symbolrate=ptr->SymbolRate;
			qam =ptr->QAM;
			return true;
		}
	}
	return false;
}

bool ChannelInfoMgr::GetPmtPidPcrPid(enumDVBType dvbtype,std::string freq,std::string serviceid,std::string& pmtpid,std::string& pcrpid)
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);
	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[dvbtype].begin();
	for (;ptr!= mChannelInfoMap[dvbtype].end();++ptr)
	{
		if ((*ptr).Freq == freq && (*ptr).ServiceID == serviceid)
		{
			pmtpid=ptr->PmtPID;
			pcrpid =ptr->PcrPID;
			return true;
		}
	}
	return false;
}


//��ȡĳ��������͵�Ƶ����Ϣ
bool ChannelInfoMgr::GetChannelInfoByDvbtype(enumDVBType eDvbtype, std::vector<sChannelInfo>& vecChanInfo )	
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);

	//û�иü�����͵�Ƶ������Ϊʧ��
	if ( mChannelInfoMap.find(eDvbtype)==mChannelInfoMap.end() )
		return false;
	
	vecChanInfo = mChannelInfoMap[eDvbtype];
	return true;
}

bool ChannelInfoMgr::GetFreqInfoByDvbtype(enumDVBType eDvbtype, std::vector<std::string>& vecFreqInfo )	
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);

	//û�иü�����͵�Ƶ������Ϊʧ��
	if ( mChannelInfoMap.find(eDvbtype)==mChannelInfoMap.end() )
		return false;
	std::vector<sChannelInfo> vecChanInfo=mChannelInfoMap[eDvbtype];
	for(int i=0;i<vecChanInfo.size();i++)
	{
		vecFreqInfo.push_back(vecChanInfo[i].Freq);
	}
	return true;
}

//ͨ��chanId��ȡchanCode
bool ChannelInfoMgr::GetChanCodeByChanId(enumDVBType eDvbtype, const std::string chanId, std::string& chanCode)	
{
	ACE_Guard<ACE_Thread_Mutex> guard(ChannelMapMutex);

	if ( mChannelInfoMap.find(eDvbtype) == mChannelInfoMap.end() )
		return false;

	vector<sChannelInfo>::iterator ptr = mChannelInfoMap[eDvbtype].begin();
	for (;ptr!= mChannelInfoMap[eDvbtype].end();++ptr)
	{
		if ( (*ptr).ChannelID == chanId )
		{
			chanCode = (*ptr).ChannelCode;
			return true;
		}
	}
	return false;
}