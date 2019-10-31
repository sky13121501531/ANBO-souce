
#include "DeviceAccess.h"

DeviceAccess::DeviceAccess(int deviceid, std::string strIP,int nPort)
{
	DeviceId = deviceid;
	strIPAddress=strIP;
	port=nPort;
	tcTunerFreq.chan = -1;

	tLastSpectrum =  time(0)-10;
	LastStartFreq = 0 ; 
	LastEndFreq = 0;
	LastScanStep = 0;

}
DeviceAccess::DeviceAccess(void)
{

}
DeviceAccess:: ~DeviceAccess()
{

}