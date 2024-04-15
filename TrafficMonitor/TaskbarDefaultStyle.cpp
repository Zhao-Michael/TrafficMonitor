#include "stdafx.h"
#include "TaskbarDefaultStyle.h"
#include "IniHelper.h"
#include "TrafficMonitor.h"
#include "WindowsSettingHelper.h"

CTaskbarDefaultStyle::CTaskbarDefaultStyle()
{
}

CTaskbarDefaultStyle::~CTaskbarDefaultStyle()
{
}

void CTaskbarDefaultStyle::LoadConfig()
{
	for (int i = 0; i < TASKBAR_DEFAULT_STYLE_NUM; i++)
	{
        COLORREF default_text_color         = (i == TASKBAR_DEFAULT_LIGHT_STYLE_INDEX ? RGB(0, 0, 0)        : RGB(255, 255, 255));
        COLORREF default_back_color         = (i == TASKBAR_DEFAULT_LIGHT_STYLE_INDEX ? RGB(210, 210, 211)  : 0);
        COLORREF default_transparent_color  = (i == TASKBAR_DEFAULT_LIGHT_STYLE_INDEX ? RGB(210, 210, 211)  : 0);
        COLORREF default_status_bar_color   = (i == TASKBAR_DEFAULT_LIGHT_STYLE_INDEX ? RGB(165, 165, 165)  : 0x005A5A5A);
        ELayoutItemAttributesOwner      eOwner = LIAO_TASKBAR_DEFAULT_STYLE_1;
        if (i == 1)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_2;
        else if (i == 2)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_3;
        else if (i == 3)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_4;
        else;

        //缺省字体和任务栏一致
        //载入用于TaskbarDefaultStyle的所有监控项(包括内置监控项和插件项)的标签、标签颜色、数值颜色设置
        m_default_style[i].LoadConfig(eOwner, theApp.m_config_layouts_path, theApp.m_taskbar_default_font, default_text_color, default_back_color, default_transparent_color, default_status_bar_color);
	}
}

void CTaskbarDefaultStyle::SaveConfig(int index) //const     //使用map后不能用const
{
//	for (unsigned char index = 0; index < TASKBAR_DEFAULT_STYLE_NUM; index++)
    if (index >= 0 && index < TASKBAR_DEFAULT_STYLE_NUM)
	{
        ELayoutItemAttributesOwner      eOwner = LIAO_TASKBAR_DEFAULT_STYLE_1;
        if (index == 1)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_2;
        else if (index == 2)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_3;
        else if (index == 3)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_4;
        else;

		wchar_t buff[64];
		swprintf_s(buff, L"%d", index + 1);
		wstring key_name = buff;
        if (IsTaskBarStyleDataValid(m_default_style[index]))           //保存前检查当前颜色预设是否有效
        {
            //保存用于TaskbarDefaultStyle的所有监控项(包括内置监控项和插件项)的标签、标签颜色、数值颜色设置
            m_default_style[index].SaveConfig(eOwner, theApp.m_config_layouts_path);
        }
        else
        {
            //写入日志
            CString log_str;
            log_str.Format(_T("在保存预设%d时检测到背景色和文字颜色都为黑色，该预设未被保存。"), index);
            CCommon::WriteLog(log_str, theApp.m_log_path.c_str());
            return;
        }
	}
}

void CTaskbarDefaultStyle::ApplyDefaultStyle(int index, TaskBarSettingData & data) //const  //使用map后不能用const
{
    CLayout&                                    rLayout                 = data.layout;
    std::map<CommonDisplayItem, LayoutItem>&    rTaskbar_M_LayoutItems  = rLayout.M_LayoutItems;

    /*if (index == TASKBAR_DEFAULT_LIGHT_STYLE_INDEX)
    {
        ApplyDefaultLightStyle(data);
    }
    else */if (index >= 0 && index < TASKBAR_DEFAULT_STYLE_NUM)
	{
        if (!IsTaskBarStyleDataValid(m_default_style[index]))
            return;

        rLayout.font_info           = m_default_style[index].font_info;
		rLayout.back_color          = m_default_style[index].back_color;
		rLayout.transparent_color   = m_default_style[index].transparent_color;
		rLayout.status_bar_color    = m_default_style[index].status_bar_color;
        for (auto iter = m_default_style[index].M_LayoutItems.begin(); iter != m_default_style[index].M_LayoutItems.end(); ++iter)
        {
            rTaskbar_M_LayoutItems[iter->first].PrefixColor     = iter->second.PrefixColor;
            rTaskbar_M_LayoutItems[iter->first].ValueColor      = iter->second.ValueColor;
        }
		if (rLayout.transparent_color == rLayout.back_color)
		{
			CCommon::TransparentColorConvert(rLayout.back_color);
			CCommon::TransparentColorConvert(rLayout.transparent_color);
		}
	}
}

//void CTaskbarDefaultStyle::ApplyDefaultLightStyle(TaskBarSettingData& data)
//{
//	for (auto& item : data.text_colors)
//    {
//        item.second.label = RGB(0, 0, 0);
//        item.second.value = RGB(0, 0, 0);
//    }
//	data.back_color = RGB(210, 210, 211);
//	data.transparent_color = RGB(210, 210, 211);
//	data.status_bar_color = RGB(165, 165, 165);
//}

void CTaskbarDefaultStyle::ModifyDefaultStyle(int index, TaskBarSettingData & data)
{
	if (index < 0 || index >= TASKBAR_DEFAULT_STYLE_NUM)
		return;

    CLayout&                                    rLayout                 = data.layout;
    std::map<CommonDisplayItem, LayoutItem>&    rTaskbar_M_LayoutItems  = rLayout.M_LayoutItems;

    m_default_style[index].font_info            = rLayout.font_info;
	m_default_style[index].back_color           = rLayout.back_color;
	m_default_style[index].transparent_color    = rLayout.transparent_color;
	m_default_style[index].status_bar_color     = rLayout.status_bar_color;
    for (auto iter = rTaskbar_M_LayoutItems.begin(); iter != rTaskbar_M_LayoutItems.end(); ++iter)
    {
        m_default_style[index].M_LayoutItems[iter->first].PrefixColor   = iter->second.PrefixColor;
        m_default_style[index].M_LayoutItems[iter->first].ValueColor    = iter->second.ValueColor;
    }
    //立即保存到配置文件，并且只在这里(也就是有修改时)才保存，没有修改时不应该重复保存。
    SaveConfig(index);
}

bool CTaskbarDefaultStyle::IsTaskBarStyleDataValid(const CLayout& data)
{
    for (const auto& item : data.M_LayoutItems)
    {
        if (item.second.PrefixColor != data.back_color || item.second.ValueColor != data.back_color)
            return true;
    }
    return false;     //如果文本颜色全部等于背景颜色，则该颜色预设无效
}
