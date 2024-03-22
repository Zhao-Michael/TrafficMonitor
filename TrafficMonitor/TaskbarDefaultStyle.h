#pragma once
#include "CommonData.h"

#define TASKBAR_DEFAULT_STYLE_NUM 4
#define TASKBAR_DEFAULT_LIGHT_STYLE_INDEX 3

class CTaskbarDefaultStyle
{
public:
	struct TaskBarStyleData
	{
		COLORREF back_color{};
		COLORREF transparent_color{};
		COLORREF status_bar_color{};
		bool specify_each_item_color{};
#ifdef	STORE_MONITOR_ITEM_DATA_IN_NEW_WAY
		//新增功能代码
		std::map<CommonDisplayItem, LayoutItem>     M_LayoutItems{};                          //存放所有项目的属性配置
#else
		DispStrings disp_str;                           //标签    //里面存放了所有标签的map
		std::map<CommonDisplayItem, TaskbarItemColor> text_colors{};
#endif
	};

	CTaskbarDefaultStyle();
	~CTaskbarDefaultStyle();

	void LoadConfig();
	void SaveConfig();// const;																			//使用map后不能用const

	void ApplyDefaultStyle(int index, TaskBarSettingData& data); //const;		//应用一个颜色预设		//使用map后不能用const
	//static void ApplyDefaultLightStyle(TaskBarSettingData& data);		//应用默认的浅色模式预设
	void ModifyDefaultStyle(int index, TaskBarSettingData& data);     //将当前颜色设置保存到一个预设方案

    static bool IsTaskBarStyleDataValid(const TaskBarStyleData& data);     //判断一个颜色模式是否有效

private:
	TaskBarStyleData m_default_style[TASKBAR_DEFAULT_STYLE_NUM];    //预设样式

};
