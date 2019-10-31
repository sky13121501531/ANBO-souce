#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include "../Foundation/PropManager.h"
#include "../Foundation/XmlParser.h"
#include "../Foundation/TimeUtil.h"
#include "../Foundation/OSFunction.h"
#include "PSISIParser.h"
#include <fstream>
#include <algorithm>
#include <iostream>
#include "../DBAccess/DBManager.h"
#include "../Foundation/StrUtil.h"
using namespace std;

/*	对EIT信息进行排序，使生成的epg信息有序
*	优先按照serviceId排序，serviceId相同，按照开始时间排序
*/
bool less_EIT(const EIT & eitInfo1, const EIT & eitInfo2) 
{
	if (eitInfo1.ServiceID < eitInfo2.ServiceID)
	{
		return true;
	}
	else if (eitInfo1.ServiceID > eitInfo2.ServiceID)
	{
		return false;
	}
	else if (eitInfo1.ServiceID == eitInfo2.ServiceID)
	{
		if (eitInfo1.ProgramStartTime < eitInfo2.ProgramStartTime)
		{
			return true;
		}
		else
			return false;
	}
	else
		return true;
}


PSISIParser::PSISIParser()
{
}
PSISIParser::PSISIParser(eDVBType dvbtype)
{
	PROPMANAGER::instance()->GetXmlTablePath(dvbtype,Path);
	DVBType=dvbtype;
}
PSISIParser::~PSISIParser()
{

}
bool PSISIParser::CreateChannelXML()
{
	ACE_Guard<ACE_Thread_Mutex> guard(Mutex);
	string filename = Path + string("ChannelScanQuery.xml");
	char * strXML = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
	XmlParser pXP(strXML);

	pXMLNODE rootNode = pXP.GetRootNode();
	if(DVBType==DVBC)
	{
		pXP.SetAttrNode(string("DVBType"),string("DVBC"),rootNode);
	}
	else if(DVBType==CTTB)
	{
		pXP.SetAttrNode(string("DVBType"),string("CTTB"),rootNode);
	}
	else if(DVBType==DVBS)
	{
		pXP.SetAttrNode(string("DVBType"),string("DVBS"),rootNode);
	}
	else if(DVBType==THREED)
	{
		pXP.SetAttrNode(string("DVBType"),string("THREED"),rootNode);
	}
	pXMLNODE  returnNode = pXP.CreateNodePtr(rootNode,"Return");   //创建Return节点
	pXP.SetAttrNode(string("Type"),string("ChannelScanQuery"),returnNode);
	pXP.SetAttrNode(string("Value"),string("0"),returnNode);
	pXP.SetAttrNode(string("Desc"),string("成功"),returnNode);
	pXP.SetAttrNode(string("Comment"),string(""),returnNode);

	pXMLNODE reportNode = pXP.CreateNodePtr(rootNode,"ChannelScanReport");

	if (DVBType == THREED)
	{
		if (mapSDT.empty())
		{
			cout<<"获得节目信息错误或不完整"<<endl;
			pXP.SaveAsFile(filename.c_str());
			return false;
		}
		else
		{
			mapSDTInfo::iterator mapIter = mapSDT.begin();
			while (mapIter != mapSDT.end())
			{
				/* 没有sdt信息的频点直接略过 */
				if ((mapIter->second).empty())
				{
					continue;
				}

				std::string freq3D = mapIter->first;
				bool hasCreateScanNode = false;		
				pXMLNODE scanNode = NULL;
				std::vector<SDT>::iterator sdtIter = (mapIter->second).begin();
				for (; sdtIter!=(mapIter->second).end(); sdtIter++)
				{
					if (sdtIter->ServiceName.find("3D") != std::string::npos)	//节目名包含3D
					{
						/* ChannelScan节点及其相关属性创建 */
						if (!hasCreateScanNode)		//如果同一频点还没有创建过ChannelScan节点
						{
							scanNode = pXP.CreateNodePtr(reportNode,"ChannelScan");    //创建ChannelScan节点
							pXP.SetAttrNode(string("STD"),string(""),scanNode);
							pXP.SetAttrNode(string("HeaderMode"),string(""),scanNode);
							pXP.SetAttrNode(string("CarrierNumber"),string(""),scanNode);
							pXP.SetAttrNode(string("EncodeEfficiency"),string(""),scanNode);
							pXP.SetAttrNode(string("InterwovenMode"),string(""),scanNode);
							pXP.SetAttrNode(string("DoublePilot"),string(""),scanNode);
							pXP.SetAttrNode(string("PnPhase"),string(""),scanNode);
							pXP.SetAttrNode(string("Freq"),freq3D,scanNode);

							/* 从NIT表中获取symbolrate,qam,orgnetId,tsId */
							vector<NIT>::iterator nit_ptr = vecNIT.begin();
							for (;nit_ptr!=vecNIT.end();nit_ptr++)
							{
								if ((*nit_ptr).Freq == freq3D)
								{
									pXP.SetAttrNode(string("SymbolRate"),(*nit_ptr).SymbolRate,scanNode);
									pXP.SetAttrNode(string("QAM"),(*nit_ptr).QAM,scanNode);
									pXP.SetAttrNode(string("OrgNetID"),(*nit_ptr).OrgNetID,scanNode);
									pXP.SetAttrNode(string("TsID"),(*nit_ptr).TsID,scanNode);
									break;
								}
							}
							if (nit_ptr == vecNIT.end())
							{
								pXP.SetAttrNode(string("SymbolRate"),string("6875"),scanNode);
								pXP.SetAttrNode(string("QAM"),string("3"),scanNode);
								pXP.SetAttrNode(string("OrgNetID"),string(""),scanNode);
								pXP.SetAttrNode(string("TsID"),string(""),scanNode);
							}
							
							hasCreateScanNode = true;	//置为true，避免重复创建ChannelScan节点
						}	//!hasCreateScanNode
						
						pXMLNODE channelNode = pXP.CreateNodePtr(scanNode, "Channel");

						/* 从PAT表中获取ProgramID */
						vector<PAT>::iterator pat_ptr = mapPAT[freq3D].begin();
						for (;pat_ptr!=mapPAT[freq3D].end();pat_ptr++)
						{
							if ((*pat_ptr).ProgramNumber == sdtIter->ServiceID)
							{
								pXP.SetAttrNode(string("ProgramID"),(*pat_ptr).ProgramID,channelNode);
								break;
							}
						}
						if (pat_ptr==mapPAT[freq3D].end())
						{
							pXP.SetAttrNode(string("ProgramID"),string(""),channelNode);
						}

						std::string programName3D = sdtIter->ServiceName;
						pXP.SetAttrNode("Program", programName3D, channelNode);
						pXP.SetAttrNode("Encryption", sdtIter->FreeCAMode, channelNode);
						/* 根据频道名判断是否高清 */
						if (programName3D.find("高清") != std::string::npos &&
							programName3D.find("HD") != string::npos)
						{
							pXP.SetAttrNode("HDTV", std::string("1"), channelNode);
						}
						else
							pXP.SetAttrNode("HDTV", std::string("0"), channelNode);

						/* 从PMT表中获取流相关节目信息 */
						std::vector<PMT>::iterator pmt3DIter = mapPMT[freq3D].begin();
						for (; pmt3DIter!=mapPMT[freq3D].end(); pmt3DIter++)
						{
							if (pmt3DIter->ProgramNumber == sdtIter->ServiceID)
							{
								pXP.SetAttrNode(string("PmtPid"),(*pmt3DIter).PmtPID,channelNode);  //设置pmtpid属性值 
								pXP.SetAttrNode(string("PcrPid"),(*pmt3DIter).PcrPID,channelNode);		 //设置pcrpid属性值

								pXP.SetAttrNode(string("ServiceID"),(*pmt3DIter).ProgramNumber,channelNode);  //设置ServiceID属性值 
								pXP.SetAttrNode(string("VideoPID"),(*pmt3DIter).VideoPID,channelNode);		 //设置VideoPID属性值
								pXP.SetAttrNode(string("AudioPID"),(*pmt3DIter).AudioPID,channelNode);		 //设置AudioPID属性值
								pXP.SetAttrNode(string("Encrypt"),string("0"),channelNode);

								vector<STREAM>::iterator stream3DIter = (*pmt3DIter).vecStream.begin();
								for(; stream3DIter!=(*pmt3DIter).vecStream.end(); stream3DIter++)
								{
									pXMLNODE StreamNode = pXP.CreateNodePtr(channelNode,"elementryPID");
									pXP.SetAttrNode("StreamType",(*stream3DIter).StreamType,StreamNode);
									pXP.SetAttrNode("PID",(*stream3DIter).StreamPid,StreamNode);
								}
							}
						}
					}	//节目名是否包含3D
				}	//for loop

				mapIter ++;
			}	//while loop
		}	//mapSDT not empty

		std::string tempstr;
		pXP.SaveToString( tempstr );
		pXP.SaveAsFile(filename.c_str());
		return true;
	}
	if (mapPAT.empty())
	{
		cout<<"获得节目信息错误或不完整"<<endl;
		pXP.SaveAsFile(filename.c_str());
		return false;
	}
	else
	{
		map< string,vector<PAT> >::iterator ptr = mapPAT.begin();
		for (;ptr!= mapPAT.end();ptr++)
		{
			string OrgNetID,TsID;
			pXMLNODE scanNode = pXP.CreateNodePtr(reportNode,"ChannelScan");    //创建ChannelScan节点
			pXP.SetAttrNode(string("STD"),string(""),scanNode);
			//pXP.SetAttrNode(string("QAM"),string("64QAM"),scanNode);
			pXP.SetAttrNode(string("HeaderMode"),string(""),scanNode);
			pXP.SetAttrNode(string("CarrierNumber"),string(""),scanNode);
			pXP.SetAttrNode(string("EncodeEfficiency"),string(""),scanNode);
			pXP.SetAttrNode(string("InterwovenMode"),string(""),scanNode);
			pXP.SetAttrNode(string("DoublePilot"),string(""),scanNode);
			pXP.SetAttrNode(string("PnPhase"),string(""),scanNode);

			pXP.SetAttrNode(string("Freq"),(*ptr).first,scanNode);

			vector<NIT>::iterator nit_ptr = vecNIT.begin();
			for (;nit_ptr!=vecNIT.end();nit_ptr++)
			{
				if ((*nit_ptr).Freq == (*ptr).first)
				{
					pXP.SetAttrNode(string("SymbolRate"),(*nit_ptr).SymbolRate,scanNode);
					pXP.SetAttrNode(string("QAM"),(*nit_ptr).QAM,scanNode);
					break;
				}
			}
			if (nit_ptr == vecNIT.end())
			{
				pXP.SetAttrNode(string("SymbolRate"),string("6875"),scanNode);
				pXP.SetAttrNode(string("QAM"),string("3"),scanNode);
			}

			if (mapSDT.find((*ptr).first) == mapSDT.end())
			{
				vector<NIT>::iterator in_ptr = vecNIT.begin();
				for (;in_ptr!=vecNIT.end();in_ptr++)
				{
					if ((*in_ptr).Freq == (*ptr).first)
					{
						pXP.SetAttrNode(string("OrgNetID"),(*in_ptr).OrgNetID,scanNode);
						pXP.SetAttrNode(string("TsID"),(*in_ptr).TsID,scanNode);
						OrgNetID = (*in_ptr).OrgNetID;
						TsID = (*in_ptr).TsID;

						break;
					}
				}
				if (in_ptr == vecNIT.end())
				{
					pXP.SetAttrNode(string("OrgNetID"),string(""),scanNode);
					pXP.SetAttrNode(string("TsID"),string(""),scanNode);
					OrgNetID = "";
					TsID = "";
				}
			}
			else
			{
				if (mapSDT[(*ptr).first].empty())
				{
					pXP.SetAttrNode(string("OrgNetID"),string(""),scanNode);
					pXP.SetAttrNode(string("TsID"),string(""),scanNode);					
					OrgNetID = "";
					TsID = "";
				}
				else
				{
					pXP.SetAttrNode(string("OrgNetID"),mapSDT[(*ptr).first][0].OrgNetID,scanNode);
					pXP.SetAttrNode(string("TsID"),mapSDT[(*ptr).first][0].TsID,scanNode);						
					OrgNetID = mapSDT[(*ptr).first][0].OrgNetID;
					TsID = mapSDT[(*ptr).first][0].TsID;
				}
			}
			if ((*ptr).second.empty())
			{
				cout<<"获得频点"<<(*ptr).first<<"中节目信息错误或不完整"<<endl;
				continue;
			}
			else
			{
				if (mapPMT.find((*ptr).first) == mapPMT.end())
				{
					continue;
				}
				else
				{
					for (size_t m = 0; m < mapPMT[(*ptr).first].size(); m++)
					{
						try
						{
							pXMLNODE channelNode = pXP.CreateNodePtr(scanNode,"Channel");  //创建Channel节点

							vector<PAT>::iterator pat_ptr = mapPAT[(*ptr).first].begin();
							for (;pat_ptr!=mapPAT[(*ptr).first].end();pat_ptr++)
							{
								if ((*pat_ptr).ProgramNumber ==mapPMT[(*ptr).first][m].ProgramNumber )
								{
									pXP.SetAttrNode(string("ProgramID"),(*pat_ptr).ProgramID,channelNode);
									break;
								}
							}
							if (pat_ptr==mapPAT[(*ptr).first].end())
							{
								pXP.SetAttrNode(string("ProgramID"),string(""),channelNode);
							}

							if (mapSDT.find((*ptr).first) == mapSDT.end())
							{
								pXP.SetAttrNode(string("Program"),string(""),channelNode);
								pXP.SetAttrNode(string("Encryption"),string(""),channelNode);
							}
							else
							{
								size_t p = 0;
								for ( ;p< mapSDT[(*ptr).first].size();p++)
								{
									if (mapPMT[(*ptr).first][m].ProgramNumber == mapSDT[(*ptr).first][p].ServiceID)
									{
										pXP.SetAttrNode(string("Program"),mapSDT[(*ptr).first][p].ServiceName,channelNode);   //设置Program属性值 
										pXP.SetAttrNode(string("Encryption"),mapSDT[(*ptr).first][p].FreeCAMode,channelNode); //设置Encryption属性值 
										//gaoxd 2011.03.21 暂时用名称判断高标清
										string servicename = mapSDT[(*ptr).first][p].ServiceName;
										if (servicename.find("高清") != string::npos || servicename.find("HD") != string::npos)
										{
											pXP.SetAttrNode(string("HDTV"),string("1"),channelNode);
										}
										else
										{
											pXP.SetAttrNode(string("HDTV"),string("0"),channelNode);
										}
										break;
									}
								}
								if (p ==mapSDT[(*ptr).first].size())
								{
									pXP.SetAttrNode(string("Program"),string(""),channelNode);
									pXP.SetAttrNode(string("Encryption"),string(""),channelNode);
								}
							}
							
							pXP.SetAttrNode(string("PmtPid"),mapPMT[(*ptr).first][m].PmtPID,channelNode);  //设置pmtpid属性值 
							pXP.SetAttrNode(string("PcrPid"),mapPMT[(*ptr).first][m].PcrPID,channelNode);		 //设置pcrpid属性值

							pXP.SetAttrNode(string("ServiceID"),mapPMT[(*ptr).first][m].ProgramNumber,channelNode);  //设置ServiceID属性值 
							pXP.SetAttrNode(string("VideoPID"),mapPMT[(*ptr).first][m].VideoPID,channelNode);		 //设置VideoPID属性值
							pXP.SetAttrNode(string("AudioPID"),mapPMT[(*ptr).first][m].AudioPID,channelNode);		 //设置AudioPID属性值
							pXP.SetAttrNode(string("Encrypt"),string("0"),channelNode);
							

							//vector<HDTV>::iterator in_ptr = vecHDTV.begin();
							//for (;in_ptr!=vecHDTV.end();in_ptr++)
							//{
							//	if ((*in_ptr).Freq == (*ptr).first&&/*(*in_ptr).OrgNetID == OrgNetID&&*/(*in_ptr).TsID == TsID &&(*in_ptr).ServiceID ==mapPMT[(*ptr).first][m].ProgramNumber )
							//	{
							//		pXP.SetAttrNode(string("HDTV"),string("1"),channelNode);
							//		break;
							//	}
							//}
							//if (in_ptr == vecHDTV.end())//gaoxd 2011.03.21
							//{
							//	pXP.SetAttrNode(string("HDTV"),string("0"),channelNode);
							//}

							vector<STREAM>::iterator pStream = mapPMT[(*ptr).first][m].vecStream.begin();
							for(;pStream!=mapPMT[(*ptr).first][m].vecStream.end();pStream++)
							{
								pXMLNODE StreamNode = pXP.CreateNodePtr(channelNode,"elementryPID");
								pXP.SetAttrNode("StreamType",(*pStream).StreamType,StreamNode);
								pXP.SetAttrNode("PID",(*pStream).StreamPid,StreamNode);
							}
						}
						catch(...)
						{
							cout<<"设置节目属性时失败"<<endl;
						}
					}
				}
			}
		}
	}
	pXP.SaveAsFile(filename.c_str());
	return true;
}
bool PSISIParser::CreateEPGXML()
{
	ACE_Guard<ACE_Thread_Mutex> guard(Mutex);
	string datetime = TimeUtil::DateTimeToString(time(0));
	string filename = Path + string("EPGQuery") + datetime + string(".xml");
	string zipFilePath = Path + string("EPGQuery") + datetime + string(".zip");
	char * strXML = "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?><Msg></Msg>";
	XmlParser pXP(strXML);

	//构造XML头信息
	pXMLNODE rootNode = pXP.GetRootNode();
	pXP.SetAttrNode(string("Version"),string("4"),rootNode);
//	string MsgID;
//	DBMANAGER::instance()->GetMsgID(MsgID);
//	int msgid=StrUtil::Str2Int(MsgID);
//	DBMANAGER::instance()->UpdateMsgID(StrUtil::Int2Str(++msgid));

	string strDateTime = TimeUtil::GetCurDateTime();
	string  MsgID = OSFunction::GetXmlMsgID(strDateTime);

	pXP.SetAttrNode(string("MsgID"),MsgID,rootNode);
	pXP.SetAttrNode(string("Type"),string("MonUp"),rootNode);
//	pXP.SetAttrNode(string("DateTime"),TimeUtil::GetCurDateTime(),rootNode);
	pXP.SetAttrNode(string("DateTime"),strDateTime,rootNode);
	string srccode = PROPMANAGER::instance()->GetDefSrcCode(DVBType);
	string dstcode = PROPMANAGER::instance()->GetDefDstCode(DVBType);
	pXP.SetAttrNode(string("SrcCode"),srccode,rootNode);
	pXP.SetAttrNode(string("DstCode"),dstcode,rootNode);
	pXP.SetAttrNode(string("ReplyID"),string(""),rootNode);

	pXMLNODE returnNode = pXP.CreateNodePtr(rootNode,"Return");  //创建Return节点
	pXP.SetAttrNode(string("Type"),string("EPGQuery"),returnNode);
	pXP.SetAttrNode(string("Value"),string("0"),returnNode);
	pXP.SetAttrNode(string("Desc"),string("成功"),returnNode);

	pXMLNODE returnInfoNode = pXP.CreateNodePtr(rootNode,"ReturnInfo");  //创建ReturnInfo节点
	pXMLNODE queryNode = pXP.CreateNodePtr(returnInfoNode,"EPGQuery");   //创建EPGQuery节点
	pXP.SetAttrNode(string("ScanTime"),TimeUtil::GetCurDateTime(),queryNode);

	if (DVBType == THREED)
	{
		if (mapSDT.empty())
		{
			cout<<"获得节目信息错误或不完整"<<endl;
			pXP.SaveAsFile(filename.c_str());
			return false;
		}
		else
		{
			mapSDTInfo::iterator mapIter = mapSDT.begin();
			while (mapIter != mapSDT.end())
			{
				/* 没有sdt信息的频点直接略过 */
				if ((mapIter->second).empty())
				{
					continue;
				}

				bool hasCreateEPGInfoNode = false;
				pXMLNODE epgInfoNode = NULL;
				std::string freq3D = mapIter->first;	
				std::vector<SDT>::iterator sdtIter = (mapIter->second).begin();
				for (; sdtIter!=(mapIter->second).end(); sdtIter++)
				{
					if (sdtIter->ServiceName.find("3D") != std::string::npos)	//节目名包含3D
					{
						if (mapEIT.empty())
						{
							cout<<"获得EPG信息不完整"<<endl;
							pXP.SaveAsFile(filename.c_str());
							OSFunction::CompressFile(zipFilePath,filename);
							return false;
						}
						else
						{
							std::vector<EIT>::iterator eit3DIter = mapEIT[freq3D].begin();
							for (; eit3DIter!= mapEIT[freq3D].end(); eit3DIter++)
							{
								if (eit3DIter->ServiceID == sdtIter->ServiceID)
								{
									if (!hasCreateEPGInfoNode)
									{
										epgInfoNode = pXP.CreateNodePtr(queryNode, "EPGInfo");   //创建EPGInfo节点
										pXP.SetAttrNode(string("Freq"), freq3D, epgInfoNode);
										hasCreateEPGInfoNode = true;
									}

									pXMLNODE epgNode = pXP.CreateNodePtr(epgInfoNode,"EPG");   //创建EPG节点
									//
									std::vector<PAT>::iterator pat3DIter = mapPAT[freq3D].begin();
									for (; pat3DIter!=mapPAT[freq3D].end(); pat3DIter++)
									{
										if (pat3DIter->ProgramNumber == sdtIter->ServiceID)
										{
											pXP.SetAttrNode(string("ProgramID"),pat3DIter->ProgramNumber,epgNode);   //设置ProgramID属性值
											break;
										}
									}
									if (pat3DIter == mapPAT[freq3D].end())
									{
										pXP.SetAttrNode(string("ProgramID"), std::string(""), epgNode);   //设置ProgramID属性值
									}

									/* 添加EIT属性信息 */
									pXP.SetAttrNode(string("Program"), eit3DIter->ProgramName, epgNode);
									std::string starttime=eit3DIter->ProgramStartTime;
									std::string temp;
									char* result=strtok((char*)starttime.c_str(),"/");
									while(result!=NULL)
									{
										temp+=string(result);
										result=strtok(NULL,"/");
									}
									temp=temp.insert(4,"-");
									temp=temp.insert(7,"-");
									temp.erase(10,1);
									TimeUtil::CalHour(temp, 8);		//eit表中时间与真实时间相差8小时
									pXP.SetAttrNode(string("StartTime"),temp,epgNode);  //设置StartTime
									pXP.SetAttrNode(string("ProgramLen"),eit3DIter->ProgramDuration,epgNode);  //设置ProgramLen属性值
									pXP.SetAttrNode(string("State"),eit3DIter->RunningStatus,epgNode);         //设置State属性值
									pXP.SetAttrNode(string("Encryption"),eit3DIter->FreeCAMode,epgNode);       //设置Encryption属性值
								}
							}	
						}	//!mapEIT.empty()
					}	//节目名包含3D
				}
				mapIter ++;
			}
		}	//!mapSDT.empty()
		pXP.SaveAsFile(filename.c_str());
		OSFunction::CompressFile(zipFilePath,filename);
		return true;
	}
	if (mapEIT.empty())
	{
		cout<<"获得EPG信息不完整"<<endl;
		pXP.SaveAsFile(filename.c_str());
		OSFunction::CompressFile(zipFilePath,filename);
		return false;
	}
	else
	{
		map< string,vector<EIT> >::iterator ptr = mapEIT.begin();
		for (; ptr!= mapEIT.end(); ptr++)
		{
			pXMLNODE epgInfoNode = pXP.CreateNodePtr(queryNode,"EPGInfo");   //创建EPGInfo节点
			pXP.SetAttrNode(string("Freq"),(*ptr).first,epgInfoNode);
			if ((*ptr).second.empty())
			{ 
				cout<<"获得频点"<<(*ptr).first<<"中EPG信息不完整"<<endl;
				continue;
			}
			else
			{
				try
				{
					for (size_t j = 0; j < (*ptr).second.size(); j++)
					{
						pXMLNODE epgNode = pXP.CreateNodePtr(epgInfoNode,"EPG");   //创建EPG节点
						if (mapPAT.find((*ptr).first) == mapPAT.end())
						{
							pXP.SetAttrNode(string("ProgramID"),string(""),epgNode);
						}
						else
						{
							size_t p = 0;
							for (;p<mapPAT[(*ptr).first].size();p++)
							{
								if (((*ptr).second)[j].ServiceID ==mapPAT[(*ptr).first][p].ProgramNumber)
								{
									pXP.SetAttrNode(string("ProgramID"),mapPAT[(*ptr).first][p].ProgramNumber,epgNode);   //设置ProgramID属性值
									break;
								}
							}
							if (p==mapPAT[(*ptr).first].size())
							{
								pXP.SetAttrNode(string("ProgramID"),string(""),epgNode);
							}
						}
						pXP.SetAttrNode(string("Program"),((*ptr).second)[j].ProgramName,epgNode);    //设置Program属性值

						if (mapSDT.find((*ptr).first) ==mapSDT.end())
						{
							pXP.SetAttrNode(string("ProgramType"),string(""),epgNode);
						}
						else
						{
							size_t q = 0;
							for (;q<mapSDT[(*ptr).first].size();q++)
							{
								if (((*ptr).second)[j].ServiceID ==mapSDT[(*ptr).first][q].ServiceID)
								{
									pXP.SetAttrNode(string("ProgramType"),mapSDT[(*ptr).first][q].ServiceType,epgNode);  //设置ProgramType属性值
									break;
								}
							}
							if (q == mapSDT[(*ptr).first].size())
							{
								pXP.SetAttrNode(string("ProgramType"),string(""),epgNode);
							}
						}
						std::string starttime=((*ptr).second)[j].ProgramStartTime;
						std::string temp;
						char* result=strtok((char*)starttime.c_str(),"/");
						while(result!=NULL)
						{
							temp+=string(result);
							result=strtok(NULL,"/");
						}
						temp=temp.insert(4,"-");
						temp=temp.insert(7,"-");
						temp.erase(10,1);
						TimeUtil::CalHour(temp, 8);		//eit表中时间与真实时间相差8小时
						pXP.SetAttrNode(string("StartTime"),temp,epgNode);  //设置StartTime
						pXP.SetAttrNode(string("ProgramLen"),((*ptr).second)[j].ProgramDuration,epgNode);  //设置ProgramLen属性值
						pXP.SetAttrNode(string("State"),((*ptr).second)[j].RunningStatus,epgNode);         //设置State属性值
						pXP.SetAttrNode(string("Encryption"),((*ptr).second)[j].FreeCAMode,epgNode);       //设置Encryption属性值
					}
				}
				catch(...)
				{
					cout<<"设置EPG信息错误"<<endl;
				}
			}
		}
	}
	pXP.SaveAsFile(filename.c_str());
	OSFunction::CompressFile(zipFilePath,filename);
	return true;
}


bool PSISIParser::CreateTableXML()
{
	ACE_Guard<ACE_Thread_Mutex> guard(Mutex);
	string CurTime=TimeUtil::DateTimeToString(time(0));
	string zipFileName=Path+"TableQuery"+CurTime+".zip";
	for (size_t i = 0; i < vecNIT.size(); i++)
	{
		string TempPath = Path + vecNIT[i].Freq + string("/*.*");
		string TempInPath = Path  + vecNIT[i].Freq;
		string filename=Path+"TableQuery"+vecNIT[i].Freq+".xml";
		ofstream ofile(filename.c_str(),ios::out|ios::app);
		//循环查找文件名
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		hFind = FindFirstFile(TempPath.c_str(), &FindFileData);
		string MsgID,DateTime,SrcCode,DstCode;

//		DBMANAGER::instance()->GetMsgID(MsgID);
//		int msgid=StrUtil::Str2Int(MsgID);
//		DBMANAGER::instance()->UpdateMsgID(StrUtil::Int2Str(++msgid));

		string strDateTime = TimeUtil::GetCurDateTime();
		MsgID = OSFunction::GetXmlMsgID(strDateTime);

		DateTime=TimeUtil::GetCurDateTime();
		SrcCode =PROPMANAGER::instance()->GetDefSrcCode(DVBType);
		DstCode =PROPMANAGER::instance()->GetDefDstCode(DVBType);
		string strURL;
		strURL= "http://" + PROPMANAGER::instance()->GetHttpServerIP() + ":" + \
			PROPMANAGER::instance()->GetHttpServerPort() + "/";
		string UpType;
		if(DVBType==DVBC)
		{	
			UpType="MonUp";
		}
		else if(DVBType==CTTB)
		{
			UpType="DVBTHUp";
		}
		else if(DVBType==DVBS)
		{
			UpType="SatUp";
		}
		else if(DVBType==THREED)
		{
			UpType="3DUp";
		}
		string head="<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\" ?>";
		string msg="<Msg Version=\"4\" MsgID=\""+MsgID+"\" Type=\""+UpType+"\" DateTime=\""+strDateTime+"\" SrcCode=\""+SrcCode+"\" DstCode=\""+DstCode+"\" ReplyID=\"0\">";
		string ret="<Return Type=\"table\" Value=\"0\" Desc=\"成功\" Redirect=\"\" Freq=\""+vecNIT[i].Freq+"\"/>"+"<ReturnInfo>";
		head=head+msg+ret;
		if(!ofile)
		{
			cout<<"error"<<endl;
		}
		else
		{
			ofile<<head;
		}
		while (hFind != INVALID_HANDLE_VALUE)
		{
			if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
			{
				string FileNameTemp = string(FindFileData.cFileName);
				char * strtoks;
				char* pmark = strtok_s(FindFileData.cFileName,"_",&strtoks);
				string TablePath = TempInPath + "\\" + FileNameTemp;
				if(strcmp(pmark, "PAT") == 0)
				{
					char* content="<table></table>";
					XmlParser pXP;
					XmlParser pContent(content);
					string tableid,tablename="pat",extId,QueryTime=DateTime;
					pXP.LoadFromFile(TablePath.c_str());
					pXMLNODE rootNode=pXP.GetRootNode();
					pXMLNODELIST SecNodeList=pXP.GetNodeList(rootNode);
					int count=pXP.GetChildCount(rootNode);
					for(int i=0;i<count;i++)
					{
						pXMLNODE CRootNode=pContent.GetRootNode();

						string SectionSyntaxIndicator,SectionLength,TransportStreamID,VersionNumber,CurrentNextIndicator,SectionNumber,LastSectionNumber,CrcValue;

						pXMLNODE SecNode = pXP.GetNextNode(SecNodeList);
						pXP.GetAttrNode(SecNode,"TableID",tableid);
						pXP.GetAttrNode(SecNode,"SectionSyntaxIndicator",SectionSyntaxIndicator);
						pXP.GetAttrNode(SecNode,"SectionLength",SectionLength);
						pXP.GetAttrNode(SecNode,"TransportStreamID",TransportStreamID);
						pXP.GetAttrNode(SecNode,"VersionNumber",VersionNumber);
						pXP.GetAttrNode(SecNode,"CurrentNextIndicator",CurrentNextIndicator);
						pXP.GetAttrNode(SecNode,"SectionNumber",SectionNumber);
						pXP.GetAttrNode(SecNode,"LastSectionNumber",LastSectionNumber);
						pXP.GetAttrNode(SecNode,"CrcValue",CrcValue);


						pContent.SetAttrNode("name",tablename,CRootNode);
						pContent.SetAttrNode("tableId",tableid,CRootNode);
						pContent.SetAttrNode("extId",extId,CRootNode);
						pContent.SetAttrNode("QueryTime",QueryTime,CRootNode);

						pXMLNODE CSecNode=pContent.CreateNodePtr(CRootNode,"section");

						pContent.SetAttrNode("sno",SectionNumber,CSecNode);
						for(int k=0;k<9;k++)
						{
							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");
							switch(k)
							{
							case 0:
								pContent.SetAttrNode("name",string("tableid"),CchildNode);
								pContent.SetAttrNode("value",tableid,CchildNode);
								break;
							case 1:
								pContent.SetAttrNode("name",string("section_syntax_indicator"),CchildNode);
								pContent.SetAttrNode("value",SectionSyntaxIndicator,CchildNode);
								break;
							case 2:
								pContent.SetAttrNode("name",string("section_length"),CchildNode);
								pContent.SetAttrNode("value",SectionLength,CchildNode);
								break;
							case 3:
								pContent.SetAttrNode("name",string("transport_stream_id"),CchildNode);
								pContent.SetAttrNode("value",TransportStreamID,CchildNode);
								break;
							case 4:
								pContent.SetAttrNode("name",string("version_number"),CchildNode);
								pContent.SetAttrNode("value",VersionNumber,CchildNode);
								break;
							case 5:
								pContent.SetAttrNode("name",string("current_next_indicator"),CchildNode);
								pContent.SetAttrNode("value",CurrentNextIndicator,CchildNode);
								break;
							case 6:
								pContent.SetAttrNode("name",string("section_number"),CchildNode);
								pContent.SetAttrNode("value",SectionNumber,CchildNode);
								break;
							case 7:
								pContent.SetAttrNode("name",string("last_section_number"),CchildNode);
								pContent.SetAttrNode("value",LastSectionNumber,CchildNode);
								break;
							case 8:
								pContent.SetAttrNode("name",string("crc32"),CchildNode);
								pContent.SetAttrNode("value",CrcValue,CchildNode);
								break;
							}
						}
						pXMLNODE ProgramListNode=pXP.GetNodeFirstChild(SecNode);
						pXMLNODELIST ProgramList= pXP.GetNodeList(ProgramListNode);
						for(int j=0;j<ProgramList->Size();j++)
						{
							pXMLNODE ProgramNode = pXP.GetNextNode(ProgramList);
							string ProgramNumber,Pid;
							pXP.GetAttrNode(ProgramNode,"ProgramNumber",ProgramNumber);
							pXP.GetAttrNode(ProgramNode,"Pid",Pid);

							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");

							pContent.SetAttrNode("name",string("program"),CchildNode);
							pContent.SetAttrNode("value",ProgramNumber,CchildNode);

							pXMLNODE ProgramChildNode = pContent.CreateNodePtr(CchildNode,"item");

							pContent.SetAttrNode("name",string("program_number"),ProgramChildNode);
							pContent.SetAttrNode("value",Pid,ProgramChildNode);
						}
					}
					string tablestr;
					pContent.SaveToString(tablestr);
					ofile<<tablestr;
				}
				else if(strcmp(pmark, "PMT") == 0)
				{
					char * content="<table></table>";
					XmlParser pXP;
					XmlParser pContent(content);
					string tableid,tablename="pmt",extId,QueryTime=DateTime;
					pXP.LoadFromFile(TablePath.c_str());
					pXMLNODE rootNode=pXP.GetRootNode();
					pXMLNODELIST SecNodeList=pXP.GetNodeList(rootNode);
					int count=pXP.GetChildCount(rootNode);
					for(int i=0;i<count;i++)
					{
						pXMLNODE CRootNode=pContent.GetRootNode();

						string SectionSyntaxIndicator, SectionLength,ProgramNumber, VersionNumber, CurrentNextIndicator, SectionNumber, LastSectionNumber, PcrPid, CrcValue, ProgramInfoLength;
						pXMLNODE SecNode = pXP.GetNextNode(SecNodeList);
						pXP.GetAttrNode(SecNode,"TableID",tableid);
						pXP.GetAttrNode(SecNode,"SectionSyntaxIndicator",SectionSyntaxIndicator);
						pXP.GetAttrNode(SecNode,"SectionLength",SectionLength);
						pXP.GetAttrNode(SecNode,"ProgramNumber",ProgramNumber);
						pXP.GetAttrNode(SecNode,"VersionNumber",VersionNumber);
						pXP.GetAttrNode(SecNode,"CurrentNextIndicator",CurrentNextIndicator);
						pXP.GetAttrNode(SecNode,"SectionNumber",SectionNumber);
						pXP.GetAttrNode(SecNode,"LastSectionNumber",LastSectionNumber);
						pXP.GetAttrNode(SecNode,"PcrPid",PcrPid);
						pXP.GetAttrNode(SecNode,"CrcValue",CrcValue);
						pXP.GetAttrNode(SecNode,"ProgramInfoLength",ProgramInfoLength);

						pContent.SetAttrNode("name",tablename,CRootNode);
						pContent.SetAttrNode("tableId",tableid,CRootNode);
						pContent.SetAttrNode("extId",extId,CRootNode);
						pContent.SetAttrNode("QueryTime",QueryTime,CRootNode);

						pXMLNODE CSecNode=pContent.CreateNodePtr(CRootNode,"section");

						pContent.SetAttrNode("sno",SectionNumber,CSecNode);
						for(int k=0;k<11;k++)
						{
							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");
							switch(k)
							{
							case 0:
								pContent.SetAttrNode("name",string("tableid"),CchildNode);
								pContent.SetAttrNode("value",tableid,CchildNode);
								break;
							case 1:
								pContent.SetAttrNode("name",string("section_syntax_indicator"),CchildNode);
								pContent.SetAttrNode("value",SectionSyntaxIndicator,CchildNode);
								break;
							case 2:
								pContent.SetAttrNode("name",string("section_length"),CchildNode);
								pContent.SetAttrNode("value",SectionLength,CchildNode);
								break;
							case 3:
								pContent.SetAttrNode("name",string("program_number"),CchildNode);
								pContent.SetAttrNode("value",ProgramNumber,CchildNode);
								break;
							case 4:
								pContent.SetAttrNode("name",string("version_number"),CchildNode);
								pContent.SetAttrNode("value",VersionNumber,CchildNode);
								break;
							case 5:
								pContent.SetAttrNode("name",string("current_next_indicator"),CchildNode);
								pContent.SetAttrNode("value",CurrentNextIndicator,CchildNode);
								break;
							case 6:
								pContent.SetAttrNode("name",string("section_number"),CchildNode);
								pContent.SetAttrNode("value",SectionNumber,CchildNode);
								break;
							case 7:
								pContent.SetAttrNode("name",string("last_section_number"),CchildNode);
								pContent.SetAttrNode("value",LastSectionNumber,CchildNode);
								break;
							case 8:
								pContent.SetAttrNode("name",string("pcr_pid"),CchildNode);
								pContent.SetAttrNode("value",PcrPid,CchildNode);
								break;	
							case 9:
								pContent.SetAttrNode("name",string("program_info_length"),CchildNode);
								pContent.SetAttrNode("value",ProgramInfoLength,CchildNode);
								break;
							case 10:
								pContent.SetAttrNode("name",string("crc32"),CchildNode);
								pContent.SetAttrNode("value",CrcValue,CchildNode);
								break;
							}
						}
						pXMLNODE ProgramListNode=pXP.GetNodeFirstChild(SecNode);
						pXMLNODELIST ProgramList = pXP.GetNodeList(ProgramListNode);
						for(int j=0;j<ProgramList->Size();j++)
						{
							pXMLNODE ProgramNode = pXP.GetNextNode(ProgramList);
							string StreamType,StreamPid;
							pXP.GetAttrNode(ProgramNode,"StreamType",StreamType);
							pXP.GetAttrNode(ProgramNode,"StreamPid",StreamPid);

							pXMLNODE CPidNode = pContent.CreateNodePtr(CSecNode,"item");

							pXMLNODE CTypeNode=pContent.CreateNodePtr(CSecNode,"item");

							if(StreamType=="1" || StreamType=="2" || StreamType=="27" || StreamType=="30")
							{
								pContent.SetAttrNode("name",string("video_pid"),CPidNode);
								pContent.SetAttrNode("value",StreamPid,CPidNode);

								pContent.SetAttrNode("name",string("video_type"),CTypeNode);
								pContent.SetAttrNode("value",StreamType,CTypeNode);
							}
							else if(StreamType=="3" || StreamType=="4" || StreamType=="6" || \
									StreamType=="15" || StreamType=="17" || StreamType=="28")
							{
								pContent.SetAttrNode("name",string("audio_pid"),CPidNode);
								pContent.SetAttrNode("value",StreamPid,CPidNode);

								pContent.SetAttrNode("name",string("audio_type"),CTypeNode);
								pContent.SetAttrNode("value",StreamType,CTypeNode);
							}
						}
					}
					string tablestr;
					pContent.SaveToString(tablestr);
					ofile<<tablestr;
				}
				else if(strcmp(pmark, "SDT") == 0)
				{
					char * content="<table></table>";
					XmlParser pXP;
					XmlParser pContent(content);
					string tableid,tablename="sdt",extId,QueryTime=DateTime;
					pXP.LoadFromFile(TablePath.c_str());
					pXMLNODE rootNode=pXP.GetRootNode();
					pXMLNODELIST SecNodeList=pXP.GetNodeList(rootNode);
					int count=pXP.GetChildCount(rootNode);
					for(int i=0;i<count;i++)
					{
						pXMLNODE CRootNode=pContent.GetRootNode();
						string SectionSyntaxIndicator, SectionLength, TransportStreamID, VersionNumber, CurrentNextIndicator,SectionNumber, LastSectionNumber, OriginalNetworkID, CrcValue;
						pXMLNODE SecNode = pXP.GetNextNode(SecNodeList);
						pXP.GetAttrNode(SecNode,"TableID",tableid);
						pXP.GetAttrNode(SecNode,"SectionSyntaxIndicator",SectionSyntaxIndicator);
						pXP.GetAttrNode(SecNode,"SectionLength",SectionLength);
						pXP.GetAttrNode(SecNode,"TransportStreamID",TransportStreamID);
						pXP.GetAttrNode(SecNode,"VersionNumber",VersionNumber);
						pXP.GetAttrNode(SecNode,"CurrentNextIndicator",CurrentNextIndicator);
						pXP.GetAttrNode(SecNode,"SectionNumber",SectionNumber);
						pXP.GetAttrNode(SecNode,"LastSectionNumber",LastSectionNumber);
						pXP.GetAttrNode(SecNode,"OriginalNetworkID",OriginalNetworkID);
						pXP.GetAttrNode(SecNode,"CrcValue",CrcValue);

						pContent.SetAttrNode("name",tablename,CRootNode);
						pContent.SetAttrNode("tableId",tableid,CRootNode);
						pContent.SetAttrNode("extId",extId,CRootNode);
						pContent.SetAttrNode("QueryTime",QueryTime,CRootNode);

						pXMLNODE CSecNode=pContent.CreateNodePtr(CRootNode,"section");

						pContent.SetAttrNode("sno",SectionNumber,CSecNode);
						for(int k=0;k<10;k++)
						{
							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");
							switch(k)
							{
							case 0:
								pContent.SetAttrNode("name",string("tableid"),CchildNode);
								pContent.SetAttrNode("value",tableid,CchildNode);
								break;
							case 1:
								pContent.SetAttrNode("name",string("section_syntax_indicator"),CchildNode);
								pContent.SetAttrNode("value",SectionSyntaxIndicator,CchildNode);
								break;
							case 2:
								pContent.SetAttrNode("name",string("section_length"),CchildNode);
								pContent.SetAttrNode("value",SectionLength,CchildNode);
								break;
							case 3:
								pContent.SetAttrNode("name",string("transport_stream_id"),CchildNode);
								pContent.SetAttrNode("value",TransportStreamID,CchildNode);
								break;
							case 4:
								pContent.SetAttrNode("name",string("version_number"),CchildNode);
								pContent.SetAttrNode("value",VersionNumber,CchildNode);
								break;
							case 5:
								pContent.SetAttrNode("name",string("current_next_indicator"),CchildNode);
								pContent.SetAttrNode("value",CurrentNextIndicator,CchildNode);
								break;
							case 6:
								pContent.SetAttrNode("name",string("section_number"),CchildNode);
								pContent.SetAttrNode("value",SectionNumber,CchildNode);
								break;
							case 7:
								pContent.SetAttrNode("name",string("last_section_number"),CchildNode);
								pContent.SetAttrNode("value",LastSectionNumber,CchildNode);
								break;
							case 8:
								pContent.SetAttrNode("name",string("original_network_id"),CchildNode);
								pContent.SetAttrNode("value",OriginalNetworkID,CchildNode);
								break;
							case 9:
								pContent.SetAttrNode("name",string("crc32"),CchildNode);
								pContent.SetAttrNode("value",CrcValue,CchildNode);
								break;
							}
						}
						pXMLNODE ProgramListNode=pXP.GetNodeFirstChild(SecNode);
						pXMLNODELIST ProgramList = pXP.GetNodeList(ProgramListNode);
						for(int j=0;j<ProgramList->Size();j++)
						{
							pXMLNODE ProgramNode = pXP.GetNextNode(ProgramList);
							string ServiceID, EitScheduleFlag, EitPresentFollowingFlag, RunningStatus, FreeCAMode, DescriptorsLoopLength;
							pXP.GetAttrNode(ProgramNode,"ServiceID",ServiceID);
							pXP.GetAttrNode(ProgramNode,"EitScheduleFlag",EitScheduleFlag);
							pXP.GetAttrNode(ProgramNode,"EitPresentFollowingFlag",EitPresentFollowingFlag);
							pXP.GetAttrNode(ProgramNode,"RunningStatus",RunningStatus);
							pXP.GetAttrNode(ProgramNode,"FreeCAMode",FreeCAMode);
							pXP.GetAttrNode(ProgramNode,"DescriptorsLoopLength",DescriptorsLoopLength);

							string ServiceProviderName,ServiceName;
							pXMLNODE SeviceNode = pXP.GetNodeFirstChild(ProgramNode);
							pXP.GetAttrNode(SeviceNode,"ServiceProviderName",ServiceProviderName);
							pXP.GetAttrNode(SeviceNode,"ServiceName",ServiceName);

							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");

							pContent.SetAttrNode("name",string("service_id"),CchildNode);
							pContent.SetAttrNode("value",ServiceID,CchildNode);

							for(int t=0;t<7;t++)
							{
								pXMLNODE ProgramChildNode = pContent.CreateNodePtr(CchildNode,"item");
								switch(t)
								{
								case 0:
									pContent.SetAttrNode("name",string("EIT_schedule_flag"),ProgramChildNode);
									pContent.SetAttrNode("value",EitScheduleFlag,ProgramChildNode);
									break;
								case 1:
									pContent.SetAttrNode("name",string("EIT_present_following_flag"),ProgramChildNode);
									pContent.SetAttrNode("value",EitPresentFollowingFlag,ProgramChildNode);
									break;
								case 2:
									pContent.SetAttrNode("name",string("running_status"),ProgramChildNode);
									pContent.SetAttrNode("value",RunningStatus,ProgramChildNode);
									break;
								case 3:
									pContent.SetAttrNode("name",string("free_CA_mode"),ProgramChildNode);
									pContent.SetAttrNode("value",FreeCAMode,ProgramChildNode);
									break;
								case 4:
									pContent.SetAttrNode("name",string("descriptors_loop_length"),ProgramChildNode);
									pContent.SetAttrNode("value",DescriptorsLoopLength,ProgramChildNode);
									break;
								case 5:
									pContent.SetAttrNode("name",string("service_provider_name"),ProgramChildNode);
									pContent.SetAttrNode("value",ServiceProviderName,ProgramChildNode);
									break;
								case 6:
									pContent.SetAttrNode("name",string("service_name"),ProgramChildNode);
									pContent.SetAttrNode("value",ServiceName,ProgramChildNode);
									break;
								}
							}
						}
					}
					string tablestr;
					pContent.SaveToString(tablestr);
					ofile<<tablestr;
				}
				else if(strcmp(pmark, "NIT") == 0)
				{
					char * content="<table></table>";
					XmlParser pXP;
					XmlParser pContent(content);
					string tableid,tablename="nit",extId,QueryTime=DateTime;
					pXP.LoadFromFile(TablePath.c_str());
					pXMLNODE rootNode=pXP.GetRootNode();
					pXMLNODELIST SecNodeList=pXP.GetNodeList(rootNode);
					int count=pXP.GetChildCount(rootNode);
					for(int i=0;i<count;i++)
					{
						pXMLNODE CRootNode=pContent.GetRootNode();

						string SectionSyntaxIndicator,SectionLength,NetworkID,VersionNumber, CurrentNextIndicator, SectionNumber,LastSectionNumber, CrcValue, NetworkDescriptorsLength;
						pXMLNODE SecNode = pXP.GetNextNode(SecNodeList);
						pXP.GetAttrNode(SecNode,"TableID",tableid);
						pXP.GetAttrNode(SecNode,"SectionSyntaxIndicator",SectionSyntaxIndicator);
						pXP.GetAttrNode(SecNode,"SectionLength",SectionLength);
						pXP.GetAttrNode(SecNode,"NetworkID",NetworkID);		//wz_110328
						pXP.GetAttrNode(SecNode,"VersionNumber",VersionNumber);
						pXP.GetAttrNode(SecNode,"CurrentNextIndicator",CurrentNextIndicator);
						pXP.GetAttrNode(SecNode,"SectionNumber",SectionNumber);
						pXP.GetAttrNode(SecNode,"LastSectionNumber",LastSectionNumber);
						pXP.GetAttrNode(SecNode,"NetworkDescriptorsLength",NetworkDescriptorsLength);
						pXP.GetAttrNode(SecNode,"CrcValue",CrcValue);

						pContent.SetAttrNode("name",tablename,CRootNode);
						pContent.SetAttrNode("tableId",tableid,CRootNode);
						pContent.SetAttrNode("extId",extId,CRootNode);
						pContent.SetAttrNode("QueryTime",QueryTime,CRootNode);

						pXMLNODE CSecNode=pContent.CreateNodePtr(CRootNode,"section");

						pContent.SetAttrNode("sno",SectionNumber,CSecNode);
						for(int k=0;k<10;k++)
						{
							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");
							switch(k)
							{
							case 0:
								pContent.SetAttrNode("name",string("tableid"),CchildNode);
								pContent.SetAttrNode("value",tableid,CchildNode);
								break;
							case 1:
								pContent.SetAttrNode("name",string("section_syntax_indicator"),CchildNode);
								pContent.SetAttrNode("value",SectionSyntaxIndicator,CchildNode);
								break;
							case 2:
								pContent.SetAttrNode("name",string("section_length"),CchildNode);
								pContent.SetAttrNode("value",SectionLength,CchildNode);
								break;
							case 3:
								pContent.SetAttrNode("name",string("network_id"),CchildNode);	//wz_110328
								pContent.SetAttrNode("value",NetworkID,CchildNode);
								break;
							case 4:
								pContent.SetAttrNode("name",string("version_number"),CchildNode);
								pContent.SetAttrNode("value",VersionNumber,CchildNode);
								break;
							case 5:
								pContent.SetAttrNode("name",string("current_next_indicator"),CchildNode);
								pContent.SetAttrNode("value",CurrentNextIndicator,CchildNode);
								break;
							case 6:
								pContent.SetAttrNode("name",string("section_number"),CchildNode);
								pContent.SetAttrNode("value",SectionNumber,CchildNode);
								break;
							case 7:
								pContent.SetAttrNode("name",string("last_section_number"),CchildNode);
								pContent.SetAttrNode("value",LastSectionNumber,CchildNode);
								break;
							case 8:
								pContent.SetAttrNode("name",string("network_descriptor_length"),CchildNode);
								pContent.SetAttrNode("value",NetworkDescriptorsLength,CchildNode);
								break;
							case 9:
								pContent.SetAttrNode("name",string("crc32"),CchildNode);
								pContent.SetAttrNode("value",CrcValue,CchildNode);
								break;
							}
						}
						pXMLNODE ProgramListNode=pXP.findNode(SecNode,"TransportStreamList");
						pXMLNODELIST ProgramList = pXP.GetNodeList(ProgramListNode);
						for(int j=0;j<ProgramList->Size();j++)
						{
							pXMLNODE ProgramNode = pXP.GetNextNode(ProgramList);
							string TransportStreamID,OriginalNetworkID,TransportDescriptorsLength,CentreFrequency, Modulation,SymbolRate;
							pXP.GetAttrNode(ProgramNode,"TransportStreamID",TransportStreamID);
							pXP.GetAttrNode(ProgramNode,"OriginalNetworkID",OriginalNetworkID);
							pXP.GetAttrNode(ProgramNode,"TransportDescriptorsLength",TransportDescriptorsLength);

							pXMLNODE ListChildNode = pXP.findNode(ProgramNode,"CabDelivSysDescriptor");
							if(ListChildNode==NULL)
								ListChildNode=pXP.findNode(ProgramNode,"TerrDelivSysDescriptor");
						    if(ListChildNode==NULL)
								ListChildNode=pXP.findNode(ProgramNode,"SatDelivSysDescriptor");
							if(ListChildNode!=NULL)
							{
								pXP.GetAttrNode(ListChildNode,"CentreFrequency",CentreFrequency);
								pXP.GetAttrNode(ListChildNode,"Modulation",Modulation);
								pXP.GetAttrNode(ListChildNode,"SymbolRate",SymbolRate);
							}
							pXMLNODE CchildNode=pContent.CreateNodePtr(CSecNode,"item");

							pContent.SetAttrNode("name",string("transport_stream_id"),CchildNode);
							pContent.SetAttrNode("value",TransportStreamID,CchildNode);

							for(int t=0;t<5;t++)
							{
								pXMLNODE ProgramChildNode = pContent.CreateNodePtr(CchildNode,"item");
								switch(t)
								{
								case 0:
									pContent.SetAttrNode("name",string("original_network_id"),ProgramChildNode);
									pContent.SetAttrNode("value",OriginalNetworkID,ProgramChildNode);
									break;

								case 1:
									pContent.SetAttrNode("name",string("transport_descriptor_length"),ProgramChildNode);
									pContent.SetAttrNode("value",TransportDescriptorsLength,ProgramChildNode);
									break;
								case 2:
									pContent.SetAttrNode("name",string("frequency"),ProgramChildNode);
									pContent.SetAttrNode("value",CentreFrequency,ProgramChildNode);
									break;
								case 3:
									pContent.SetAttrNode("name",string("modulation"),ProgramChildNode);
									pContent.SetAttrNode("value",Modulation,ProgramChildNode);
									break;
								case 4:
									pContent.SetAttrNode("name",string("symbol_rate"),ProgramChildNode);
									pContent.SetAttrNode("value",SymbolRate,ProgramChildNode);
									break;
								}
							}
						}
					}
					string tablestr;
					pContent.SaveToString(tablestr);
					ofile<<tablestr;
				}
			}

			if (!FindNextFile(hFind, &FindFileData))
			{
				break;
			}
		}
		string tail="</ReturnInfo></Msg>";
		ofile<<tail;
		ofile.close();
		FindClose(hFind);
	}
	string tableFiles=Path+"TableQuery*.xml";
	OSFunction::CompressFile(zipFileName,tableFiles);
	return true;
}

bool PSISIParser::LoadPSISIInfoFromFile(string taskname)
{
	string dvbtype = OSFunction::GetStrDVBType(DVBType);
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s PSISI表信息分析开始!\n",dvbtype.c_str()));

	mapPAT.clear();
	mapPMT.clear();
	mapSDT.clear();
	vecNIT.clear();
	mapEIT.clear();
	mapVersion.clear();
	if (Path.length() == 0)
	{
		ACE_DEBUG ((LM_DEBUG,"(%T | %t)获取PSISI信息路径错误 !\n"));
		return false;
	}
	else
	{
		//遍历所有文件，获取PSISI信息
		try
		{
			string TempPath = Path + string("*.*");
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			//先提取NIT信息
			hFind = FindFirstFile(TempPath.c_str(), &FindFileData);

			while (hFind != INVALID_HANDLE_VALUE)
			{
				if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
				{
					string freq= string(FindFileData.cFileName);  //文件夹名为频点
					 //频道扫描不扫描EIT表
					if (taskname=="ChannelScanQueryTask" && strcmp(FindFileData.cFileName, "EIT_Table") == 0)
					{
						if (!FindNextFile(hFind, &FindFileData))
						{
							break;
						}
						continue;
					}
					if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
					{
						WIN32_FIND_DATA ChildFindFileData;
						HANDLE ChildhFind;
						string DirName=Path+freq+string("/")+string("*.*");
						string TempInPath =Path+freq;
						vector<FileVersion> vecFileversion;
						ChildhFind = FindFirstFile(DirName.c_str(), &ChildFindFileData);
						while(ChildhFind !=INVALID_HANDLE_VALUE)
						{
							if (strcmp(ChildFindFileData.cFileName, ".") != 0 && strcmp(ChildFindFileData.cFileName, "..") != 0)
							{
								string FileNameTemp = string(ChildFindFileData.cFileName);
								char * strtoks;
								char* pmark = strtok_s(ChildFindFileData.cFileName,"_",&strtoks);
								string TablePath = TempInPath + string("/") + FileNameTemp;

								if (strcmp(pmark, "PMT") == 0||strcmp(pmark, "PAT") == 0||strcmp(pmark, "NIT") == 0||	\
									strcmp(pmark, "SDT") == 0||strcmp(pmark, "CAT") == 0||strcmp(pmark, "BAT") == 0||	\
									strcmp(pmark, "TDT") == 0||strcmp(pmark, "TOT") == 0||strcmp(pmark, "EIT") == 0|| strcmp(pmark,"HDTV") == 0)
								{
									FileVersion fileversion;
									XmlParser parser;
									parser.LoadFromFile(TablePath.c_str());
									pXMLNODE rootNode= parser.GetNodeFromPath("PSISI");
									pXMLNODE SectionNode =parser.GetNodeFirstChild(rootNode);
									parser.GetAttrNode(SectionNode,"VersionNumber",fileversion.Version);
									fileversion.FileName=FileNameTemp;
									vecFileversion.push_back(fileversion);
								}

								if (strcmp(pmark, "NIT") == 0)//解析NIT相关信息的xml文件
								{
									XmlParser pXP;
									pXP.LoadFromFile(TablePath.c_str());

									pXMLNODE pNode= pXP.GetNodeFromPath("PSISI");
									pXMLNODELIST pSectionList=pXP.GetNodeList(pNode);
									int count=pXP.GetChildCount(pNode);
									for(int i=0;i<count;i++)
									{
										pXMLNODE SectionNode=pXP.GetNextNode(pSectionList);
										pXMLNODE ListNode=pXP.findNode(SectionNode,"TransportStreamList");
										pXMLNODELIST pNodeList=pXP.GetNodeList(ListNode);
										int childcount=pXP.GetChildCount(ListNode);
										for(int k=0;k<childcount;k++)
										{
											NIT tempNIT;
											pXMLNODE childNode=pXP.GetNextNode(pNodeList);
											if(childNode==NULL)
												continue;
											pXP.GetAttrNode(childNode,"TransportStreamID",tempNIT.TsID);
											pXP.GetAttrNode(childNode,"OriginalNetworkID",tempNIT.OrgNetID);

											pXMLNODE CabNode=pXP.findNode(childNode,"CabDelivSysDescriptor");
											if(CabNode==NULL)
												CabNode=pXP.findNode(childNode,"TerrDelivSysDescriptor");
											if(CabNode==NULL)
												CabNode=pXP.findNode(childNode,"SatDelivSysDescriptor");
											if(CabNode == NULL)
												continue;
											
											pXP.GetAttrNode(CabNode,"CentreFrequency",tempNIT.Freq);
											pXP.GetAttrNode(CabNode,"SymbolRate",tempNIT.SymbolRate);
											pXP.GetAttrNode(CabNode,"Modulation",tempNIT.QAM);
											if((tempNIT.Freq).size()<=3 && (tempNIT.Freq).size()>0)
											{
												tempNIT.Freq=tempNIT.Freq+"000";
											}
											vector<NIT>::iterator iter;
											for(iter=vecNIT.begin();iter!=vecNIT.end();iter++)
											{
												if(iter->TsID==tempNIT.TsID && iter->OrgNetID==tempNIT.OrgNetID)
												{
													vecNIT.erase(iter);
													break;
												}
											}
											vecNIT.push_back(tempNIT);
										}
									}
								}
							}
							if (!FindNextFile(ChildhFind, &ChildFindFileData))
							{
								break;
							}
						}
						FindClose(ChildhFind);

						mapVersion.insert(make_pair(freq,vecFileversion));
					}
				}
				if (!FindNextFile(hFind, &FindFileData))
				{
					break;
				}
			}
			FindClose(hFind);
			//遍历其他PAT PMT SDT EIT
			hFind = FindFirstFile(TempPath.c_str(), &FindFileData);

			while (hFind != INVALID_HANDLE_VALUE)//遍历目录 如c:/Table/DVBC
			{
				if (FindFileData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
				{ 
					string folderName = string(FindFileData.cFileName);  //文件夹名为频点
					string freq = "";

					//频道扫描不扫描EIT表
					if (taskname=="ChannelScanQueryTask" && strcmp(FindFileData.cFileName, "EIT_Table") == 0)
					{
						if (!FindNextFile(hFind, &FindFileData))
						{
							break;
						}
						continue;
					}

					if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
					{
						vctPATInfo vecPAT;
						vctPMTInfo vecPMT;

						WIN32_FIND_DATA ChildFindFileData;
						HANDLE ChildhFind;
						string DirName=Path+folderName+string("/")+string("*.*");
						string TempInPath =Path+folderName;
						ChildhFind = FindFirstFile(DirName.c_str(), &ChildFindFileData);
						while(ChildhFind !=INVALID_HANDLE_VALUE)//遍历不同目录（目录名以频点命名）下的其它表文件 
						{
							if (strcmp(ChildFindFileData.cFileName, ".") != 0 && strcmp(ChildFindFileData.cFileName, "..") != 0)
							{
								string FileNameTemp = string(ChildFindFileData.cFileName);
								char * strtoks;
								char* pmark = strtok_s(ChildFindFileData.cFileName,"_",&strtoks);
								string TablePath = TempInPath + string("/") + FileNameTemp;

								if (strcmp(pmark, "PAT") == 0)//解析PAT相关信息的xml文件
								{
									XmlParser pXP;
									pXP.LoadFromFile(TablePath.c_str());

									pXMLNODE rootNode = pXP.GetRootNode();
									pXP.GetAttrNode(rootNode, "Freq", freq);
									pXMLNODELIST SectionList= pXP.GetNodeList(rootNode);
									int sectioncount = pXP.GetChildCount(rootNode);
									for(int k=0;k<sectioncount;k++)
									{

										pXMLNODE SectionNode = pXP.GetNextNode(SectionList);
										pXMLNODE pNode= pXP.findNode(SectionNode,"ProgrameList");
										pXMLNODELIST pNodeList=pXP.GetNodeList(pNode);
										int count=pXP.GetChildCount(pNode);
										for(int i=0;i<count;i++)
										{
											PAT tempPAT;
											pXMLNODE childNode=pXP.GetNextNode(pNodeList);
											pXP.GetAttrNode(childNode,"ProgramNumber",tempPAT.ProgramNumber);
											pXP.GetAttrNode(childNode,"Pid",tempPAT.ProgramID);

											vecPAT.push_back(tempPAT);
										}
									}
								}

								else if (strcmp(pmark, "PMT") == 0)//解析PMT相关信息的xml文件
								{
									XmlParser pXP;
									PMT tempPMT;
									pXP.LoadFromFile(TablePath.c_str());
									pXMLNODE rootNode = pXP.GetRootNode();
									pXP.GetAttrNode(rootNode, "Freq", freq);
									pXMLNODELIST SectionList = pXP.GetNodeList(rootNode);
									int sectioncount=pXP.GetChildCount(rootNode);

									pXP.GetAttrNode(rootNode,"pid",tempPMT.PmtPID);
									for(int j=0;j<sectioncount;j++)
									{
										pXMLNODE pSectionNode = pXP.GetNextNode(SectionList);
										pXP.GetAttrNode(pSectionNode,"ProgramNumber",tempPMT.ProgramNumber);
										pXP.GetAttrNode(pSectionNode,"PcrPid",tempPMT.PcrPID);
										pXMLNODE pNode= pXP.findNode(pSectionNode,"StreamList");
										pXMLNODELIST pNodeList=pXP.GetNodeList(pNode);
										int count=pXP.GetChildCount(pNode);
										bool audioflag=false;
										for(int i=0;i<count;i++)
										{
											string streamtype;
											string streampid;
											STREAM tempStream;
											pXMLNODE childNode=pXP.GetNextNode(pNodeList);
											pXP.GetAttrNode(childNode,"StreamType",streamtype);
											pXP.GetAttrNode(childNode,"StreamPid",streampid);
											tempStream.StreamType=streamtype;
											tempStream.StreamPid=streampid;
											tempPMT.vecStream.push_back(tempStream);
											if(streamtype=="1" || streamtype=="2" || streamtype=="27" || streamtype=="30")
											{
												tempPMT.VideoPID=streampid;
											}
											else if(!audioflag && (streamtype=="3" || streamtype=="4" || streamtype=="6"  \
																|| streamtype=="15" || streamtype=="17" || streamtype=="28"))
											{
												tempPMT.AudioPID=streampid;
												audioflag=true;
											}	
										}								
										vecPMT.push_back(tempPMT);
									}
								}
								else if (strcmp(pmark, "SDT") == 0)//解析SDT相关信息的xml文件
								{
									XmlParser pXP;
									SDT tempSDT;
									size_t j=0;
									string tempFreq;
									pXP.LoadFromFile(TablePath.c_str());
									pXMLNODE rootNode = pXP.GetRootNode();
									pXP.GetAttrNode(rootNode, "Freq", freq);
									pXMLNODELIST sectionList=pXP.GetNodeList(rootNode);
									int sectioncount=pXP.GetChildCount(rootNode);
									for(int k=0;k<sectioncount;k++)
									{
										string tableid;
										pXMLNODE pSectionNode = pXP.GetNextNode(sectionList);
										pXP.GetAttrNode(pSectionNode,"TransportStreamID",tempSDT.TsID);
										pXP.GetAttrNode(pSectionNode,"OriginalNetworkID",tempSDT.OrgNetID);
										pXP.GetAttrNode(pSectionNode,"TableID",tableid);
										

										if(tableid=="66")
										{
											tempFreq=freq;
										}
										else
										{
											for(j=0;j<vecNIT.size();j++)
											{
												if(vecNIT[j].TsID==tempSDT.TsID&&vecNIT[j].OrgNetID==tempSDT.OrgNetID)
												{
													tempFreq=vecNIT[j].Freq;
													break;
												}
											}
											if(j==vecNIT.size())
											{
												continue;
											}
										}					

										pXMLNODE pNode= pXP.findNode(pSectionNode,"ServiceList");
										pXMLNODELIST pNodeList=pXP.GetNodeList(pNode);
										int count=pXP.GetChildCount(pNode);
										for(int i=0;i<count;i++)
										{
											pXMLNODE childNode=pXP.GetNextNode(pNodeList);
											if(childNode==NULL)
												continue;
											pXP.GetAttrNode(childNode,"ServiceID",tempSDT.ServiceID);
											pXP.GetAttrNode(childNode,"FreeCAMode",tempSDT.FreeCAMode);
											pXMLNODE curNode = pXP.findNode(childNode,"ServiceDescriptor");
											if(curNode==NULL)
												continue;
											pXP.GetAttrNode(curNode,"ServiceType",tempSDT.ServiceType);
											pXP.GetAttrNode(curNode,"ServiceName",tempSDT.ServiceName);

											mapSDTInfo::iterator iter;
											if((iter=mapSDT.find(tempFreq))!=mapSDT.end())
											{
												vctSTDInfo::iterator pitor;
												for(pitor=(*iter).second.begin();pitor!=(*iter).second.end();pitor++)
												{
													if(pitor->TsID==tempSDT.TsID && pitor->OrgNetID==tempSDT.OrgNetID &&pitor->ServiceID==tempSDT.ServiceID)
													{
														if(tempSDT.FreeCAMode!=""&&tempSDT.ServiceName!=""&&tempSDT.ServiceType!="")
														{
															(*iter).second.erase(pitor);
														}
														break;
													}
												}
												if(tempSDT.FreeCAMode!=""&&tempSDT.ServiceName!=""&&tempSDT.ServiceType!="")
												{
													(*iter).second.push_back(tempSDT);
												}
											}
											else
											{
												vctSTDInfo vecSDT;
												vecSDT.push_back(tempSDT);
												mapSDT.insert(make_pair(tempFreq,vecSDT));
											}
										}
									}
								}
								else if (strcmp(pmark, "EIT") == 0)//解析EIT相关信息的xml文件
								{
									XmlParser pXP;
									EIT tempEIT;
									pXP.LoadFromFile(TablePath.c_str());
									pXMLNODE pSectionNode = pXP.GetNodeFromPath("PSISI");
									pXMLNODELIST pNodeList=pXP.GetNodeList(pSectionNode);
									int count=pXP.GetChildCount(pSectionNode);
									for(int i=0;i<count;i++)		//section 循环
									{
										string tempFreq;
										size_t j=0;
										pXMLNODE childNode=pXP.GetNextNode(pNodeList);
										pXP.GetAttrNode(childNode,"ServiceID",tempEIT.ServiceID);
										pXP.GetAttrNode(childNode,"TransportStreamID",tempEIT.TsID);
										pXP.GetAttrNode(childNode,"OriginalNetworkID",tempEIT.OrgNetID);
										for(j=0;j<vecNIT.size();j++)
										{
											if(vecNIT[j].TsID==tempEIT.TsID&&vecNIT[j].OrgNetID==tempEIT.OrgNetID)
											{
												tempFreq=vecNIT[j].Freq;
												break;
											}
										}
										if(j==vecNIT.size())
										{

											mapSDTInfo::iterator iter=mapSDT.begin();
											for(;iter!=mapSDT.end();iter++)
											{
												vctSTDInfo::iterator ptr=(iter->second).begin();
												if(ptr!=(iter->second).end())
												{
													if(ptr->TsID==tempEIT.TsID&&ptr->OrgNetID==tempEIT.OrgNetID)
													{
														tempFreq=iter->first;
														break;
													}
												}
											}
											if(iter==mapSDT.end())
												continue;
										}


										pXMLNODE ListNode=pXP.GetNodeFirstChild(childNode);
										pXMLNODELIST EventList = pXP.GetNodeList(ListNode);
										int childcount=pXP.GetChildCount(ListNode);
										for(int k=0;k<childcount;k++)
										{
											pXMLNODE EventNode=pXP.GetNextNode(EventList);
											if(EventNode==NULL)
												continue;
											pXP.GetAttrNode(EventNode,"EventID",tempEIT.EventID);
											pXP.GetAttrNode(EventNode,"TimeOfChange",tempEIT.ProgramStartTime);
											pXP.GetAttrNode(EventNode,"Duration",tempEIT.ProgramDuration);
											pXP.GetAttrNode(EventNode,"RunningStatus",tempEIT.RunningStatus);
											pXP.GetAttrNode(EventNode,"FreeCAMode",tempEIT.FreeCAMode);
											pXMLNODE curNode = pXP.findNode(EventNode,"EventDescriptor");
											if(curNode==NULL)
												continue;
											pXP.GetAttrNode(curNode,"EventName",tempEIT.ProgramName);

											mapEITInfo::iterator iter;
											if((iter=mapEIT.find(tempFreq))!=mapEIT.end())
											{
												vctEITInfo::iterator pitor;
												for(pitor=(*iter).second.begin();pitor!=(*iter).second.end();pitor++)
												{
													if(pitor->TsID==tempEIT.TsID && pitor->OrgNetID==tempEIT.OrgNetID &&pitor->ServiceID==tempEIT.ServiceID && 
														pitor->EventID==tempEIT.EventID && pitor->ProgramStartTime==tempEIT.ProgramStartTime)	//添加根据starttime去判断event是否相同
													{
														if(tempEIT.FreeCAMode!=""&&tempEIT.ProgramDuration!=""&&tempEIT.ProgramName!=""&&tempEIT.ProgramStartTime!=""&&tempEIT.RunningStatus!="")
														{
															(*iter).second.erase(pitor);
														}
														break;
													}
												}
												if(tempEIT.FreeCAMode!=""&&tempEIT.ProgramDuration!=""&&tempEIT.ProgramName!=""&&tempEIT.ProgramStartTime!=""&&tempEIT.RunningStatus!="")
												{
													(*iter).second.push_back(tempEIT);
												}
											}
											else
											{
												vctEITInfo vecEIT;
												vecEIT.push_back(tempEIT);
												mapPATInfo::iterator ptr = mapPAT.begin();
												for (;ptr!=mapPAT.end();++ptr)
												{
													if (ptr->first==tempFreq)
														break;
												}
												if (ptr!=mapPAT.end())
												{
													mapEIT.insert(make_pair(tempFreq,vecEIT));
												}
											}
										}
									}
								}
								else if(strcmp(pmark, "HDTV") == 0)//解析高清频道相关信息的xml文件
								{
									HDTV tempHDTV;
									int pos=FileNameTemp.find_first_of("_");
									int pos1=FileNameTemp.find("_",pos+1);
									int pos2=FileNameTemp.find("_",pos1+1);
									int pos3=FileNameTemp.find(".",pos2+1);

									XmlParser pXP;
									pXP.LoadFromFile(TablePath.c_str());
									pXMLNODE rootNode = pXP.GetRootNode();
									pXP.GetAttrNode(rootNode, "Freq", freq);

									tempHDTV.Freq=freq;
									tempHDTV.OrgNetID=FileNameTemp.substr(pos+1,pos1-pos-1);
									tempHDTV.TsID=FileNameTemp.substr(pos1+1,pos2-pos1-1);
									tempHDTV.ServiceID=FileNameTemp.substr(pos2+1,pos3-pos2-1);
									vecHDTV.push_back(tempHDTV);
								}
							}
							if (!FindNextFile(ChildhFind, &ChildFindFileData))
							{
								break;
							}
						}
						FindClose(ChildhFind);

						if (freq != "")
						{
							mapPAT.insert(make_pair(freq,vecPAT));
							mapPMT.insert(make_pair(freq,vecPMT));
						}
					}
				}
				if (!FindNextFile(hFind, &FindFileData))
				{
					break;
				}
			}
			FindClose(hFind);
		}
		catch(...)
		{

		}
	}
	//移除无用信息	--- gxd 2010-10-10
	for (map< string,vector<PAT> >::iterator pat_ptr = mapPAT.begin();pat_ptr!=mapPAT.end();)
	{
		if ((*pat_ptr).second.empty() == true)
		{
			mapPAT.erase(pat_ptr);
			pat_ptr = mapPAT.begin();
			continue;
		}
		++pat_ptr;
	}	
	for (map< string,vector<PMT> >::iterator pmt_ptr = mapPMT.begin();pmt_ptr!=mapPMT.end();)
	{
		if ((*pmt_ptr).second.empty() == true)
		{
			mapPMT.erase(pmt_ptr);
			pmt_ptr = mapPMT.begin();
			continue;
		}
		++pmt_ptr;
	}
	for (map< string,vector<SDT> >::iterator sdt_ptr = mapSDT.begin();sdt_ptr!=mapSDT.end();)
	{
		if ((*sdt_ptr).second.empty() == true)
		{
			mapSDT.erase(sdt_ptr);
			sdt_ptr = mapSDT.begin();
			continue;
		}
		++sdt_ptr;
	}
	for (map< string,vector<EIT> >::iterator eit_ptr = mapEIT.begin();eit_ptr!=mapEIT.end();)
	{
		if ((*eit_ptr).second.empty() == true)
		{
			mapEIT.erase(eit_ptr);
			eit_ptr = mapEIT.begin();
			continue;
		}

		//对EIT容器进行排序，使生成的epg信息有序
		sort((*eit_ptr).second.begin(), (*eit_ptr).second.end(), less_EIT);

		++eit_ptr;
	}

	ACE_DEBUG ((LM_DEBUG,"(%T | %t)%s PSISI表信息分析完成!\n",dvbtype.c_str()));

	return true;
}

bool PSISIParser::UpdatePSISI( std::string strXML )
{
	return true;//暂时返回，不实时更新表信息		--- gxd 2010-10-10

	ACE_Guard<ACE_Thread_Mutex> guard(Mutex);
	string freq,tabletype,serviceid,tsid,orgnetid;
	XmlParser pXP;
	pXP.Set_xml(strXML);
	try
	{
		pXMLNODE rootNode=pXP.GetRootNode();
		pXMLNODE childNode=pXP.GetNodeFirstChild(rootNode);

		pXP.GetAttrNode(rootNode,"Freq",freq);
		pXP.GetAttrNode(rootNode,"type",tabletype);
		if(tabletype=="NIT")
		{
			//更新NIT表信息
			pXMLNODE pNode= pXP.GetNodeFromPath("PSISI");
			pXMLNODELIST pSectionList=pXP.GetNodeList(pNode);
			int count=pXP.GetChildCount(pNode);
			for(int i=0;i<count;i++)
			{
				pXMLNODE SectionNode=pXP.GetNextNode(pSectionList);
				pXMLNODE ListNode=pXP.findNode(SectionNode,"TransportStreamList");
				pXMLNODELIST pNodeList=pXP.GetNodeList(ListNode);
				int childcount=pXP.GetChildCount(ListNode);
				for(int k=0;k<childcount;k++)
				{
					NIT tempNIT;
					pXMLNODE childNode=pXP.GetNextNode(pNodeList);
					if(childNode==NULL)
						continue;
					pXP.GetAttrNode(childNode,"TransportStreamID",tempNIT.TsID);
					pXP.GetAttrNode(childNode,"OriginalNetworkID",tempNIT.OrgNetID);
					pXMLNODE CabNode=pXP.findNode(childNode,"CabDelivSysDescriptor");
					if(CabNode==NULL)
						CabNode=pXP.findNode(childNode,"TerrDelivSysDescriptor");
					if(CabNode==NULL)
						CabNode=pXP.findNode(childNode,"SatDelivSysDescriptor");
					if(CabNode==NULL)
						continue;
					pXP.GetAttrNode(CabNode,"CentreFrequency",tempNIT.Freq);
					pXP.GetAttrNode(CabNode,"SymbolRate",tempNIT.SymbolRate);
					if((tempNIT.Freq).size()<=3 && (tempNIT.Freq).size()>0)
					{
						tempNIT.Freq=tempNIT.Freq+"000";
					}
					vector<NIT>::iterator iter;
					bool NITflag=false;
					for(iter=vecNIT.begin();iter!=vecNIT.end();iter++)
					{
						if(iter->TsID==tempNIT.TsID && iter->OrgNetID==tempNIT.OrgNetID)
						{
							vecNIT.erase(iter);
							NITflag = true;
							break;
						}
					}
					if(NITflag == false)//如果vecNIT找不到相同的TsID和OrgNetID则为一个新的频点
					{
						if(vecEITtemp.size()>0)//将vecEITtemp中的相同频点插入到mapEIT
						{
							vctEITInfo vecEITFreq;
							for(size_t n=0;n<vecEITtemp.size();n++)
							{
								if(vecEITtemp[n].TsID==tempNIT.TsID&&vecEITtemp[n].OrgNetID==tempNIT.OrgNetID)
								{
									vecEITFreq.push_back(vecEITtemp[n]);
								}
							}
							if(vecEITFreq.size()>0)
							{
								mapEIT.insert(make_pair(tempNIT.Freq,vecEITFreq));
							}
						}
						if(vecSDTtemp.size()>0)//将vecSDTtemp中的相同频点插入到mapSDT
						{
							vctSTDInfo vecSDTFreq;
							for(size_t n=0;n<vecSDTtemp.size();n++)
							{
								if(vecSDTtemp[n].TsID==tempNIT.TsID&&vecSDTtemp[n].OrgNetID==tempNIT.OrgNetID)
								{
									vecSDTFreq.push_back(vecSDTtemp[n]);
								}
							}
							if(vecSDTFreq.size()>0)
							{
								mapSDT.insert(make_pair(tempNIT.Freq,vecSDTFreq));
							}
						}
					}
					vecNIT.push_back(tempNIT);
				}
			}
		}
		if (tabletype=="EIT")
		{
			//更新mapEIT信息
			EIT tempEIT;
			pXMLNODE pSectionNode = pXP.GetNodeFromPath("PSISI");
			pXMLNODELIST pNodeList=pXP.GetNodeList(pSectionNode);
			int count=pXP.GetChildCount(pSectionNode);
			for(int i=0;i<count;i++)
			{
				string tempFreq;
				size_t j=0;
				pXMLNODE childNode=pXP.GetNextNode(pNodeList);
				pXP.GetAttrNode(childNode,"ServiceID",tempEIT.ServiceID);
				pXP.GetAttrNode(childNode,"TransportStreamID",tempEIT.TsID);
				pXP.GetAttrNode(childNode,"OriginalNetworkID",tempEIT.OrgNetID);
				for(j=0;j<vecNIT.size();j++)
				{
					if(vecNIT[j].TsID==tempEIT.TsID&&vecNIT[j].OrgNetID==tempEIT.OrgNetID)
					{
						tempFreq=vecNIT[j].Freq;
						break;
					}
				}
				pXMLNODE ListNode=pXP.GetNodeFirstChild(childNode);
				pXMLNODELIST EventList = pXP.GetNodeList(ListNode);
				int childcount=pXP.GetChildCount(ListNode);
				for(int k=0;k<childcount;k++)
				{
					pXMLNODE EventNode=pXP.GetNextNode(EventList);
					if(EventNode==NULL)
						continue;
					pXP.GetAttrNode(EventNode,"TimeOfChange",tempEIT.ProgramStartTime);
					pXP.GetAttrNode(EventNode,"Duration",tempEIT.ProgramDuration);
					pXP.GetAttrNode(EventNode,"RunningStatus",tempEIT.RunningStatus);
					pXP.GetAttrNode(EventNode,"FreeCAMode",tempEIT.FreeCAMode);
					pXMLNODE curNode = pXP.findNode(EventNode,"EventDescriptor");
					if(curNode==NULL)
						continue;
					pXP.GetAttrNode(curNode,"EventName",tempEIT.ProgramName);

					//如果当前频点不在vecNIT中则保存在vecEITtemp中
					if(j==vecNIT.size())
					{
						vecEITtemp.push_back(tempEIT);
						continue;
					}
					mapEITInfo::iterator iter;
					if((iter=mapEIT.find(tempFreq))!=mapEIT.end())
					{
						vctEITInfo::iterator pitor;
						for(pitor=(*iter).second.begin();pitor!=(*iter).second.end();pitor++)
						{
							if(pitor->TsID==tempEIT.TsID && pitor->OrgNetID==tempEIT.OrgNetID &&pitor->ServiceID==tempEIT.ServiceID && pitor->EventID==tempEIT.EventID)
							{
								if(tempEIT.FreeCAMode!=""&&tempEIT.ProgramDuration!=""&&tempEIT.ProgramName!=""&&tempEIT.ProgramStartTime!=""&&tempEIT.RunningStatus!="")
								{
									(*iter).second.erase(pitor);
								}
								break;
							}
						}
						if(tempEIT.FreeCAMode!=""&&tempEIT.ProgramDuration!=""&&tempEIT.ProgramName!=""&&tempEIT.ProgramStartTime!=""&&tempEIT.RunningStatus!="")
						{
							(*iter).second.push_back(tempEIT);
						}
					}
					else
					{
						vctEITInfo vecEIT;
						vecEIT.push_back(tempEIT);
						mapEIT.insert(make_pair(tempFreq,vecEIT));
					}
				}
			}
		}
		if (tabletype=="SDT")
		{
			//更新mapSDT信息
			SDT tempSDT;
			size_t j=0;
			string tempFreq;
			pXMLNODE rootNode = pXP.GetRootNode();
			pXMLNODELIST sectionList=pXP.GetNodeList(rootNode);
			int sectioncount=pXP.GetChildCount(rootNode);
			for(int k=0;k<sectioncount;k++)
			{
				pXMLNODE pSectionNode = pXP.GetNextNode(sectionList);
				pXP.GetAttrNode(pSectionNode,"TransportStreamID",tempSDT.TsID);
				pXP.GetAttrNode(pSectionNode,"OriginalNetworkID",tempSDT.OrgNetID);

				for(j=0;j<vecNIT.size();j++)
				{
					if(vecNIT[j].TsID==tempSDT.TsID&&vecNIT[j].OrgNetID==tempSDT.OrgNetID)
					{
						tempFreq=vecNIT[j].Freq;
						break;
					}
				}

				pXMLNODE pNode= pXP.findNode(pSectionNode,"ServiceList");
				pXMLNODELIST pNodeList=pXP.GetNodeList(pNode);
				int count=pXP.GetChildCount(pNode);
				for(int i=0;i<count;i++)
				{
					pXMLNODE childNode=pXP.GetNextNode(pNodeList);
					if(childNode==NULL)
						continue;
					pXP.GetAttrNode(childNode,"ServiceID",tempSDT.ServiceID);
					pXP.GetAttrNode(childNode,"FreeCAMode",tempSDT.FreeCAMode);
					pXMLNODE curNode = pXP.findNode(childNode,"ServiceDescriptor");
					if(curNode==NULL)
						continue;
					pXP.GetAttrNode(curNode,"ServiceType",tempSDT.ServiceType);
					pXP.GetAttrNode(curNode,"ServiceName",tempSDT.ServiceName);

					//如果当前频点不在vecNIT中则保存在vecSDTtemp中
					if(j==vecNIT.size())
					{
						vecSDTtemp.push_back(tempSDT);
						continue;
					}
					mapSDTInfo::iterator iter;
					if((iter=mapSDT.find(tempFreq))!=mapSDT.end())
					{
						vctSTDInfo::iterator pitor;
						for(pitor=(*iter).second.begin();pitor!=(*iter).second.end();pitor++)
						{
							if(pitor->TsID==tempSDT.TsID && pitor->OrgNetID==tempSDT.OrgNetID &&pitor->ServiceID==tempSDT.ServiceID)
							{
								if(tempSDT.FreeCAMode!=""&&tempSDT.ServiceName!=""&&tempSDT.ServiceType!="")
								{
									(*iter).second.erase(pitor);
								}
								break;
							}
						}
						if(tempSDT.FreeCAMode!=""&&tempSDT.ServiceName!=""&&tempSDT.ServiceType!="")
						{
							(*iter).second.push_back(tempSDT);
						}
					}
					else
					{
						vctSTDInfo vecSDT;
						vecSDT.push_back(tempSDT);
						mapSDT.insert(make_pair(tempFreq,vecSDT));
					}

				}
			}
		}
		if (tabletype=="PAT")
		{
			//更新mapPAT信息
			pXMLNODE rootNode = pXP.GetRootNode();
			pXMLNODELIST SectionList= pXP.GetNodeList(rootNode);
			int sectioncount = pXP.GetChildCount(rootNode);
			for(int k=0;k<sectioncount;k++)
			{
				pXMLNODE SectionNode = pXP.GetNextNode(SectionList);
				pXMLNODE pNode= pXP.findNode(SectionNode,"ProgrameList");
				pXMLNODELIST pNodeList=pXP.GetNodeList(pNode);
				int count=pXP.GetChildCount(pNode);
				for(int i=0;i<count;i++)
				{
					PAT tempPAT;
					pXMLNODE childNode=pXP.GetNextNode(pNodeList);
					pXP.GetAttrNode(childNode,"ProgramNumber",tempPAT.ProgramNumber);
					pXP.GetAttrNode(childNode,"Pid",tempPAT.ProgramID);

					mapPATInfo::iterator iter;
					if((iter=mapPAT.find(freq))!=mapPAT.end())
					{
						vctPATInfo::iterator pitor;
						for(pitor=(*iter).second.begin();pitor!=(*iter).second.end();pitor++)
						{
							if(pitor->ProgramNumber==tempPAT.ProgramNumber && pitor->ProgramID==tempPAT.ProgramID )
							{
								(*iter).second.erase(pitor);
								break;
							}
						}
						(*iter).second.push_back(tempPAT);
					}
					else
					{
						vctPATInfo vecPAT;
						vecPAT.push_back(tempPAT);
						mapPAT.insert(make_pair(freq,vecPAT));
					}
				}
			}
		}
		if (tabletype=="PMT")
		{
			//更新mapPMT信息
			PMT tempPMT;
			pXMLNODE rootNode = pXP.GetRootNode();
			pXMLNODELIST SectionList = pXP.GetNodeList(rootNode);
			int sectioncount=pXP.GetChildCount(rootNode);

			pXP.GetAttrNode(rootNode,"pid",tempPMT.PmtPID);
			for(int j=0;j<sectioncount;j++)
			{
				pXMLNODE pSectionNode = pXP.GetNextNode(SectionList);
				pXP.GetAttrNode(pSectionNode,"ProgramNumber",tempPMT.ProgramNumber);
				pXP.GetAttrNode(pSectionNode,"PcrPid",tempPMT.PcrPID);
				pXMLNODE pNode= pXP.findNode(pSectionNode,"StreamList");
				pXMLNODELIST pNodeList=pXP.GetNodeList(pNode);
				int count=pXP.GetChildCount(pNode);
				bool audioflag=false;
				for(int i=0;i<count;i++)
				{
					string streamtype;
					string streampid;
					STREAM tempStream;
					pXMLNODE childNode=pXP.GetNextNode(pNodeList);
					pXP.GetAttrNode(childNode,"StreamType",streamtype);
					pXP.GetAttrNode(childNode,"StreamPid",streampid);
					tempStream.StreamPid=streampid;
					tempStream.StreamType=streamtype;
					tempPMT.vecStream.push_back(tempStream);
					if(streamtype=="1" || streamtype=="2" || streamtype=="27" || streamtype=="30")
					{
						tempPMT.VideoPID=streampid;
					}
					else if(!audioflag && (streamtype=="3" || streamtype == "4" || streamtype == "6" \
										|| streamtype=="15" || streamtype == "17" || streamtype == "28"))
					{
						tempPMT.AudioPID = streampid;
						audioflag = true;
					}
				}
				mapPMTInfo::iterator iter;
				if((iter=mapPMT.find(freq))!=mapPMT.end())
				{
					vctPMTInfo::iterator pitor;
					for(pitor=(*iter).second.begin();pitor!=(*iter).second.end();pitor++)
					{
						if(pitor->ProgramNumber==tempPMT.ProgramNumber && pitor->VideoPID==tempPMT.VideoPID &&pitor->AudioPID==tempPMT.AudioPID )
						{
							(*iter).second.erase(pitor);
							break;
						}
					}
					(*iter).second.push_back(tempPMT);
				}
				else
				{
					vctPMTInfo vecPMT;
					vecPMT.push_back(tempPMT);
					mapPMT.insert(make_pair(freq,vecPMT));
				}				
			}
		}
		else if(tabletype=="HDTV")//解析高清频道相关信息的xml文件
		{
			HDTV tempHDTV;
			pXP.GetAttrNode(childNode,"ServiceID",tempHDTV.ServiceID);
			pXP.GetAttrNode(childNode,"TransportStreamID",tempHDTV.TsID);
			pXP.GetAttrNode(childNode,"OriginalNetworkID",tempHDTV.OrgNetID);
			tempHDTV.Freq=freq;
			vctHDTVInfo::iterator iter=vecHDTV.begin();
			for(;iter!=vecHDTV.end();iter++)
			{
				if((*iter).Freq == tempHDTV.Freq && /*(*iter).OrgNetID == tempHDTV.OrgNetID && */(*iter).ServiceID == tempHDTV.ServiceID && (*iter).TsID == tempHDTV.TsID)
				{
					vecHDTV.erase(iter);
					break;
				}
			}
			vecHDTV.push_back(tempHDTV);
		}
	}
	catch(...)
	{

	}
	return true;
}

bool PSISIParser::GetVersionInfo( mapFileVersion& mapfileversion )
{
	mapfileversion=mapVersion;
	return true;
}

bool PSISIParser::Init()
{
	ACE_Guard<ACE_Thread_Mutex> guard(Mutex);
	return LoadPSISIInfoFromFile("");
}

bool PSISIParser::Init(string taskname)
{
	ACE_Guard<ACE_Thread_Mutex> guard(Mutex);
	return LoadPSISIInfoFromFile(taskname);
}