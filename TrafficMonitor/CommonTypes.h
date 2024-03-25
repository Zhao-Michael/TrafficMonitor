//此文件只包含基本的全局结构体、枚举类型的定义
#pragma once
#include "stdafx.h"

struct Date
{
    int year{};
    int month{};
    int day{};

    int week() const;       //该日期是一年的第几周

    //比较两个HistoryTraffic对象的日期，如果a的时间大于b，则返回true
    static bool DateGreater(const Date& a, const Date& b);

    //判断两个HistoryTraffic对象的日期是否相等
    static bool DateEqual(const Date& a, const Date& b);
};

//储存某一天的历史流量
struct HistoryTraffic : public Date
{
    //当天使用的流量（以KB为单位）
    unsigned __int64 up_kBytes{};
    unsigned __int64 down_kBytes{};
    bool mixed{ true };     //如果不区分上传和下载流量，则为true

    unsigned __int64 kBytes() const;
};

//历史流量统计中用于指示不同范围内的流量的颜色
#define TRAFFIC_COLOR_BLUE RGB(0, 183, 238)
#define TRAFFIC_COLOR_GREEN RGB(128, 194, 105)
#define TRAFFIC_COLOR_YELLOE RGB(255, 216, 58)
#define TRAFFIC_COLOR_RED RGB(255, 95, 74)
#define TRAFFIC_COLOR_DARK_RED RGB(166, 19, 0)

//网速单位
enum class SpeedUnit
{
    AUTO,       //自动
    KBPS,       //KB/s
    MBPS        //MB/s
};

//内存显示方式
enum class MemoryDisplay
{
    USAGE_PERCENTAGE,       //已使用百分比
    MEMORY_USED,            //内存已使用
    MEMORY_AVAILABLE        //内存可用
};

//硬件监控的项目
enum HardwareItem
{
    HI_CPU = 1 << 0,        //CPU
    HI_GPU = 1 << 1,        //显卡
    HI_HDD = 1 << 2,        //硬盘
    HI_MBD = 1 << 3         //主板
};

enum class Alignment
{
    LEFT,       //左对齐
    RIGHT,      //右对齐
    CENTER,     //居中
    SIDE        //两端对齐
};

enum ELayoutItemAttributesOwner
{
    LIAO_MAINWND,
    LIAO_TASKBAR,
    LIAO_TASKBAR_DEFAULT_STYLE_1,
    LIAO_TASKBAR_DEFAULT_STYLE_2,
    LIAO_TASKBAR_DEFAULT_STYLE_3,
    LIAO_TASKBAR_DEFAULT_STYLE_4
};

struct DrawStr
{
    CString label;
    CString value;
    CString GetStr() const
    {
        return label + value;
    }
};

//定义显示项可配置的属性个数
//当前版本先支持3个(标签、标签颜色、数值颜色)
#define LAYOUT_ITEM_ATTRIBUTE_NUM 3

//皮肤中每个显示项的布局信息。
struct LayoutItem
{
    //全局唯一。//插件dll也应该保证在添加与删除插件项后，剩余各项的id依然保持不变，即使在软件重启后也不变。但是，某些插件没有做到这点。
    wstring                 id;                     //唯一标识
//  wstring                 name;                   //用于显示而已，id才是唯一标识。内置项的name已经固定，调用函数随时可以获得，不用保存。插件项的name从插件项接口随时可以获得，不用保存。
    ////////////////////////////////////////////////////////////////////////////////////////
    //      只能从皮肤配置文件中定义，GUI无法配置。
    ////////////////////////////////////////////////////////////////////////////////////////
    int                     x{}, y{}, width{};              //X位置、Y位置、宽度                
//  int                     height{};                       //目前只能从Layout.height复制。
    Alignment               align{ Alignment::LEFT };       //对齐方式
    bool                    show { false };                 //是否显示
    ////////////////////////////////////////////////////////////////////////////////////////
    //      皮肤配置文件和GUI都可以配置
    ////////////////////////////////////////////////////////////////////////////////////////
    //标签、标签颜色、数值颜色
    DrawStr                 LabelValueStr;                  //存放项目的标签和数值      //皮肤配置文件和GUI都可以配置标签，但数值是实时变化的，两者都无法配置。这里把这两项存放一起。
    COLORREF                LabelColor{};
    COLORREF                ValueColor{};
/*
    //显示项数值的所有属性设置
    bool                    B_HideNetUnit;                          //隐藏网络单位(对于速度来说，是MB/s、KB/s)(对于统计来说，是GB、MB、KB)
    bool                    B_HidePercent;                          //隐藏百分号
    bool                    B_HideDegree;                           //隐藏温度度数
    bool                    B_SeparateValueUnitWithSpace{ true };   //数值和单位用空格分隔                                //以后改名为：数值和单位用空格分隔(不再只限于网速)
    bool                    speed_short_mode{ false };              //网速显示简洁模式（减少小数点的位数，单位不显示“B”）
    SpeedUnit               speed_unit;                             //网速的单位
    bool                    unit_byte{ true };                      //使用字节(B)而不是比特(b)为单位                        //以后某天要取消对bit的支持，因此到时将删除此项。
*/
};

//历史流量统计列表视图中显示模式
enum class HistoryTrafficViewType
{
    HV_DAY,         //日视图
    HV_WEEK,        //周视图
    HV_MONTH,       //月视图
    HV_QUARTER,     //季视图
    HV_YEAR         //年视图
};

//鼠标双击窗口的动作
enum class DoubleClickAction
{
    CONNECTION_INFO,        //连接详情
    HISTORY_TRAFFIC,        //历史流量统计
    SHOW_MORE_INFO,         //显示更多信息
    OPTIONS,                //选项设置
    TASK_MANAGER,           //任务管理器
    SEPCIFIC_APP,           //指定应用程序
    CHANGE_SKIN,            //更换皮肤
    NONE                    //不执行任何动作
};

//语言
enum class Language
{
    FOLLOWING_SYSTEM,       //跟随系统
    ENGLISH,                //英语
    SIMPLIFIED_CHINESE,     //简体中文
    TRADITIONAL_CHINESE     //繁体中文
};

//颜色模式
enum class ColorMode
{
    Default,                //默认颜色
    Light                   //浅色
};

//所有内置显示项目
enum EBuiltinDisplayItem
{
    TDI_UP              = 1 << 0,
    TDI_DOWN            = 1 << 1,
    TDI_CPU             = 1 << 2,
    TDI_MEMORY          = 1 << 3,
    TDI_GPU_USAGE       = 1 << 4,
    TDI_CPU_TEMP        = 1 << 5,
    TDI_GPU_TEMP        = 1 << 6,
    TDI_HDD_TEMP        = 1 << 7,
    TDI_MAIN_BOARD_TEMP = 1 << 8,
    TDI_HDD_USAGE       = 1 << 9,
    TDI_TOTAL_SPEED     = 1 << 10,
    TDI_CPU_FREQ        = 1 << 11
};

//所有内置显示项目的集合
const std::set<EBuiltinDisplayItem> gS_AllBuiltinDisplayItems
{
    TDI_UP, TDI_DOWN, TDI_CPU, TDI_MEMORY
#ifndef WITHOUT_TEMPERATURE
    , TDI_GPU_USAGE
    , TDI_CPU_TEMP, TDI_GPU_TEMP, TDI_HDD_TEMP, TDI_MAIN_BOARD_TEMP
    , TDI_HDD_USAGE,TDI_CPU_FREQ
#endif
    , TDI_TOTAL_SPEED
};

#define DEF_CH L'\"'        //写入和读取ini文件字符串时，在字符串前后添加的字符
#define NONE_STR L"@@@"     //用于指定一个无效字符串

//定义监控时间间隔有效的最大值和最小值
#define MONITOR_TIME_SPAN_MIN 200
#define MONITOR_TIME_SPAN_MAX 30000
