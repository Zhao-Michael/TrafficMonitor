#include "stdafx.h"
#include "LayoutSettingListCtrl.h"

IMPLEMENT_DYNAMIC(CLayoutSettingListCtrl, CListCtrl)

CLayoutSettingListCtrl::CLayoutSettingListCtrl()
{
}

CLayoutSettingListCtrl::~CLayoutSettingListCtrl()
{
}

BEGIN_MESSAGE_MAP(CLayoutSettingListCtrl, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW,                &CLayoutSettingListCtrl::OnNMCustomdraw)
    ON_EN_KILLFOCUS(IDC_ITEM_EDITBOX,               &CLayoutSettingListCtrl::OnEnKillfocusEdit1)
    ON_NOTIFY_REFLECT_EX(NM_DBLCLK,                 &CLayoutSettingListCtrl::OnNMDblclk)
    ON_NOTIFY_REFLECT(LVN_BEGINSCROLL,              &CLayoutSettingListCtrl::OnLvnBeginScroll)
    ON_MESSAGE(WM_TABLET_QUERYSYSTEMGESTURESTATUS,  &CLayoutSettingListCtrl::OnTabletQuerysystemgesturestatus)
END_MESSAGE_MAP()

void CLayoutSettingListCtrl::SetItemColor(int row, int col, COLORREF color)
{
    m_colors[row][col] = color;
}

COLORREF CLayoutSettingListCtrl::GetItemColor(int row, int col)
{
    return m_colors[row][col];
}

void CLayoutSettingListCtrl::SetEditColMethod(eEditColMethod method)
{
    m_edit_col_method = method;
}

void CLayoutSettingListCtrl::SetEditableCol(const std::initializer_list<int>& paras)
{
    m_edit_cols = paras;
}

void CLayoutSettingListCtrl::Edit(int row, int col)
{
    EnsureVisible(row, FALSE);				//编辑一行时确保该行可见
    m_editing = true;

    m_edit_row = row;
    m_edit_col = col;

    CRect item_rect;
    GetSubItemRect(row, col, LVIR_LABEL, item_rect);	//取得子项的矩形
    CString text = GetItemText(row, col);		//取得子项的内容

    m_item_edit.SetWindowText(text);		//将子项的内容显示到编辑框中
    m_item_edit.ShowWindow(SW_SHOW);		//显示编辑框
    m_item_edit.MoveWindow(item_rect);		//将编辑框移动到子项上面，覆盖在子项上
    m_item_edit.SetFocus();					//使编辑框取得焦点
    m_item_edit.SetSel(0, -1);
}

void CLayoutSettingListCtrl::EndEdit()
{
    if (m_editing)
    {
        if (m_edit_row >= 0 && m_edit_row < GetItemCount())
        {
            CString str;
            m_item_edit.GetWindowText(str);	//取得编辑框的内容
            SetItemText(m_edit_row, m_edit_col, str);	//将该内容更新到CListCtrl中
        }
        m_item_edit.ShowWindow(SW_HIDE);//隐藏编辑框
        m_editing = false;
    }
}

void CLayoutSettingListCtrl::PreSubclassWindow()
{
    // TODO: 在此添加专用代码和/或调用基类
    m_item_edit.Create(WS_BORDER | ES_AUTOHSCROLL, CRect(), this, IDC_ITEM_EDITBOX);
    m_item_edit.SetFont(GetFont());

    CListCtrl::PreSubclassWindow();
}

BOOL CLayoutSettingListCtrl::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码

    //如果双击的列是要编辑的列
    if (m_edit_col_method == EC_ALL
        || (m_edit_col_method == EC_SPECIFIED && m_edit_cols.find(pNMItemActivate->iSubItem) != m_edit_cols.end()))
    {
        Edit(pNMItemActivate->iItem, pNMItemActivate->iSubItem);
        *pResult = 0;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void CLayoutSettingListCtrl::OnEnKillfocusEdit1()
{
    //当文本编辑控件控件失去焦点时响应
    EndEdit();
}

void CLayoutSettingListCtrl::OnLvnBeginScroll(NMHDR* pNMHDR, LRESULT* pResult)
{
    // 此功能要求 Internet Explorer 5.5 或更高版本。
    // 符号 _WIN32_IE 必须是 >= 0x0560。
    LPNMLVSCROLL pStateChanged = reinterpret_cast<LPNMLVSCROLL>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码
    EndEdit();

    *pResult = 0;
}

afx_msg LRESULT CLayoutSettingListCtrl::OnTabletQuerysystemgesturestatus(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void CLayoutSettingListCtrl::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = CDRF_DODEFAULT;
	LPNMLVCUSTOMDRAW lplvdr = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	NMCUSTOMDRAW& nmcd = lplvdr->nmcd;
	switch (lplvdr->nmcd.dwDrawStage)	//判断状态   
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:			//如果为画ITEM之前就要进行颜色的改变
		if (nmcd.dwItemSpec >= 0 && nmcd.dwItemSpec < GetItemCount())
		{
			//double range = m_item_rage_data[nmcd.dwItemSpec].data_value;
			//CDC* pDC = CDC::FromHandle(nmcd.hdc);		//获取绘图DC
			//CRect item_rect, draw_rect;
			//GetSubItemRect(nmcd.dwItemSpec,m_draw_item_range_row, LVIR_BOUNDS, item_rect);	//获取绘图单元格的矩形区域
			//CDrawCommon::SetDrawRect(pDC, item_rect);		//设置绘图区域为当前列
			//draw_rect = item_rect;
			//if (draw_rect.Height() > 2 * m_margin)
			//{
			//	draw_rect.top += m_margin;
			//	draw_rect.bottom -= m_margin;
			//}
			//int width;
			//if (m_use_log_scale)	//使用对数比例（y=ln(x+1)）
			//{
			//	range = std::log(range + 1);
			//	width = static_cast<int>(range*draw_rect.Width() / std::log(1000 + 1));
			//}
			//else		//使用线性比例（y=x）
			//{
			//	width = static_cast<int>(range*draw_rect.Width() / 1000);
			//}
			//draw_rect.right = draw_rect.left + width;
			//pDC->FillSolidRect(draw_rect, m_item_rage_data[nmcd.dwItemSpec].color);

			////当前列绘制完成后将绘图区域设置为左边的区域，防止当前列的区域被覆盖
			//CRect rect1{ item_rect };
			//rect1.left = 0;
			//rect1.right = item_rect.left;
			//CDrawCommon::SetDrawRect(pDC, rect1);

            CDC* pDC = CDC::FromHandle(nmcd.hdc);		//获取绘图DC
            CRect item_rect;
            auto& col_color_map = m_colors[nmcd.dwItemSpec];
            for (auto iter = col_color_map.begin(); iter != col_color_map.end(); ++iter)
            {
				if (iter->first != 3 && iter->first != 4)
					continue;
                GetSubItemRect(nmcd.dwItemSpec, iter->first, LVIR_BOUNDS, item_rect);	//获取绘图单元格的矩形区域
                //设置绘图剪辑区域
                CDrawCommon::SetDrawRect(pDC, item_rect);
                //使用双缓冲绘图
                CDrawDoubleBuffer draw_double_buffer(pDC, item_rect);
                //填充背景
                item_rect.MoveToXY(0, 0);
                draw_double_buffer.GetMemDC()->FillSolidRect(item_rect, GetSysColor(COLOR_WINDOW));
                //绘制颜色矩形
                item_rect.DeflateRect(m_margin, m_margin);
                draw_double_buffer.GetMemDC()->FillSolidRect(item_rect, iter->second);
                //绘制矩形边框
                CDrawCommon drawer;
                drawer.Create(draw_double_buffer.GetMemDC(), this);
                drawer.DrawRectOutLine(item_rect, RGB(192, 192, 192));
            }
            //当前列绘制完成后将绘图区域设置为第一列的区域，防止颜色列的区域被覆盖
            CRect rect1{};
            GetSubItemRect(nmcd.dwItemSpec, 3, LVIR_BOUNDS, rect1);	//获取第1列单元格的矩形区域
            rect1.right = rect1.left;
            rect1.left = 0;
            CDrawCommon::SetDrawRect(pDC, rect1);
		}
		*pResult = CDRF_DODEFAULT;
		break;
	}
}
