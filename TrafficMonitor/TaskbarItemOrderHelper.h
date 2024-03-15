#pragma once
#include "PluginInterface.h"
#include "CommonTypes.h"

//显示的项目
struct CommonDisplayItem
{
    bool                            is_plugin{};    //是否为插件项目
    EBuiltinDisplayItem             item_type{};    //内建的显示项目
    IPluginItem*                    plugin_item{};  //插件显示项目

    CommonDisplayItem() {}
    CommonDisplayItem(EBuiltinDisplayItem item);
    CommonDisplayItem(IPluginItem* item);

    bool operator<(const CommonDisplayItem&) const;
    bool operator==(const CommonDisplayItem&) const;

    CString GetItemName() const;
};

class CTaskbarItemOrderHelper
{
public:
    CTaskbarItemOrderHelper(bool displayed_only = false);

    void Init();

    //根据设定的顺序获取任务窗口每个显示项目
    std::vector<CommonDisplayItem> GetAllDisplayItemsWithOrder() const;

    void FromString(const std::wstring& str);
    std::wstring ToString() const;
    void SetOrder(const vector<int>& item_order);
    const vector<int>& GetItemOrderConst() const;
    vector<int>& GetItemOrder();

    static CString GetItemDisplayName(CommonDisplayItem item);
    static bool IsItemDisplayed(CommonDisplayItem item);

private:
    //规范m_item_order里的项目，如果m_item_order里有序号超过了显示项目的个数，则将其移除，并在后面添加缺少的项目的序号
    void NormalizeItemOrder();

private:
    vector<int> m_item_order;   //保存每个项目的顺序
    vector<CommonDisplayItem> m_all_item_in_default_order;    //以默认顺序保存的所有显示项目
    bool m_displayed_only;
};
