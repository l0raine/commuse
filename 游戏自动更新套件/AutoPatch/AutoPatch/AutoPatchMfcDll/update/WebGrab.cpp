/////////////////////////////////////////////////////////////////////////////
// WebGrab.cpp : implementation file
//
// CWebGrab - CHttpFile wrapper class
//
// Written by Chris Maunder <cmaunder@mail.com>
// Copyright (c) 1998-2002. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
// History: 19 Nov 1999 - Release
//          26 Jan 2002 - Update by Bryce to include Proxy support and
//                        property accessors (transfer rate, error msg
//                        etc)
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <afxdtctl.h>		// Internet Explorer 4 公共控件的 MFC 支持
#include "WebGrab.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define BUFFER_SIZE 4095

/////////////////////////////////////////////////////////////////////////////
// CWebGrab

CWebGrab::CWebGrab()
{
	m_pSession = NULL;
	m_timeOut = 0;
	m_useProxy = false;
	m_infoStatusCode=0;

	m_pFile		= NULL;
	m_pBuf		= NULL;
	m_dwCount	= 0;
	m_dwDownSize= 0;
}

CWebGrab::~CWebGrab()
{
    Close();
}

BOOL CWebGrab::Initialise(LPCTSTR szAgentName /*=NULL*/, CWnd* pWnd /*=NULL*/)
{
    Close();
	m_infoStatusCode=0;
    m_pSession = new CWebGrabSession(szAgentName, pWnd);

	if (m_timeOut != 0)
		m_pSession->SetOption(INTERNET_OPTION_DATA_RECEIVE_TIMEOUT  ,m_timeOut);

	// added Bryce
	if (m_useProxy)
	{
		char buf[10];
		itoa(m_Port,buf,10);
		CString temp = m_Proxy+":"+(CString)buf;
		INTERNET_PROXY_INFO proxyinfo;
		proxyinfo.dwAccessType = INTERNET_OPEN_TYPE_PROXY;
		proxyinfo.lpszProxy = temp;
		proxyinfo.lpszProxyBypass = NULL;
		m_pSession->SetOption(INTERNET_OPTION_PROXY, (LPVOID)&proxyinfo, sizeof(INTERNET_PROXY_INFO));
	}

	
	return (m_pSession != NULL);
}

void CWebGrab::Close()
{
	EndDownload();
    if (m_pSession)
    {
        m_pSession->Close();
		delete m_pSession;
    }
    m_pSession = NULL;
}

CString CWebGrab::GetErrorMessage()
{
	return m_pSession->GetErrorMessage();
}

void CWebGrab::SetTimeOut(DWORD timeOut)
{
	m_timeOut = timeOut;
}

double CWebGrab::GetRate()
{
	return m_transferRate;
}

void CWebGrab::SetProxyServer(LPCSTR server)
{
	m_Proxy = server;
}

void CWebGrab::SetProxyPort(UINT port)
{
	m_Port = port;
}

void CWebGrab::SetUseProxy(bool use)
{
	m_useProxy = use;
}

void CWebGrab::SetProxy(LPCSTR proxy, WORD port, bool useProxy )
{
	SetProxyServer(proxy);
	SetProxyPort(port);
	SetUseProxy(useProxy);
}

SHORT CWebGrab::GetErrorCode()
{
	return (!m_ErrorMessage.IsEmpty()); //just for now say...
}



BOOL CWebGrab::BeginDownload(	LPCTSTR szURL,
								LPCTSTR szAgentName /*=NULL*/, 
								CWnd* pWnd /*=NULL*/)
{
	m_rawHeaders ="";
	m_infoStatusCode=0;
	m_strBuffer.Empty();

	if (!m_pSession && !Initialise(szAgentName, pWnd))
		return FALSE;

	if (pWnd)
		m_pSession->SetStatusWnd(pWnd);
	m_dwCount = 0;
	m_pFile = NULL;
	try
	{
		m_pFile = (CHttpFile*) m_pSession->OpenURL(szURL, 1,
			INTERNET_FLAG_TRANSFER_BINARY 
			);
	}
	catch (CInternetException* e)
	{
		TCHAR   szCause[255];
		e->GetErrorMessage(szCause, 255);
		m_pSession->SetStatus(szCause);
		// e->ReportError();
		e->Delete();
		::delete m_pFile;
		m_pFile = NULL;
		return FALSE;
	}
	COleDateTime startTime = COleDateTime::GetCurrentTime();
	m_pBuf = NULL;
	if (m_pFile)
	{
		m_pBuf = (LPSTR) ::GlobalAlloc(GMEM_FIXED, BUFFER_SIZE+1);
		if (!m_pBuf)
		{
			m_pFile->Close();
			::delete m_pFile;
			return FALSE;
		}
	}
	return TRUE;
}


DWORD CWebGrab::DownloadSome()
{
	if( !m_pFile )
	{
		assert( false );
		return eDownloadError;
	}
	UINT nRead = 0;
	try 
	{
		BYTE buffer[BUFFER_SIZE+1];
		
		
		nRead = m_pFile->Read(buffer, BUFFER_SIZE);
		if (nRead > 0)
		{
			buffer[nRead] = 0;

			LPTSTR ptr = m_strBuffer.GetBufferSetLength(m_dwCount + nRead + 1);
			memcpy(ptr+m_dwCount, buffer, nRead);

			m_dwCount += nRead;
			m_strBuffer.ReleaseBuffer(m_dwCount+1);
		}
	}
	catch (CFileException *e)
	{
		TCHAR   szCause[255];
		e->GetErrorMessage(szCause, 255);
		m_ErrorMessage = szCause;
		m_pSession->SetStatus(szCause);
		//e->ReportError();
		e->Delete();
		::delete m_pFile;
		::GlobalFree(m_pBuf);	// mem leak fix by Niek Albers 
		m_pBuf = NULL;
		return eDownloadError;
	}
	if( nRead == 0 )
	{
		EndDownload();
		return eDownloadOver;
	}
	
	return eDownloadContinue;
}

void CWebGrab::EndDownload()
{
	if( m_pFile )
	{
		m_pFile->QueryInfoStatusCode(m_infoStatusCode);       
		m_pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS ,m_rawHeaders);
		m_pFile->Close();
		::delete m_pFile;
		m_pFile = NULL;
	}
	if( m_pBuf )
	{
		::GlobalFree(m_pBuf);	// mem leak fix by Niek Albers 
		m_pBuf = NULL;
	}	
}

DWORD	CWebGrab::GetDownSize() const
{
	return m_dwDownSize;
}
void	CWebGrab::SetDownSize(DWORD dwSize)
{
	m_dwDownSize = dwSize;
}

BOOL CWebGrab::GetFile(LPCTSTR szURL, CString& strBuffer, 
                       LPCTSTR szAgentName /*=NULL*/, CWnd* pWnd /*=NULL*/)
{
	//TRACE("++++++++++ is %d\n", ::GetCurrentThreadId());
	m_rawHeaders ="";
	m_infoStatusCode=0;
	strBuffer.Empty();

    if (!m_pSession && !Initialise(szAgentName, pWnd))
        return FALSE;

    if (pWnd)
        m_pSession->SetStatusWnd(pWnd);

    //m_pSession->SetStatus("Downloading file...");

    DWORD dwCount = 0;
    CHttpFile* pFile = NULL;
    try
    {
        pFile = (CHttpFile*) m_pSession->OpenURL(szURL, 1,
                                                 INTERNET_FLAG_TRANSFER_BINARY 
                                                 //| INTERNET_OPEN_FLAG_USE_EXISTING_CONNECT |
                                                 //| INTERNET_FLAG_DONT_CACHE
                                                 //| INTERNET_FLAG_RELOAD
                                                 );
    
		
	}
    catch (CInternetException* e)
    {
        TCHAR   szCause[255];
        e->GetErrorMessage(szCause, 255);
        m_pSession->SetStatus(szCause);
        // e->ReportError();
        e->Delete();
		::delete pFile;
        pFile = NULL;
        return FALSE;
    }
    
    COleDateTime startTime = COleDateTime::GetCurrentTime();
    LPSTR pBuf = NULL;
    if (pFile)
    {
		HGLOBAL hMem=::GlobalAlloc(GMEM_FIXED, BUFFER_SIZE+1);
        pBuf = (LPSTR)::GlobalLock(hMem);
		
        if (!pBuf)
        {
            pFile->Close();
			::delete pFile;
            return FALSE;
        }

        //BYTE buffer[BUFFER_SIZE+1];
        try {
			
            UINT nRead = 0;
            dwCount = 0;
            do
            {
				if (pFile)
				{
					nRead = pFile->Read(pBuf, BUFFER_SIZE);
				}
                
                if (nRead > 0)
                {
                     pBuf[nRead] = 0;
					 LPTSTR ptr = strBuffer.GetBufferSetLength(dwCount + nRead + 1);
					 memcpy(ptr+dwCount, pBuf, nRead);

					 dwCount += nRead;
					 strBuffer.ReleaseBuffer(dwCount+1);
					
                    COleDateTimeSpan elapsed = COleDateTime::GetCurrentTime() - startTime;
                    double dSecs = elapsed.GetTotalSeconds();
                    if (dSecs > 0.0)
					{
                        m_transferRate = (double)dwCount / 1024.0 / dSecs;
						m_pSession->SetStatus(TEXT("Read %d bytes (%0.1f Kb/s)"), 
                                             dwCount, m_transferRate );
 						m_dwDownSize += nRead;	
					}
					else
					{
                        m_pSession->SetStatus(TEXT("Read %d bytes"), dwCount);
						m_transferRate = dwCount;
						m_dwDownSize += nRead;	
					}
					
                }
            }
            while (nRead > 0 );

        }
        catch (CFileException *e)
        {
            TCHAR   szCause[255];
            e->GetErrorMessage(szCause, 255);
			m_ErrorMessage = szCause;
            m_pSession->SetStatus(szCause);
            //e->ReportError();
            e->Delete();
			::delete pFile;
            ::GlobalFree(pBuf);	// mem leak fix by Niek Albers 
			::GlobalUnlock(hMem);
            return FALSE;
        }
        pFile->QueryInfoStatusCode(m_infoStatusCode);       
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS ,m_rawHeaders);
        pFile->Close();
	  ::GlobalFree(pBuf);	// mem leak fix by Niek Albers 
	  ::GlobalUnlock(hMem);
	  delete pFile;
    }

    m_pSession->SetStatus(TEXT(""));

    return TRUE;
}

BOOL CWebGrab::GetFile(LPCTSTR szURL, std::vector<char>& buffer, 
					   DWORD& dwCurDowned,DWORD& dwTotal,
					   LPCTSTR szAgentName , CWnd* pWnd )
{
	//TRACE("++++++++++ is %d\n", ::GetCurrentThreadId());
	m_rawHeaders ="";
	m_infoStatusCode=0;

	if (!m_pSession && !Initialise(szAgentName, pWnd))
		return FALSE;

	if (pWnd)
		m_pSession->SetStatusWnd(pWnd);

	//m_pSession->SetStatus("Downloading file...");

	DWORD dwCount = 0;
	dwCurDowned=dwTotal=0;
	CHttpFile* pFile = NULL;
	try
	{
		pFile = (CHttpFile*) m_pSession->OpenURL(szURL, 1,
			INTERNET_FLAG_TRANSFER_BINARY 
			//| INTERNET_OPEN_FLAG_USE_EXISTING_CONNECT |
			//| INTERNET_FLAG_DONT_CACHE
			//| INTERNET_FLAG_RELOAD
			);


	}
	catch (CInternetException* e)
	{
		TCHAR   szCause[255];
		e->GetErrorMessage(szCause, 255);
		m_pSession->SetStatus(szCause);
		// e->ReportError();
		e->Delete();
		::delete pFile;
		pFile = NULL;
		return FALSE;
	}

	COleDateTime startTime = COleDateTime::GetCurrentTime();
	LPSTR pBuf = NULL;
	if (pFile)
	{
		dwTotal=pFile->GetLength();
		//ULONGLONG nSize=pFile->SeekToEnd();
		//pFile->SeekToBegin();
		HGLOBAL hMem=::GlobalAlloc(GMEM_FIXED, BUFFER_SIZE+1);
		pBuf = (LPSTR)::GlobalLock(hMem);

		if (!pBuf)
		{
			pFile->Close();
			::delete pFile;
			return FALSE;
		}

		//BYTE buffer[BUFFER_SIZE+1];
		try {
			buffer.clear();

			UINT nRead = 0;
			dwCount = 0;
			do
			{
				if (pFile)
				{
					nRead = pFile->Read(pBuf, BUFFER_SIZE);
				}

				if (nRead > 0)
				{
					pBuf[nRead] = 0;
					//                     LPTSTR ptr = strBuffer.GetBufferSetLength(dwCount + nRead + 1);
					//                     memcpy(ptr+dwCount, pBuf, nRead);
					// 
					//                     dwCount += nRead;
					//                     strBuffer.ReleaseBuffer(dwCount+1);
					dwCount += nRead;
					dwCurDowned=dwCount;
					buffer.insert(buffer.begin()+buffer.size(),pBuf,pBuf+nRead);


					COleDateTimeSpan elapsed = COleDateTime::GetCurrentTime() - startTime;
					double dSecs = elapsed.GetTotalSeconds();
					if (dSecs > 0.0)
					{
						m_transferRate = (double)dwCount / 1024.0 / dSecs;
						m_pSession->SetStatus(TEXT("Read %d bytes (%0.1f Kb/s)"), 
							dwCount, m_transferRate );
						m_dwDownSize += nRead;	
					}
					else
					{
						m_pSession->SetStatus(TEXT("Read %d bytes"), dwCount);
						m_transferRate = dwCount;
						m_dwDownSize += nRead;	
					}

				}
			}
			while (nRead > 0 );

		}
		catch (CFileException *e)
		{
			TCHAR   szCause[255];
			e->GetErrorMessage(szCause, 255);
			m_ErrorMessage = szCause;
			m_pSession->SetStatus(szCause);
			//e->ReportError();
			e->Delete();
			::delete pFile;
			::GlobalFree(pBuf);	// mem leak fix by Niek Albers 
			::GlobalUnlock(hMem);
			return FALSE;
		}
		pFile->QueryInfoStatusCode(m_infoStatusCode);       
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS ,m_rawHeaders);
		pFile->Close();
		::GlobalFree(pBuf);	// mem leak fix by Niek Albers 
		::GlobalUnlock(hMem);
		delete pFile;
	}

	m_pSession->SetStatus(TEXT(""));

	return TRUE;
}

DWORD CWebGrab::GetPageStatusCode()
{
	return m_infoStatusCode;
}

CString CWebGrab::GetRawHeaders()
{
	return m_rawHeaders;
}



/////////////////////////////////////////////////////////////////////////////
// CWebGrabSession

CWebGrabSession::CWebGrabSession(LPCTSTR szAgentName) 
                : CInternetSession(szAgentName) // , 1, INTERNET_OPEN_TYPE_PRECONFIG, 
{
    CommonConstruct();
}

CWebGrabSession::CWebGrabSession(LPCTSTR szAgentName, CWnd* pStatusWnd) 
                : CInternetSession(szAgentName) //, 1, INTERNET_OPEN_TYPE_PRECONFIG, 
                //                  NULL, NULL, INTERNET_FLAG_ASYNC)
{
    CommonConstruct();
    m_pStatusWnd = pStatusWnd;
}

CWebGrabSession::~CWebGrabSession()
{
}

void CWebGrabSession::CommonConstruct() 
{
    m_pStatusWnd = NULL;
    try {
        EnableStatusCallback(TRUE);
    }
    catch (...)
    {}
}

// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CWebGrabSession, CInternetSession)
	//{{AFX_MSG_MAP(CWebGrabSession)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CWebGrabSession member functions

void CWebGrabSession::OnStatusCallback(DWORD dwContext, 
                                       DWORD dwInternetStatus, 
                                       LPVOID lpvStatusInformation, 
                                       DWORD dwStatusInformationLength)
{
	UNUSED_ALWAYS(dwContext);

    // Status callbacks need thread-state protection. 
    AFX_MANAGE_STATE( AfxGetAppModuleState( ) );

    CString str;

	//TRACE1("Internet context=%d: %d\n", dwContext);

	switch (dwInternetStatus)
	{
	case INTERNET_STATUS_RESOLVING_NAME:
		str.Format(TEXT("Resolving name for %s"), lpvStatusInformation);
		break;

	case INTERNET_STATUS_NAME_RESOLVED:
		str.Format(TEXT("Resolved name for %s"), lpvStatusInformation);
		break;

	case INTERNET_STATUS_HANDLE_CREATED:
		//str.Format("Handle %8.8X created", hInternet);
		break;

	case INTERNET_STATUS_CONNECTING_TO_SERVER:
		{
		//sockaddr* pSockAddr = (sockaddr*) lpvStatusInformation;
		str.Format(TEXT("Connecting to socket address ")); //, pSockAddr->sa_data);
		}
		break;

	case INTERNET_STATUS_REQUEST_SENT:
		str.Format(TEXT("Request sent"));
		break;

	case INTERNET_STATUS_SENDING_REQUEST:
		str.Format(TEXT("Sending request..."));
		break;

	case INTERNET_STATUS_CONNECTED_TO_SERVER:
		str.Format(TEXT("Connected to socket address"));
		break;

	case INTERNET_STATUS_RECEIVING_RESPONSE:
        return;
		str.Format(TEXT("Receiving response..."));
		break;

	case INTERNET_STATUS_RESPONSE_RECEIVED:
		str.Format(TEXT("Response received"));
		break;

	case INTERNET_STATUS_CLOSING_CONNECTION:
		str.Format(TEXT("Closing the connection to the server"));
		break;

	case INTERNET_STATUS_CONNECTION_CLOSED:
		str.Format(TEXT("Connection to the server closed"));
		break;

	case INTERNET_STATUS_HANDLE_CLOSING:
        return;
		str.Format(TEXT("Handle closed"));
		break;

	case INTERNET_STATUS_REQUEST_COMPLETE:
        // See the CInternetSession constructor for details on INTERNET_FLAG_ASYNC.
        // The lpvStatusInformation parameter points at an INTERNET_ASYNC_RESULT 
        // structure, and dwStatusInformationLength contains the final completion 
        // status of the asynchronous function. If this is ERROR_INTERNET_EXTENDED_ERROR, 
        // the application can retrieve the server error information by using the 
        // Win32 function InternetGetLastResponseInfo. See the ActiveX SDK for more 
        // information about this function. 
		if (dwStatusInformationLength == sizeof(INTERNET_ASYNC_RESULT))
		{
			INTERNET_ASYNC_RESULT* pResult = (INTERNET_ASYNC_RESULT*) lpvStatusInformation;
			str.Format(TEXT("Request complete, dwResult = %8.8X, dwError = %8.8X"),
				        pResult->dwResult, pResult->dwError);
		}
		else
			str.Format(TEXT("Request complete"));
		break;

	case INTERNET_STATUS_CTL_RESPONSE_RECEIVED:
	case INTERNET_STATUS_REDIRECT:
	default:
		str.Format(TEXT("Unknown status: %d"), dwInternetStatus);
		break;
	}

    SetStatus(str);

    //TRACE("CWebGrabSession::OnStatusCallback: %s\n",str);
}

void CWebGrabSession::SetStatus(LPCTSTR fmt, ...)
{
    va_list args;
    TCHAR buffer[512];

    va_start(args, fmt);
    _vstprintf(buffer, fmt, args);
    va_end(args);

    //TRACE1("CWebGrabSession::SetStatus: %s\n", buffer);
	errorMessage = (CString) buffer;
    if (m_pStatusWnd)
    {
        m_pStatusWnd->SetWindowText(buffer);
        m_pStatusWnd->RedrawWindow();
    }
}

CString CWebGrabSession::GetErrorMessage()
{
	return errorMessage;
}
