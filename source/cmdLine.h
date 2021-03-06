
#ifndef __CMDLINE_H_
#define __CMDLINE_H_

#include <iostream>
#include <string>
#include "./Foundation/TypeDef.h"
using namespace std;

const string VSCTTB_VERSION("6.0.19.1212");
const string VSCTTB_PROG_VERSION("VSCTTB V6.0.19.1212");
const string VSCTTB_DISPACH("20191212");				//批次：版本发布时间(新安播接口中用到)
const string VSCTTB_URL("http://www.viewscenes.com/");
const string VSCTTB_COPYRIGHT("(c) 2018-2019 Beijing ViewScenes Technogy Co.,Ltd");

void PrintTitle(void);
void PrintUsage(void);
bool SendDeviceOrder(void);
void LoadXMLTask(void);
void CheckSoleProcess();
void CheckDirectoryExsit(void);

#endif
