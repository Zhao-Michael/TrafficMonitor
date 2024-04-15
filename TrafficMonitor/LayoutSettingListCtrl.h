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
	//      �༭ǰ׺���������
	////////////////////////////////////////////////////////////////////////////////////////
	enum eEditColMethod       //Ҫ�༭���еķ�ʽ
	{
		EC_NONE,        //��
		EC_ALL,         //ȫ��
		EC_SPECIFIED    //ָ������
	};
	void SetEditColMethod(eEditColMethod method);
	void SetEditableCol(const std::initializer_list<int>& paras);   //��������༭����

	////////////////////////////////////////////////////////////////////////////////////////
	//      �༭��ɫ���
	////////////////////////////////////////////////////////////////////////////////////////
    void SetItemColor(int row, int col, COLORREF color);
    COLORREF GetItemColor(int row, int col);
	void SetDrawItemRangMargin(int margin) { m_margin = margin; }			//���û��ƻ��Ƶľ��εı�Ե����Ԫ��߿�ľ��Σ���ֵԽ������Ƶľ���Խϸ�����ǲ��ܳ����б��о��һ��

protected:
	int m_margin{};
	std::map<int, std::map<int, COLORREF>> m_colors;	//���ڱ���ÿһ��ÿһ�е���ɫ

	////////////////////////////////////////////////////////////////////////////////////////
	//      �༭ǰ׺���������
	////////////////////////////////////////////////////////////////////////////////////////
	CEdit m_item_edit;
	int m_edit_row{};
	int m_edit_col{};
	bool m_editing{};
	eEditColMethod m_edit_col_method{ EC_NONE };
	std::set<int> m_edit_cols;
	void Edit(int row, int col);			//�༭ָ����Ԫ��
	void EndEdit();

	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg BOOL OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEnKillfocusEdit1();
	afx_msg void OnLvnBeginScroll(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnTabletQuerysystemgesturestatus(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
};

