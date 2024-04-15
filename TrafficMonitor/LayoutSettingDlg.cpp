// MonitorItemAttributesDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "LayoutSettingDlg.h"
#include "afxdialogex.h"
#include "CMFCColorDialogEx.h"
#include "TrafficMonitorDlg.h"
#include "SkinFile.h"

// CLayoutSettingDlg 对话框

IMPLEMENT_DYNAMIC(CLayoutSettingDlg, CBaseDialog)

CLayoutSettingDlg::CLayoutSettingDlg(CLayout& layout, bool bMainWnd, CWnd* pParent /*=NULL*/)
	: CBaseDialog(IDD_MONITOR_ITEM_ATTRIBUTES_DIALOG, pParent), m_layout(layout), B_MainWnd(bMainWnd)
{
}

CLayoutSettingDlg::~CLayoutSettingDlg()
{
}

CString CLayoutSettingDlg::GetDialogName() const
{
    return _T("MonitorItemAttributesDlg");
}

void CLayoutSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CBaseDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, m_list_ctrl);
}

BEGIN_MESSAGE_MAP(CLayoutSettingDlg, CBaseDialog)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CLayoutSettingDlg::OnNMDblclkList1)
    ON_BN_CLICKED(IDC_RESTORE_DEFAULT_BUTTON, &CLayoutSettingDlg::OnBnClickedRestoreDefaultButton)
    ON_BN_CLICKED(IDC_RESTORE_SKIN_DEFAULT_BUTTON, &CLayoutSettingDlg::OnClickedRestoreSkinDefaultButton)
    ON_BN_CLICKED(IDC_SET_FONT_BUTTON, &CLayoutSettingDlg::OnBnClickedSetFontButton)
END_MESSAGE_MAP()

// CLayoutSettingDlg 消息处理程序

BOOL CLayoutSettingDlg::OnInitDialog()
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
    m_list_ctrl.InsertColumn(0, CCommon::LoadText(IDS_DISP_ITEM_ID),    LVCFMT_LEFT, width0);
    m_list_ctrl.InsertColumn(1, CCommon::LoadText(IDS_ITEM),            LVCFMT_LEFT, width1);
    m_list_ctrl.InsertColumn(2, CCommon::LoadText(IDS_PREFIX),          LVCFMT_LEFT, width2);
    m_list_ctrl.InsertColumn(3, CCommon::LoadText(IDS_COLOR_PREFIX),    LVCFMT_LEFT, width3);
    m_list_ctrl.InsertColumn(4, CCommon::LoadText(IDS_COLOR_VALUE),     LVCFMT_LEFT, width4);
    m_list_ctrl.SetDrawItemRangMargin(theApp.DPI(2));

    //向列表中插入行
    std::map<CommonDisplayItem, LayoutItem>& rM_LayoutItems = m_layout.M_LayoutItems;
    for (auto iter = rM_LayoutItems.begin(); iter != rM_LayoutItems.end(); ++iter)
    {
        LayoutItem& layout_item = iter->second;
        wstring         item_id = layout_item.unique_id;

        CString item_name = iter->first.GetItemName();
        int index = m_list_ctrl.GetItemCount();
        m_list_ctrl.InsertItem(index, item_id.c_str());                 //参数类型为CString
        m_list_ctrl.SetItemText(index, 1, item_name);
        m_list_ctrl.SetItemText(index, 2, layout_item.Prefix);
        m_list_ctrl.SetItemData(index, (DWORD_PTR) & (iter->first));    //传递iter->second也行，就是取的时候是LayoutItem类型了。

        //将颜色数据初始化到ListCtrl的成员变量中
        m_list_ctrl.SetItemColor(index, 3, layout_item.PrefixColor);
        m_list_ctrl.SetItemColor(index, 4, layout_item.ValueColor);
    }
    m_list_ctrl.SetEditColMethod(CLayoutSettingListCtrl::EC_SPECIFIED);      //设置列表可编辑
    m_list_ctrl.SetEditableCol({ 2 });                                                      //设置可编辑的列

    if (!m_layout.name.empty())
    {
        CString str;
        GetWindowText(str);
        str += "(";
        wstring     WindowTextName{};
        WindowTextName = str.GetString() + m_layout.name;
        WindowTextName += _T(")");
        SetWindowText(WindowTextName.c_str());
    }
    //字体设置相关编辑框不允许编辑，只用来显示。
    ((CButton*)GetDlgItem(IDC_FONT_NAME_EDIT))->EnableWindow(false);
    ((CButton*)GetDlgItem(IDC_FONT_SIZE_EDIT))->EnableWindow(false);
    //初始化字体设置相关控件
    FontInfo& rFontInfo = m_layout.font_info;
    SetDlgItemText(IDC_FONT_NAME_EDIT, rFontInfo.name);
    SetDlgItemText(IDC_FONT_SIZE_EDIT, std::to_wstring(rFontInfo.size).c_str());

    if (B_MainWnd == false)
        GetDlgItem(IDC_RESTORE_SKIN_DEFAULT_BUTTON)->ShowWindow(false);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void CLayoutSettingDlg::OnOK()
{
    // TODO: 在此添加专用代码和/或调用基类
    std::map<CommonDisplayItem, LayoutItem>&    rM_LayoutItems  = m_layout.M_LayoutItems;
    FontInfo&                                   rFontInfo       = m_layout.font_info;
    
    int item_count = m_list_ctrl.GetItemCount();
    for (int index{}; index < item_count; index++)
    {
        CommonDisplayItem* item             = (CommonDisplayItem*)(m_list_ctrl.GetItemData(index));
        rM_LayoutItems[*item].Prefix        = m_list_ctrl.GetItemText(index, 2).GetString();
        rM_LayoutItems[*item].PrefixColor   = m_list_ctrl.GetItemColor(index, 3);
        rM_LayoutItems[*item].ValueColor    = m_list_ctrl.GetItemColor(index, 4);
    }
    //保存字体设置相关数据
    CString str;
    GetDlgItemText(IDC_FONT_NAME_EDIT, rFontInfo.name);
    GetDlgItemText(IDC_FONT_SIZE_EDIT, str);
    rFontInfo.size = atoi(CCommon::UnicodeToStr(str).c_str());

    CBaseDialog::OnOK();
}

void CLayoutSettingDlg::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
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
            m_list_ctrl.SetItemColor(index, col, color);        //将设置的单个颜色保存到ListCtrl的成员变量中
        }
    }

    *pResult = 0;
}

void CLayoutSettingDlg::OnBnClickedRestoreDefaultButton()
{
    // TODO: 在此添加控件通知处理程序代码
    int item_count = m_list_ctrl.GetItemCount();
    for (int index{}; index < item_count; index++)
    {
        CommonDisplayItem* item = (CommonDisplayItem*)(m_list_ctrl.GetItemData(index));
        CString default_text;
        if (item->is_plugin)
        {
            default_text = item->plugin_item->GetItemLableText();
        }
        else
        {
            switch (item->item_type)
            {
            case TDI_UP:
                if (B_MainWnd)
                    default_text = CCommon::LoadText(IDS_UPLOAD_DISP, _T(": "));
                else
                    default_text = _T("↑: ");
                break;
            case TDI_DOWN:
                if (B_MainWnd)
                    default_text = CCommon::LoadText(IDS_DOWNLOAD_DISP, _T(": "));
                else
                    default_text = _T("↓: ");
                break;
            case TDI_TOTAL_SPEED:
                default_text = _T("↑↓: ");
                break;
            case TDI_CPU:
                default_text = _T("CPU: ");
                break;
            case TDI_CPU_FREQ:
                default_text = CCommon::LoadText(IDS_CPU_FREQ, _T(": "));
                break;
            case TDI_MEMORY:
                default_text = CCommon::LoadText(IDS_MEMORY_DISP, _T(": "));
                break;
            case TDI_GPU_USAGE:
                default_text = CCommon::LoadText(IDS_GPU_DISP, _T(": "));
                break;
            case TDI_CPU_TEMP:
                default_text = _T("CPU: ");
                break;
            case TDI_GPU_TEMP:
                default_text = CCommon::LoadText(IDS_GPU_DISP, _T(": "));
                break;
            case TDI_HDD_TEMP:
                default_text = CCommon::LoadText(IDS_HDD_DISP, _T(": "));
                break;
            case TDI_MAIN_BOARD_TEMP:
                default_text = CCommon::LoadText(IDS_MAINBOARD_DISP, _T(": "));
                break;
            case TDI_HDD_USAGE:
                default_text = CCommon::LoadText(IDS_HDD_DISP, _T(": "));
                break;
            default:
                break;
            }
        }
        m_list_ctrl.SetItemText(index, 2, default_text);
    }
}

void CLayoutSettingDlg::OnClickedRestoreSkinDefaultButton()
{
    // TODO: 在此添加控件通知处理程序代码
    CTrafficMonitorDlg* tmDlg           = (CTrafficMonitorDlg*)theApp.m_pMainWnd;
    CLayout layout                      = tmDlg->GetSkinLayout();
    SetDlgItemText(IDC_FONT_NAME_EDIT, layout.font_info.name);
    SetDlgItemText(IDC_FONT_SIZE_EDIT, std::to_wstring(layout.font_info.size).c_str());
    int item_count = m_list_ctrl.GetItemCount();
    for (int index{}; index < item_count; index++)
    {
        CommonDisplayItem* item     = (CommonDisplayItem*)(m_list_ctrl.GetItemData(index));
        LayoutItem& layout_item     = layout.GetItem(*item);
        m_list_ctrl.SetItemText (index, 2, layout_item.Prefix);
        m_list_ctrl.SetItemColor(index, 3, layout_item.PrefixColor);
        m_list_ctrl.SetItemColor(index, 4, layout_item.ValueColor);
    }
    m_list_ctrl.Invalidate(false);
}


void CLayoutSettingDlg::OnBnClickedSetFontButton()
{
    // TODO: 在此添加控件通知处理程序代码
    FontInfo& rFontInfo = m_layout.font_info;

    LOGFONT lf{};
    lf.lfHeight = FontSizeToLfHeight(rFontInfo.size);
    lf.lfWeight = (rFontInfo.bold ? FW_BOLD : FW_NORMAL);
    lf.lfItalic = rFontInfo.italic;
    lf.lfUnderline = rFontInfo.underline;
    lf.lfStrikeOut = rFontInfo.strike_out;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    //wcsncpy_s(lf.lfFaceName, rFontInfo.name.GetString(), 32);
    CCommon::WStringCopy(lf.lfFaceName, 32, rFontInfo.name.GetString());
    CCommon::NormalizeFont(lf);
    CFontDialog fontDlg(&lf);   //构造字体对话框，初始选择字体为之前字体
    if (IDOK == fontDlg.DoModal())     // 显示字体对话框
    {
        //获取字体信息
        rFontInfo.name = fontDlg.GetFaceName();
        rFontInfo.size = fontDlg.GetSize() / 10;
        rFontInfo.bold = (fontDlg.IsBold() != FALSE);
        rFontInfo.italic = (fontDlg.IsItalic() != FALSE);
        rFontInfo.underline = (fontDlg.IsUnderline() != FALSE);
        rFontInfo.strike_out = (fontDlg.IsStrikeOut() != FALSE);
        //将字体信息显示出来
        SetDlgItemText(IDC_FONT_NAME_EDIT, rFontInfo.name);
        SetDlgItemText(IDC_FONT_SIZE_EDIT, std::to_wstring(rFontInfo.size).c_str());
    }
}
