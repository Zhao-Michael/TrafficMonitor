#pragma once
#include "CommonTypes.h"
#include "TaskbarItemOrderHelper.h"

//皮肤布局
class CLayout
{
public:
    CLayout();
    ~CLayout();

    ////////////////////////////////////////////////////////////////////////////////////////
    //      只能从皮肤配置文件中定义，GUI无法配置。
    ////////////////////////////////////////////////////////////////////////////////////////
    int         width{}, height{};                                      //整个layout的宽度与高度
    int         DrawRectHeight{};                                       //每个显示项的数值、前缀等的显示高度。即使整个layout只有一行，这个高度也可能低于layout高度。
    bool        no_label{ false };                                      //是否不显示前缀，缺省为false。只影响皮肤配置本身以及主窗口是否显示前缀。但是，即使为true时，也不影响GUI设置前缀。//这项设置没有太多用处，以后可能为删除。
    COLORREF    PrefixColor{}, ValueColor{};                            //layout缺省的前缀颜色、数值颜色

    ////////////////////////////////////////////////////////////////////////////////////////
    //      皮肤配置文件和GUI都可以配置
    ////////////////////////////////////////////////////////////////////////////////////////
    FontInfo    font_info;                                              //字体信息
    //////////////////////////////////////////////////////////////
    //      保存单个Layout配置的每一项的布局信息。
    //      当用于保存皮肤配置文件中的布局配置数据时，切换皮肤(包括重新加载当前皮肤)时存放新数据，否则不改变数据。
    //      当用于保存GUI配置数据时，保存实时数据。切换皮肤(包括重新加载当前皮肤)或同个皮肤内切换布局时存放新布局的GUI配置数据或皮肤配置文件中的布局配置数据。
    //////////////////////////////////////////////////////////////
    std::map<CommonDisplayItem, LayoutItem>             M_LayoutItems{};
    LayoutItem& GetItem(CommonDisplayItem display_item)
    {
        return M_LayoutItems[display_item];
    }
    /*
        LayoutItem GetItem(CommonDisplayItem display_item) const
        {
            auto iter = M_LayoutItems.find(display_item);
            if (iter != M_LayoutItems.end())
                return iter->second;
            return LayoutItem();
        }
    */
};