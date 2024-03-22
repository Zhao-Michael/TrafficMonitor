#pragma once
#include "ColorStatic.h"
#include "afxwin.h"
#include "ColorSettingListCtrl.h"
#include "BaseDialog.h"

// CTaskbarColorDlg 对话框

class CTaskbarColorDlg : public CBaseDialog
{
	DECLARE_DYNAMIC(CTaskbarColorDlg)

public:
	CTaskbarColorDlg(const std::map<CommonDisplayItem, LayoutItem>& layoutItems, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CTaskbarColorDlg();

    const std::map<CommonDisplayItem, LayoutItem>& GetLayoutItems() const { return m_layout_items; }

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TASKBAR_COLOR_DIALOG };
#endif

protected:
	std::map<CommonDisplayItem, LayoutItem> m_layout_items;
    CColorSettingListCtrl m_list_ctrl;

    virtual CString GetDialogName() const override;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
    afx_msg void OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult);
};
