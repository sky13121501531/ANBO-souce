///////////////////////////////////////////////////////////////////////////////////////////
// 文件名：UdpAlarmRecvThreadMgr.cpp
///////////////////////////////////////////////////////////////////////////////////////////
#include "SpecTrumOrderSet.h"
#include "../Foundation/PropManager.h"
#include "../BusinessProcess/ChannelInfoMgr.h"
#include "../DeviceAccess/HTTPDeviceAccess.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/AppLog.h"
#include "./SpectrumTCPBaseC.h"
bool SpecFlagSet = false;
std::string SpecOrderSet = "";
int FgNum = -1;
std::string SetFreq = "";
std::string RADIO_SetFreq = "";
extern ACE_Thread_Mutex ALLReadMutex;
SpecTrumOrderSet::SpecTrumOrderSet()
{
}
SpecTrumOrderSet::~SpecTrumOrderSet()
{
}

int SpecTrumOrderSet::Start()
{
	//发送线程开始
	open(0);
	return 0;
}
int SpecTrumOrderSet::open(void*)
{
    bFlag = false;
	activate();
	return 0;
}
int SpecTrumOrderSet::svc()
{
    std::string strIP;
    int strPORT;
    int DevNum = PROPMANAGER::instance()->GetMonitorDevNum();
    for(int i =0;i<DevNum;++i)
    {
        sDeviceInfo DevInfo;
        PROPMANAGER::instance()->GetDevMonitorInfo(i,DevInfo);
        if(DevInfo.logindex=="17")
        {
            strIP = DevInfo.baseIP;
            strPORT = StrUtil::Str2Int(DevInfo.cmdport);
        }
    }
	while(!bFlag)
    {
        //Sleep(10);
        if(SpecFlagSet)
        {
            //设置
            if(FgNum!=-1)
            {
                switch (FgNum)
                {
				case 1://ATV频道扫描  FgNum==1  
                    //SpecOrderSet = "MEASure:SPECtrum:SETTing 0,452500000,805500000,auto-att,ampoff,1MHz,300KHz,average,1000,fft,auto,806,0,0,0,0\r\n";
					SpecOrderSet = "MEASure:SPECtrum:SETTing 0,454000000,812000000,auto-att,ampoff,3MHz,100KHz,average,1000,fft,auto,813,0,0,0,0";
					SpecOrderSet += "\r\n";
					break;
                case 2://RADIO频道扫描  FgNum==2
                    SpecOrderSet = "MEASure:SPECtrum:SETTing 0,97500000,21000000,auto-att,ampoff,30KHz,10KHz,average,1000,fft,auto,211,0,0,0,0";
                    break;
                case 3://ATV-CNR指标  FgNum==3
                    SpecOrderSet = "MEASure:TV:CNR:SETTing ";
                    SpecOrderSet += SetFreq;
                    SpecOrderSet += "Hz";
                    SpecOrderSet += ",";
                    SpecOrderSet += "8000000Hz,5750000Hz,0,gated-off";
					SpecOrderSet += "\r\n";
                    break;
                case 4://ATV-DiffFreq指标  FgNum==4
                    SpecOrderSet = "MEASure:TV:FREQerror:SETTing ";
                    SpecOrderSet += SetFreq;
                    SpecOrderSet += "Hz";
                    SpecOrderSet += ",";
                    SpecOrderSet += "0";
					SpecOrderSet += "\r\n";
                    break;
                case 5://MEASure:FM:FMDev:STOP
                    SpecOrderSet = "MEASure:FM:FMDev:SETTing ";
					SpecOrderSet += RADIO_SetFreq;
					SpecOrderSet += "Hz";
					SpecOrderSet += ",";
					SpecOrderSet += "0,431,auto-att,0,ampoff";
					SpecOrderSet += "\r\n";
					break;
				case 6://调制度 关闭
					SpecOrderSet = "MEASure:FM:FMDev:STOP\r\n";
					break;
				case 7://CNR 关闭
					SpecOrderSet = "MEASure:TV:CNR:STOP\r\n";
					break;
				case 8://DiffFreq 关闭
					SpecOrderSet = "MEASure:TV:FREQerror:STOP\r\n";
					break;
				case 9:
                    SpecOrderSet = "MEASure:FREQuence ";
                    SpecOrderSet += SetFreq;
                    SpecOrderSet += "\r\n";
					break;
                default:
                    break;
                }
				
            }
            std::string strmsg = "";
            bool specFG = false;
			ALLReadMutex.acquire();
			SpectrumTcpBaseC Spectrum(strIP.c_str(),strPORT);
			time_t timeBeg = time(0);
            while(!specFG && SpecOrderSet!="")
            {
                //Sleep(1000);
                if(!Spectrum.ConnectSpecEquiCard())
                {
					if(time(0) - timeBeg > 3)
					{
						specFG = true;
						SpecFlagSet = false;
						if(FgNum != -1)
						{
							FgNum = -1;
						}
						break;
					}
                    continue;
                }
                else
                {
                    if(Spectrum.sendSpecOrder(SpecOrderSet.c_str(),SpecOrderSet.length()))
                    {
						if(FgNum==-1)
						{
							Sleep(2500);
						}
						else if(FgNum==9)//模拟频道取电平
						{
							Sleep(300);
						}
						//else if(FgNum==4)//频偏
						//{
						//	Sleep(500);
						//}
						else
						{
							Sleep(1000);
						}
						cout<<"设置指令："<<SpecOrderSet<<endl;
                        specFG = true;
                        SpecFlagSet = false;
                        if(FgNum != -1)
                        {
                            FgNum = -1;
                        }
                    }
                }
            }
			ALLReadMutex.release();
        }
		Sleep(10);
    }
	return 0;
}
int SpecTrumOrderSet::Stop()
{
    bFlag = true;
    this->wait();
    return 0;
}
