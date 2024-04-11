#include "stdafx.h"
#include "Layout.h"
#include "Common.h"

CLayout::CLayout()
{
}

CLayout::~CLayout()
{
}

void CLayout::LoadConfig(const ELayoutItemAttributesOwner eOwner, const wstring& config_path, const FontInfo& default_font, COLORREF default_color,
                        COLORREF default_back_color, COLORREF default_transparent_color, COLORREF default_status_bar_color)
{
    CIniHelper ini{ config_path };
    std::map<CommonDisplayItem, LayoutItem>& rM_LayoutItems = M_LayoutItems;

    wchar_t* AppName = nullptr;
    if (LIAO_MAINWND == eOwner)
        AppName = _T("config");
    else if (LIAO_TASKBAR == eOwner)
        AppName = _T("taskbar");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_1 == eOwner)
        AppName = _T("taskbar_default_style_1");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_2 == eOwner)
        AppName = _T("taskbar_default_style_2");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_3 == eOwner)
        AppName = _T("taskbar_default_style_3");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_4 == eOwner)
        AppName = _T("taskbar_default_style_4");
    else
        return;

    //载入字体设置
    ini.LoadFontData(AppName, font_info, default_font);
    //仅用于任务栏设置的颜色
    back_color          = ini.GetInt(AppName, _T("back_color"),         default_back_color,         16);
    transparent_color   = ini.GetInt(AppName, _T("transparent_color"),  default_transparent_color,  16);
    status_bar_color    = ini.GetInt(AppName, _T("status_bar_color"),   default_status_bar_color,   16);

    if (LIAO_MAINWND == eOwner)
    {
        //载入用于主窗口的内置显示项所有属性设置(当前版本情况：只支持全局性设置)  
        ini.LoadLayoutItemAttributes(eOwner, _T("up_string"),               rM_LayoutItems, TDI_UP, nullptr, CCommon::LoadText(IDS_UPLOAD_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("down_string"),             rM_LayoutItems, TDI_DOWN, nullptr, CCommon::LoadText(IDS_DOWNLOAD_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_string"),              rM_LayoutItems, TDI_CPU, nullptr, L"CPU: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("memory_string"),           rM_LayoutItems, TDI_MEMORY, nullptr, CCommon::LoadText(IDS_MEMORY_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_string"),              rM_LayoutItems, TDI_GPU_USAGE, nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_temp_string"),         rM_LayoutItems, TDI_CPU_TEMP, nullptr, L"CPU: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_temp_string"),         rM_LayoutItems, TDI_GPU_TEMP, nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_temp_string"),         rM_LayoutItems, TDI_HDD_TEMP, nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("main_board_temp_string"),  rM_LayoutItems, TDI_MAIN_BOARD_TEMP, nullptr, CCommon::LoadText(IDS_MAINBOARD_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_string"),              rM_LayoutItems, TDI_HDD_USAGE, nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("total_speed_string"),      rM_LayoutItems, TDI_TOTAL_SPEED, nullptr, _T("↑↓: $"), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_freq_string"),         rM_LayoutItems, TDI_CPU_FREQ, nullptr, CCommon::LoadText(IDS_CPU_FREQ, _T(": $")), default_color);
        //载入用于主窗口的插件显示项标签、标签颜色、数值颜色设置
        ini.LoadPluginItemsAttributes(eOwner, rM_LayoutItems);
    }
    else if (LIAO_TASKBAR == eOwner)
    {
        //载入所有内置监控项的所有属性设置
        ini.LoadLayoutItemAttributes(eOwner, _T("up_string"),                 rM_LayoutItems, TDI_UP,                 nullptr, L"↑: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("down_string"),               rM_LayoutItems, TDI_DOWN,               nullptr, L"↓: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_string"),                rM_LayoutItems, TDI_CPU,                nullptr, L"CPU: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("memory_string"),             rM_LayoutItems, TDI_MEMORY,             nullptr, CCommon::LoadText(IDS_MEMORY_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_string"),                rM_LayoutItems, TDI_GPU_USAGE,          nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_temp_string"),           rM_LayoutItems, TDI_CPU_TEMP,           nullptr, L"CPU: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_temp_string"),           rM_LayoutItems, TDI_GPU_TEMP,           nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_temp_string"),           rM_LayoutItems, TDI_HDD_TEMP,           nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("main_board_temp_string"),    rM_LayoutItems, TDI_MAIN_BOARD_TEMP,    nullptr, CCommon::LoadText(IDS_MAINBOARD_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_string"),                rM_LayoutItems, TDI_HDD_USAGE,          nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("total_speed_string"),        rM_LayoutItems, TDI_TOTAL_SPEED,        nullptr, _T("↑↓: $"), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_freq_string"),           rM_LayoutItems, TDI_CPU_FREQ,           nullptr, CCommon::LoadText(IDS_CPU_FREQ, _T(": $")), default_color);
        //载入所有插件项的标签、标签颜色、数值颜色设置
        ini.LoadPluginItemsAttributes(eOwner, rM_LayoutItems);
    }
    else if (LIAO_TASKBAR_DEFAULT_STYLE_1 == eOwner || LIAO_TASKBAR_DEFAULT_STYLE_2 == eOwner || LIAO_TASKBAR_DEFAULT_STYLE_3 == eOwner || LIAO_TASKBAR_DEFAULT_STYLE_4 == eOwner)
    {
        //载入用于TaskbarDefaultStyle的内置显示项所有属性设置
        ini.LoadLayoutItemAttributes(eOwner, _T("up_string"), rM_LayoutItems, TDI_UP, nullptr, L"↑: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("down_string"), rM_LayoutItems, TDI_DOWN, nullptr, L"↓: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_string"), rM_LayoutItems, TDI_CPU, nullptr, L"CPU: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("memory_string"), rM_LayoutItems, TDI_MEMORY, nullptr, CCommon::LoadText(IDS_MEMORY_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_string"), rM_LayoutItems, TDI_GPU_USAGE, nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": $")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_temp_string"), rM_LayoutItems, TDI_CPU_TEMP, nullptr, L"CPU: $", default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("gpu_temp_string"), rM_LayoutItems, TDI_GPU_TEMP, nullptr, CCommon::LoadText(IDS_GPU_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_temp_string"), rM_LayoutItems, TDI_HDD_TEMP, nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("main_board_temp_string"), rM_LayoutItems, TDI_MAIN_BOARD_TEMP, nullptr, CCommon::LoadText(IDS_MAINBOARD_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("hdd_string"), rM_LayoutItems, TDI_HDD_USAGE, nullptr, CCommon::LoadText(IDS_HDD_DISP, _T(": ")), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("total_speed_string"), rM_LayoutItems, TDI_TOTAL_SPEED, nullptr, _T("↑↓: $"), default_color);
        ini.LoadLayoutItemAttributes(eOwner, _T("cpu_freq_string"), rM_LayoutItems, TDI_CPU_FREQ, nullptr, CCommon::LoadText(IDS_CPU_FREQ, _T(": $")), default_color);
        //载入所有插件项的标签、标签颜色、数值颜色设置
        ini.LoadPluginItemsAttributes(eOwner, rM_LayoutItems);
    }
    else
        return;
}

void CLayout::SaveConfig(const ELayoutItemAttributesOwner eOwner, const wstring& config_path)
{
    CIniHelper ini{ config_path };
    std::map<CommonDisplayItem, LayoutItem>& rM_LayoutItems = M_LayoutItems;

    wchar_t* AppName = nullptr;
    if (LIAO_MAINWND == eOwner)
        AppName = _T("config");
    else if (LIAO_TASKBAR == eOwner)
        AppName = _T("taskbar");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_1 == eOwner)
        AppName = _T("taskbar_default_style_1");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_2 == eOwner)
        AppName = _T("taskbar_default_style_2");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_3 == eOwner)
        AppName = _T("taskbar_default_style_3");
    else if (LIAO_TASKBAR_DEFAULT_STYLE_4 == eOwner)
        AppName = _T("taskbar_default_style_4");
    else
        return;

    //保存字体设置
    ini.SaveFontData(AppName, font_info);
    //仅用于任务栏设置的颜色
    ini.WriteInt(AppName, L"back_color",        back_color,         16);
    ini.WriteInt(AppName, L"transparent_color", transparent_color,  16);
    ini.WriteInt(AppName, L"status_bar_color",  status_bar_color,   16);
    //保存所有内置监控项的标签、标签颜色、数值颜色设置
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
    //保存插件项的标签、标签颜色、数值颜色设置
    ini.SavePluginItemsAttributes(eOwner, rM_LayoutItems);

    ini.Save();
}
