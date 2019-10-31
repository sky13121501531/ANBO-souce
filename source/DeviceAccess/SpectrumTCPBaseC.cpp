#include "SpectrumTCPBaseC.h"
#include "../Foundation/StrUtil.h"
SpectrumTcpBaseC::SpectrumTcpBaseC( const char* IP,int port )
{
    this->m_ip = IP;
    this->m_port = port;
	SpectrumInitTCPSocketNet();
}

SpectrumTcpBaseC::~SpectrumTcpBaseC()
{
	if (sclient != INVALID_SOCKET)
	{
		closesocket(sclient);
		sclient = INVALID_SOCKET;
	}
	WSACleanup();
}
bool SpectrumTcpBaseC::ConnectSpecEquiCard()
{
    sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sclient == INVALID_SOCKET)
    {
        printf("invalid socket !");
        return false;
    }
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    //int SpectrumPort = atoi(this->m_port.c_str());
    serAddr.sin_port = htons(m_port);
    serAddr.sin_addr.S_un.S_addr = inet_addr(this->m_ip.c_str());

    if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {
        printf("TCP����Ƶ���ǳ���!\n");
        return false;
    }
    return true;
}
bool SpectrumTcpBaseC::SpectrumInitTCPSocketNet()
{
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(sockVersion, &data) != 0)
    {
        return false;
    }
    else
	{
		return true;
	}  
}
bool SpectrumTcpBaseC::sendSpecOrder( const char* strMsg,int strMsglen)
{
    int iSend = send(sclient, strMsg, strMsglen, 0);
    if (iSend == SOCKET_ERROR) 
    {
        printf("����Ƶ������ָ��ʧ��!\n");
        return false;
    }
    return true;
}
bool SpectrumTcpBaseC::recSpecOrder(std::string& strBack)
{
    char recData[1024*32] = {0};
    int nNetTimeout = 2000;
    setsockopt(sclient, SOL_SOCKET, SO_RCVTIMEO, (char*)&nNetTimeout,sizeof(int));
    int ret = recv(sclient, recData, sizeof(char)*1024*32, 0);
    if(ret>=0)
    {
        if(ret==0)
        {
            //printf("����ָ�����,�豸�ر�����\n");
            return true;
        }
        else
        {
            int count = 0;
            int ByteNum = 0;
            if (recData[0] == '#')//
            {
                ByteNum = recData[1] - '0';
                char retByte[64] = { 0 };
                switch (ByteNum)
                {
                case 1:sprintf(retByte, "%c", recData[2]); break;//1λ��
                case 2:sprintf(retByte, "%c%c", recData[2], recData[3]); break;//2λ��
                case 3:sprintf(retByte, "%c%c%c", recData[2], recData[3], recData[4]); break;//3λ��
                case 4:sprintf(retByte, "%c%c%c%c", recData[2], recData[3], recData[4], recData[5]); break;//4λ��
                case 5:sprintf(retByte, "%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6]); break;//5λ��
                case 6:sprintf(retByte, "%c%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6], recData[7]); break;//6λ��
                default:
                    break;
                }
                count = atoi(retByte);
                if(count==0)//��Ч����ռ���ֽ���Ϊ0���
                {
                    strBack="";
                    return false;
                }
                int num = 0;
                int nCount = 0;
				string strPointSum = "";
                for (int i = ret - count - 2; i < ret - 2; )
                {
                    if (i < ret - count + 18)//Type��PointSum��PixelSum��FullFlag��SweepTIme
                    {
                        int Rvalue = 0;
                        memcpy(&Rvalue, recData + i,4);
                        int sum = abs(Rvalue);
						num++;
                        i += 4;
                    }
                    else//2���ֽڱ�ʾһ��Ƶ������
                    {
                        short int value = 0;
                        memcpy(&value, recData + i, 2);
                        double sum = abs(value);
                        double Resum = sum /10.00;
                        SpectrumMap.insert(make_pair(nCount,Resum));
                        strBack += StrUtil::Float2Str(Resum);
                        strBack += ";";
                        nCount++;
                        i += 2;
                    }
                }
            }
			else
			{
				strBack = recData;
			}
        }
        return true;
    }
    else
    {
        //printf("����Ƶ���ǻظ�ʧ�ܣ�\n");
        return false;
    }
}

void SpectrumTcpBaseC::closeSocet_Now()
{
	if (sclient != INVALID_SOCKET)
	{
		closesocket(sclient);
		sclient = INVALID_SOCKET;
	}
	WSACleanup();
}

bool SpectrumTcpBaseC::recSpecOrderforATVQualityCNR( std::string& strBack )
{
    char recData[1024*32] = {0};
    int nNetTimeout = 500;
    setsockopt(sclient, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nNetTimeout,sizeof(int));
    int ret = recv(sclient, recData, sizeof(char)*1024*32, 0);
    if(ret>=0)
    {
        if(ret==0)
        {
            //printf("����ָ�����,�豸�ر�����\n");
            return true;
        }
        else
        {
            int count = 0;
            int ByteNum = 0;
            if (recData[0] == '#')//
            {
                ByteNum = recData[1] - '0';
                char retByte[64] = { 0 };
                switch (ByteNum)
                {
                case 1:sprintf(retByte, "%c", recData[2]); break;//1λ��
                case 2:sprintf(retByte, "%c%c", recData[2], recData[3]); break;//2λ��
                case 3:sprintf(retByte, "%c%c%c", recData[2], recData[3], recData[4]); break;//3λ��
                case 4:sprintf(retByte, "%c%c%c%c", recData[2], recData[3], recData[4], recData[5]); break;//4λ��
                case 5:sprintf(retByte, "%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6]); break;//5λ��
                case 6:sprintf(retByte, "%c%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6], recData[7]); break;//6λ��
                default:
                    break;
                }
                count = atoi(retByte);
                if(count==0)//��Ч����ռ���ֽ���Ϊ0���
                {
                    strBack="";
                    return false;
                }
                int num = 0;
                int nCount = 0;
                for (int i = ret - count - 2; i < ret - 2; )
                {
                    if (i < ret - count + 18)
                    {
                        int Rvalue = 0;
                        memcpy(&Rvalue, recData + i,4);
                        float sum = abs(Rvalue);
                        strBack += StrUtil::RoundFloat((double)sum,1);
                        strBack += ";";
                        i += 4;
                    }
                    else
                    {
                        short int value = 0;
                        memcpy(&value, recData + i, 2);
                        float sum = abs(value);
                        float Resum = sum / 10;
                        SpectrumMap.insert(make_pair(nCount,Resum));
                        strBack += StrUtil::Float2Str(Resum);
                        strBack += "---";
                        nCount++;
                        i += 2;
                    }
                    num++;
                }
            }
        }
        return true;
    }
    else
    {
        //printf("����Ƶ���ǻظ�ʧ�ܣ�\n");
        return false;
    }
}

bool SpectrumTcpBaseC::recSpecOrderforATVQualityDiffFreq( std::string& strBack )
{
    char recData[1024*32] = {0};
    int nNetTimeout = 500; 
	setsockopt(sclient,SOL_SOCKET,SO_RCVTIMEO,(const char*)&nNetTimeout,sizeof(int));
	int ret = recv(sclient, recData, sizeof(char)*1024*32, 0);
	if(ret>=0)
	{
		if(ret==0)
		{
			//printf("����ָ�����,�豸�ر�����\n");
			return true;
		}
		else
		{
			int count = 0;
			int ByteNum = 0;
			if (recData[0] == '#')//
			{
				ByteNum = recData[1] - '0';
				char retByte[64] = { 0 };
				switch (ByteNum)
				{
				case 1:sprintf(retByte, "%c", recData[2]); break;//1λ��
				case 2:sprintf(retByte, "%c%c", recData[2], recData[3]); break;//2λ��
				case 3:sprintf(retByte, "%c%c%c", recData[2], recData[3], recData[4]); break;//3λ��
				case 4:sprintf(retByte, "%c%c%c%c", recData[2], recData[3], recData[4], recData[5]); break;//4λ��
				case 5:sprintf(retByte, "%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6]); break;//5λ��
				case 6:sprintf(retByte, "%c%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6], recData[7]); break;//6λ��
				default:
					break;
				}
				count = atoi(retByte);
				if(count==0)//��Ч����ռ���ֽ���Ϊ0���
				{
					strBack="";
					return false;
				}
				int num = 0;
				int nCount = 0;
				string strstr = "";
				for (int i = ret - count - 2; i < ret; )
				{
					if (i < ret - count - 2 + count)
					{
						int Rvalue = 0;
						memcpy(&Rvalue, recData + i,4);
						float sum = abs(Rvalue);
						strstr += StrUtil::RoundFloat((double)sum,1);
						strstr += ";";
						i += 4;
					}
					else//2���ֽڱ�ʾһ��Ƶ������
					{
						short int value = 0;
						memcpy(&value, recData + i, 2);
						float sum = abs(value);
						float Resum = sum / 10;
						strstr += StrUtil::RoundFloat((double)Resum,1);
						strstr += ";";
						nCount++;
						i += 2;
					}
					num++;
				}
				strBack = strstr;
			}
		}
		return true;
	}
	else
	{
		//printf("����Ƶ���ǻظ�ʧ�ܣ�\n");
		return false;
	}
}

bool SpectrumTcpBaseC::recSpecOrderforRADIOQualityDEV( std::string& strBack )
{
    char recData[1024*32] = {0};
    int nNetTimeout = 500; 
    setsockopt(sclient, SOL_SOCKET, SO_RCVTIMEO, (const char*)&nNetTimeout,sizeof(int));
    int ret = recv(sclient, recData, sizeof(char)*1024*32, 0);
    if(ret>=0)
    {
        if(ret==0)
        {
            //printf("����ָ�����,�豸�ر�����\n");
            return true;
        }
        else
        {
            int count = 0;
            int ByteNum = 0;
            if (recData[0] == '#')//
            {
                ByteNum = recData[1] - '0';
                char retByte[64] = { 0 };
                switch (ByteNum)
                {
                case 1:sprintf(retByte, "%c", recData[2]); break;//1λ��
                case 2:sprintf(retByte, "%c%c", recData[2], recData[3]); break;//2λ��
                case 3:sprintf(retByte, "%c%c%c", recData[2], recData[3], recData[4]); break;//3λ��
                case 4:sprintf(retByte, "%c%c%c%c", recData[2], recData[3], recData[4], recData[5]); break;//4λ��
                case 5:sprintf(retByte, "%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6]); break;//5λ��
                case 6:sprintf(retByte, "%c%c%c%c%c%c", recData[2], recData[3], recData[4], recData[5], recData[6], recData[7]); break;//6λ��
                default:
                    break;
                }
                count = atoi(retByte);
                if(count==0)//��Ч����ռ���ֽ���Ϊ0���
                {
                    strBack="";
                    return false;
                }
                int num = 0;
                int nCount = 0;
                for (int i = ret - count - 2; i < ret - 2; )
                {
                    if (i < ret - count + 18)
                    {
                        int Rvalue = 0;
                        memcpy(&Rvalue, recData + i,4);
                        float sum = abs(Rvalue);
                        strBack += StrUtil::RoundFloat((double)sum,1);
                        strBack += ";";
                        i += 4;
                    }
                    else
                    {
                        short int value = 0;
                        memcpy(&value, recData + i, 2);
                        float sum = abs(value);
                        float Resum = sum / 10;
                        SpectrumMap.insert(make_pair(nCount,Resum));
                        strBack += StrUtil::Float2Str(Resum);
                        strBack += "---";
                        nCount++;
                        i += 2;
                    }
                    num++;
                }
            }
        }
        return true;
    }
    else
    {
        //printf("����Ƶ���ǻظ�ʧ�ܣ�\n");
        return false;
    }
}
