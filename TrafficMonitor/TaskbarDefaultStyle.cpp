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
	SaveConfig();
}

void CTaskbarDefaultStyle::LoadConfig()
{
	CIniHelper ini{ theApp.m_config_path };
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

        wchar_t buff[64];
        swprintf_s(buff, L"%d", i + 1);
        wstring key_name = buff;
        m_default_style[i].back_color               = ini.GetInt(   (L"taskbar_default_style_" + key_name).c_str(), L"back_color",          default_back_color, 16);
        m_default_style[i].transparent_color        = ini.GetInt(   (L"taskbar_default_style_" + key_name).c_str(), L"transparent_color",   default_transparent_color,  16);
        m_default_style[i].status_bar_color         = ini.GetInt(   (L"taskbar_default_style_" + key_name).c_str(), L"status_bar_color",    default_status_bar_color,   16);

        //载入用于TaskbarDefaultStyle的内置显示项所有属性设置(当前版本情况：只支持全局性设置)
        std::map<CommonDisplayItem, LayoutItem>& rM_LayoutItems = m_default_style[i].M_LayoutItems;
        ini.LoadLayoutItemAttributes(eOwner, _T("up_string"),               rM_LayoutItems, TDI_UP, nullptr, L"↑: $", default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("down_string"),             rM_LayoutItems, TDI_DOWN, nullptr, L"↓: $", default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_string"),              rM_LayoutItems, TDI_CPU, nullptr, L"CPU: $", default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("memory_string"),           rM_LayoutItems, TDI_MEMORY, nullptr, CCommon::LoadText(IDS_MEMORY_DISP, _T(": $")), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_string"),              rM_LayoutItems, TDI_GPU_USAGE, nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": $")), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_temp_string"),         rM_LayoutItems, TDI_CPU_TEMP, nullptr, L"CPU: $", default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_temp_string"),         rM_LayoutItems, TDI_GPU_TEMP, nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": ")), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_temp_string"),         rM_LayoutItems, TDI_HDD_TEMP, nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": ")), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("main_board_temp_string"),  rM_LayoutItems, TDI_MAIN_BOARD_TEMP, nullptr, CCommon::LoadText(IDS_MAINBOARD_DISP, _T(": ")), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_string"),              rM_LayoutItems, TDI_HDD_USAGE, nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": ")), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("total_speed_string"),      rM_LayoutItems, TDI_TOTAL_SPEED, nullptr, _T("↑↓: $"), default_text_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_freq_string"),         rM_LayoutItems, TDI_CPU_FREQ, nullptr, CCommon::LoadText(IDS_CPU_FREQ, _T(": $")), default_text_color);

        //载入插件项目的显示文本设置(false表示任务栏窗口)
        ini.LoadPluginItemsAttributes(eOwner, rM_LayoutItems);
	}
}

void CTaskbarDefaultStyle::SaveConfig() //const     //使用map后不能用const
{
	CIniHelper ini{ theApp.m_config_path };
	for (int i = 0; i < TASKBAR_DEFAULT_STYLE_NUM; i++)
	{
        ELayoutItemAttributesOwner      eOwner = LIAO_TASKBAR_DEFAULT_STYLE_1;
        if (i == 1)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_2;
        else if (i == 2)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_3;
        else if (i == 3)
            eOwner = LIAO_TASKBAR_DEFAULT_STYLE_4;
        else;

		wchar_t buff[64];
		swprintf_s(buff, L"%d", i + 1);
		wstring key_name = buff;
        if (IsTaskBarStyleDataValid(m_default_style[i]))           //保存前检查当前颜色预设是否有效
        {

            ini.WriteInt((L"taskbar_default_style_" + key_name).c_str(), L"back_color",              m_default_style[i].back_color, 16);
            ini.WriteInt((L"taskbar_default_style_" + key_name).c_str(), L"transparent_color",       m_default_style[i].transparent_color, 16);
            ini.WriteInt((L"taskbar_default_style_" + key_name).c_str(), L"status_bar_color",        m_default_style[i].status_bar_color, 16);

            //保存用于任务栏窗口的内置显示项标签设置(当前版本情况：只支持全局性设置)
            std::map<CommonDisplayItem, LayoutItem>& rM_LayoutItems = m_default_style[i].M_LayoutItems;
            ini.SaveLayoutItemAttributes(eOwner, _T("up_string"),               rM_LayoutItems[TDI_UP]);
            ini.SaveLayoutItemAttributes(eOwner, _T("down_string"),             rM_LayoutItems[TDI_DOWN]);
            ini.SaveLayoutItemAttributes(eOwner, _T("cpu_string"),              rM_LayoutItems[TDI_CPU]);
            ini.SaveLayoutItemAttributes(eOwner, _T("memory_string"),           rM_LayoutItems[TDI_MEMORY]);
            ini.SaveLayoutItemAttributes(eOwner, _T("gpu_string"),              rM_LayoutItems[TDI_GPU_USAGE]);
            ini.SaveLayoutItemAttributes(eOwner, _T("cpu_temp_string"),         rM_LayoutItems[TDI_CPU_TEMP]);
            ini.SaveLayoutItemAttributes(eOwner, _T("gpu_temp_string"),         rM_LayoutItems[TDI_GPU_TEMP]);
            ini.SaveLayoutItemAttributes(eOwner, _T("hdd_temp_string"),         rM_LayoutItems[TDI_HDD_TEMP]);
            ini.SaveLayoutItemAttributes(eOwner, _T("main_board_temp_string"),  rM_LayoutItems[TDI_MAIN_BOARD_TEMP]);
            ini.SaveLayoutItemAttributes(eOwner, _T("hdd_string"),              rM_LayoutItems[TDI_HDD_USAGE]);
            ini.SaveLayoutItemAttributes(eOwner, _T("total_speed_string"),      rM_LayoutItems[TDI_TOTAL_SPEED]);
            ini.SaveLayoutItemAttributes(eOwner, _T("cpu_freq_string"),         rM_LayoutItems[TDI_CPU_FREQ]);

            //保存插件项目的显示文本设置(false表示任务栏窗口)
            ini.SavePluginItemsAttributes(eOwner, rM_LayoutItems);
        }
        else
        {
            //写入日志
            CString log_str;
            log_str.Format(_T("在保存预设%d时检测到背景色和文字颜色都为黑色，该预设未被保存。"), i);
            CCommon::WriteLog(log_str, theApp.m_log_path.c_str());
            return;
        }
	}
	ini.Save();
}

void CTaskbarDefaultStyle::ApplyDefaultStyle(int index, TaskBarSettingData & data) //const  //使用map后不能用const
{
    TaskBarSettingData&                         rTaskbarData            = data;
    std::map<CommonDisplayItem, LayoutItem>&    rTaskbar_M_LayoutItems  = rTaskbarData.layout.M_LayoutItems;

    /*if (index == TASKBAR_DEFAULT_LIGHT_STYLE_INDEX)
    {
        ApplyDefaultLightStyle(data);
    }
    else */if (index >= 0 && index < TASKBAR_DEFAULT_STYLE_NUM)
	{
        if (!IsTaskBarStyleDataValid(m_default_style[index]))
            return;

		data.back_color                 = m_default_style[index].back_color;
		data.transparent_color          = m_default_style[index].transparent_color;
		data.status_bar_color           = m_default_style[index].status_bar_color;
        for (auto iter = m_default_style[index].M_LayoutItems.begin(); iter != m_default_style[index].M_LayoutItems.end(); ++iter)
        {
            rTaskbar_M_LayoutItems[iter->first].PrefixColor = iter->second.PrefixColor;
            rTaskbar_M_LayoutItems[iter->first].ValueColor = iter->second.ValueColor;
        }
		if (data.transparent_color == data.back_color)
		{
			CCommon::TransparentColorConvert(data.back_color);
			CCommon::TransparentColorConvert(data.transparent_color);
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

    TaskBarSettingData&                         rTaskbarData            = data;
    std::map<CommonDisplayItem, LayoutItem>&    rTaskbar_M_LayoutItems  = rTaskbarData.layout.M_LayoutItems;

	m_default_style[index].back_color = data.back_color;
	m_default_style[index].transparent_color = data.transparent_color;
	m_default_style[index].status_bar_color = data.status_bar_color;
    for (auto iter = rTaskbar_M_LayoutItems.begin(); iter != rTaskbar_M_LayoutItems.end(); ++iter)
    {
        m_default_style[index].M_LayoutItems[iter->first].PrefixColor = iter->second.PrefixColor;
        m_default_style[index].M_LayoutItems[iter->first].ValueColor = iter->second.ValueColor;
    }
}

bool CTaskbarDefaultStyle::IsTaskBarStyleDataValid(const TaskBarStyleData& data)
{
    for (const auto& item : data.M_LayoutItems)
    {
        if (item.second.PrefixColor != data.back_color || item.second.ValueColor != data.back_color)
            return true;
    }
    return false;     //如果文本颜色全部等于背景颜色，则该颜色预设无效
}
