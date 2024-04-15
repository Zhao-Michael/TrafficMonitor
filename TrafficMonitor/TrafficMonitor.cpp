
// TrafficMonitor.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "TrafficMonitor.h"
#include "TrafficMonitorDlg.h"
#include "crashtool.h"
#include "UpdateHelper.h"
#include "Test.h"
#include "WIC.h"
#include "auto_start_helper.h"
#include "AppAlreadyRuningDlg.h"
#include "WindowsSettingHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTrafficMonitorApp

BEGIN_MESSAGE_MAP(CTrafficMonitorApp, CWinApp)
    //ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
    ON_COMMAND(ID_HELP, &CTrafficMonitorApp::OnHelp)
    ON_COMMAND(ID_FREQUENTY_ASKED_QUESTIONS, &CTrafficMonitorApp::OnFrequentyAskedQuestions)
    ON_COMMAND(ID_UPDATE_LOG, &CTrafficMonitorApp::OnUpdateLog)
END_MESSAGE_MAP()

// 唯一的一个 CTrafficMonitorApp 对象
CTrafficMonitorApp theApp;

CTrafficMonitorApp* CTrafficMonitorApp::self = NULL;


// CTrafficMonitorApp 构造
CTrafficMonitorApp::CTrafficMonitorApp()
{
    self = this;
    // 支持重新启动管理器
    //m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

    // TODO: 在此处添加构造代码，
    // 将所有重要的初始化放置在 InitInstance 中
    CRASHREPORT::StartCrashReport();
}

void CTrafficMonitorApp::LoadConfig()
{
    CIniHelper ini{ m_config_path };
    GeneralSettingData&                         rGeneralData    = m_general_data;
    MainWndSettingData&                         rMainWndData    = m_main_wnd_data;
    TaskBarSettingData&                         rTaskbarData    = m_taskbar_data;
    AppSettingData&                             rAppData        = m_cfg_data;
    std::map<CommonDisplayItem, LayoutItem>&    rMainWnd_M_LayoutItems  = rMainWndData.layout.M_LayoutItems;
    std::map<CommonDisplayItem, LayoutItem>&    rTaskbar_M_LayoutItems  = rTaskbarData.layout.M_LayoutItems;
    LayoutItemValueAttributes&                  rMainWnd_LIVA           = rMainWndData.layout_item_value_attributes;
    LayoutItemValueAttributes&                  rTaskbar_LIVA           = rTaskbarData.layout_item_value_attributes;

    ////////////////////////////////////////////////////////////////////////////////////////
    //      (一)载入APP全局性设置 = 选项对话框中的常规设置 + 鼠标右键中的部分设置 + 其它设置
    ////////////////////////////////////////////////////////////////////////////////////////
    //(1)选项对话框中的常规设置(全局性唯一性设置)
    rGeneralData.check_update_when_start        = ini.GetBool(_T("general"),  _T("check_update_when_start"), true);
    rGeneralData.language = static_cast<Language>(ini.GetInt( _T("general"),  _T("language"),               0));
    bool is_chinese_language{};     //当前语言是否为简体中文
    if (rGeneralData.language == Language::FOLLOWING_SYSTEM)
        is_chinese_language = CCommon::LoadText(IDS_LANGUAGE_CODE) == _T("2");
    else
        is_chinese_language = (rGeneralData.language == Language::SIMPLIFIED_CHINESE);
    rGeneralData.update_source                  = ini.GetInt   (L"general",     L"update_source", is_chinese_language ? 1 : 0);   //如果当前语言为简体，则默认更新源为Gitee，否则为GitHub   
    //提示信息选项
    rGeneralData.traffic_tip_enable             = ini.GetBool  (L"notify_tip", L"traffic_tip_enable",               false);
    rGeneralData.traffic_tip_value              = ini.GetInt   (L"notify_tip", L"traffic_tip_value",                200);
    rGeneralData.traffic_tip_unit               = ini.GetInt   (L"notify_tip", L"traffic_tip_unit",                 0);
    rGeneralData.memory_usage_tip.enable        = ini.GetBool  (L"notify_tip", L"memory_usage_tip_enable",          false);
    rGeneralData.memory_usage_tip.tip_value     = ini.GetInt   (L"notify_tip", L"memory_tip_value",                 80);
    rGeneralData.cpu_temp_tip.enable            = ini.GetBool  (L"notify_tip", L"cpu_temperature_tip_enable",       false);
    rGeneralData.cpu_temp_tip.tip_value         = ini.GetInt   (L"notify_tip", L"cpu_temperature_tip_value",        80);
    rGeneralData.gpu_temp_tip.enable            = ini.GetBool  (L"notify_tip", L"gpu_temperature_tip_enable",       false);
    rGeneralData.gpu_temp_tip.tip_value         = ini.GetInt   (L"notify_tip", L"gpu_temperature_tip_value",        80);
    rGeneralData.hdd_temp_tip.enable            = ini.GetBool  (L"notify_tip", L"hdd_temperature_tip_enable",       false);
    rGeneralData.hdd_temp_tip.tip_value         = ini.GetInt   (L"notify_tip", L"hdd_temperature_tip_value",        80);
    rGeneralData.mainboard_temp_tip.enable      = ini.GetBool  (L"notify_tip", L"mainboard_temperature_tip_enable", false);
    rGeneralData.mainboard_temp_tip.tip_value   = ini.GetInt   (L"notify_tip", L"mainboard_temperature_tip_value",  80);
    //硬件监控
    rGeneralData.hardware_monitor_item          = ini.GetInt   (L"general",     L"hardware_monitor_item",   0);
    rGeneralData.hard_disk_name                 = ini.GetString(L"general",     L"hard_disk_name",          L"");
    rGeneralData.cpu_core_name                  = ini.GetString(L"general",     L"cpu_core_name",           L"Core Average");
    rGeneralData.show_all_interface             = ini.GetBool  (L"general",     L"show_all_interface",      false);
    //载入获取CPU利用率的方式，默认使用GetSystemTimes获取
    rGeneralData.m_get_cpu_usage_by_cpu_times   = ini.GetBool  (L"general",     L"get_cpu_usage_by_cpu_times", /*m_win_version.GetMajorVersion() < 10*/ true);
    rGeneralData.monitor_time_span              = ini.GetInt   (L"general",     L"monitor_time_span", 1000);
    if (rGeneralData.monitor_time_span < MONITOR_TIME_SPAN_MIN || rGeneralData.monitor_time_span > MONITOR_TIME_SPAN_MAX)
        rGeneralData.monitor_time_span = 1000;
    //最后一项配置
    rGeneralData.show_notify_icon               = ini.GetBool(_T("config"),   _T("show_notify_icon"),       true);
    //(2)鼠标右键中的部分设置
    rAppData.m_show_taskbar_wnd                 = ini.GetBool(_T("config"),   _T("show_taskbar_wnd"),       false);
    rAppData.m_hide_main_window                 = ini.GetBool(_T("config"),   _T("hide_main_window"),       false);
    //网络连接设置
    rAppData.m_auto_select                      = ini.GetBool(_T("connection"), _T("auto_select"),          true);
    rAppData.m_select_all                       = ini.GetBool(_T("connection"), _T("select_all"),           false);
    rAppData.m_connection_name =CCommon::UnicodeToStr(ini.GetString(L"connection",    L"connection_name", L"").c_str());
    std::vector<std::wstring> connections_hide;
    ini.GetStringList(L"general", L"connections_hide", connections_hide, std::vector<std::wstring>{});
    rGeneralData.connections_hide.FromVector(connections_hide);
    //通知图标设置
    rAppData.m_notify_icon_selected = ini.GetInt(_T("config"), _T("notify_icon_selected"), (m_win_version.IsWindows7() || m_win_version.IsWindows8Or8point1() ? 2 : rAppData.m_dft_notify_icon));       //Win7/8/8.1默认使用蓝色通知区图标，因为隐藏通知区图标后白色图标会看不清，其他系统默认使用白色图标
    rAppData.m_notify_icon_auto_adapt = ini.GetBool(_T("config"), _T("notify_icon_auto_adapt"), true);
    if (rAppData.m_notify_icon_auto_adapt)
        AutoSelectNotifyIcon();

    //Windows10颜色模式设置
    bool is_windows10_light_theme = CWindowsSettingHelper::IsWindows10LightTheme();
    if (is_windows10_light_theme)
        CCommon::SetColorMode(ColorMode::Light);
    else
        CCommon::SetColorMode(ColorMode::Default);

    ////////////////////////////////////////////////////////////////////////////////////////
    //      (二)加载主窗口设置 = 选项对话框中的主窗口设置 + 鼠标右键中的部分设置 + 其它设置
    ////////////////////////////////////////////////////////////////////////////////////////
    //(1)加载选项对话框中的主窗口设置(当前版本情况：只支持全局性设置)
    //不再在这里加载

    //载入其它设置
    rMainWnd_LIVA.hide_unit                     = ini.GetBool(_T("config"), _T("hide_unit"),            false);
    rMainWnd_LIVA.hide_percent                  = ini.GetBool(_T("config"), _T("hide_percent"),         false);
    rMainWnd_LIVA.separate_value_unit_with_space= ini.GetBool(_T("config"), _T("separate_value_unit_with_space"), true);
    rMainWnd_LIVA.speed_short_mode              = ini.GetBool(_T("config"), _T("speed_short_mode"),     false);
    rMainWnd_LIVA.speed_unit = static_cast<SpeedUnit>(ini.GetInt(_T("config"), _T("speed_unit"),         0));
    rMainWndData.show_tool_tip                  = ini.GetBool(_T("config"), _T("show_tool_tip"),        true);
    rMainWndData.memory_display = static_cast<MemoryDisplay>(ini.GetInt(L"config", L"memory_display", static_cast<int>(MemoryDisplay::USAGE_PERCENTAGE)));
    rMainWndData.m_always_on_top                = ini.GetBool(_T("config"), _T("always_on_top"),        true);
    rMainWndData.m_lock_window_pos              = ini.GetBool(_T("config"), _T("lock_window_pos"),      false);
    rMainWndData.m_mouse_penetrate              = ini.GetBool(_T("config"), _T("mouse_penetrate"),      false);
    rMainWndData.m_alow_out_of_border           = ini.GetBool(_T("config"), _T("alow_out_of_border"),   false);
    rMainWndData.hide_main_wnd_when_fullscreen  = ini.GetBool(_T("config"), _T("hide_main_wnd_when_fullscreen"), true);
    rMainWndData.double_click_action = static_cast<DoubleClickAction>(ini.GetInt(_T("config"), _T("double_click_action"), 0));
    rMainWndData.double_click_exe               = ini.GetString(L"config", L"double_click_exe", (theApp.m_system_dir + L"\\Taskmgr.exe").c_str());
    //(2)鼠标右键中的部分设置
    rMainWndData.m_show_more_info               = ini.GetBool(_T("config"), _T("show_more_info"),       false);
    rMainWndData.m_transparency                 = ini.GetInt (_T("config"), _T("transparency"),     80);
    rMainWndData.m_skin_name                    = ini.GetString(_T("config"), _T("skin_selected"), _T(""));
    if (rMainWndData.m_skin_name.substr(0, 8) == L".\\skins\\")       //如果读取到的皮肤名称前面有".\\skins\\"，则把它删除。（用于和前一个版本保持兼容性）
        rMainWndData.m_skin_name = rMainWndData.m_skin_name.substr(7);
    //(3)其它设置
    rMainWndData.m_position_x                   = ini.GetInt (_T("config"), _T("position_x"),       -1);
    rMainWndData.m_position_y                   = ini.GetInt (_T("config"), _T("position_y"),       -1);

    ////////////////////////////////////////////////////////////////////////////////////////
    //      (三)加载任务栏窗口设置 = 选项对话框中的任务栏窗口设置 + 鼠标右键中的部分设置 + 其它设置
    ////////////////////////////////////////////////////////////////////////////////////////
    //加载任务栏窗口设置
    rTaskbarData.m_tbar_display_item            = ini.GetInt(_T("taskbar"), _T("taskbar_display_item"), TDI_UP | TDI_DOWN);

    //不含温度监控的版本，不显示温度监控相关项目
#ifdef WITHOUT_TEMPERATURE
    rTaskbarData.m_tbar_display_item &= ~TDI_GPU_USAGE;
    rTaskbarData.m_tbar_display_item &= ~TDI_CPU_TEMP;
    rTaskbarData.m_tbar_display_item &= ~TDI_GPU_TEMP;
    rTaskbarData.m_tbar_display_item &= ~TDI_HDD_TEMP;
    rTaskbarData.m_tbar_display_item &= ~TDI_MAIN_BOARD_TEMP;
    rTaskbarData.m_tbar_display_item &= ~TDI_HDD_USAGE;
#endif

    //如果选项设置中关闭了某个硬件监控，则不显示对应的温度监控相关项目
    if (!rGeneralData.IsHardwareEnable(HI_CPU))
        rTaskbarData.m_tbar_display_item &= ~TDI_CPU_TEMP;
    if (!rGeneralData.IsHardwareEnable(HI_GPU))
    {
        rTaskbarData.m_tbar_display_item &= ~TDI_GPU_USAGE;
        rTaskbarData.m_tbar_display_item &= ~TDI_GPU_TEMP;
    }
    if (!rGeneralData.IsHardwareEnable(HI_HDD))
    {
        rTaskbarData.m_tbar_display_item &= ~TDI_HDD_TEMP;
        rTaskbarData.m_tbar_display_item &= ~TDI_HDD_USAGE;
    }
    if (!rGeneralData.IsHardwareEnable(HI_MBD))
        rTaskbarData.m_tbar_display_item &= ~TDI_MAIN_BOARD_TEMP;

    //任务栏选项设置
    rTaskbar_LIVA.hide_unit                             = ini.GetBool(_T("taskbar"), _T("taskbar_hide_unit"),               false);
    rTaskbar_LIVA.hide_percent                          = ini.GetBool(_T("taskbar"), _T("taskbar_hide_percent"),            false);
    rTaskbar_LIVA.separate_value_unit_with_space        = ini.GetBool(_T("taskbar"), _T("separate_value_unit_with_space"),  true);
    rTaskbar_LIVA.speed_short_mode                      = ini.GetBool(_T("taskbar"), _T("taskbar_speed_short_mode"),        false);
    rTaskbar_LIVA.speed_unit     = static_cast<SpeedUnit>(ini.GetInt (_T("taskbar"), _T("taskbar_speed_unit"),              0));
    rTaskbarData.tbar_wnd_on_left                       = ini.GetBool(_T("taskbar"), _T("taskbar_wnd_on_left"),             false);
    rTaskbarData.tbar_wnd_snap                          = ini.GetBool(_T("taskbar"), _T("taskbar_wnd_snap"),                false);
    rTaskbarData.value_right_align                      = ini.GetBool(_T("taskbar"), _T("value_right_align"),               false);
    rTaskbarData.horizontal_arrange                     = ini.GetBool(_T("taskbar"), _T("horizontal_arrange"),              false);
    rTaskbarData.b_show_resource_figure                 = ini.GetBool(_T("taskbar"), _T("show_status_bar"),                 false);
    rTaskbarData.show_tool_tip                          = ini.GetBool(_T("taskbar"), _T("show_tool_tip"),                   true);
    rTaskbarData.digits_number                          = ini.GetInt (_T("taskbar"), _T("digits_number"),                   4);
    rTaskbarData.memory_display = static_cast<MemoryDisplay>(ini.GetInt(L"taskbar",    L"memory_display", static_cast<int>(MemoryDisplay::USAGE_PERCENTAGE)));
    rTaskbarData.double_click_action = static_cast<DoubleClickAction>(ini.GetInt(_T("taskbar"), _T("double_click_action"),  0));
    rTaskbarData.double_click_exe                       = ini.GetString(L"taskbar",    L"double_click_exe", (theApp.m_system_dir + L"\\Taskmgr.exe").c_str());
    rTaskbarData.cm_graph_type                          = ini.GetBool(_T("taskbar"), _T("cm_graph_type"),                   true);
    rTaskbarData.b_show_graph_dashed_box                = ini.GetBool  (L"taskbar",    L"show_graph_dashed_box",            false);
    rTaskbarData.item_space                             = ini.GetInt   (L"taskbar",    L"item_space",                       4);
    rTaskbarData.window_offset_top                      = ini.GetInt   (L"taskbar",    L"window_offset_top",                0);
    rTaskbarData.vertical_margin                        = ini.GetInt   (L"taskbar",    L"vertical_margin",                  0);
    rTaskbarData.ValidItemSpace();
    rTaskbarData.ValidWindowOffsetTop();

    if (m_win_version.IsWindows10OrLater())     //只有Win10才支持自动适应系统深色/浅色主题
        rTaskbarData.auto_adapt_light_theme     = ini.GetBool(L"taskbar", L"auto_adapt_light_theme", false);
    else
        rTaskbarData.auto_adapt_light_theme     = false;
    rTaskbarData.dark_default_style             = ini.GetInt(L"taskbar", L"dark_default_style", 0);
    rTaskbarData.light_default_style            = ini.GetInt(L"taskbar", L"light_default_style", TASKBAR_DEFAULT_LIGHT_STYLE_INDEX);

    if (m_win_version.IsWindows8OrLater())
        rTaskbarData.auto_set_background_color  = ini.GetBool(L"taskbar", L"auto_set_background_color", false);
    else
        rTaskbarData.auto_set_background_color  = false;

    //任务栏各监控项显示顺序设置
    rTaskbarData.item_order.Init();
    rTaskbarData.item_order.FromString(ini.GetString(L"taskbar", L"item_order", L""));
    rTaskbarData.plugin_display_item.FromString(ini.GetString(L"taskbar", L"plugin_display_item", L""));
    rTaskbarData.auto_save_taskbar_color_settings_to_preset = ini.GetBool(L"taskbar", L"auto_save_taskbar_color_settings_to_preset", true);

    rTaskbarData.b_show_netspeed_figure         = ini.GetBool(L"taskbar", L"show_netspeed_figure", false);
    rTaskbarData.netspeed_figure_max_value      = ini.GetInt(L"taskbar", L"netspeed_figure_max_value", 512);
    rTaskbarData.netspeed_figure_max_value_unit = ini.GetInt(L"taskbar", L"netspeed_figure_max_value_unit", 0);

    if (CTaskBarDlgDrawCommonSupport::CheckSupport())
        rTaskbarData.disable_d2d = ini.GetBool(L"taskbar", L"disable_d2d", true);
    else
        rTaskbarData.disable_d2d = true;

    //其他设置
    //rAppData.m_show_internet_ip = ini.GetBool(L"connection_details", L"show_internet_ip", false);
    rAppData.m_use_log_scale                = ini.GetBool(_T("histroy_traffic"), _T("use_log_scale"), true);
    rAppData.m_sunday_first                 = ini.GetBool(_T("histroy_traffic"), _T("sunday_first"), true);
    rAppData.m_view_type = static_cast<HistoryTrafficViewType>(ini.GetInt(_T("histroy_traffic"), _T("view_type"), static_cast<int>(HistoryTrafficViewType::HV_DAY)));

    m_no_multistart_warning                 = ini.GetBool(_T("other"), _T("no_multistart_warning"), false);
    m_notify_interval                       = ini.GetInt(_T("other"), _T("notify_interval"), 60);
    m_exit_when_start_by_restart_manager    = ini.GetBool(_T("other"), _T("exit_when_start_by_restart_manager"), true);
    m_debug_log                             = ini.GetBool(_T("other"), _T("debug_log"), false);
    //由于Win7系统中设置任务栏窗口透明色会导致任务栏窗口不可见，因此默认在Win7中禁用透明色的设定
    m_taksbar_transparent_color_enable      = ini.GetBool(L"other", L"taksbar_transparent_color_enable", !m_win_version.IsWindows7());
    m_last_light_mode                       = ini.GetBool(L"other", L"last_light_mode", CWindowsSettingHelper::IsWindows10LightTheme());
    m_show_mouse_panetrate_tip              = ini.GetBool(L"other", L"show_mouse_panetrate_tip", true);
    m_show_dot_net_notinstalled_tip         = ini.GetBool(L"other", L"show_dot_net_notinstalled_tip", true);

    rAppData.taskbar_left_space_win11       = ini.GetInt(L"taskbar", L"taskbar_left_space_win11", 160);

    //(a)载入任务栏窗口字体设置(b)载入用于任务栏窗口的所有监控项(包括内置监控项和插件项)的标签、标签颜色、数值颜色设置
    rTaskbarData.layout.LoadConfig(LIAO_TASKBAR, m_config_layouts_path, m_taskbar_default_font, m_default_color, rTaskbarData.dft_back_color, rTaskbarData.dft_transparent_color, rTaskbarData.dft_status_bar_color);
    if (rTaskbarData.IsTaskbarTransparent()) //如果任务栏背景透明，则需要将颜色转换一下
    {
        CCommon::TransparentColorConvert(rTaskbarData.layout.back_color);
        CCommon::TransparentColorConvert(rTaskbarData.layout.transparent_color);
    }
    if (rTaskbarData.layout.back_color == 0 && !rTaskbar_M_LayoutItems.empty() && rTaskbar_M_LayoutItems.begin()->second.PrefixColor == 0)     //万一读取到的背景色和文本颜色都为0（黑色），则将文本色和背景色设置成默认颜色
    {
        rTaskbarData.layout.back_color = rTaskbarData.dft_back_color;
        rTaskbar_M_LayoutItems.begin()->second.PrefixColor = rTaskbarData.dft_text_colors;
    }
}

void CTrafficMonitorApp::SaveConfig()
{
    CIniHelper ini{ m_config_path };
    GeneralSettingData& rGeneralData    = m_general_data;
    MainWndSettingData& rMainWndData    = m_main_wnd_data;
    TaskBarSettingData& rTaskbarData    = m_taskbar_data;
    AppSettingData&     rAppData        = m_cfg_data;
    std::map<CommonDisplayItem, LayoutItem>&    rMainWnd_M_LayoutItems  = rMainWndData.layout.M_LayoutItems;
    std::map<CommonDisplayItem, LayoutItem>&    rTaskbar_M_LayoutItems  = rTaskbarData.layout.M_LayoutItems;
    LayoutItemValueAttributes&                  rMainWnd_LIVA           = rMainWndData.layout_item_value_attributes;
    LayoutItemValueAttributes&                  rTaskbar_LIVA           = rTaskbarData.layout_item_value_attributes;

    ////////////////////////////////////////////////////////////////////////////////////////
    //      (一)保存APP全局性设置 = 选项对话框中的常规设置 + 鼠标右键中的部分设置 + 其它设置
    ////////////////////////////////////////////////////////////////////////////////////////
    //(1)选项对话框中的常规设置(全局性唯一性设置)
    ini.WriteBool     (_T("general"), _T("check_update_when_start"),     rGeneralData.check_update_when_start);
    ini.WriteInt      (_T("general"), _T("language"),   static_cast<int>(rGeneralData.language));
    //提示信息选项
    ini.WriteBool       (L"notify_tip", L"traffic_tip_enable",                  rGeneralData.traffic_tip_enable);
    ini.WriteInt        (L"notify_tip", L"traffic_tip_value",                   rGeneralData.traffic_tip_value);
    ini.WriteInt        (L"notify_tip", L"traffic_tip_unit",                    rGeneralData.traffic_tip_unit);
    ini.WriteBool       (L"notify_tip", L"memory_usage_tip_enable",             rGeneralData.memory_usage_tip.enable);
    ini.WriteInt        (L"notify_tip", L"memory_tip_value",                    rGeneralData.memory_usage_tip.tip_value);
    ini.WriteBool       (L"notify_tip", L"cpu_temperature_tip_enable",          rGeneralData.cpu_temp_tip.enable);
    ini.WriteInt        (L"notify_tip", L"cpu_temperature_tip_value",           rGeneralData.cpu_temp_tip.tip_value);
    ini.WriteBool       (L"notify_tip", L"gpu_temperature_tip_enable",          rGeneralData.gpu_temp_tip.enable);
    ini.WriteInt        (L"notify_tip", L"gpu_temperature_tip_value",           rGeneralData.gpu_temp_tip.tip_value);
    ini.WriteBool       (L"notify_tip", L"hdd_temperature_tip_enable",          rGeneralData.hdd_temp_tip.enable);
    ini.WriteInt        (L"notify_tip", L"hdd_temperature_tip_value",           rGeneralData.hdd_temp_tip.tip_value);
    ini.WriteBool       (L"notify_tip", L"mainboard_temperature_tip_enable",    rGeneralData.mainboard_temp_tip.enable);
    ini.WriteInt        (L"notify_tip", L"mainboard_temperature_tip_value",     rGeneralData.mainboard_temp_tip.tip_value);
    //硬件监控
    ini.WriteInt        (L"general",    L"hardware_monitor_item",               rGeneralData.hardware_monitor_item);
    ini.WriteString     (L"general",    L"hard_disk_name",                      rGeneralData.hard_disk_name);
    ini.WriteString     (L"general",    L"cpu_core_name",                       rGeneralData.cpu_core_name);
    ini.WriteBool       (L"general",    L"show_all_interface",                  rGeneralData.show_all_interface);
    //CPU利用率的方式
    ini.WriteBool       (L"general",    L"get_cpu_usage_by_cpu_times",          rGeneralData.m_get_cpu_usage_by_cpu_times);
    ini.WriteInt        (L"general",    L"monitor_time_span",                   rGeneralData.monitor_time_span);
    //最后一项配置
    ini.WriteBool       (L"config",     L"show_notify_icon",                    rGeneralData.show_notify_icon);
    //(2)鼠标右键中的部分设置
    ini.WriteBool       (L"config",     L"show_taskbar_wnd",                   rAppData.m_show_taskbar_wnd);
    ini.WriteInt        (L"config",     L"hide_main_window",                    rAppData.m_hide_main_window);
    //网络连接设置
    ini.WriteBool       (L"connection", L"auto_select",                         rAppData.m_auto_select);
    ini.WriteBool       (L"connection", L"select_all",                          rAppData.m_select_all);
    ini.WriteString     (L"connection", L"connection_name", CCommon::StrToUnicode(rAppData.m_connection_name.c_str()));
    ini.WriteStringList (L"general",    L"connections_hide",                    rGeneralData.connections_hide.ToVector());
    //通知图标设置
    ini.WriteInt        (L"config",     L"notify_icon_selected",                rAppData.m_notify_icon_selected);
    ini.WriteBool       (L"config",     L"notify_icon_auto_adapt",              rAppData.m_notify_icon_auto_adapt);
    ////////////////////////////////////////////////////////////////////////////////////////
    //      (二)保存主窗口设置 = 选项对话框中的主窗口设置 + 鼠标右键中的部分设置 + 其它设置
    ////////////////////////////////////////////////////////////////////////////////////////
    //(1)选项对话框中的主窗口设置(当前版本情况：只支持全局性设置)

    //保存其它设置
    ini.WriteBool           (L"config",     L"hide_unit",                       rMainWnd_LIVA.hide_unit);
    ini.WriteBool           (L"config",     L"hide_percent",                    rMainWnd_LIVA.hide_percent);
    ini.WriteBool           (L"config",     L"separate_value_unit_with_space",  rMainWnd_LIVA.separate_value_unit_with_space);
    ini.WriteBool           (L"config",     L"speed_short_mode",                rMainWnd_LIVA.speed_short_mode);
    ini.WriteInt            (L"config",     L"speed_unit",     static_cast<int>(rMainWnd_LIVA.speed_unit));
    ini.WriteBool           (L"config",     L"show_tool_tip",                   rMainWndData.show_tool_tip);
    ini.WriteInt            (L"config",     L"memory_display", static_cast<int>(rMainWndData.memory_display));
    ini.WriteBool           (L"config",     L"always_on_top",                   rMainWndData.m_always_on_top);
    ini.WriteBool           (L"config",     L"lock_window_pos",                 rMainWndData.m_lock_window_pos);
    ini.WriteBool           (L"config",     L"mouse_penetrate",                 rMainWndData.m_mouse_penetrate);
    ini.WriteInt            (L"config",     L"alow_out_of_border",              rMainWndData.m_alow_out_of_border);
    ini.WriteBool           (L"config",     L"hide_main_wnd_when_fullscreen",   rMainWndData.hide_main_wnd_when_fullscreen);
    ini.WriteInt            (L"config",     L"double_click_action",static_cast<int>(rMainWndData.double_click_action));
    ini.WriteString         (L"config",     L"double_click_exe",                rMainWndData.double_click_exe);
    //(2)鼠标右键中的部分设置
    ini.WriteBool           (L"config",     L"show_more_info",                  rMainWndData.m_show_more_info);
    ini.WriteInt            (L"config",     L"transparency",                    rMainWndData.m_transparency);
    ini.WriteString       (_T("config"),  _T("skin_selected"),                  rMainWndData.m_skin_name.c_str());
    //(3)其它设置
    ini.WriteInt            (L"config",     L"position_x",                      rMainWndData.m_position_x);
    ini.WriteInt            (L"config",     L"position_y",                      rMainWndData.m_position_y);

    ////////////////////////////////////////////////////////////////////////////////////////
    //      (三)保存任务栏窗口设置 = 选项对话框中的任务栏窗口设置 + 鼠标右键中的部分设置 + 其它设置
    ////////////////////////////////////////////////////////////////////////////////////////
    //任务栏窗口设置
    ini.WriteInt            (L"taskbar",    L"taskbar_display_item",    rTaskbarData.m_tbar_display_item);

    //(1)选项对话框中的任务栏窗口设置

    //任务栏选项设置
    ini.WriteBool(L"taskbar", L"taskbar_hide_unit",                     rTaskbar_LIVA.hide_unit);
    ini.WriteBool(L"taskbar", L"taskbar_hide_percent",                  rTaskbar_LIVA.hide_percent);
    ini.WriteBool(L"taskbar", L"separate_value_unit_with_space",        rTaskbar_LIVA.separate_value_unit_with_space);
    ini.WriteBool(L"taskbar", L"taskbar_speed_short_mode",              rTaskbar_LIVA.speed_short_mode);
    ini.WriteInt (L"taskbar", L"taskbar_speed_unit",   static_cast<int>(rTaskbar_LIVA.speed_unit));
    ini.WriteBool(L"taskbar", L"taskbar_wnd_on_left",                   rTaskbarData.tbar_wnd_on_left);
    ini.WriteBool(L"taskbar", L"taskbar_wnd_snap",                      rTaskbarData.tbar_wnd_snap);
    ini.WriteBool(L"taskbar", L"value_right_align",                     rTaskbarData.value_right_align);
    ini.WriteBool(L"taskbar", L"horizontal_arrange",                    rTaskbarData.horizontal_arrange);
    ini.WriteBool(L"taskbar", L"show_status_bar",                       rTaskbarData.b_show_resource_figure);
    ini.WriteBool(L"taskbar", L"show_tool_tip",                         rTaskbarData.show_tool_tip);
    ini.WriteInt (L"taskbar", L"digits_number",                         rTaskbarData.digits_number);
    ini.WriteInt (L"taskbar", L"memory_display",       static_cast<int>(rTaskbarData.memory_display));
    ini.WriteInt (L"taskbar", L"double_click_action",  static_cast<int>(rTaskbarData.double_click_action));
    ini.WriteString(L"taskbar", L"double_click_exe",                    rTaskbarData.double_click_exe);
    ini.WriteBool(L"taskbar", L"cm_graph_type",                         rTaskbarData.cm_graph_type);
    ini.WriteBool(L"taskbar", L"show_graph_dashed_box",                 rTaskbarData.b_show_graph_dashed_box);
    ini.WriteInt (L"taskbar", L"item_space",                            rTaskbarData.item_space);
    ini.WriteInt (L"taskbar", L"window_offset_top",                     rTaskbarData.window_offset_top);
    ini.WriteInt (L"taskbar", L"vertical_margin",                       rTaskbarData.vertical_margin);

    ini.WriteBool(L"taskbar", L"auto_adapt_light_theme",                rTaskbarData.auto_adapt_light_theme);
    ini.WriteInt (L"taskbar", L"dark_default_style",                    rTaskbarData.dark_default_style);
    ini.WriteInt (L"taskbar", L"light_default_style",                   rTaskbarData.light_default_style);
    ini.WriteBool(L"taskbar", L"auto_set_background_color",             rTaskbarData.auto_set_background_color);

    //任务栏各监控项显示顺序设置
    ini.WriteString(L"taskbar", L"item_order",                          rTaskbarData.item_order.ToString());
    ini.WriteString(L"taskbar", L"plugin_display_item",                 rTaskbarData.plugin_display_item.ToString());
    ini.WriteBool(L"taskbar", L"auto_save_taskbar_color_settings_to_preset", rTaskbarData.auto_save_taskbar_color_settings_to_preset);

    ini.WriteBool(L"taskbar", L"show_netspeed_figure",                  rTaskbarData.b_show_netspeed_figure);
    ini.WriteInt (L"taskbar", L"netspeed_figure_max_value",             rTaskbarData.netspeed_figure_max_value);
    ini.WriteInt (L"taskbar", L"netspeed_figure_max_value_unit",        rTaskbarData.netspeed_figure_max_value_unit);

    ini.WriteBool(L"taskbar", L"disable_d2d",                          rTaskbarData.disable_d2d);

    //其他设置
    //ini.WriteBool(L"connection_details", L"show_internet_ip",         rAppData.m_show_internet_ip);
    ini.WriteBool(L"histroy_traffic", L"use_log_scale",                 rAppData.m_use_log_scale);
    ini.WriteBool(L"histroy_traffic", L"sunday_first",                  rAppData.m_sunday_first);
    ini.WriteInt (L"histroy_traffic", L"view_type",     static_cast<int>(rAppData.m_view_type));

    ini.WriteBool(_T("other"), _T("no_multistart_warning"), m_no_multistart_warning);
    ini.WriteBool(_T("other"), _T("exit_when_start_by_restart_manager"), m_exit_when_start_by_restart_manager);
    ini.WriteBool(_T("other"), _T("debug_log"), m_debug_log);
    ini.WriteInt (_T("other"), _T("notify_interval"), m_notify_interval);
    ini.WriteBool(_T("other"), _T("taksbar_transparent_color_enable"), m_taksbar_transparent_color_enable);
    ini.WriteBool(_T("other"), _T("last_light_mode"), m_last_light_mode);
    ini.WriteBool(_T("other"), _T("show_mouse_panetrate_tip"), m_show_mouse_panetrate_tip);
    ini.WriteBool(_T("other"), _T("show_dot_net_notinstalled_tip"), m_show_dot_net_notinstalled_tip);

    ini.WriteString(L"config", L"plugin_disabled", rAppData.plugin_disabled.ToString());

    ini.WriteInt(L"taskbar", L"taskbar_left_space_win11", rAppData.taskbar_left_space_win11);

    ini.WriteString(L"app", L"version", VERSION);

    //检查是否保存成功
    if (!ini.Save())
    {
        if (m_cannot_save_config_warning)
        {
            CString info;
            info.LoadString(IDS_CONNOT_SAVE_CONFIG_WARNING);
            info.Replace(_T("<%file_path%>"), m_config_path.c_str());
            AfxMessageBox(info, MB_ICONWARNING);
        }
        m_cannot_save_config_warning = false;
        return;
    }
    else
    {
        //(a)保存任务栏窗口字体设置(b)保存用于任务栏窗口的所有监控项(包括内置监控项和插件项)的标签、标签颜色、数值颜色设置
        rTaskbarData.layout.SaveConfig(LIAO_TASKBAR, m_config_layouts_path);
        //(a)保存主窗口字体设置(b)保存用于主窗口的所有监控项(包括内置监控项和插件项)的标签、标签颜色、数值颜色设置(当前版本情况：只支持全局性设置)
        rMainWndData.layout.SaveConfig(LIAO_MAINWND, m_config_layouts_path);
    }
}

void CTrafficMonitorApp::LoadPluginDisabledSettings()
{
    CIniHelper ini{ m_config_path };
    m_cfg_data.plugin_disabled.FromString(ini.GetString(L"config", L"plugin_disabled", L""));
}

void CTrafficMonitorApp::LoadGlobalConfig()
{
    GeneralSettingData& rGeneralData = m_general_data;
    bool portable_mode_default{ false };
    wstring global_cfg_path{ m_module_dir + L"global_cfg.ini" };
    if (!CCommon::FileExist(global_cfg_path.c_str()))       //如果global_cfg.ini不存在，则根据AppData/Roaming/TrafficMonitor目录下是否存在config.ini来判断配置文件的保存位置
    {
        portable_mode_default = !CCommon::FileExist((m_appdata_dir + L"config.ini").c_str());
    }

    CIniHelper ini{ global_cfg_path };
    rGeneralData.portable_mode = ini.GetBool(L"config", L"portable_mode", portable_mode_default);

    //执行一次保存操作，以检查当前目录是否有写入权限
    m_module_dir_writable = ini.Save();

    if (m_module_dir.find(CCommon::GetTemplateDir()) != wstring::npos)      //如果当前路径是在Temp目录下，则强制将数据保存到Appdata
    {
        m_module_dir_writable = false;
    }

    if (!m_module_dir_writable)              //如果当前目录没有写入权限，则设置配置保存到AppData目录
    {
        rGeneralData.portable_mode = false;
    }
}

void CTrafficMonitorApp::SaveGlobalConfig()
{
    GeneralSettingData& rGeneralData = m_general_data;
    CIniHelper ini{ m_module_dir + L"global_cfg.ini" };
    ini.WriteBool(L"config", L"portable_mode", rGeneralData.portable_mode);

    //检查是否保存成功
    if (!ini.Save())
    {
        //if (m_cannot_save_global_config_warning)
        //{
        //    CString info;
        //    info.LoadString(IDS_CONNOT_SAVE_CONFIG_WARNING);
        //    info.Replace(_T("<%file_path%>"), m_module_dir.c_str());
        //    AfxMessageBox(info, MB_ICONWARNING);
        //}
        //m_cannot_save_global_config_warning = false;
        //return;
    }
}

int CTrafficMonitorApp::DPI(int pixel)
{
    return m_dpi * pixel / 96;
}

void CTrafficMonitorApp::DPI(CRect& rect)
{
    rect.left = DPI(rect.left);
    rect.right = DPI(rect.right);
    rect.top = DPI(rect.top);
    rect.bottom = DPI(rect.bottom);
}

void CTrafficMonitorApp::DPIFromWindow(CWnd* pWnd)
{
    CWindowDC dc(pWnd);
    HDC hDC = dc.GetSafeHdc();
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
}

void CTrafficMonitorApp::CheckUpdate(bool message)
{
    if (m_checking_update)      //如果还在检查更新，则直接返回
        return;
    GeneralSettingData& rGeneralData = m_general_data;
    CFlagLocker update_locker(m_checking_update);
    CWaitCursor wait_cursor;

    wstring version;        //程序版本
    wstring link;           //下载链接
    wstring contents_zh_cn; //更新内容（简体中文）
    wstring contents_en;    //更新内容（English）
    wstring contents_zh_tw; //更新内容（繁体中文）
    CUpdateHelper update_helper;
    update_helper.SetUpdateSource(static_cast<CUpdateHelper::UpdateSource>(rGeneralData.update_source));
    if (!update_helper.CheckForUpdate())
    {
        if (message)
            AfxMessageBox(CCommon::LoadText(IDS_CHECK_UPDATE_FAILD), MB_OK | MB_ICONWARNING);
        return;
    }
    version = update_helper.GetVersion();
#ifdef _M_X64
    link = update_helper.GetLink64();
#else
    link = update_helper.GetLink();
#endif
    contents_zh_cn = update_helper.GetContentsZhCn();
    contents_en = update_helper.GetContentsEn();
    contents_zh_tw = update_helper.GetContentsZhTw();
    if (version.empty() || link.empty())
    {
        if (message)
        {
            CString info = CCommon::LoadText(IDS_CHECK_UPDATE_ERROR);
            info += _T("\r\nrow_data=");
            info += std::to_wstring(update_helper.IsRowData()).c_str();

            AfxMessageBox(info, MB_OK | MB_ICONWARNING);
        }
        return;
    }
    if (version > VERSION)      //如果服务器上的版本大于本地版本
    {
        CString info;
        //根据语言设置选择对应语言版本的更新内容
        int language_code = _ttoi(CCommon::LoadText(IDS_LANGUAGE_CODE));
        wstring contents_lan;
        switch (language_code)
        {
        case 2: contents_lan = contents_zh_cn; break;
        case 3: contents_lan = contents_zh_tw; break;
        default: contents_lan = contents_en; break;
        }

        if (contents_lan.empty())
            info.Format(CCommon::LoadText(IDS_UPDATE_AVLIABLE), version.c_str());
        else
            info.Format(CCommon::LoadText(IDS_UPDATE_AVLIABLE2), version.c_str(), contents_lan.c_str());

        if (AfxMessageBox(info, MB_YESNO | MB_ICONQUESTION) == IDYES)
        {
            ShellExecute(NULL, _T("open"), link.c_str(), NULL, NULL, SW_SHOW);      //转到下载链接
        }
    }
    else
    {
        if (message)
            AfxMessageBox(CCommon::LoadText(IDS_ALREADY_UPDATED), MB_OK | MB_ICONINFORMATION);
    }
}

void CTrafficMonitorApp::CheckUpdateInThread(bool message)
{
    AfxBeginThread(CheckUpdateThreadFunc, (LPVOID)message);
}

UINT CTrafficMonitorApp::CheckUpdateThreadFunc(LPVOID lpParam)
{
    GeneralSettingData& rGeneralData = theApp.m_general_data;
    CCommon::SetThreadLanguage(rGeneralData.language);     //设置线程语言
    theApp.CheckUpdate(lpParam);        //检查更新
    return 0;
}

UINT CTrafficMonitorApp::InitOpenHardwareMonitorLibThreadFunc(LPVOID lpParam)
{
#ifndef WITHOUT_TEMPERATURE
    CSingleLock sync(&theApp.m_minitor_lib_critical, TRUE);
    theApp.m_pMonitor = OpenHardwareMonitorApi::CreateInstance();
    if (theApp.m_pMonitor == nullptr)
    {
        AfxMessageBox(OpenHardwareMonitorApi::GetErrorMessage().c_str(), MB_ICONERROR | MB_OK);
    }
    //设置硬件监控的启用状态
    theApp.UpdateOpenHardwareMonitorEnableState();
#endif
    return 0;
}

bool  CTrafficMonitorApp::SetAutoRun(bool auto_run)
{
    //不含温度监控的版本使用添加注册表项的方式实现开机自启动
#ifdef WITHOUT_TEMPERATURE
    return SetAutoRunByRegistry(auto_run);
#else
    //包含温度监控的版本使用任务计划的方式实现开机自启动
    return SetAutoRunByTaskScheduler(auto_run);
#endif
}

bool CTrafficMonitorApp::GetAutoRun(wstring* auto_run_path)
{
    if (auto_run_path != nullptr)
        auto_run_path->clear();
    //不含温度监控的版本使用添加注册表项的方式实现开机自启动
#ifdef WITHOUT_TEMPERATURE
    CRegKey key;
    if (key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
    {
        //打开注册表“Software\\Microsoft\\Windows\\CurrentVersion\\Run”失败，则返回false
        return false;
    }
    wchar_t buff[256];
    ULONG size{ 256 };
    if (key.QueryStringValue(APP_NAME, buff, &size) == ERROR_SUCCESS)       //如果找到了“TrafficMonitor”键
    {
        if (auto_run_path != nullptr)
        {
            //保存路径
            *auto_run_path = buff;
            //去掉前后的引号
            if (auto_run_path->front() == L'\"')
                *auto_run_path = auto_run_path->substr(1);
            if (auto_run_path->back() = L'\"')
                auto_run_path->pop_back();
        }
        return (m_module_path_reg == buff); //如果“TrafficMonitor”的值是当前程序的路径，就返回true，否则返回false
    }
    else
    {
        return false;       //没有找到“TrafficMonitor”键，返回false
    }
#else
    //包含温度监控的版本使用任务计划的方式实现开机自启动
    return is_auto_start_task_active_for_this_user(auto_run_path);
#endif
}

bool CTrafficMonitorApp::SetAutoRunByRegistry(bool auto_run)
{
    CRegKey key;
    //打开注册表项
    if (key.Open(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run")) != ERROR_SUCCESS)
    {
        AfxMessageBox(CCommon::LoadText(IDS_AUTORUN_FAILED_NO_KEY), MB_OK | MB_ICONWARNING);
        return false;
    }
    if (auto_run)       //写入注册表项
    {
        //通过注册表设置开机自启动项时删除计划任务中的自启动项
        SetAutoRunByTaskScheduler(false);

        if (key.SetStringValue(APP_NAME, m_module_path_reg.c_str()) != ERROR_SUCCESS)
        {
            AfxMessageBox(CCommon::LoadText(IDS_AUTORUN_FAILED_NO_ACCESS), MB_OK | MB_ICONWARNING);
            return false;
        }
    }
    else        //删除注册表项
    {
        //删除前先检查注册表项是否存在，如果不存在，则直接返回
        wchar_t buff[256];
        ULONG size{ 256 };
        if (key.QueryStringValue(APP_NAME, buff, &size) != ERROR_SUCCESS)
            return false;
        if (key.DeleteValue(APP_NAME) != ERROR_SUCCESS)
        {
            AfxMessageBox(CCommon::LoadText(IDS_AUTORUN_DELETE_FAILED), MB_OK | MB_ICONWARNING);
            return false;
        }
    }
    return true;
}

bool CTrafficMonitorApp::SetAutoRunByTaskScheduler(bool auto_run)
{
    bool succeed = delete_auto_start_task_for_this_user();     //先删除开机自启动
    if (auto_run)
    {
        //通过计划任务设置开机自启动项时删除注册表中的自启动项
        SetAutoRunByRegistry(false);

        succeed = create_auto_start_task_for_this_user(true);
    }
    return succeed;
}

CString CTrafficMonitorApp::GetSystemInfoString()
{
    CString info;
    info += _T("System Info:\r\n");

    CString strTmp;
    strTmp.Format(_T("Windows Version: %d.%d build %d\r\n"), m_win_version.GetMajorVersion(),
        m_win_version.GetMinorVersion(), m_win_version.GetBuildNumber());
    info += strTmp;

    strTmp.Format(_T("DPI: %d"), m_dpi);
    info += strTmp;
    info += _T("\r\n");

    info += _T("Version: ");
    info += VERSION;
    info += _T(" ");
#ifdef _M_X64
    info += _T("x64");
#else
    info += _T("x86");
#endif

#ifdef WITHOUT_TEMPERATURE
    info += CCommon::LoadText(_T(" ("), IDS_WITHOUT_TEMPERATURE, _T(")"));
#endif

    info += _T("\r\nLast compiled date: ");
    info += CCommon::GetLastCompileTime();

    return info;
}


void CTrafficMonitorApp::InitMenuResourse()
{
    //载入菜单
    m_main_menu.LoadMenu(IDR_MENU1);
    m_main_menu_plugin.LoadMenu(IDR_MENU1);
    m_taskbar_menu.LoadMenu(IDR_TASK_BAR_MENU);
    m_taskbar_menu_plugin.LoadMenu(IDR_TASK_BAR_MENU);

    //为插件菜单添加额外项目
    m_main_menu_plugin_sub_menu.CreatePopupMenu();
    m_main_menu_plugin_sub_menu.AppendMenu(MF_STRING | MF_ENABLED, ID_PLUGIN_OPTIONS, CCommon::LoadText(IDS_PLUGIN_OPTIONS, _T("...")));
    m_main_menu_plugin_sub_menu.AppendMenu(MF_STRING | MF_ENABLED, ID_PLUGIN_DETAIL, CCommon::LoadText(IDS_PLUGIN_INFO, _T("...")));
    CMenuIcon::AddIconToMenuItem(m_main_menu_plugin_sub_menu.GetSafeHmenu(), ID_PLUGIN_OPTIONS, FALSE, GetMenuIcon(IDI_SETTINGS));
    CMenuIcon::AddIconToMenuItem(m_main_menu_plugin_sub_menu.GetSafeHmenu(), ID_PLUGIN_DETAIL, FALSE, GetMenuIcon(IDI_ITEM));
    CMenu* main_menu_plugin = m_main_menu_plugin.GetSubMenu(0);
    main_menu_plugin->AppendMenu(MF_SEPARATOR);
    main_menu_plugin->AppendMenu(MF_POPUP | MF_STRING, (UINT)m_main_menu_plugin_sub_menu.m_hMenu, _T("<plugin name>"));

    m_taskbar_menu_plugin_sub_menu.CreatePopupMenu();
    m_taskbar_menu_plugin_sub_menu.AppendMenu(MF_STRING | MF_ENABLED, ID_PLUGIN_OPTIONS_TASKBAR, CCommon::LoadText(IDS_PLUGIN_OPTIONS, _T("...")));
    m_taskbar_menu_plugin_sub_menu.AppendMenu(MF_STRING | MF_ENABLED, ID_PLUGIN_DETAIL_TASKBAR, CCommon::LoadText(IDS_PLUGIN_INFO, _T("...")));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu_plugin_sub_menu.GetSafeHmenu(), ID_PLUGIN_OPTIONS_TASKBAR, FALSE, GetMenuIcon(IDI_SETTINGS));
    CMenuIcon::AddIconToMenuItem(m_taskbar_menu_plugin_sub_menu.GetSafeHmenu(), ID_PLUGIN_DETAIL_TASKBAR, FALSE, GetMenuIcon(IDI_ITEM));
    CMenu* taskbar_menu_plugin = m_taskbar_menu_plugin.GetSubMenu(0);
    taskbar_menu_plugin->AppendMenu(MF_SEPARATOR);
    taskbar_menu_plugin->AppendMenu(MF_POPUP | MF_STRING, (UINT)m_taskbar_menu_plugin_sub_menu.m_hMenu, _T("<plugin name>"));

    //为菜单项添加图标
    auto addIconsForMainWindowMenu = [&](const CMenu& menu)
    {
        CMenuIcon::AddIconToMenuItem(menu.GetSubMenu(0)->GetSafeHmenu(), 0, TRUE, GetMenuIcon(IDI_CONNECTION));
        CMenuIcon::AddIconToMenuItem(menu.GetSubMenu(0)->GetSafeHmenu(), 11, TRUE, GetMenuIcon(IDI_FUNCTION));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_NETWORK_INFO, FALSE, GetMenuIcon(IDI_INFO));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_ALWAYS_ON_TOP, FALSE, GetMenuIcon(IDI_PIN));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_MOUSE_PENETRATE, FALSE, GetMenuIcon(IDI_MOUSE));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_LOCK_WINDOW_POS, FALSE, GetMenuIcon(IDI_LOCK));
        //if (!m_win_version.IsWindows11OrLater())
        //    CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_NOTIFY_ICON, FALSE, GetMenuIcon(IDI_NOTIFY));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_CPU_MEMORY, FALSE, GetMenuIcon(IDI_MORE));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_TASKBAR_WND, FALSE, GetMenuIcon(IDI_TASKBAR_WINDOW));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_MAIN_WND, FALSE, GetMenuIcon(IDI_MAIN_WINDOW));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_CHANGE_SKIN, FALSE, GetMenuIcon(IDI_SKIN));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_CHANGE_NOTIFY_ICON, FALSE, GetMenuIcon(IDI_NOTIFY));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_TRAFFIC_HISTORY, FALSE, GetMenuIcon(IDI_STATISTICS));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_PLUGIN_MANAGE, FALSE, GetMenuIcon(IDI_PLUGINS));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_OPTIONS, FALSE, GetMenuIcon(IDI_SETTINGS));
        CMenuIcon::AddIconToMenuItem(menu.GetSubMenu(0)->GetSafeHmenu(), 14, TRUE, GetMenuIcon(IDI_HELP));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_HELP, FALSE, GetMenuIcon(IDI_HELP));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_APP_ABOUT, FALSE, GetMenuIcon(IDR_MAINFRAME));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_APP_EXIT, FALSE, GetMenuIcon(IDI_EXIT));
    };
    //主窗口右键菜单
    addIconsForMainWindowMenu(m_main_menu);
    addIconsForMainWindowMenu(m_main_menu_plugin);

    //任务栏窗口右键菜单
    auto addIconsForTaksbarWindowMenu = [&](const CMenu& menu)
    {
        CMenuIcon::AddIconToMenuItem(menu.GetSubMenu(0)->GetSafeHmenu(), 0, TRUE, GetMenuIcon(IDI_CONNECTION));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_NETWORK_INFO, FALSE, GetMenuIcon(IDI_INFO));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_TRAFFIC_HISTORY, FALSE, GetMenuIcon(IDI_STATISTICS));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_DISPLAY_SETTINGS, FALSE, GetMenuIcon(IDI_ITEM));
        //if (!m_win_version.IsWindows11OrLater())
        //    CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_NOTIFY_ICON, FALSE, GetMenuIcon(IDI_NOTIFY));
        if (!m_win_version.IsWindows11OrLater())
            CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_MAIN_WND, FALSE, GetMenuIcon(IDI_MAIN_WINDOW));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_SHOW_TASKBAR_WND,  FALSE, GetMenuIcon(IDI_CLOSE));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_OPEN_TASK_MANAGER, FALSE, GetMenuIcon(IDI_TASK_MANAGER));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_OPTIONS2, FALSE, GetMenuIcon(IDI_SETTINGS));
        CMenuIcon::AddIconToMenuItem(menu.GetSubMenu(0)->GetSafeHmenu(), 12, TRUE, GetMenuIcon(IDI_HELP));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_HELP, FALSE, GetMenuIcon(IDI_HELP));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_APP_ABOUT, FALSE, GetMenuIcon(IDR_MAINFRAME));
        CMenuIcon::AddIconToMenuItem(menu.GetSafeHmenu(), ID_APP_EXIT, FALSE, GetMenuIcon(IDI_EXIT));
    };
    addIconsForTaksbarWindowMenu(m_taskbar_menu);
    addIconsForTaksbarWindowMenu(m_taskbar_menu_plugin);

#ifdef _DEBUG
    m_main_menu.GetSubMenu(0)->AppendMenu(MF_BYCOMMAND, ID_CMD_TEST, _T("Test Command"));
#endif
}

HICON CTrafficMonitorApp::GetMenuIcon(UINT id)
{
    auto iter = m_menu_icons.find(id);
    if (iter != m_menu_icons.end())
    {
        return iter->second;
    }
    else
    {
        HICON hIcon = CCommon::LoadIconResource(id, DPI(16));
        m_menu_icons[id] = hIcon;
        return hIcon;
    }
}

void CTrafficMonitorApp::AutoSelectNotifyIcon()
{
    if (m_win_version.GetMajorVersion() >= 10)
    {
        AppSettingData& rAppData = m_cfg_data;
        bool light_mode = CWindowsSettingHelper::IsWindows10LightTheme();
        if (light_mode)     //浅色模式下，如果图标是白色，则改成黑色
        {
            if (rAppData.m_notify_icon_selected == 0)
                rAppData.m_notify_icon_selected = 4;
            if (rAppData.m_notify_icon_selected == 1)
                rAppData.m_notify_icon_selected = 5;
        }
        else                //深色模式下，如果图标是黑色，则改成白色
        {
            if (rAppData.m_notify_icon_selected == 4)
                rAppData.m_notify_icon_selected = 0;
            if (rAppData.m_notify_icon_selected == 5)
                rAppData.m_notify_icon_selected = 1;
        }
    }
}

// CTrafficMonitorApp 初始化
BOOL CTrafficMonitorApp::InitInstance()
{
    //替换掉对话框程序的默认类名
    WNDCLASS wc;
    ::GetClassInfo(AfxGetInstanceHandle(), _T("#32770"), &wc);       //MFC默认的所有对话框的类名为#32770
    wc.lpszClassName = APP_CLASS_NAME;      //将对话框的类名修改为新类名
    AfxRegisterClass(&wc);

    //设置配置文件的路径
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(NULL, path, MAX_PATH);
    m_module_path = path;
    if (m_module_path.find(L' ') != wstring::npos)
    {
        //如果路径中有空格，则在程序路径前后添加双引号
        m_module_path_reg = L'\"';
        m_module_path_reg += m_module_path;
        m_module_path_reg += L'\"';
    }
    else
    {
        m_module_path_reg = m_module_path;
    }
    m_module_dir = CCommon::GetModuleDir();
    m_system_dir = CCommon::GetSystemDir();
    m_appdata_dir = CCommon::GetAppDataConfigDir();

    LoadGlobalConfig();

    GeneralSettingData& rGeneralData = m_general_data;
#ifdef _DEBUG
    m_config_dir = L".\\";
    m_skin_dir = L".\\skins";
#else
    if (rGeneralData.portable_mode)
        m_config_dir = m_module_dir;
    else
        m_config_dir = m_appdata_dir;
    m_skin_dir = m_module_dir + L"skins";
#endif
    //AppData里面的程序配置文件路径
    m_config_path           = m_config_dir + L"config.ini";
    m_config_layouts_path   = m_config_dir + L"config_layouts.ini";
    m_history_traffic_path  = m_config_dir + L"history_traffic.dat";
    m_log_path              = m_config_dir + L"error.log";

//#ifndef _DEBUG
    //  //原来的、程序所在目录下的配置文件的路径
    //  wstring config_path_old = m_module_dir + L"config.ini";
    //  wstring history_traffic_path_old = m_module_dir + L"history_traffic.dat";
    //  wstring log_path_old = m_module_dir + L"error.log";
    //  //如果程序所在目录下含有配置文件，则将其移动到AppData对应的目录下面
    //  CCommon::MoveAFile(config_path_old.c_str(), m_config_path.c_str());
    //  CCommon::MoveAFile(history_traffic_path_old.c_str(), m_history_traffic_path.c_str());
    //  CCommon::MoveAFile(log_path_old.c_str(), m_log_path.c_str());
//#endif // !_DEBUG

    //初始化程序缺省值
    m_main_wnd_default_font.name    = CCommon::LoadText(IDS_DEFAULT_FONT);
    m_main_wnd_default_font.size    = 10;
    m_taskbar_default_font.name     = CCommon::LoadText(IDS_DEFAULT_FONT);
    m_taskbar_default_font.size     = 9;
    //判断皮肤是否存在
    std::vector<wstring> skin_files;
    CCommon::GetFiles((theApp.m_skin_dir + L"\\*").c_str(), skin_files);
    bool is_skin_exist = (!skin_files.empty());
    m_default_color = is_skin_exist ? 16384 : 16777215;          //根据皮肤是否存在来设置默认的文本颜色，皮肤文件不存在时文本颜色默认为白色

    bool is_windows10_fall_creator = m_win_version.IsWindows10FallCreatorOrLater();

    //载入插件
    LoadPluginDisabledSettings();
    m_plugin_manager.LoadPlugins();

    //从ini文件载入设置
    LoadConfig();

    //初始化界面语言
    CCommon::SetThreadLanguage(rGeneralData.language);

    //wstring cmd_line{ m_lpCmdLine };
    //bool is_restart{ cmd_line.find(L"RestartByRestartManager") != wstring::npos };        //如果命令行参数中含有字符串“RestartByRestartManager”则说明程序是被Windows重新启动的
    ////bool when_start{ CCommon::WhenStart(m_no_multistart_warning_time) };
    //if (m_exit_when_start_by_restart_manager && is_restart && is_windows10_fall_creator)      //当前Windows版本是秋季创意者更新时，如果程序被重新启动，则直接退出程序
    //{
    //  //AfxMessageBox(_T("调试信息：程序已被Windows的重启管理器重新启动。"));
    //  return FALSE;
    //}

    //检查是否已有实例正在运行
    LPCTSTR mutex_name{};
#ifdef _DEBUG
    mutex_name = _T("TrafficMonitor-e8Ahk24HP6JC8hDy");
#else
    mutex_name = _T("TrafficMonitor-1419J3XLKL1w8OZc");
#endif
    HANDLE hMutex = ::CreateMutex(NULL, TRUE, mutex_name);
    if (hMutex != NULL)
    {
        if (GetLastError() == ERROR_ALREADY_EXISTS)
        {
            //char buff[128];
            //string cmd_line_str{ CCommon::UnicodeToStr(cmd_line.c_str()) };
            //sprintf_s(buff, "when_start=%d, m_no_multistart_warning=%d, cmd_line=%s", when_start, m_no_multistart_warning, cmd_line_str.c_str());
            //CCommon::WriteLog(buff, _T(".\\start.log"));
            if (!m_no_multistart_warning)
            {
                //查找已存在TrafficMonitor进程的主窗口的句柄
                HWND exist_handel = ::FindWindow(APP_CLASS_NAME, NULL);
                if (exist_handel != NULL)
                {
                    //弹出“TrafficMonitor已在运行”对话框
                    CAppAlreadyRuningDlg dlg(exist_handel);
                    dlg.DoModal();
                }
                else
                {
                    AfxMessageBox(CCommon::LoadText(IDS_AN_INSTANCE_RUNNING));
                }
            }
            return FALSE;
        }
    }

    //从ini文件载入设置
    m_taskbar_default_style.LoadConfig();

    //SaveConfig();

    // 如果一个运行在 Windows XP 上的应用程序清单指定要
    // 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
    //则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    // 将它设置为包括所有要在应用程序中使用的
    // 公共控件类。
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();


    AfxEnableControlContainer();

    // 创建 shell 管理器，以防对话框包含
    // 任何 shell 树视图控件或 shell 列表视图控件。
    CShellManager* pShellManager = new CShellManager;

    // 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
    CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

    // 标准初始化
    // 如果未使用这些功能并希望减小
    // 最终可执行文件的大小，则应移除下列
    // 不需要的特定初始化例程
    // 更改用于存储设置的注册表项
    // TODO: 应适当修改该字符串，
    // 例如修改为公司或组织名
    //SetRegistryKey(_T("应用程序向导生成的本地应用程序"));        //暂不使用注册表保存数据

    //启动时检查更新
#ifndef _DEBUG      //DEBUG下不在启动时检查更新
    if (rGeneralData.check_update_when_start)
    {
        CheckUpdateInThread(false);
    }
#endif // !_DEBUG

#ifndef WITHOUT_TEMPERATURE
    //检测是否安装.net framework 4.5
    if (!CWindowsSettingHelper::IsDotNetFramework4Point5Installed())
    {
        if (theApp.m_show_dot_net_notinstalled_tip)
        {
            if (AfxMessageBox(CCommon::LoadText(IDS_DOTNET_NOT_INSTALLED_TIP), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)       //点击“取消”不再提示
            {
                theApp.m_show_dot_net_notinstalled_tip = false;
                SaveConfig();
            }
        }
    }
    else
    {
        //如果没有开启任何一项的硬件监控，则不初始化OpenHardwareMonitor
        if (theApp.m_general_data.IsHardwareEnable(HI_CPU) || theApp.m_general_data.IsHardwareEnable(HI_GPU)
            || theApp.m_general_data.IsHardwareEnable(HI_HDD) || theApp.m_general_data.IsHardwareEnable(HI_MBD))
        {
            //启动初始化OpenHardwareMonitor的线程。由于OpenHardwareMonitor初始化需要一定的时间，为了防止启动时程序卡顿，将其放到后台线程中处理
            InitOpenHardwareLibInThread();
        }
    }
#endif

    //执行测试代码
#ifdef _DEBUG
    CTest::Test();
#endif

    SendSettingsToPlugin();

    CTrafficMonitorDlg dlg;
    m_pMainWnd = &dlg;
    INT_PTR nResponse = dlg.DoModal();
    if (nResponse == IDOK)
    {
        // TODO: 在此放置处理何时用
        //  “确定”来关闭对话框的代码
    }
    else if (nResponse == IDCANCEL)
    {
        // TODO: 在此放置处理何时用
        //  “取消”来关闭对话框的代码
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
        TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
    }

    // 删除上面创建的 shell 管理器。
    if (pShellManager != NULL)
    {
        delete pShellManager;
    }

#ifndef _AFXDLL
    ControlBarCleanUp();
#endif

    // 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
    //  而不是启动应用程序的消息泵。
    return FALSE;
}

void CTrafficMonitorApp::InitOpenHardwareLibInThread()
{
#ifndef WITHOUT_TEMPERATURE
    AfxBeginThread(InitOpenHardwareMonitorLibThreadFunc, NULL);
#endif
}


void CTrafficMonitorApp::UpdateOpenHardwareMonitorEnableState()
{
#ifndef WITHOUT_TEMPERATURE
    if (m_pMonitor != nullptr)
    {
        GeneralSettingData& rGeneralData = m_general_data;
        CSingleLock sync(&theApp.m_minitor_lib_critical, TRUE);
        m_pMonitor->SetCpuEnable      (rGeneralData.IsHardwareEnable(HI_CPU));
        m_pMonitor->SetGpuEnable      (rGeneralData.IsHardwareEnable(HI_GPU));
        m_pMonitor->SetHddEnable      (rGeneralData.IsHardwareEnable(HI_HDD));
        m_pMonitor->SetMainboardEnable(rGeneralData.IsHardwareEnable(HI_MBD));
    }
#endif
}

//void CTrafficMonitorApp::UpdateTaskbarWndMenu()
//{
//    //获取“显示设置”子菜单
//    CMenu* pMenu = m_taskbar_menu.GetSubMenu(0)->GetSubMenu(5);
//    ASSERT(pMenu != nullptr);
//    if (pMenu != nullptr)
//    {
//        //将ID_SHOW_MEMORY_USAGE后面的所有菜单项删除
//        if (pMenu->GetMenuItemCount() > 4)
//        {
//            int start_pos = CCommon::GetMenuItemPosition(pMenu, ID_SHOW_MEMORY_USAGE) + 1;
//            while (pMenu->GetMenuItemCount() > start_pos)
//            {
//                pMenu->DeleteMenu(start_pos, MF_BYPOSITION);
//            }
//        }
//
//        //添加温度相关菜单项
//#ifndef WITHOUT_TEMPERATURE
//        if (m_general_data.IsHardwareEnable(HI_GPU))
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_GPU, CCommon::LoadText(IDS_GPU_USAGE));
//        if (m_general_data.IsHardwareEnable(HI_CPU))
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_CPU_TEMPERATURE, CCommon::LoadText(IDS_CPU_TEMPERATURE));
//        if (m_general_data.IsHardwareEnable(HI_GPU))
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_GPU_TEMPERATURE, CCommon::LoadText(IDS_GPU_TEMPERATURE));
//        if (m_general_data.IsHardwareEnable(HI_HDD))
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_HDD_TEMPERATURE, CCommon::LoadText(IDS_HDD_TEMPERATURE));
//        if (m_general_data.IsHardwareEnable(HI_MBD))
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_MAIN_BOARD_TEMPERATURE, CCommon::LoadText(IDS_MAINBOARD_TEMPERATURE));
//        if (m_general_data.IsHardwareEnable(HI_HDD))
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_HDD, CCommon::LoadText(IDS_HDD_USAGE));
//#endif
//
//        pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_TOTAL_SPEED, CCommon::LoadText(IDS_TOTAL_NET_SPEED));
//
//        //添加插件菜单项
//        if (!m_plugin_manager.GetPluginItems().empty())
//            pMenu->AppendMenu(MF_SEPARATOR);
//        int plugin_index = 0;
//        for (const auto& plugin_item : m_plugin_manager.GetPluginItems())
//        {
//            pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_SHOW_PLUGIN_ITEM_START + plugin_index, plugin_item->GetItemName());
//            plugin_index++;
//        }
//    }
//}

bool CTrafficMonitorApp::IsForceShowNotifyIcon()
{
    AppSettingData& rAppData = m_cfg_data;
    return ((!rAppData.m_show_taskbar_wnd /*|| m_win_version.IsWindows11OrLater()*/)
        && (rAppData.m_hide_main_window || m_main_wnd_data.m_mouse_penetrate));    //如果没有显示任务栏窗口，且隐藏了主窗口或设置了鼠标穿透，则禁用“显示通知区图标”菜单项
}

std::wstring CTrafficMonitorApp::GetPlauginTooltipInfo() const
{
    std::wstring tip_info;
    for (const auto& item : m_plugin_manager.GetAllPluginManageUnit())
    {
        if (item.plugin != nullptr && item.plugin->GetAPIVersion() >= 2)
        {
            std::wstring plugin_tooltip = item.plugin->GetTooltipInfo();
            CCommon::StringNormalize(plugin_tooltip);
            if (!plugin_tooltip.empty())
            {
                tip_info += L"\r\n";
                tip_info += plugin_tooltip.c_str();
            }
        }
    }
    return tip_info;
}

bool CTrafficMonitorApp::IsTaksbarItemDisplayed(CommonDisplayItem item) const
{
    if (item.is_plugin)
    {
        if (item.plugin_item != nullptr)
            return m_taskbar_data.plugin_display_item.Contains(item.plugin_item->GetItemId());
    }
    else
    {
        return m_taskbar_data.m_tbar_display_item & item.item_type;
    }
    return false;
}

void CTrafficMonitorApp::SendSettingsToPlugin()
{
    LayoutItemValueAttributes& rMainWnd_LIVA = m_main_wnd_data.layout_item_value_attributes;
    LayoutItemValueAttributes& rTaskbar_LIVA = m_taskbar_data.layout_item_value_attributes;

    for (const auto& pluginManageUnit : m_plugin_manager.GetAllPluginManageUnit())
    {
        if (pluginManageUnit.plugin != nullptr && pluginManageUnit.plugin->GetAPIVersion() >= 2)
        {
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_NAIN_WND_NET_SPEED_SHORT_MODE,     std::to_wstring(rMainWnd_LIVA.speed_short_mode).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_MAIN_WND_SPERATE_WITH_SPACE,       std::to_wstring(rMainWnd_LIVA.separate_value_unit_with_space).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_MAIN_WND_UNIT_BYTE,                std::to_wstring(true).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_MAIN_WND_UNIT_SELECT,              std::to_wstring(static_cast<int>(rMainWnd_LIVA.speed_unit)).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_MAIN_WND_NOT_SHOW_UNIT,            std::to_wstring(rMainWnd_LIVA.hide_unit).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_MAIN_WND_NOT_SHOW_PERCENT,         std::to_wstring(rMainWnd_LIVA.hide_percent).c_str());

            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_NET_SPEED_SHORT_MODE,  std::to_wstring(rTaskbar_LIVA.speed_short_mode).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_SPERATE_WITH_SPACE,    std::to_wstring(rTaskbar_LIVA.separate_value_unit_with_space).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_VALUE_RIGHT_ALIGN,     std::to_wstring(m_taskbar_data.value_right_align).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_NET_SPEED_WIDTH,       std::to_wstring(m_taskbar_data.digits_number).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_UNIT_BYTE,             std::to_wstring(true).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_UNIT_SELECT,           std::to_wstring(static_cast<int>(rTaskbar_LIVA.speed_unit)).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_NOT_SHOW_UNIT,         std::to_wstring(rTaskbar_LIVA.hide_unit).c_str());
            pluginManageUnit.plugin->OnExtenedInfo(ITMPlugin::EI_TASKBAR_WND_NOT_SHOW_PERCENT,      std::to_wstring(rTaskbar_LIVA.hide_percent).c_str());
        }
    }
}

void CTrafficMonitorApp::UpdatePluginMenu(CMenu* pMenu, ITMPlugin* plugin)
{
    if (pMenu != nullptr && plugin != nullptr)
    {
        //删除菜单已经存在的插件命令
        while (pMenu->GetMenuItemCount() > 2)
        {
            if (!pMenu->DeleteMenu(pMenu->GetMenuItemCount() - 1, MF_BYPOSITION))
                break;
        }
        //添加插件命令
        if (plugin->GetAPIVersion() >= 5)
        {
            int plugin_cmd_count = plugin->GetCommandCount();
            //添加分隔符
            if (plugin_cmd_count > 0)
                pMenu->AppendMenu(MF_SEPARATOR);
            for (int i = 0; i < plugin_cmd_count; i++)
            {
                const wchar_t* cmd_name = plugin->GetCommandName(i);
                if (cmd_name != nullptr)
                {
                    pMenu->AppendMenu(MF_STRING | MF_ENABLED, ID_PLUGIN_COMMAND_START + i, cmd_name);
                    //添加命令图标
                    HICON cmd_icon = (HICON)plugin->GetCommandIcon(i);
                    CMenuIcon::AddIconToMenuItem(pMenu->GetSafeHmenu(), ID_PLUGIN_COMMAND_START + i, FALSE, cmd_icon);
                }
            }
        }
    }
}

void CTrafficMonitorApp::OnHelp()
{
    // TODO: 在此添加命令处理程序代码
    ShellExecute(NULL, _T("open"), _T("https://github.com/zhongyang219/TrafficMonitor/wiki"), NULL, NULL, SW_SHOW);
}


void CTrafficMonitorApp::OnFrequentyAskedQuestions()
{
    // TODO: 在此添加命令处理程序代码
    GeneralSettingData& rGeneralData = m_general_data;
    CString url_domain;
    if (static_cast<CUpdateHelper::UpdateSource>(rGeneralData.update_source) == CUpdateHelper::UpdateSource::GiteeSource)
        url_domain = _T("gitee.com");
    else
        url_domain = _T("github.com");
    CString language_code{ CCommon::LoadText(IDS_LANGUAGE_CODE) };
    CString file_name;
    if (language_code == _T("2"))
        file_name = _T("Help.md");
    else
        file_name = _T("Help_en-us.md");
    CString url;
    url.Format(_T("https://%s/zhongyang219/TrafficMonitor/blob/master/%s"), url_domain.GetString(), file_name.GetString());
    ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOW);
}


void CTrafficMonitorApp::OnUpdateLog()
{
    // TODO: 在此添加命令处理程序代码
    GeneralSettingData& rGeneralData = m_general_data;
    CString url_domain;
    if (static_cast<CUpdateHelper::UpdateSource>(rGeneralData.update_source) == CUpdateHelper::UpdateSource::GiteeSource)
        url_domain = _T("gitee.com");
    else
        url_domain = _T("github.com");
    CString language_code{ CCommon::LoadText(IDS_LANGUAGE_CODE) };
    CString file_name;
    if (language_code == _T("2"))
        file_name = _T("update_log.md");
    else if (language_code == _T("3"))
        file_name = _T("update_log_zh-tw.md");
    else
        file_name = _T("update_log_en-us.md");
    CString url;
    url.Format(_T("https://%s/zhongyang219/TrafficMonitor/blob/master/UpdateLog/%s"), url_domain.GetString(), file_name.GetString());
    ShellExecute(NULL, _T("open"), url, NULL, NULL, SW_SHOW);
}
