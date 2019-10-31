
#include "MySqlDBConnection.h"
#include "../Foundation/AppLog.h"
#include "MySql.h"
#include "../Foundation/PropManager.h"
#include "../Foundation/StrUtil.h"
#include "../Foundation/TimeUtil.h"
MySqlDBConnection::MySqlDBConnection(void)
{
	DBCon=new MySql;
}

MySqlDBConnection::~MySqlDBConnection(void)
{
	if (DBCon!=NULL)
	{
		delete DBCon;
		DBCon=NULL;
	}
}

bool MySqlDBConnection::InitConnect(void)
{
	bool ret=false;
	int count=0;
	try
	{
		while ((count<3)&&(!DBCon->BConnIsConnected()))
		{
			DBCon->Server(PROPMANAGER::instance()->GetDbIp());//数据库ip
			DBCon->Username(PROPMANAGER::instance()->GetDbUsername());//数据库用户名
			DBCon->Password(PROPMANAGER::instance()->GetDbPwd());//数据库密码
			DBCon->Port(PROPMANAGER::instance()->GetDbPort());//数据库端口
			DBCon->Database(PROPMANAGER::instance()->GetDbName());//数据库名字
			DBCon->BConnByStr();//连接数据库
			count ++;
		}
		if(DBCon->BConnIsConnected())
			ret = true;			
		return ret;
	}

	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER, LOG_EVENT_ERROR, "打开数据库出错");
		return false;
	}
}
bool MySqlDBConnection::DisConnect(void)
{
	bool ret=false;
	if (DBCon->BConnIsConnected())
	{
		ret= DBCon->BConnDisConnect();//断开数据库
	}
	return ret;
}

bool MySqlDBConnection::AddXmlTask( eDVBType type,std::string xml )
{
	bool ret=false;
	int id=type;
	DeleteTask(type,xml);
	std::string sql="insert into task(dvbtype,taskinfo) values('"+StrUtil::Int2Str(id)+"','"+xml+"')";//sql插入
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行插入
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"向数据库中添加任务出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::DeleteTask( eDVBType type,std::string xml )
{
	int id=type;
	bool ret=false;
	std::string sql="delete from task where dvbtype='"+StrUtil::Int2Str(id)+"' and taskinfo='"+xml+"'";//删除sql
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行删除
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除任务出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryTask( eDVBType type,std::vector<std::string>& vecXml )
{
	int id=type;
	bool ret=false;
	std::string sql="select taskinfo from task where dvbtype='"+StrUtil::Int2Str(id)+"'";//查询sql
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))
		{
			std::string str;
			while(!DBCon->BRecIsEof())
			{
				str=DBCon->SFldItemToString("taskinfo");//获得taskinfo列数据
				vecXml.push_back(str);
				DBCon->BRecMoveToNext();//移动至下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryTask(std::vector<std::string>& vecXml )
{
	bool ret=false;
	std::string sql="select taskinfo from task";//查询sql
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))
		{
			std::string str;
			while(!DBCon->BRecIsEof())
			{
				str=DBCon->SFldItemToString("taskinfo");//获得taskinfo列数据
				vecXml.push_back(str);
				DBCon->BRecMoveToNext();//移动至下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询任务出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::AddChannel( eDVBType type,std::string channelid,std::string xml )
{
	
	int id=type;
	bool ret=false;
	//std::string delsql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";//删除sql
	std::string addsql="insert into channel(dvbtype,channelid,channelinfo) values('"+StrUtil::Int2Str(id)+"','"+channelid+"','"+xml+"')";//添加sql
	try
	{
		ret=DBCon->BDoSql(addsql.c_str());//先删除，后添加
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"向数据库中添加频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::DeleteChannel( eDVBType type,std::string channelid )
{
	int id=type;
	bool ret=false;
	std::string sql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";//删除sql
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行删除sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::DeleteChannelByType( eDVBType type )
{
	int id=type;
	bool ret=false;
	std::string sql="delete from channel where dvbtype='"+StrUtil::Int2Str(id)+"' ";//删除sql
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行删除sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"从数据库中删除频道扫描结果出错",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryChannel( eDVBType type,std::string channelid,std::string& xml )
{
	int id=type;
	bool ret=false;
	std::string sql="select channelinfo from channel where dvbtype='"+StrUtil::Int2Str(id)+"' and channelid='"+channelid+"'";//查询sql
	try
	{

		if (DBCon->BOpenSql(sql.c_str()))
		{
			while(!DBCon->BRecIsEof())//记录不为空
			{
				xml=DBCon->SFldItemToString("channelinfo");//获得channelinfo信息
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频道扫描结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryAllChannel( std::vector<std::string>&vecXml )
{
	bool ret=false;
	std::string sql="select channelinfo from channel";//查询sql
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))
		{
			while(!DBCon->BRecIsEof())//记录不为空
			{
				string xml=DBCon->SFldItemToString("channelinfo");//得到channelinfo列数据
				vecXml.push_back(xml);
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频道扫描结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryTypeChannel( eDVBType type,std::vector<std::string>&vecXml )
{
	bool ret=false;
	int id=type;
	std::string sql="select channelinfo from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";//查询sql
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))
		{
			while(!DBCon->BRecIsEof())//记录不为空
			{
				string xml=DBCon->SFldItemToString("channelinfo");//获得channelinfo列数据
				vecXml.push_back(xml);
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频道扫描结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::AddRecord( sRecordInfo record )
{
	bool ret=false;
	int type=record.dvbtype;
	string sql="insert into record(dvbtype,channelid,taskid,filename,starttime,endtime,expiredays) values('";
	sql+=StrUtil::Int2Str(type)+"','";
	sql+=record.channelID+"','";
	sql+=record.taskid+"','";
	sql+=record.filename+"','";
	sql+=record.starttime+"','";
	sql+=record.endtime+"','";
	sql+=record.expiredays+"')";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行添加录像sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}

bool MySqlDBConnection::QueryRecordByTaskid( eDVBType type,std::string taskId,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	int id=type;
	bool ret=false;
	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id) + string("' and taskid='") + taskId + string("'");
	
	if (StartTime != "" && EndTime != "")
	{
		sql += string(" and StartTime >='") + StartTime+ string("'");
		sql += string(" and EndTime <='") + EndTime + string("'");
	}
	else if (StartTime != "" && EndTime == "")
	{
		sql += string(" and StartTime >='") + StartTime + string("'");
	}
	else if (StartTime == "" && EndTime != "")
	{
		sql += string(" and EndTime <='") + EndTime + string("'");
	}
	sql+=" order by starttime asc";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				sRecordInfo record;
				record.filename=DBCon->SFldItemToString("filename");//获得文件名
				record.starttime=DBCon->SFldItemToString("starttime");//文件开始时间
				record.endtime=DBCon->SFldItemToString("endtime");//文件结束时间
				vecRecordInfo.push_back(record);
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryRecordByChannelID( eDVBType type,std::string taskID,std::string channelID,std::vector<sRecordInfo>& vecRecordInfo,string StartTime,string EndTime)
{
	int id=type;
	bool ret=false;
	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id)+ string("' and channelid='") + channelID+"' and taskid='"+taskID+"'";

	if (StartTime != "" && EndTime != "")
	{
		sql += string(" and StartTime >='") + StartTime + string("'");
		sql += string(" and EndTime <='") + EndTime + string("'");
	}
	else if (StartTime != "" && EndTime == "")
	{
		sql += string(" and StartTime >='") + StartTime + string("'");
	}
	else if (StartTime == "" && EndTime != "")
	{
		sql += string(" and EndTime <='") + EndTime + string("'");
	}
	sql+=" order by starttime asc";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				sRecordInfo record;
				record.filename=DBCon->SFldItemToString("filename");//获得文件名
				record.starttime=DBCon->SFldItemToString("starttime");//文件开始时间
				record.endtime=DBCon->SFldItemToString("endtime");//文件结束时间
				vecRecordInfo.push_back(record);
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}
bool MySqlDBConnection::QueryRecordByDeviceID(eDVBType type,std::string taskID,std::string DeviceID,std::vector<sRecordInfo>& vecRecordInfo,std::string StartTime,std::string EndTime)
{
	int id=type;
	bool ret=false;
	string sql=string("select * from record where dvbtype='") + StrUtil::Int2Str(id);
	sql += string(string("' and taskid='")+taskID);

	if(StrUtil::Str2Int(DeviceID) <= 0) return false;

	sql += string(string("' and filename like'%/")+ DeviceID + "[_]%'");
	if (StartTime != "" && EndTime != "")
	{
		sql += string(" and StartTime >='") + StartTime + string("'");
		sql += string(" and EndTime <='") + EndTime + string("'");
	}
	else if (StartTime != "" && EndTime == "")
	{
		sql += string(" and StartTime >='") + StartTime + string("'");
	}
	else if (StartTime == "" && EndTime != "")
	{
		sql += string(" and EndTime <='") + EndTime + string("'");
	}
	sql+=" order by starttime asc";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				sRecordInfo record;
				record.filename=DBCon->SFldItemToString("filename");//获得文件名
				record.starttime=DBCon->SFldItemToString("starttime");//文件开始时间
				record.endtime=DBCon->SFldItemToString("endtime");//文件结束时间
				vecRecordInfo.push_back(record);
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::DeleteRecord( std::string filename )
{
	bool ret=false;
	string sql="delete from record where filename='"+filename+"'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行删除录像sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryRecordByDays( unsigned int days,std::vector<std::string>& vecFilename )
{
	return true;		//该函数有问题，暂时忽略 //gxd 2011-03-31
	/************************************************************************/
	
	bool ret=false;
	string sql="select filename from record where expiredays='"+StrUtil::Int2Str(days)+"'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询录像sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("filename");//获得文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryRecordTopInPath(unsigned int count, std::string recPath, std::vector<std::string>& vecFilename)
{
	//没有mysql数据库，未测试
	bool ret = false;

	string sql = "select * from record where filename like \'%"+recPath+"%\' order by expiredays,starttime";
	if (count > 0)
		sql += " limit " + StrUtil::Int2Str(count);

	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询录像sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("filename");//获得文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
	
}
bool MySqlDBConnection::QueryRecordTop(unsigned int count, std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string sql = "select * from record order by expiredays,starttime";
	if (count > 0)
		sql += " limit " + StrUtil::Int2Str(count);
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询录像sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("filename");//获得文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryRecordExpire( std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string sql = "select * from record where DATE_FORMAT(starttime, '%Y%m%d')+ExpireDays <DATE_FORMAT(now(), '%Y%m%d') order by starttime ";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询录像sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("filename");//获得文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询录像信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::IsConnected( void )
{
	return DBCon->BConnIsConnected();
}

bool MySqlDBConnection::QueryChannelID( eDVBType type,std::vector<std::string>&vecChannelID )
{
	bool ret=false;
	int id=type;
	std::string sql="select channelid from channel where dvbtype='"+StrUtil::Int2Str(id)+"'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询sql
		{
			while(!DBCon->BRecIsEof())//判断记录是否为空
			{
				string channelid=DBCon->SFldItemToString("channelid");//获得channelid
				vecChannelID.push_back(channelid);
				DBCon->BRecMoveToNext();//移动到下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频道扫描结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}
//报警参数
bool MySqlDBConnection::UpdateAlarmParam(sAlarmParam& alarmparam)
{
	bool ret = false;

	string insertsql = "insert into AlarmParam(DVBType,DeviceID,AlarmType,TypeID,TypeDesc,STD,Freq,SymbolRate,";

	insertsql += "ChannelID,Duration,Num,TimeInterval,DownThreshold,UpThreshold,Switch) values('";
	insertsql += StrUtil::Int2Str(alarmparam.DVBType)+string("','")+alarmparam.DeviceID+string("','")+StrUtil::Int2Str(alarmparam.AlarmType)+string("','");
	insertsql += alarmparam.TypeID+string("','")+alarmparam.TypeDesc+string("','")+alarmparam.STD+string("','")+alarmparam.Freq+string("','");
	insertsql += alarmparam.SymbolRate+string("','")+alarmparam.ChannelID+string("','")+alarmparam.Duration+string("','");
	insertsql += alarmparam.Num+string("','")+alarmparam.TimeInterval+string("','")+alarmparam.DownThreshold+string("','");
	insertsql += alarmparam.UpThreshold+string("','")+alarmparam.Switch+string("')");

	string selsql = string("select * from AlarmParam  ") ;
	string wheresql=string("where DVBType = '") +  StrUtil::Int2Str(alarmparam.DVBType)+  string("' and DeviceID = '") + alarmparam.DeviceID+
		string("' and AlarmType = '") + StrUtil::Int2Str(alarmparam.AlarmType) + string("' and TypeID = '") + alarmparam.TypeID +"'";

	if (alarmparam.AlarmType == ALARM_FREQ || alarmparam.AlarmType == ALARM_TR101_290||alarmparam.AlarmType == ALARM_PROGRAM)
	{
		wheresql += string((" and Freq = '")+alarmparam.Freq + string("' and SymbolRate = '") + alarmparam.SymbolRate+string("'  and ChannelID = '")+alarmparam.ChannelID)+"'";
	}
	else if (alarmparam.AlarmType == ALARM_ENVIRONMENT)
	{
		;
	}
	else
	{
		return false;
	}
	selsql+=wheresql;
	string updatesql="";
	if(alarmparam.Switch!="")
	{
		updatesql="update AlarmParam set Switch = '"+alarmparam.Switch+"' ";
	}
	if(alarmparam.Duration!=""||alarmparam.TimeInterval!=""||alarmparam.Num!=""||alarmparam.DownThreshold!=""||alarmparam.UpThreshold!="")
	{
		updatesql="update AlarmParam set Duration = '"+alarmparam.Duration+"' ,Num = '"+alarmparam.Num+"' ,TimeInterval = '"+alarmparam.TimeInterval+"' ,DownThreshold = '"+alarmparam.DownThreshold+"' ,UpThreshold = '"+alarmparam.UpThreshold+"' ";
		if(alarmparam.Switch!="")
		{
			updatesql+=" ,Switch = '"+alarmparam.Switch+"' ";
		}
	}
	updatesql+=wheresql;

	try
	{
		ret=DBCon->BDoSql(selsql.c_str());//执行查询sql
		if(DBCon->IRecGetRecordCount()==0)
		{
			ret=DBCon->BDoSql(insertsql.c_str());//执行添加sql
		}
		else
		{
			ret=DBCon->BDoSql(updatesql.c_str());
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新报警参数出错！",LOG_OUTPUT_FILE);
		return false;

	}
	return ret;
}
bool MySqlDBConnection::QueryAlarmParam(eDVBType type,std::vector<sAlarmParam>& AlarmParamVec)
{
	bool ret=false;
	std::string sql="select * from AlarmParam where DVBType = ";
	sql += StrUtil::Int2Str(type);
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询sql
		{
			while (!DBCon->BRecIsEof())//记录不为空
			{
				sAlarmParam param;
				param.DVBType=type;
				int alarmtype = StrUtil::Str2Int(DBCon->SFldItemToString("AlarmType"));
				switch(alarmtype)
				{
				case 0:
					param.AlarmType=ALARM_NONE;
					break;
				case 1:
					param.AlarmType=ALARM_ENVIRONMENT;
					break;
				case 2:
					param.AlarmType=ALARM_FREQ;
					break;
				case 3:
					param.AlarmType=ALARM_PROGRAM;
					break;
				case 4:
					param.AlarmType=ALARM_TR101_290;
					break;
				default:
					param.AlarmType=ALARM_NONE;
					break;
				}
				//获得报警参数信息
				param.TypeID = DBCon->SFldItemToString("TypeID");
				param.TypeDesc = DBCon->SFldItemToString("TypeDesc");
				param.STD = DBCon->SFldItemToString("STD");
				param.Freq = DBCon->SFldItemToString("Freq");
				param.SymbolRate = DBCon->SFldItemToString("SymbolRate");
				param.ChannelID = DBCon->SFldItemToString("ChannelID");
				param.Duration = DBCon->SFldItemToString("Duration");
				param.Num =DBCon->SFldItemToString("Num");
				param.TimeInterval = DBCon->SFldItemToString("TimeInterval");
				param.DownThreshold = DBCon->SFldItemToString("DownThreshold");
				param.UpThreshold = DBCon->SFldItemToString("UpThreshold");
				param.Switch = DBCon->SFldItemToString("Switch");
				param.DeviceID=DBCon->SFldItemToString("DeviceID");
				AlarmParamVec.push_back(param);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryAlarmParam(std::vector<sAlarmParam>& AlarmParamVec)
{
	bool ret=false;
	std::string sql="select * from AlarmParam";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询sql
		{
			while (!DBCon->BRecIsEof())//记录不为空
			{
				sAlarmParam param;
				string dvbtype;
				dvbtype=DBCon->SFldItemToString("dvbtype");
				switch(StrUtil::Str2Int(dvbtype))
				{
				case 1:
					param.DVBType=CTTB;
					break;
				case 2:
					param.DVBType=CMMB;
					break;
				case 3:
					param.DVBType=DVBC;
					break;
				case 4:
					param.DVBType=RADIO;
					break;
				case 5:
					param.DVBType=ATV;
					break;
				case 6:
					param.DVBType=AM;
					break;
				case 7:
					param.DVBType=DVBS;
					break;
				case 8:
					param.DVBType=CTV;
					break;
				default:
					param.DVBType=UNKNOWN;
					break;
				}
				int alarmtype = StrUtil::Str2Int(DBCon->SFldItemToString("AlarmType"));
				switch(alarmtype)
				{
				case 0:
					param.AlarmType=ALARM_NONE;
					break;
				case 1:
					param.AlarmType=ALARM_ENVIRONMENT;
					break;
				case 2:
					param.AlarmType=ALARM_FREQ;
					break;
				case 3:
					param.AlarmType=ALARM_PROGRAM;
					break;
				case 4:
					param.AlarmType=ALARM_TR101_290;
					break;
				default:
					param.AlarmType=ALARM_NONE;
					break;
				}
				//获得报警参数信息
				param.TypeID = DBCon->SFldItemToString("TypeID");
				param.TypeDesc = DBCon->SFldItemToString("TypeDesc");
				param.STD = DBCon->SFldItemToString("STD");
				param.Freq = DBCon->SFldItemToString("Freq");
				param.SymbolRate = DBCon->SFldItemToString("SymbolRate");
				param.ChannelID = DBCon->SFldItemToString("ChannelID");
				param.Duration = DBCon->SFldItemToString("Duration");
				param.Num =DBCon->SFldItemToString("Num");
				param.TimeInterval = DBCon->SFldItemToString("TimeInterval");
				param.DownThreshold = DBCon->SFldItemToString("DownThreshold");
				param.UpThreshold = DBCon->SFldItemToString("UpThreshold");
				param.Switch = DBCon->SFldItemToString("Switch");
				param.DeviceID=DBCon->SFldItemToString("DeviceID");
				AlarmParamVec.push_back(param);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::AddRunPlanParam( sRunPlanParam runPlanParam )
{
	bool ret=false;
	std::string sql="insert into runplanparam(dvbtype,channelid,type,dayofweek,mmonth,dayofmonth,starttime,endtime,validstartdatetime,validenddatetime,startdatetime,enddatetime) values('"+StrUtil::Int2Str(runPlanParam.dvbtype)+"','";
	sql+=runPlanParam.ChannelID+"','";
	sql+=StrUtil::Int2Str(runPlanParam.Type)+"','";
	sql+=runPlanParam.DayOfWeek+"','";
	sql+=runPlanParam.Month+"','";
	sql+=runPlanParam.DayOfMonth+"','";
	sql+=runPlanParam.StartTime+"','";
	sql+=runPlanParam.EndTime+"','";
	sql+=runPlanParam.ValidStartDateTime+"','";
	sql+=runPlanParam.ValidEndDateTime+"','";
	sql+=runPlanParam.StartDateTime+"','";
	sql+=runPlanParam.EndDateTime+"')";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行添加运行图参数
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加运行图参数出错！",LOG_OUTPUT_FILE);
		return false;

	}
	return ret;
}

bool MySqlDBConnection::QueryRunPlanParam( eDVBType type,std::vector<sRunPlanParam>& runPlanParam )
{
	bool ret=false;
	std::string sql="select * from runplanparam where dvbtype='"+StrUtil::Int2Str(type)+"' order by dvbtype,channelid";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行运行图参数查询
		{
			while (!DBCon->BRecIsEof())//记录不为空
			{
				//获得运行图参数信息
				sRunPlanParam param;
				param.dvbtype=type;
				param.ChannelID=DBCon->SFldItemToString("channelid");
				param.Type=StrUtil::Str2Int(DBCon->SFldItemToString("type"));
				param.DayOfWeek=DBCon->SFldItemToString("dayofweek");
				param.Month=DBCon->SFldItemToString("mmonth");
				param.DayOfMonth=DBCon->SFldItemToString("dayofmonth");
				param.StartTime=DBCon->SFldItemToString("starttime");
				param.EndTime=DBCon->SFldItemToString("endtime");
				param.ValidStartDateTime=DBCon->SFldItemToString("validstartdatetime");
				param.ValidEndDateTime=DBCon->SFldItemToString("validenddatetime");
				param.StartDateTime=DBCon->SFldItemToString("startdatetime");
				param.EndDateTime=DBCon->SFldItemToString("enddatetime");
				runPlanParam.push_back(param);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

bool MySqlDBConnection::QueryAllRunPlanParam( std::vector<sRunPlanParam>& runPlanParam )
{
	return true;bool ret=false;
	std::string sql="select * from runplanparam order by dvbtype,channelid";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询sql
		{
			while (!DBCon->BRecIsEof())//记录是否为空
			{
				sRunPlanParam param;
				std::string dvbtype;
				dvbtype=DBCon->SFldItemToString("dvbtype");
				switch(StrUtil::Str2Int(dvbtype))
				{
				case 1:
					param.dvbtype=CTTB;
					break;
				case 2:
					param.dvbtype=CMMB;
					break;
				case 3:
					param.dvbtype=DVBC;
					break;
				case 4:
					param.dvbtype=RADIO;
					break;
				case 5:
					param.dvbtype=ATV;
					break;
				case 6:
					param.dvbtype=AM;
					break;
				case 7:
					param.dvbtype=DVBS;
					break;
				case 8:
					param.dvbtype=CTV;
					break;
				default:
					param.dvbtype=UNKNOWN;
					break;

				}
				//生成报警参数
				param.ChannelID=DBCon->SFldItemToString("channelid");
				param.Type=StrUtil::Str2Int(DBCon->SFldItemToString("type"));
				param.DayOfWeek=DBCon->SFldItemToString("dayofweek");
				param.Month=DBCon->SFldItemToString("mmonth");
				param.DayOfMonth=DBCon->SFldItemToString("dayofmonth");
				param.StartTime=DBCon->SFldItemToString("starttime");
				param.EndTime=DBCon->SFldItemToString("endtime");
				param.ValidStartDateTime=DBCon->SFldItemToString("validstartdatetime");
				param.ValidEndDateTime=DBCon->SFldItemToString("validenddatetime");
				param.StartDateTime=DBCon->SFldItemToString("startdatetime");
				param.EndDateTime=DBCon->SFldItemToString("enddatetime");
				runPlanParam.push_back(param);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

bool MySqlDBConnection::DeleteRunPlanParam( eDVBType type,string channelid )
{
	bool ret=false;
	std::string sql="delete from runplanparam where dvbtype='"+StrUtil::Int2Str(type)+"'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行删除运行图sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除运行图参数出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::UpdateRunPlanParam(eDVBType dvbtype,string channelid, vector<sRunPlanParam> runPlanParam )
{
	if (runPlanParam.size()>0)
		DeleteRunPlanParam(dvbtype,channelid);//删除运行图参数信息
	for (size_t i=0;i!=runPlanParam.size();++i)
	{
		AddRunPlanParam(runPlanParam[i]);//添加运行图参数
	}
	return true;
}

bool MySqlDBConnection::AddQualityInfo( eDVBType dvbtype,std::string freq,std::string taskid,eQualityInfo quality,std::string checktime )
{
	bool ret=false;
	string sql="insert into quality(dvbtype,freq,taskid,type,des,valu,checktime) values('"+StrUtil::Int2Str(dvbtype)+"','";
	sql+=freq+"','";
	sql+=taskid+"','";
	sql+=quality.type+"','";
	sql+=quality.desc+"','";
	sql+=quality.valu+"','";
	sql+=checktime+"')";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行添加指标信息sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryQualityInfo( eDVBType dvbtype,std::string freq,std::string taskid,std::vector<eQualityInfo>& vecQuality,string startdatetime,string enddatetime )
{
	bool ret=false;
	string sql;
	if(freq!="")
	{
		sql="select * from quality where dvbtype='"+StrUtil::Int2Str(dvbtype)+"' and freq='"+freq;
	}
	else
	{
		sql="select * from quality where dvbtype='"+StrUtil::Int2Str(dvbtype);
	}
	sql+="' and taskid='";
	sql+=taskid+"' and checktime>='";
	sql+=startdatetime+"' and checktime<='";
	sql+=enddatetime+"'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				//获得指标信息
				eQualityInfo qulity;
				qulity.type=DBCon->SFldItemToString("type");
				qulity.valu=DBCon->SFldItemToString("valu");
				qulity.desc=DBCon->SFldItemToString("des");
				qulity.checktime=DBCon->SFldItemToString("checktime");
				qulity.freq=DBCon->SFldItemToString("freq");
				vecQuality.push_back(qulity);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询指标出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::GetMsgID( std::string &MsgID )
{
	bool ret=false;
	string sql="select msgid from MsgID";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				MsgID=DBCon->SFldItemToString("msgid");
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询MsgID出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::UpdateMsgID( std::string MsgID )
{
	bool ret=false;
	string sql="update MsgID set msgid='"+MsgID+"'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//更新MsgID
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新MsgID息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryCompensationValu(std::vector<sQualityCompensation>& qualityCmpVec)
{
	bool ret=false;
	string sql="select * from qualitycompensation";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				std::string dvbtype;
				sQualityCompensation quality;
				dvbtype=DBCon->SFldItemToString("dvbtype");
				switch(StrUtil::Str2Int(dvbtype))
				{
				case 1:
					quality.dvbtype=CTTB;
					break;
				case 2:
					quality.dvbtype=CMMB;
					break;
				case 3:
					quality.dvbtype=DVBC;
					break;
				case 4:
					quality.dvbtype=RADIO;
					break;
				case 5:
					quality.dvbtype=ATV;
					break;
				case 6:
					quality.dvbtype=AM;
					break;
				case 7:
					quality.dvbtype=DVBS;
					break;
				case 8:
					quality.dvbtype=CTV;
					break;
				default:
					quality.dvbtype=UNKNOWN;
					break;
				}
				quality.deviceid=DBCon->SFldItemToString("deviceid");
				quality.type=DBCon->SFldItemToString("type");
				quality.valu=DBCon->SFldItemToString("valu");
				qualityCmpVec.push_back(quality);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::UpdateCompensationValu( sQualityCompensation& qualityCmp )
{
	return DelCompensationValu(qualityCmp)&&AddCompensationValu(qualityCmp);
}

bool MySqlDBConnection::AddCompensationValu(sQualityCompensation& qualityCmp)
{
	bool ret=false;
	string sql="insert into qualitycompensation(dvbtype,deviceid,type,valu) values('";
	sql+=StrUtil::Int2Str(qualityCmp.dvbtype)+"','";
	sql+=qualityCmp.deviceid+"','";
	sql+=qualityCmp.type+"','";
	sql+=qualityCmp.valu+"')";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//更新MsgID
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::DelCompensationValu( sQualityCompensation& qualityCmp)
{
	bool ret=false;
	string sql="delete from qualitycompensation where dvbtype='";
	sql+=string(StrUtil::Int2Str(qualityCmp.dvbtype)+"' and deviceid='");
	sql+=string(qualityCmp.deviceid+"' and type='");
	sql+=string(qualityCmp.type+"'");
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//更新MsgID
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加指标补偿信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::AddAlarmInfo( sCheckParam& alarminfo,std::string mode )
{
	bool ret=false;
	string sql="insert into alarminfo(dvbtype,alarmtype,std,freq,symbolrate,deviceid,channelid,typeid,typedesc,alarmvalue,checktime,alarmid,mode) values('";
	sql+=string(StrUtil::Int2Str(alarminfo.DVBType)+"','");
	sql+=StrUtil::Int2Str(alarminfo.AlarmType)+"','";
	sql+=string(alarminfo.STD+"','");
	sql+=string(alarminfo.Freq+"','");
	sql+=string(alarminfo.SymbolRate+"','");
	sql+=string(alarminfo.DeviceID+"','");
	sql+=string(alarminfo.ChannelID+"','");
	sql+=string(alarminfo.TypeID+"','");
	sql+=string(alarminfo.TypeDesc+"','");
	sql+=string(alarminfo.TypedValue+"','");
	sql+=TimeUtil::DateTimeToStr(alarminfo.CheckTime)+"','";
	sql+=string(alarminfo.AlarmID+"','");
	sql+=string(mode+"')");
	try
	{
		ret=DBCon->BDoSql(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加报警结果出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryAlarmInfo(string mode,std::vector<sCheckParam>& vecAlarmInfo,bool bRadioNosignal)
{
	bool ret=false;
	string sql="select * from alarminfo where mode='";
	sql+=mode;
	sql+="'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				sCheckParam alarm;
				alarm.DVBType=(enumDVBType)StrUtil::Str2Int(DBCon->SFldItemToString("dvbtype"));
				alarm.STD=DBCon->SFldItemToString("std");
				alarm.Freq=DBCon->SFldItemToString("freq");
				alarm.ChannelID=DBCon->SFldItemToString("channelid");
				alarm.SymbolRate=DBCon->SFldItemToString("symbolrate");
				alarm.TypeID=DBCon->SFldItemToString("typeid");
				alarm.TypeDesc=DBCon->SFldItemToString("typedesc");
				alarm.TypedValue=DBCon->SFldItemToString("alarmvalue");
				alarm.DeviceID=DBCon->SFldItemToString("deviceid");
				//alarm.CheckTime=TimeUtil::StrToDateTime(DBCon->SFldItemToString("starttime"));
				alarm.CheckTime = TimeUtil::StrToDateTime( TimeUtil::GetCurDateTime() );
				alarm.AlarmID=DBCon->SFldItemToString("alarmid");
				int alarmtype = StrUtil::Str2Int(DBCon->SFldItemToString("AlarmType"));
				switch(alarmtype)
				{
				case 0:
					alarm.AlarmType=ALARM_NONE;
					break;
				case 1:
					alarm.AlarmType=ALARM_ENVIRONMENT;
					break;
				case 2:
					alarm.AlarmType=ALARM_FREQ;
					break;
				case 3:
					alarm.AlarmType=ALARM_PROGRAM;
					break;
				case 4:
					alarm.AlarmType=ALARM_TR101_290;
					break;
				default:
					alarm.AlarmType=ALARM_NONE;
					break;
				}
				if((bRadioNosignal&&("23" == alarm.TypeID))||(!bRadioNosignal&&("23" != alarm.TypeID)))
				{
					vecAlarmInfo.push_back(alarm);
				}
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}

bool MySqlDBConnection::UpdateAlarmInfo( eDVBType dvbtype,std::string Alarmid,string mode,string unAlarmTime)
{
	bool ret=false;
	string sql="update alarminfo set endtime='" + unAlarmTime + "' , mode='" + mode;
	sql+="' where dvbtype='"+StrUtil::Int2Str(dvbtype);
	sql+="' and alarmid='"+Alarmid;
	sql+="'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除alarmid出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QueryFreqAlarmID( eDVBType dvbtype,string deviceid,string freq,string typeID,string& alarmid )
{
	bool ret=false;
	string sql="select alarmid from alarminfo where dvbtype='";
	sql+=StrUtil::Int2Str(dvbtype)+"' and deviceid='";
	sql+=deviceid+"' and freq='";
	sql+=freq+"' and typeid='";
	sql+=typeID+"'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				alarmid=DBCon->SFldItemToString("alarmid");
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}

bool MySqlDBConnection::QueryProgramAlarmID( eDVBType dvbtype,string deviceid,string channelid,string typeID,string& alarmid )
{
	bool ret=false;
	string sql="select alarmid from alarminfo where dvbtype='";
	sql+=StrUtil::Int2Str(dvbtype)+"' and deviceid='";
	sql+=deviceid+"' and channelid='";
	sql+=channelid+"' and typeid='";
	sql+=typeID+"' and mode='";
	sql+="0'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				alarmid=DBCon->SFldItemToString("alarmid");
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询报警结果信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}

bool MySqlDBConnection::UpdateFileEndTime( std::string filename,std::string endtime )
{
	bool ret=false;
	string sql="update record set endtime='"+endtime;
	sql+="' where filename='";
	sql+=filename;
	sql+="'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::AddSpectrumInfo( eDVBType dvbtype,std::string freq,std::string taskid,eSpecInfo spec,std::string checktime )
{
	bool ret=false;
	string sql="insert into spec(dvbtype,freq,taskid,type,des,valu,status,checktime) values('"+StrUtil::Int2Str(dvbtype)+"','";
	sql+=freq+"','";
	sql+=taskid+"','";
	sql+=spec.type+"','";
	sql+=spec.desc+"','";
	sql+=spec.valu+"','";
	sql+=spec.status+"','";
	sql+=checktime+"')";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行添加指标信息sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加频谱信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

bool MySqlDBConnection::QuerySpecInfo( eDVBType dvbtype,std::string taskid,std::vector<eSpecInfo>& vecSpec,string startdatetime,string enddatetime )
{
	bool ret=false;
	string sql="select * from spec where dvbtype='"+StrUtil::Int2Str(dvbtype);
	sql+="' and taskid='";
	sql+=taskid+"' and checktime>='";
	sql+=startdatetime+"' and checktime<='";
	sql+=enddatetime+"'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询指标sql
		{
			while (!DBCon->BRecIsEof())
			{
				//获得指标信息
				eSpecInfo spec;
				spec.type=DBCon->SFldItemToString("type");
				spec.valu=DBCon->SFldItemToString("valu");
				spec.desc=DBCon->SFldItemToString("des");
				spec.checktime=DBCon->SFldItemToString("checktime");
				spec.freq=DBCon->SFldItemToString("freq");
				spec.status=DBCon->SFldItemToString("status");
				vecSpec.push_back(spec);
				DBCon->BRecMoveToNext();
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询频谱出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool MySqlDBConnection::AddTempFile(string filename, string createtime, string expiredays)
{
	bool ret=false;
	string sql="insert into TempFile(FileName,CreateTime,ExpireDays) values('";
	sql += string(filename + "','");
	sql += string(createtime + "','");
	sql += string(expiredays + "')");

	try
	{
		ret=DBCon->BDoSql(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"添加临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool MySqlDBConnection::QueryTempFilesExpire( std::vector<std::string>& vecFilename )
{
	bool ret=false;
	string sql = "select * from TempFile where DATE_FORMAT(CreateTime, '%Y%m%d')+ExpireDays <DATE_FORMAT(now(), '%Y%m%d') order by CreateTime ";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询临时文件sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("FileName");//获得临时文件的文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool MySqlDBConnection::DeleteTempFile(std::string filename)
{
	bool ret=false;
	string sql="delete from TempFile where FileName='"+filename+"'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());//执行删除临时文件sql
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"删除临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool MySqlDBConnection::QueryTempFileByDays(unsigned int days,std::vector<std::string>& vecFilename)
{
	bool ret=false;
	string sql="select FileName from TempFile where ExpireDays='"+StrUtil::Int2Str(days)+"'";
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询临时文件sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("FileName");//获得临时文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//wz_101229
bool MySqlDBConnection::QueryTempFileTop(unsigned int count,std::vector<std::string>& vecFilename)
{
	bool ret=false;
	string sql = "select * from TempFile order by CreateTime";
	if (count > 0)
		sql += " limit " + StrUtil::Int2Str(count);
	try
	{
		if (DBCon->BOpenSql(sql.c_str()))//执行查询临时文件sql
		{
			while (!DBCon->BRecIsEof())//判断记录是否为空
			{
				string file=DBCon->SFldItemToString("FileName");//获得临时文件名
				vecFilename.push_back(file);
				DBCon->BRecMoveToNext();//移动下一条记录
			}
			ret=true;
		}
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时文件信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;
}

//报警查询 2011-03-19
bool MySqlDBConnection::AlarmInfoQuery(vector<sCheckParam> &vecAlarm)
{
	vector<sCheckParam>::iterator itor=vecAlarm.begin();
	for(;itor!=vecAlarm.end();itor++)
	{
		bool ret=false;
		string sql = "select starttime,endtime,mode from alarminfo where DVBType='"+StrUtil::Int2Str(itor->DVBType)+"' and AlarmType='"+StrUtil::Int2Str(itor->AlarmType);
		sql+="' and Freq='"+itor->Freq+"' and TypeID='"+itor->TypeID+"' and ChannelID='"+itor->ChannelID+"'";

		try
		{
			ret=DBCon->BOpenSql(sql.c_str());//执行查询sql
			if(DBCon->IRecGetRecordCount()==0)
			{
				itor->StrCheckTime=TimeUtil::DateTimeToStr(time(0));
				itor->mode="0";
			}
			else
			{
				while (!DBCon->BRecIsEof())//记录不为空
				{
					string mode,starttime,endtime;
					starttime = DBCon->SFldItemToString("starttime");
					endtime = DBCon->SFldItemToString("endtime");
					mode=DBCon->SFldItemToString("mode");
					if(mode.substr(0,1)=="0")
					{
						itor->StrCheckTime=starttime;
						itor->mode="1";
					}
					else if(mode.substr(0,1)=="1")
					{
						itor->StrCheckTime=endtime;
						itor->mode="2";
					}
					DBCon->BRecMoveToNext();
				}
		
			}
		}
		catch (...)
		{
			APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询临时信息出错！",LOG_OUTPUT_FILE);
			continue;
		}
	}
	return true;
}

bool MySqlDBConnection::AlarmStateClear(string Freq)
{
	bool ret=false;
	string sql="";
	if(Freq=="ALL")
		sql="delete from alarminfo where Freq<>''";
	else
		sql="delete from alarminfo where Freq='"+Freq+"'";
	try
	{
		ret=DBCon->BDoSql(sql.c_str());
	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"报警状态删除信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return true;
}

bool MySqlDBConnection::UpdateAlarmRecordExpireTime(eDVBType dvbtype,std::string Alarmid)
{
	bool ret=false;
	string sql="select channelid,starttime,endtime from alarminfo where alarmid='"+Alarmid+"'";
	try
	{
		ret=DBCon->BOpenSql(sql.c_str());//执行查询sql

		while (!DBCon->BRecIsEof())//记录不为空
		{
			string channelid,starttime,endtime;
			channelid = DBCon->SFldItemToString("channelid");
			starttime = DBCon->SFldItemToString("starttime");
			endtime = DBCon->SFldItemToString("endtime");
			string days=PROPMANAGER::instance()->GetAlarmRecordExpire(dvbtype);

			string updatesql="update record set expiredays='"+days+ "' where dvbtype='"+StrUtil::Int2Str(dvbtype);
			updatesql+="' and channelid='"+channelid;
			updatesql+="' and starttime<='"+endtime;
			updatesql+="' and endtime>='"+starttime+"'";

			try
			{
				ret=DBCon->BDoSql(updatesql.c_str());
			}
			catch (...)
			{
				APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"更新异态录像过期时间出错！",LOG_OUTPUT_FILE);
			}
			DBCon->BRecMoveToNext();
		}

		ret=true;

	}
	catch (...)
	{
		APPLOG::instance()->WriteLog(OTHER,LOG_EVENT_ERROR,"查询异态报警信息出错！",LOG_OUTPUT_FILE);
		return false;
	}
	return ret;

}