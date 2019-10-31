
#include "AlarmSender.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/OSFunction.h"
#include "../Communications/XMLSend.h"
#include "../Communications/SysMsgSender.h"
#include "../Foundation/AppLog.h"
#include "../Foundation/StrUtil.h"
#include "ace/OS.h"
extern std::multimap<std::string,std::string> AlarmUPSnd;
AlarmSender::AlarmSender()
{

}

AlarmSender::~AlarmSender()
{

}

int AlarmSender::open(void *)
{
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)报警检查/发送任务线程开始执行 !\n"));
	msg_queue()->high_water_mark(1024*10240);
	msg_queue()->low_water_mark(1024*10240);
	this->activate();
	return 0;
}

//通过从xml中解析出来的不同类型发送到不同的监测类型报警服务器
int AlarmSender::svc()
{
	bFlag = true;
	ACE_Message_Block *mb = 0;
	while (bFlag)
	{
		try 
		{
			ACE_Time_Value OutTime(ACE_OS::gettimeofday()+ACE_Time_Value(1));
			while (getq(mb,&OutTime) != -1 && mb != NULL)
			{
				if (mb->msg_type() == ACE_Message_Block::MB_BREAK)
				{
					mb->release();
					break;
				}
				if (-1 == ProcessMessage(mb))
				{
					mb->release();
					break;
				}
				mb->release();
				OSFunction::Sleep(0,10);
			}
			OSFunction::Sleep(0,300);
		}
		catch(...)
		{
		}
	}
	ACE_DEBUG ((LM_DEBUG,"(%T | %t)报警检查/发送任务线程停止执行 !\n"));

	return 0;
}

int AlarmSender::ProcessMessage(ACE_Message_Block* mb)
{
	std::string strXML ="";
	char buf[1024*100] = {0};

	memcpy(buf,mb->rd_ptr(),mb->length());//xml拷贝至内存
	strXML = buf;
	XmlParser parser(strXML.c_str());
	pXMLNODE rootNode=parser.GetRootNode();
	string dvbtype,srccode;
	eDVBType type;
	parser.GetAttrNode(rootNode,"Type",dvbtype);
	parser.GetAttrNode(rootNode,"SrcCode",srccode);
	string strtype = "";
	pXMLNODE typeNode=parser.GetNodeFromPath("Msg/Return");
	parser.GetAttrNode(typeNode,"Type",strtype);
	if(strtype=="EquipmentAlarmHistoryQuery" || strtype=="EquipmentAlarmHistoryReport")
	{
		type=ATV;
	}
	if(dvbtype=="MonUp")
		type=DVBC;
	else if(dvbtype=="DVBTHUp")
		type=CTTB;
	else if(dvbtype=="TVMonUp"&&(srccode==PROPMANAGER::instance()->GetDefSrcCode(ATV)))
		type=ATV;
	else if(dvbtype=="TVMonUp"&&(srccode==PROPMANAGER::instance()->GetDefSrcCode(CTV)))
		type=CTV;
	else if(dvbtype=="RadioUp")
		type=RADIO;
	else if(dvbtype=="SatUp")
		type=DVBS;
	else if(dvbtype=="AMUp")
		type=AM;
	else if(dvbtype=="3DUp")
		type=THREED;

	UpAlarmInfo alarminfo;
	PROPMANAGER::instance()->GetUpAlarmInfo(type,alarminfo);
	printf(strXML.c_str());
	if ((strXML.empty()) || (alarminfo.alarmurl.empty()))
		return 0;

	if(alarminfo.type=="0")
		return 0;
    //http://192.168.3.20:8080/WebContent/xmlReceiver/upload;http://192.168.3.222:8080/WebContent/xmlReceiver/upload
    string strurl = alarminfo.alarmurl;
    vector<string> vURL;
    while(true)
    {
        if(strurl!="")
        {
            int alarmURLpos = strurl.find(";");
            if(alarmURLpos!=-1)
            {
                string url = strurl.substr(0,alarmURLpos);
                strurl = strurl.substr(++alarmURLpos,strurl.length() - alarmURLpos);
                vURL.push_back(url);
            }
            else
            {
                vURL.push_back(strurl);
                break;
            }
        }
        else
            break;
    }
    if(vURL.size()>0)
    {
        if(vURL.size() == 1)
        {
            if(false == XMLSend::SendXML(vURL[0],strXML))
            {
                std::string msg = string("发送任务对象XML文件失败！") + "URL:" + vURL[0] + "***" + strXML.c_str();
                SYSMSGSENDER::instance()->SendMsg(msg,type,VS_MSG_SYSALARM);
                string ParentDir="C:/AlarmUp/";
                if(ACE_OS::opendir(ParentDir.c_str())==NULL)   
                {
                    mkdir(ParentDir.c_str());
                } 
                string ChildDir=OSFunction::GetStrDVBType(type);
                string dir=ParentDir+ChildDir+string("/"); 
                if(ACE_OS::opendir(dir.c_str())==NULL)   
                {
                    mkdir(dir.c_str());
                } 
                static int index=1;
                if(index>1000)
                    index=1;
                string FileName=dir+TimeUtil::DateTimeToString(time(0))+string("_")+StrUtil::Int2Str(index)+string(".xml");
                parser.SaveAsFile(FileName.c_str());
                index++;
            }
            else
            {
                std::string msg = string("发送任务对象XML文件成功！") + "URL:" + vURL[0] + "***" + strXML.c_str();
                SYSMSGSENDER::instance()->SendMsg(msg,type,VS_MSG_SYSALARM);
            }
        }
        else//备用上报地址多个
        {
            for(int i =0;i<vURL.size();i++)
            {
                if(false == XMLSend::SendXML(vURL[i],strXML))
                {
                    std::string msg = string("发送任务对象XML文件失败！") + "URL:" + vURL[i] + "***" + strXML.c_str();
                    SYSMSGSENDER::instance()->SendMsg(msg,type,VS_MSG_SYSALARM);
                    GetAlarmInfoByAlarmXML(vURL[i],strXML,false);
                }
                else
                {
                    std::string msg = string("发送任务对象XML文件成功！") + "URL:" + vURL[i] + "***" + strXML.c_str();
                    SYSMSGSENDER::instance()->SendMsg(msg,type,VS_MSG_SYSALARM);
                    GetAlarmInfoByAlarmXML(vURL[i],strXML,true);
                    break;/*配置备用上报地址的，只要发送成功，不可重复多个发送*/
                }
            }
        }
    }
    if(vAlarmParam.size()>0)
    {
        for(int i=0;i<vAlarmParam.size();i++)
        {
            if(vAlarmParam[i].sucFailmode=="0")//发送失败的记录内存
            {
                AlarmUPSnd.insert(make_pair(vAlarmParam[i].url,vAlarmParam[i].alarmxml));//指向url记录发送失败xml
            }
        }
    }
    return 0;
}

int AlarmSender::Stop()
{
	bFlag = false;
	this->wait();
	return 0;
}
void AlarmSender::GetAlarmInfoByAlarmXML(std::string strurl,std::string strxml,bool isFlag)
{
    AlarmResendInfo alarmInfo;
    XmlParser parser(strxml.c_str());
    pXMLNODE qualityalarmreportNODE = parser.GetNodeFromPath("Msg/qualityalarmreport/qualityalarm");
    if(qualityalarmreportNODE==NULL)
    {
        return;
    }
    parser.GetAttrNode(qualityalarmreportNODE,string("alarmid"),alarmInfo.alarmid);
    parser.GetAttrNode(qualityalarmreportNODE,string("mode"),alarmInfo.mode);
    if(isFlag)
    {
        alarmInfo.sucFailmode = "1";
    }
    else
    {
        alarmInfo.sucFailmode = "0";
    }
    std::vector<AlarmResendInfo>::iterator pIter = vAlarmParam.begin();
    for(int i=0;i<vAlarmParam.size();i++)
    {
        if(alarmInfo.mode=="1")
        {
            if(isFlag)//成功
            {
                if(vAlarmParam[i].alarmid == alarmInfo.alarmid)
                {
                    vector<AlarmResendInfo>::iterator temppTter = vAlarmParam.begin() + i;
                    vAlarmParam.erase(temppTter);
                    break;
                }
            }
            else//失败
            {
                if(vAlarmParam[i].alarmid == alarmInfo.alarmid && vAlarmParam[i].mode != alarmInfo.mode)//解报xml不存在，且存在上报xml失败记录，解报时记录指向上报url
                {
                    alarmInfo.url = vAlarmParam[i].url;
                    alarmInfo.alarmxml = strxml;
                    vAlarmParam.push_back(alarmInfo);
                    break;
                }
                else if(vAlarmParam[i].alarmid == alarmInfo.alarmid && vAlarmParam[i].mode == alarmInfo.mode)// 解报xml已经存在，不在计数
                {
                    break;
                }
            }
        }
        else
        {
            if(isFlag && vAlarmParam[i].alarmid == alarmInfo.alarmid && vAlarmParam[i].mode == alarmInfo.mode && vAlarmParam[i].url != strurl)//上报xml第一url失败，后面成功一次，删除该记录
            {
                vector<AlarmResendInfo>::iterator tppTter = vAlarmParam.begin() + i;
                vAlarmParam.erase(tppTter);
                break;
            }
            else if(vAlarmParam[i].alarmid == alarmInfo.alarmid && vAlarmParam[i].mode == alarmInfo.mode)//上报xml都失败，存在时，不在计数
            {
                break;
            }
            else
            {
                alarmInfo.url = strurl;
                alarmInfo.alarmxml = strxml;
                vAlarmParam.push_back(alarmInfo);
                break;
            }
        }
    }
}
