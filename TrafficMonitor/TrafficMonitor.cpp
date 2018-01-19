
// TrafficMonitor.cpp : ����Ӧ�ó��������Ϊ��
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "TrafficMonitorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrafficMonitorApp

BEGIN_MESSAGE_MAP(CTrafficMonitorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CTrafficMonitorApp ����

CTrafficMonitorApp::CTrafficMonitorApp()
{
	// ֧����������������
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: �ڴ˴����ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}

void CTrafficMonitorApp::LoadConfig()
{
	//m_no_multistart_warning_time = GetPrivateProfileInt(_T("other"), _T("no_multistart_warning_time"), 300000, m_config_path.c_str());
	m_no_multistart_warning = (GetPrivateProfileInt(_T("other"), _T("no_multistart_warning"), 0, m_config_path.c_str()) != 0);
}

void CTrafficMonitorApp::SaveConfig()
{
	//CCommon::WritePrivateProfileIntW(_T("other"), _T("no_multistart_warning_time"), m_no_multistart_warning_time, m_config_path.c_str());
	CCommon::WritePrivateProfileIntW(_T("other"), _T("no_multistart_warning"), m_no_multistart_warning, m_config_path.c_str());
}

int CTrafficMonitorApp::DPI(int pixel)
{
	return m_dpi * pixel / 96;
}

void CTrafficMonitorApp::GetDPI(CWnd* pWnd)
{
	CWindowDC dc(pWnd);
	HDC hDC = dc.GetSafeHdc();
	m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
}

void CTrafficMonitorApp::CheckUpdate()
{
	CWaitCursor wait_cursor;
	wstring version_info;
	CCommon::GetURL(L"https://raw.githubusercontent.com/zhongyang219/TrafficMonitor/master/version.info", version_info);		//��ȡ�汾��Ϣ
	size_t index, index1, index2, index3;
	index = version_info.find(L"<version>");
	index1 = version_info.find(L"</version>");
	index2 = version_info.find(L"<link>");
	index3 = version_info.find(L"</link>");
	wstring version;
	wstring link;
	version = version_info.substr(index + 9, index1 - index - 9);
	link = version_info.substr(index2 + 6, index3 - index2 - 6);
	if (version > VERSION)		//����������ϵİ汾���ڱ��ذ汾
	{
		CString info;
		info.Format(_T("��⵽�°汾 V%s���Ƿ�ǰ�����£�"), version.c_str());
		if (AfxMessageBox(info, MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			ShellExecute(NULL, _T("open"), link.c_str(), NULL, NULL, SW_SHOW);		//ת����������
		}
	}
	else
	{
		AfxMessageBox(_T("��ǰ�Ѿ������°汾��"), MB_OK | MB_ICONINFORMATION);
	}
}


// Ψһ��һ�� CTrafficMonitorApp ����

CTrafficMonitorApp theApp;


// CTrafficMonitorApp ��ʼ��

BOOL CTrafficMonitorApp::InitInstance()
{
	//���������ļ���·��
	m_module_path = CCommon::GetExePath();
	m_config_path = m_module_path + L"config.ini";
	m_history_traffic_path = m_module_path + L"history_traffic.dat";
	m_log_path = m_module_path + L"error.log";
	m_skin_path = m_module_path + L"skins";

	m_is_windows10_fall_creator = CCommon::IsWindows10FallCreatorOrLater();

	//��ini�ļ���������
	LoadConfig();

#ifndef _DEBUG
	wstring cmd_line{ m_lpCmdLine };
	bool is_restart{ cmd_line.find(L"RestartByRestartManager") != wstring::npos };		//��������в����к����ַ�����RestartByRestartManager����˵�������Ǳ�Windows����������
	//bool when_start{ CCommon::WhenStart(m_no_multistart_warning_time) };
	if (is_restart && m_is_windows10_fall_creator)		//��ǰWindows�汾���＾�����߸���ʱ���������������������ֱ���˳�����
	{
		//AfxMessageBox(_T("������Ϣ�������ѱ�Windows����������������������"));
		return FALSE;
	}

	//����Ƿ�����ʵ����������
	HANDLE hMutex = ::CreateMutex(NULL, TRUE, _T("FirstName"));
	if (hMutex != NULL)
	{
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			//char buff[128];
			//string cmd_line_str{ CCommon::UnicodeToStr(cmd_line.c_str()) };
			//sprintf_s(buff, "when_start=%d, m_no_multistart_warning=%d, cmd_line=%s", when_start, m_no_multistart_warning, cmd_line_str.c_str());
			//CCommon::WriteLog(buff, _T(".\\start.log"));
			if (!m_no_multistart_warning)
			{
				AfxMessageBox(_T("�Ѿ���һ�������������С�"));
			}
			return FALSE;
		}
	}
#endif

	SaveConfig();

	// ���һ�������� Windows XP �ϵ�Ӧ�ó����嵥ָ��Ҫ
	// ʹ�� ComCtl32.dll �汾 6 ����߰汾�����ÿ��ӻ���ʽ��
	//����Ҫ InitCommonControlsEx()��  ���򣬽��޷��������ڡ�
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// ��������Ϊ��������Ҫ��Ӧ�ó�����ʹ�õ�
	// �����ؼ��ࡣ
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// ���� shell ���������Է��Ի������
	// �κ� shell ����ͼ�ؼ��� shell �б���ͼ�ؼ���
	CShellManager *pShellManager = new CShellManager;

	// ���Windows Native���Ӿ����������Ա��� MFC �ؼ�����������
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// ��׼��ʼ��
	// ���δʹ����Щ���ܲ�ϣ����С
	// ���տ�ִ���ļ��Ĵ�С����Ӧ�Ƴ�����
	// ����Ҫ���ض���ʼ������
	// �������ڴ洢���õ�ע�����
	// TODO: Ӧ�ʵ��޸ĸ��ַ�����
	// �����޸�Ϊ��˾����֯��
	SetRegistryKey(_T("Ӧ�ó��������ɵı���Ӧ�ó���"));

	CTrafficMonitorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: �ڴ˷��ô�����ʱ��
		//  ��ȷ�������رնԻ���Ĵ���
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: �ڴ˷��ô�����ʱ��
		//  ��ȡ�������رնԻ���Ĵ���
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "����: �Ի��򴴽�ʧ�ܣ�Ӧ�ó���������ֹ��\n");
		TRACE(traceAppMsg, 0, "����: ������ڶԻ�����ʹ�� MFC �ؼ������޷� #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS��\n");
	}

	// ɾ�����洴���� shell ��������
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// ���ڶԻ����ѹرգ����Խ����� FALSE �Ա��˳�Ӧ�ó���
	//  ����������Ӧ�ó������Ϣ�á�
	return FALSE;
}
