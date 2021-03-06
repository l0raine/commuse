// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
#include <string>

#include "ModuleManager/PlugManager.h"

//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE 1
//#define RES_TYPE 0   //从文件中加载资源
// #define RES_TYPE 1  //从PE资源中加载UI资源

#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif

struct CFGDATA
{
	//版本号
	std::string sVersion;
	//常用
	std::string sAgentId;
	std::string sBarId;

	//网络定义
	std::string sBarSvrIp;
	std::string sKeSvrIp;
	std::string sKeAssistSvrIp;
	std::string sKeReportSvrIp;
	WORD wBarSvrTCPPort;
	WORD wBarSvrUDPPort;
	WORD wKeSvrTCPPort;
	WORD wKeSvrUDPPort;
	WORD wKeAssistSvrTCPPort;
	WORD wKeAssistSvrUDPPort;
	WORD wKeReportSvrTCPPort;
	WORD wKeReportSvrUDPPort;
	WORD wCltSvrTCPPort;
	WORD wCltSvrUDPPort;

	//客户端保护
	BOOL  bStartCltProtect;

	//提示相关
	BOOL bAllowTipNoTrust;
	BOOL bAllowTipUnKnow;
	BOOL bAllowTipAccount;

	//运行相关
	DWORD dwCurPid;						//当前进程PID
	DWORD dwProtectPid;					//客户端保护的PID
	BOOL bShowCltMainWnd;				//是否显示客户端主窗口
	BOOL bSafeScan;						
	BOOL bShowTrayIcon;
	BOOL bStartAccelerateBall;
	BOOL bShowStartPage;
	BOOL bAllowCloseClt;
	//-------------------------------------------------------------------------------
};

CFGDATA gCFGData;

#define MAKE_CMD(mainCmd,subCmd) (((DWORD)(((DWORD)mainCmd)<<16))&0xffff0000 | (DWORD)subCmd)
    
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
    HRESULT hRes = OleInitialize(NULL);
    SASSERT(SUCCEEDED(hRes));

    int nRet = 0;
    
    SComMgr *pComMgr = new SComMgr;

    //将程序的运行路径修改到项目所在目录所在的目录
    TCHAR szCurrentDir[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));
    LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
    _tcscpy(lpInsertPos + 1, _T(".."));
    SetCurrentDirectory(szCurrentDir);
    {
        BOOL bLoaded=FALSE;
        CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
        CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
        bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [render] failed!"));
        bLoaded=pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
        SASSERT_FMT(bLoaded,_T("load interface [%s] failed!"),_T("imgdecoder"));

        pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
        SApplication *theApp = new SApplication(pRenderFactory, hInstance);
        //从DLL加载系统资源
        HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
        if (hModSysResource)
        {
            CAutoRefPtr<IResProvider> sysResProvider;
            CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
            sysResProvider->Init((WPARAM)hModSysResource, 0);
            theApp->LoadSystemNamedResource(sysResProvider);
            FreeLibrary(hModSysResource);
        }else
        {
            SASSERT(0);
        }

        CAutoRefPtr<IResProvider>   pResProvider;
#if (RES_TYPE == 0)
        CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
        if (!pResProvider->Init((LPARAM)_T("uires"), 0))
        {
            SASSERT(0);
            return 1;
        }
#else 
        CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
        pResProvider->Init((WPARAM)hInstance, 0);
#endif

        theApp->AddResProvider(pResProvider);
        theApp->Init(_T("XML_INIT"));

        
        // BLOCK: Run application
        {
            CMainDlg dlgMain;
            dlgMain.Create(GetActiveWindow());
            dlgMain.SendMessage(WM_INITDIALOG);
            dlgMain.CenterWindow(dlgMain.m_hWnd);
            dlgMain.ShowWindow(SW_SHOWNORMAL);

			//测试插件
			for (int i = 0;i<10;i++)
			{
				PlugManager::Instance().LoadAllPlugin();
				while(!PlugManager::Instance().IsPlugRunning())Sleep(1000);
				
				PlugManager::Instance().OnHandleMessage(110,"你好hello",9);
				PlugManager::Instance().OnHandleMessage(MAKE_CMD(3,1),&gCFGData,sizeof(gCFGData));

				PlugManager::Instance().StopAllPlugin();
				Sleep(10);
			}
			

            nRet = theApp->Run(dlgMain.m_hWnd);
        }

        delete theApp;
    }
    
    delete pComMgr;
    
    OleUninitialize();
	

	TerminateProcess(INVALID_HANDLE_VALUE,0);
    return nRet;
}
