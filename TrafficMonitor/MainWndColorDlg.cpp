// MainWndColorDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "MainWndColorDlg.h"
#include "afxdialogex.h"
#include "CMFCColorDialogEx.h"


// CMainWndColorDlg 对话框

IMPLEMENT_DYNAMIC(CMainWndColorDlg, CBaseDialog)

CMainWndColorDlg::CMainWndColorDlg(const std::map<CommonDisplayItem, LayoutItem>& layoutItems, CWnd* pParent /*=NULL*/)
	: CBaseDialog(IDD_MAIN_COLOR_DIALOG, pParent), m_layout_items(layoutItems)
{
}

CMainWndColorDlg::~CMainWndColorDlg()
{
}

CString CMainWndColorDlg::GetDialogName() const
{
    return _T("MainWndColorDlg");
}

void CMainWndColorDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list_ctrl);
}


BEGIN_MESSAGE_MAP(CMainWndColorDlg, CBaseDialog)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CMainWndColorDlg::OnNMDblclkList1)
END_MESSAGE_MAP()


// CMainWndColorDlg 消息处理程序


BOOL CMainWndColorDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

    SetIcon(theApp.GetMenuIcon(IDI_MAIN_WINDOW), FALSE);		// 设置小图标

    //初始化列表控件
    CRect rect;
    m_list_ctrl.GetClientRect(rect);
    m_list_ctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP);

    UCHAR   num = 5;
    int width0 =  rect.Width() / (num--);
    int width1 = (rect.Width() - width0 - theApp.DPI(20) - 1) / (num--);
    int width2 = (rect.Width() - width0 - width1 - theApp.DPI(20) - 1) / (num--);
    int width3 = (rect.Width() - width0 - width1 - width2 - theApp.DPI(20) - 1) / (num--);
    int width4 =  rect.Width() - width0 - width1 - width2 - width3 - theApp.DPI(20) - 1;
    m_list_ctrl.InsertColumn(0, CCommon::LoadText(IDS_DISP_ITEM_ID),LVCFMT_LEFT, width0);
    m_list_ctrl.InsertColumn(1, CCommon::LoadText(IDS_ITEM),        LVCFMT_LEFT, width1);
    m_list_ctrl.InsertColumn(2, CCommon::LoadText(IDS_PREFIX),       LVCFMT_LEFT, width2);
    m_list_ctrl.InsertColumn(3, CCommon::LoadText(IDS_COLOR_LABEL), LVCFMT_LEFT, width3);
    m_list_ctrl.InsertColumn(4, CCommon::LoadText(IDS_COLOR_VALUE), LVCFMT_LEFT, width4);
    m_list_ctrl.SetDrawItemRangMargin(theApp.DPI(2));

    //向列表中插入行
    for (auto iter = m_layout_items.begin(); iter != m_layout_items.end(); ++iter)
    {
        LayoutItem& layout_item = iter->second;
        wstring         item_id = layout_item.id;

        CString item_name = iter->first.GetItemName();
        int index = m_list_ctrl.GetItemCount();
        m_list_ctrl.InsertItem(index, item_id.c_str());                 //参数类型为CString
        m_list_ctrl.SetItemText(index, 1, item_name);
        m_list_ctrl.SetItemText(index, 2, layout_item.Prefix);
        m_list_ctrl.SetItemColor(index, 3, layout_item.PrefixColor);
        m_list_ctrl.SetItemColor(index, 4, layout_item.ValueColor);
        m_list_ctrl.SetItemData(index, (DWORD_PTR) & (iter->first));    //传递iter->second也行，就是取的时候是LayoutItem类型了。
    }
//    m_list_ctrl.SetEditColMethod(CListCtrlEx::EC_SPECIFIED);        //设置列表可编辑
//    m_list_ctrl.SetEditableCol({ 2 });                              //设置可编辑的列

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CMainWndColorDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    int index = pNMItemActivate->iItem;
    int col = pNMItemActivate->iSubItem;
    if (col == 3 || col == 4)
    {
        COLORREF color = m_list_ctrl.GetItemColor(index, col);
        CMFCColorDialogEx colorDlg(color, 0, this);
        if (colorDlg.DoModal() == IDOK)
        {
            color = colorDlg.GetColor();
            m_list_ctrl.SetItemColor(index, col, color);
            CommonDisplayItem* item = (CommonDisplayItem*)(m_list_ctrl.GetItemData(index));
            if (col == 3)
                m_layout_items[*item].PrefixColor = color;
            else
                m_layout_items[*item].ValueColor = color;
        }
    }

    *pResult = 0;
}
