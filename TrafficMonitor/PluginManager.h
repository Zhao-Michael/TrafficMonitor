#pragma once
#include "PluginInterface.h"
#include <map>
#include "TaskbarItemOrderHelper.h"
#include <functional>

typedef ITMPlugin* (*pfTMPluginGetInstance)();

//用于加载和管理插件
class CPluginManager
{
public:
    //插件的状态
    enum class PluginState
    {
        PS_SUCCEED,             //载入成功
        PS_MUDULE_LOAD_FAILED,  //dll加载失败
        PS_FUNCTION_GET_FAILED, //插件函数获取失败
        PS_VERSION_NOT_SUPPORT, //插件版本不被支持
        PS_DISABLE              //已禁用
    };

    //插件信息
    struct PluginManageUnit
    {
        //V: vector; S: set; M: map; I: interface; P: pointer
        wstring                                             m_file_path;        //文件路径
        HMODULE                                             m_plugin_module{};  //dll module
        PluginState                                         state{};            //插件的状态
        DWORD                                               error_code{};       //错误代码（GetLastError的返回值）
        std::map<ITMPlugin::PluginInfoIndex, std::wstring>  M_properties;       //插件属性
        ITMPlugin*                                          plugin{};           //插件接口ITMPlugin指针
        std::vector<IPluginItem*>                           V_PI_PluginItems;   //插件接口ITMPlugin提供的所有显示项目

        //成员函数
        std::wstring Property(ITMPlugin::PluginInfoIndex) const;
    };

    CPluginManager();
    ~CPluginManager();
    void LoadPlugins();

    const std::vector<PluginManageUnit>&    GetAllPluginManageUnit() const;
    const std::vector<IPluginItem*>&        GetAllIPluginItems() const;
    ITMPlugin*                              GetITMPluginByIPlguinItem(IPluginItem* pItem);
    IPluginItem*                            GetIPluginItemById(const std::wstring& item_id);
    IPluginItem*                            GetIPluginItemByIndex(size_t index);
    int                                     GetIPlguinItemIndex(IPluginItem* item) const;
    int                                     GetIPluginIndex(ITMPlugin* plugin) const;

    //遍历所有插件
    //func: 参数为遍历到的ITMPlugin对象
    void EnumPlugin(std::function<void(ITMPlugin*)> func) const;

    //遍历所有插件项目
    //func: 参数为遍历到的IPluginItem对象
    void EnumPluginItem(std::function<void(IPluginItem*)> func) const;

    const std::set<CommonDisplayItem>& AllDisplayItemsWithPlugins();

    int GetItemWidth(IPluginItem* pItem, CDC* pDC);

private:
    static void ReplacePluginDrawTextFunction(HMODULE plgin_dll_module) noexcept;
    static void ReplaceMfcDrawTextFunction() noexcept;

private:
    //用于描述函数成员变量类型(class和struct不标识)
    //V: vector; S: set; M: map;                            //用于描述成员变量是V_S_M这三种类型之一
    //I: interface; E: enum; B: bool; R: refer; P: pointer; //用于描述成员变量本身类型或是V_S_M中的成员类型
    std::vector<PluginManageUnit>       V_PluginManageUnit;
    std::vector<IPluginItem*>           V_PI_PluginItems;                   //包含所有ITMPlugin的所有IPluginItem
    std::set<CommonDisplayItem>         S_all_display_items_with_plugins;   //包含插件在内的所有任务栏显示项目
    std::map<IPluginItem*, ITMPlugin*>  M_IPlguinItem_to_ITMPlugin;         //用于根据插件项目查找对应插件的map
};
