#ifndef __CommLib_H
#define __CommLib_H
#define MARKUP_STL

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <windows.h>
#include <tchar.h>
#include <WinSock2.h>

#include <string>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <iterator>

//-------------------------------------------------------------------------------
//! 基础
#include "ComnDef/cmndef.h"
#include "Interface/CKInterface.h"

//! 加/解密
#include "Kernel/crypt/CRC.h"
#include "Kernel/crypt/Md5.h"
#include "Kernel/crypt/yrc4.h"

//! 单例基类
#include "Kernel/template/SingletonBase.h"

//! 辅助函数
#include "ToolHelper/FunctionHelper.h"
#include "ToolHelper/DeviceOper.h"
#include "ToolHelper/Markup.h"
#include "ToolHelper/NetOper.h"
#include "ToolHelper/Service.h"
#include "ToolHelper/ToolOper.h"
#include "ToolHelper/zip.h"
#include "ToolHelper/unzip.h"
#include "ToolHelper/ILibInterface.h"
#include "ToolHelper/MemLoadDll.h"
#include "ToolHelper/FileMonitor.h"
#include "ToolHelper/DigitSign.h"
#include "ToolHelper/yCMyseclock.h"

//! 线程
#include "Kernel/thread/JKThread.h"

//! COMM库
#include "Kernel/comm/Log/CommLog.h"
#include "Kernel/comm/SafeCriticalSection/SafeCriticalSection.h"
#include "Kernel/comm/ShareMem/ShareMemry.h"
#include "Kernel/comm/ProcessLock/ProcessLock.h"
#include "Kernel/comm/Log/DebugLog.h"

//! 内存池
#include "Kernel/mempool/PoolMemData.h"

//! 网络操作类
#include "Kernel/net/iocp_def.h"
#include "Kernel/net/yCTcp.h"
#include "Kernel/net/yCUdp.h"
#include "NetManager/NetManager.h"
#include "Kernel/net/TcpClient.h"
#include "Kernel/net/UdpClient.h"

//! 下载文件
#include "Kernel/downfile/HttpDownFile.h"
#include "Kernel/downfile/FileDown_C.h"

//! 安装服务
#include "ToolHelper/Server.h"
#include "ToolHelper/WinServer.h"

//! 插件管理
#include "ModuleManager/PlugManager.h"

//! 注入
#include "InjectHelper/MemoryModule.h"
#include "InjectHelper/SsdtOper.h"
#include "InjectHelper/MemoryLoad.h"
#include "InjectHelper/MemRunExe.h"

//! NetWork
#include "NetWork/NetWork.h"
#include "NetWork/MapToXml.h"
#include "NetWork/NetTool.h"
#include "NetWork/CenterNetTool.h"
#include "NetWork/P2pTransmission.h"

//随即运行exe
#include "ToolHelper/RunExeByRandom.h"
//-------------------------------------------------------------------------------

using namespace COMMUSE;
#endif //__CommLib_H