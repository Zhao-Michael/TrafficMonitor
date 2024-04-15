#pragma once
#include "afxcmn.h"
#include "DrawCommon.h"
#include <set>

class CLayoutSettingListCtrl : public CListCtrl
{
DECLARE_DYNAMIC(CLayoutSettingListCtrl)
public:
	CLayoutSettingListCtrl();
	~CLayoutSettingListCtrl();

	////////////////////////////////////////////////////////////////////////////////////////
	//      编辑前缀等属性相关
	////////////////////////////////////////////////////////////////////////////////////////
	enum eEditColMethod       //要编辑的列的方式
	{
		EC_NONE,        //无
		EC_ALL,         //全部
		EC_SPECIFIED    //指定的列
	};
	void SetEditColMethod(eEditColMethod method);
	void SetEditableCol(const std::initializer_list<int>& paras);   //设置允许编辑的列

	////////////////////////////////////////////////////////////////////////////////////////
	//      编辑颜色相关
	////////////////////////////////////////////////////////////////////////////////////////
    void SetItemColor(int row, int col, COLORREF color);
    COLORREF GetItemColor(int row, int col);
	void SetDrawItemRangMargin(int margin) { m_margin = margin; }			//设置绘制绘制的矩形的边缘到单元格边框的矩形，此值越大则绘制的矩形越细，但是不能超过列表行距的一半

protected:
	int m_margin{};
	std::map<int, std::map<int, COLORREF>> m_colors;	//用于保存每一行每一列的颜色

	////////////////////////////////////////////////////////////////////////////////////////
	//      编辑前缀等属性相关
	////////////////////////////////////////////////////////////////////////////////////////
	CEdit m_item_edit;
	int m_edit_row{};
	int m_edit_col{};
	bool m_editing{};
	eEditColMethod m_edit_col_method{ EC_NONE };
	std::set<int> m_edit_cols;
	void Edit(int row, int col);			//编辑指定单元格
	void EndEdit();

	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg BOOL OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnLvnBeginScroll(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnTabletQuerysystemgesturestatus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

