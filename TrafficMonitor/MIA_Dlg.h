#pragma once
#include "ColorStatic.h"
#include "afxwin.h"
#include "MIA_SettingListCtrl.h"
#include "BaseDialog.h"

// CMonitorItemAttributesDlg 对话框

class CMonitorItemAttributesDlg : public CBaseDialog
{
	DECLARE_DYNAMIC(CMonitorItemAttributesDlg)

public:
	CMonitorItemAttributesDlg(CLayout& layout, bool bMainWnd, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CMonitorItemAttributesDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MONITOR_ITEM_ATTRIBUTES_DIALOG };
#endif
	virtual BOOL OnInitDialog()			override;
	virtual void OnOK()					override;
protected:
	CMonitorItemAttributesSettingListCtrl		m_list_ctrl;
	CLayout&	m_layout;
	bool B_MainWnd{ false };							//如果为true，则为主窗口文本设置，否则为任务栏窗口设置

    virtual CString GetDialogName() const override;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
    afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedRestoreDefaultButton();
	afx_msg void OnClickedRestoreSkinDefaultButton();
public:
	afx_msg void OnBnClickedSetFontButton();
};
