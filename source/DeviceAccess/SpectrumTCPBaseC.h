#pragma warning(disable:4996)
#include<stdlib.h>
#include<stdio.h>
#include<Winsock2.h>
//#include<winsock.h>
#include<string>
#include<map>
#pragma comment(lib , "ws2_32.lib")
using namespace std;

class SpectrumTcpBaseC
{
public:
    SpectrumTcpBaseC(const char* IP,int port);
    ~SpectrumTcpBaseC();

public:
    bool SpectrumInitTCPSocketNet();
	bool ConnectSpecEquiCard();
    bool sendSpecOrder(const char* strMsg,int strMsglen);

    bool recSpecOrderforATVQualityCNR(std::string& strBack);
    bool recSpecOrderforATVQualityDiffFreq(std::string& strBack);
    bool recSpecOrderforRADIOQualityDEV(std::string& strBack);
    bool recSpecOrder(std::string& strBack);
	void closeSocet_Now();
public:
    std::string m_ip;
    int m_port;
	std::map<int,double> SpectrumMap;
	SOCKET sclient;
};