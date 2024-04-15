#include "stdafx.h"
#include "SkinFile.h"
#include "Common.h"
#include "FilePathHelper.h"
#include "TrafficMonitor.h"
#include "IniHelper.h"
#include "DrawCommon.h"


CSkinFile::CSkinFile()
{
}


CSkinFile::~CSkinFile()
{
}

/*
void CSkinFile::InitLayoutItemAttributes(LayoutItem&   layout_item)
{
}
*/

void CSkinFile::LoadLayoutItemFromXmlNode(CLayout& layout, LayoutItem& layout_item, tinyxml2::XMLElement* ele)
{
    const char* str = nullptr;
    str = CTinyXml2Helper::ElementAttribute(ele, "show");
    if (str[0] != '\0')
        layout_item.show = CTinyXml2Helper::StringToBool(str);
    else
        layout_item.show = true;
    if (!layout_item.show)
        return;
    layout_item.height          = layout.DrawRectHeight;        //整个Layout统一这个高度
    layout_item.x               =               theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "x")));
    layout_item.y               =               theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "y")));
    layout_item.width           =               theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "width")));
    layout_item.align           =   static_cast<Alignment>(atoi(CTinyXml2Helper::ElementAttribute(ele, "align")));
    if (layout.no_label)
        layout_item.Prefix      = _T("");                       //如果不显示前缀，则直接将所有前缀设置为空。
    else
        layout_item.Prefix      =         CCommon::StrToUnicode(CTinyXml2Helper::ElementAttribute(ele, "prefix"), true).c_str();
    str = CTinyXml2Helper::ElementAttribute(ele, "prefix_c");
    if (str[0] != '\0')
        layout_item.PrefixColor = CCommon::GetColorFromStr(CCommon::StrToUnicode(str).c_str());
    else
        layout_item.PrefixColor = layout.PrefixColor;
    str = CTinyXml2Helper::ElementAttribute(ele, "value_c");
    if (str[0] != '\0')
        layout_item.ValueColor  = CCommon::GetColorFromStr(CCommon::StrToUnicode(str).c_str());
    else
        layout_item.ValueColor  = layout.ValueColor;
}

void CSkinFile::LoadLayoutFromXmlNode(CLayout& layout, tinyxml2::XMLElement* ele)
{
    CFilePathHelper file_path_helper{ skin_file_path };
    layout.SkinName         = file_path_helper.GetFolderName();
    layout.name             = CCommon::StrToUnicode(CTinyXml2Helper::ElementName(ele), true);
    layout.width            = theApp.DPI(atoi(                               CTinyXml2Helper::ElementAttribute(ele, "width")));
    layout.height           = theApp.DPI(atoi(                               CTinyXml2Helper::ElementAttribute(ele, "height")));
    layout.DrawRectHeight   = theApp.DPI(atoi(                               CTinyXml2Helper::ElementAttribute(ele, "text_height")));
    layout.no_label         = CTinyXml2Helper::StringToBool(                 CTinyXml2Helper::ElementAttribute(ele, "no_label"));
    layout.PrefixColor      = CCommon::GetColorFromStr(CCommon::StrToUnicode(CTinyXml2Helper::ElementAttribute(ele, "prefix_c")).c_str());
    layout.ValueColor       = CCommon::GetColorFromStr(CCommon::StrToUnicode(CTinyXml2Helper::ElementAttribute(ele, "value_c")).c_str());
    CTinyXml2Helper::IterateChildNode(ele, [&](tinyxml2::XMLElement* ele_layout_item)
        {
            BOOL bFind = false;
            string layout_item_cfg_name = CTinyXml2Helper::ElementName(ele_layout_item);
            if (layout_item_cfg_name == "font")                          //字体
            {
                layout.font_info.name = CTinyXml2Helper::ElementAttribute(ele_layout_item, "name");
                layout.font_info.size = atoi(CTinyXml2Helper::ElementAttribute(ele_layout_item, "size"));
                int font_style = atoi(CTinyXml2Helper::ElementAttribute(ele_layout_item, "style"));
                layout.font_info.bold = CCommon::GetNumberBit(font_style, 0);
                layout.font_info.italic = CCommon::GetNumberBit(font_style, 1);
                layout.font_info.underline = CCommon::GetNumberBit(font_style, 2);
                layout.font_info.strike_out = CCommon::GetNumberBit(font_style, 3);
                bFind = true;
            }
            if (bFind == false)
            {
                for (auto builtin_item : gS_AllBuiltinDisplayItems)
                {
                    if (layout_item_cfg_name == CCommon::GetDisplayItemXmlNodeName(builtin_item))
                    {
                        //如果是内置的"UP"等，就找到了。
                        //插件item不能map到内置的"UP"等，否则当内置项处理了。
                        LoadLayoutItemFromXmlNode(layout, layout.M_LayoutItems[builtin_item], ele_layout_item);
                        layout.M_LayoutItems[builtin_item].id = CCommon::StrToUnicode(layout_item_cfg_name.c_str(), true);
                        bFind = true;
                        break;
                    }
                }
            }
            if (bFind == false)
            {
                wstring plugin_id = CCommon::StrToUnicode(m_plugin_map[layout_item_cfg_name].c_str(), true);
                if (!plugin_id.empty())
                {
                    for (const auto& iplugin_item : theApp.m_plugin_manager.GetAllIPluginItems())
                    {
                        if (plugin_id == iplugin_item->GetItemId())
                        {
                            LoadLayoutItemFromXmlNode(layout, layout.M_LayoutItems[iplugin_item], ele_layout_item);
                            layout.M_LayoutItems[iplugin_item].id = plugin_id;
                            bFind = true;
                            break;
                        }
                    }
                }
            }
        }
    );

    //初始化在皮肤配置文件中没有设置的监控项
    for (const auto& item : theApp.m_plugin_manager.AllDisplayItemsWithPlugins())
    {
        LayoutItem& layout_item = layout.GetItem(item);
        if (layout_item.show)
            continue;
        layout_item.PrefixColor = layout.PrefixColor;
        layout_item.ValueColor  = layout.ValueColor;
    }
}

void CSkinFile::DrawSkinText(CDrawCommon drawer, CRect rect, CString label, CString value, COLORREF label_color, COLORREF value_color, Alignment align)
{
    int whole_width = drawer.GetDC()->GetTextExtent(label + value).cx;
    int label_width = drawer.GetDC()->GetTextExtent(label).cx;
    if (label_width == 0)                   //只画数值
    {
        //绘制数值
        drawer.DrawWindowText(rect, value, value_color, align);
        return;
    }

    if (align == Alignment::SIDE)               //标签和数值两端对齐
    {
        if (whole_width <= rect.Width())      //只有标签和数值总宽度小于矩形的宽度时才使用两端对齐
        {
            //绘制标签
            drawer.DrawWindowText(rect, label, label_color, Alignment::LEFT);
            //绘制数值
            drawer.DrawWindowText(rect, value, value_color, Alignment::RIGHT);
            return;
        }
        else                                //否则以数值为优先，当右对齐处理。
        {
            align = Alignment::RIGHT;
        }
    }

    if (align == Alignment::CENTER)             //标签和数值居中
    {
        if (whole_width <= rect.Width())      //只有标签和数值总宽度小于矩形的宽度时才使用居中对齐
        {
            //计算标签和数值的rect
            CRect rect_label, rect_value;
            rect_label = rect_value = rect;
            rect_label.left     += (rect.Width() - whole_width) / 2;
            rect_label.right    = rect_label.left + label_width;
            rect_value.left     = rect_label.right;
            rect_value.right    -= (rect.Width() - whole_width) / 2;
            //画标签
            drawer.DrawWindowText(rect_label, label, label_color, align);
            //画数值
            drawer.DrawWindowText(rect_value, value, value_color, align);
            return;
        }
        else                                //否则以数值为优先，当右对齐处理。
        {
            align = Alignment::RIGHT;
        }
    }

    if (align == Alignment::LEFT)              //标签和数值左对齐
    {
        //计算标签和数值的rect
        CRect rect_label, rect_value;
        rect_label = rect_value = rect;
        rect_label.right = rect_label.left + label_width;
        rect_value.left = rect_label.right;
        //画标签
        drawer.DrawWindowText(rect_label, label, label_color, align);
        //画数值
        drawer.DrawWindowText(rect_value, value, value_color, align);
        
    }
    else if(align == Alignment::RIGHT)              //标签和数值右对齐
    {
        //计算标签和数值的rect
        CRect rect_label, rect_value;
        rect_label = rect_value = rect;
        rect_value.left = rect_value.right - (whole_width - label_width);
        rect_label.right = rect_value.left;
        rect_label.left = rect_label.right - label_width;
        //画标签
        drawer.DrawWindowText(rect_label, label, label_color, align);
        //画数值
        drawer.DrawWindowText(rect_value, value, value_color, align);
    }
    else;
}

void CSkinFile::LoadCfgAndBGImage(const wstring& file_path)
{
    CFilePathHelper file_path_helper{ file_path };
    LoadFromXml(file_path);

    //因为每个layout可以单独设置字体，所以这里不再创建用于皮肤预览的字体对象，在函数DrawPreview()里再创建。
/*
    //创建字体对象
    if (m_font.m_hObject)
        m_font.DeleteObject();
    m_skin_info.font_info.Create(m_font);
*/
    //载入背景图片
    wstring path_dir = file_path_helper.GetDir();
    m_background_s.Destroy();
    m_background_s.Load((path_dir + BACKGROUND_IMAGE_S).c_str());
    m_background_l.Destroy();
    m_background_l.Load((path_dir + BACKGROUND_IMAGE_L).c_str());
}

void CSkinFile::LoadFromXml(const wstring& file_path)
{
    m_skin_info         = SkinInfo();
    m_layout_manager    = LayoutManager();
    m_preview_info      = PreviewInfo();
    skin_file_path      = file_path;

    tinyxml2::XMLDocument doc;
    if (CTinyXml2Helper::LoadXmlFile(doc, file_path.c_str()))
    {
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //      皮肤配置文件中各个section的加载是有先后顺序要求的。例如，如果<plugin_map>是最后加载的，则使用了影射的插件项将被忽略。
        //      加载的先后顺序是：<plugin_map>、<skin>、其它。
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        CTinyXml2Helper::IterateChildNode(doc.FirstChildElement(), [this](tinyxml2::XMLElement* child)
            {
                string ele_name = CTinyXml2Helper::ElementName(child);
                if (ele_name == "plugin_map")           //插件名称映射
                {
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* plugin_item)
                        {
                            string ele_name = CTinyXml2Helper::ElementName(plugin_item);
                            string ele_text = CTinyXml2Helper::ElementText(plugin_item);
                            m_plugin_map[ele_name] = ele_text;
                        }
                    );
                }
            }
        );
        CTinyXml2Helper::IterateChildNode(doc.FirstChildElement(), [this](tinyxml2::XMLElement* child)
            {
                string ele_name = CTinyXml2Helper::ElementName(child);
                if (ele_name == "skin")             //读取皮肤信息
                {
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* ele_skin)
                        {
                            string skin_item_name = CTinyXml2Helper::ElementName(ele_skin);
                            if (skin_item_name == "skin_author")                   //皮肤作者
                            {
                                m_skin_info.skin_author = CCommon::StrToUnicode(CTinyXml2Helper::ElementText(ele_skin), true);
                            }
                        }
                    );
                }
            }
        );
        CTinyXml2Helper::IterateChildNode(doc.FirstChildElement(), [this](tinyxml2::XMLElement* child)
            {
                string ele_name = CTinyXml2Helper::ElementName(child);
                if (ele_name == "layout")              //布局信息
                {
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* ele_layout)
                        {
                            string str_layout = CTinyXml2Helper::ElementName(ele_layout);
                            if (str_layout == "layout_l")
                                LoadLayoutFromXmlNode(m_layout_manager.layout_l, ele_layout);
                            else if (str_layout == "layout_s")
                                LoadLayoutFromXmlNode(m_layout_manager.layout_s, ele_layout);
                        }
                    );
                }
                else if (ele_name == "preview")             //预览图
                {
                    m_preview_info.width = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(child, "width")));
                    m_preview_info.height = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(child, "height")));
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* ele_priview_item)
                        {
                            string str_item_name = CTinyXml2Helper::ElementName(ele_priview_item);
                            if (str_item_name == "l")
                            {
                                m_preview_info.l_pos.x = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele_priview_item, "x")));
                                m_preview_info.l_pos.y = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele_priview_item, "y")));
                            }
                            else if (str_item_name == "s")
                            {
                                m_preview_info.s_pos.x = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele_priview_item, "x")));
                                m_preview_info.s_pos.y = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele_priview_item, "y")));
                            }
                        }
                    );
                }
            }
        );
    }
}

//只使用皮肤配置文件中的配置数据来显示预览画面(包括大小两个layout)
void CSkinFile::DrawPreview(CDC* pDC, CRect rect)
{
    CDrawCommon draw;
    draw.Create(pDC, nullptr);
    draw.SetDrawRect(rect);
    draw.FillRect(rect, RGB(255, 255, 255));
    //绘制背景
    CRect rect_s(CPoint(m_preview_info.s_pos.x, m_preview_info.s_pos.y), CSize(m_layout_manager.layout_s.width, m_layout_manager.layout_s.height));
    CRect rect_l(CPoint(m_preview_info.l_pos.x, m_preview_info.l_pos.y), CSize(m_layout_manager.layout_l.width, m_layout_manager.layout_l.height));
    if (m_background_s.IsNull())
        draw.FillRect(rect_s, RGB(230, 230, 230));
    else
        draw.DrawBitmap(m_background_s, rect_s.TopLeft(), rect_s.Size());
    if (m_background_l.IsNull())
        draw.FillRect(rect_l, RGB(230, 230, 230));
    draw.DrawBitmap(m_background_l, rect_l.TopLeft(), rect_l.Size());

    MainWndSettingData&                     rMainWndData    = theApp.m_main_wnd_data;
    CPluginManager&                         rPluginManager  = theApp.m_plugin_manager;

    ////////////////////////////////////////////////////////////////////////////////////////
    //              (1)所有内置项目的显示标签数值
    ////////////////////////////////////////////////////////////////////////////////////////    
    for (auto iter = gS_AllBuiltinDisplayItems.begin(); iter != gS_AllBuiltinDisplayItems.end(); ++iter)
    {
        DrawStr draw_str;
        switch (*iter)
        {
        case TDI_UP:
            draw_str.value = _T("88.8 KB/s");
            break;
        case TDI_DOWN:
            draw_str.value = _T("88.9 KB/s");
            break;
        case TDI_TOTAL_SPEED:
            draw_str.value = _T("90 KB/s");
            break;
        case TDI_CPU:
            draw_str.value = _T("50 %");
            break;
        case TDI_MEMORY:
            draw_str.value = _T("51 %");
            break;
        case TDI_GPU_USAGE:
            draw_str.value = _T("52 %");
            break;
        case TDI_CPU_TEMP: case TDI_GPU_TEMP: case TDI_HDD_TEMP: case TDI_MAIN_BOARD_TEMP:
            draw_str.value = _T("40 °C");
            break;
        case TDI_HDD_USAGE:
            draw_str.value = _T("54 %");
            break;
        case TDI_CPU_FREQ:
            draw_str.value = _T("1.0 GHz");
            break;
        default:
            draw_str.value = _T("99");
            break;
        }
        //使用皮肤数据存储结构中不使用的空间来保存皮肤预览数值数据
        m_layout_manager.layout_l.M_LayoutItems[*iter].Value = draw_str.value;
        m_layout_manager.layout_s.M_LayoutItems[*iter].Value = draw_str.value;
    }

    //绘制预览图文本
    auto drawPreviewText = [&](CLayout& layout, const PreviewInfo::Pos& pos)
    {
        for (const auto& item : theApp.m_plugin_manager.AllDisplayItemsWithPlugins())
        {
            if (!layout.font_info.name.IsEmpty() && layout.font_info.size > 0)
            {
                //创建字体对象
                if (m_font.m_hObject)
                    m_font.DeleteObject();
                layout.font_info.Create(m_font);
                draw.SetFont(&m_font);
            }
            else
                draw.SetFont(theApp.m_pMainWnd->GetFont());

            LayoutItem layout_item = layout.GetItem(item);
            if (!layout_item.show)
                continue;
            //矩形区域
            CPoint point;
            point.SetPoint(layout_item.x, layout_item.y);
            point.Offset(pos.x, pos.y);
            CRect rect(point, CSize(layout_item.width, layout_item.height));
            if (item.is_plugin && item.plugin_item->IsCustomDraw())         //插件项目自绘
            {
                COLORREF value_color = layout.M_LayoutItems[item].ValueColor;
                ITMPlugin* plugin = rPluginManager.GetITMPluginByIPlguinItem(item.plugin_item);
                if (plugin != nullptr && plugin->GetAPIVersion() >= 2)
                {
                    plugin->OnExtenedInfo(ITMPlugin::EI_VALUE_TEXT_COLOR, std::to_wstring(value_color).c_str());
                    plugin->OnExtenedInfo(ITMPlugin::EI_DRAW_TASKBAR_WND, L"0");
                }
                draw.GetDC()->SetTextColor(value_color);
                int brightness{ (GetRValue(value_color) + GetGValue(value_color) + GetBValue(value_color)) / 2 };
                item.plugin_item->DrawItem(draw.GetDC()->GetSafeHdc(), point.x, point.y, layout_item.width, layout_item.height, brightness >= 128);
            }
            else
            {
                if (item.is_plugin)
                    layout_item.Value = item.plugin_item->GetItemValueSampleText();
                DrawSkinText(draw, rect, layout_item.Prefix, layout_item.Value, layout_item.PrefixColor, layout_item.ValueColor, layout_item.align);
            }
        }
    };

    //绘制小预览图文本
    drawPreviewText(m_layout_manager.layout_s, m_preview_info.s_pos);
    //绘制大预览图文本
    drawPreviewText(m_layout_manager.layout_l, m_preview_info.l_pos);
}

void CSkinFile::DrawInfo(CDC* pDC, CFont& font)
{
    MainWndSettingData&                         rMainWndData            = theApp.m_main_wnd_data;
    std::map<CommonDisplayItem, LayoutItem>&    rMainWnd_M_LayoutItems  = rMainWndData.layout.M_LayoutItems;
    LayoutItemValueAttributes&                  rMainWnd_LIVA           = rMainWndData.layout_item_value_attributes;
    CPluginManager&                             rPluginManager          = theApp.m_plugin_manager;
    std::map<EBuiltinDisplayItem, DrawStr>  map_builtin_str;    //存放所有内置项目的显示标签和数值
    std::map<CommonDisplayItem, COLORREF>   label_colors{};     //存放所有项目的显示标签的颜色
    std::map<CommonDisplayItem, COLORREF>   value_colors{};     //存放所有项目的显示数值的颜色
    bool show_more_info = rMainWndData.m_show_more_info;

    //绘制背景图
    CImage& background_image{ show_more_info ? m_background_l : m_background_s };
    CLayout& layoutInUse{ show_more_info ? m_layout_manager.layout_l : m_layout_manager.layout_s };

    CRect rect(CPoint(0, 0), CSize(layoutInUse.width, layoutInUse.height));
    CDrawDoubleBuffer draw_double_buffer(pDC, rect);
    CDrawCommon draw;
    draw.Create(draw_double_buffer.GetMemDC(), nullptr);
    draw.DrawBitmap(background_image, CPoint(0, 0), CSize(layoutInUse.width, layoutInUse.height));


    ////////////////////////////////////////////////////////////////////////////////////////
    //              (1)获取所有内置项目的显示标签
    ////////////////////////////////////////////////////////////////////////////////////////
    if (!layoutInUse.no_label)
    {
        map_builtin_str[TDI_UP].label               = rMainWnd_M_LayoutItems[TDI_UP].Prefix;
        map_builtin_str[TDI_DOWN].label             = rMainWnd_M_LayoutItems[TDI_DOWN].Prefix;
        map_builtin_str[TDI_CPU].label              = rMainWnd_M_LayoutItems[TDI_CPU].Prefix;
        map_builtin_str[TDI_MEMORY].label           = rMainWnd_M_LayoutItems[TDI_MEMORY].Prefix;
        map_builtin_str[TDI_GPU_USAGE].label        = rMainWnd_M_LayoutItems[TDI_GPU_USAGE].Prefix;
        map_builtin_str[TDI_CPU_TEMP].label         = rMainWnd_M_LayoutItems[TDI_CPU_TEMP].Prefix;
        map_builtin_str[TDI_GPU_TEMP].label         = rMainWnd_M_LayoutItems[TDI_GPU_TEMP].Prefix;
        map_builtin_str[TDI_HDD_TEMP].label         = rMainWnd_M_LayoutItems[TDI_HDD_TEMP].Prefix;
        map_builtin_str[TDI_MAIN_BOARD_TEMP].label  = rMainWnd_M_LayoutItems[TDI_MAIN_BOARD_TEMP].Prefix;
        map_builtin_str[TDI_HDD_USAGE].label        = rMainWnd_M_LayoutItems[TDI_HDD_USAGE].Prefix;
        map_builtin_str[TDI_TOTAL_SPEED].label      = rMainWnd_M_LayoutItems[TDI_TOTAL_SPEED].Prefix;
        map_builtin_str[TDI_CPU_FREQ].label         = rMainWnd_M_LayoutItems[TDI_CPU_FREQ].Prefix;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //              (2)获取所有内置项目的显示数值
    ////////////////////////////////////////////////////////////////////////////////////////
    //上传/下载
    CString in_speed    = CCommon::DataSizeToString(theApp.m_in_speed,                      rMainWnd_LIVA);
    CString out_speed   = CCommon::DataSizeToString(theApp.m_out_speed,                     rMainWnd_LIVA);
    CString total_speed = CCommon::DataSizeToString(theApp.m_in_speed + theApp.m_out_speed, rMainWnd_LIVA);
    if (!rMainWnd_LIVA.hide_unit || rMainWnd_LIVA.speed_unit == SpeedUnit::AUTO)
    {
        in_speed += _T("/s");
        out_speed += _T("/s");
        total_speed += _T("/s");
    }
    map_builtin_str[TDI_UP].value           = out_speed.GetString();
    map_builtin_str[TDI_DOWN].value         = in_speed.GetString();
    map_builtin_str[TDI_TOTAL_SPEED].value  = total_speed.GetString();
    //CPU/内存/显卡利用率
    map_builtin_str[TDI_CPU].value          = CCommon::UsageToString(theApp.m_cpu_usage, rMainWnd_LIVA);
    map_builtin_str[TDI_CPU_FREQ].value     = CCommon::FreqToString (theApp.m_cpu_freq, rMainWnd_LIVA);
    CString str_memory_value;
    if (rMainWndData.memory_display == MemoryDisplay::MEMORY_USED)
        str_memory_value            = CCommon::DataSizeToString(static_cast<unsigned long long>(theApp.m_used_memory) * 1024, rMainWnd_LIVA.separate_value_unit_with_space);
    else if (rMainWndData.memory_display == MemoryDisplay::MEMORY_AVAILABLE)
        str_memory_value            = CCommon::DataSizeToString((static_cast<unsigned long long>(theApp.m_total_memory) - static_cast<unsigned long long>(theApp.m_used_memory)) * 1024, 
            rMainWnd_LIVA.separate_value_unit_with_space);
    else
        str_memory_value            = CCommon::UsageToString(theApp.m_memory_usage, rMainWnd_LIVA);
    map_builtin_str[TDI_MEMORY].value       = str_memory_value;
    map_builtin_str[TDI_GPU_USAGE].value    = CCommon::UsageToString(theApp.m_gpu_usage, rMainWnd_LIVA);
    map_builtin_str[TDI_HDD_USAGE].value    = CCommon::UsageToString(theApp.m_hdd_usage, rMainWnd_LIVA);
    //4个温度
    auto getTemperatureStr = [&](EBuiltinDisplayItem display_item, float temperature)
    {
        map_builtin_str[display_item].value = CCommon::TemperatureToString(temperature, rMainWnd_LIVA);
    };
    getTemperatureStr(TDI_CPU_TEMP,         theApp.m_cpu_temperature);
    getTemperatureStr(TDI_GPU_TEMP,         theApp.m_gpu_temperature);
    getTemperatureStr(TDI_HDD_TEMP,         theApp.m_hdd_temperature);
    getTemperatureStr(TDI_MAIN_BOARD_TEMP,  theApp.m_main_board_temperature);
    ////////////////////////////////////////////////////////////////////////////////////////
    //              (3)获取所有显示项目的标签颜色和数值颜色
    ////////////////////////////////////////////////////////////////////////////////////////
    for (const auto& item : rPluginManager.AllDisplayItemsWithPlugins())
    {
        label_colors[item] = rMainWnd_M_LayoutItems[item].PrefixColor;
        value_colors[item] = rMainWnd_M_LayoutItems[item].ValueColor;
    }

    //设置字体。目前不支持每个layout单独设置字体。
    draw.SetFont(&font);
    //绘制标签和数值
    int index{};
    for (auto iter = map_builtin_str.begin(); iter != map_builtin_str.end(); ++iter)
    {
        const auto& layout_item = layoutInUse.GetItem(iter->first);
        if (layout_item.show)
        {
            //矩形区域
            CRect rect(CPoint(layout_item.x, layout_item.y), CSize(layout_item.width, layout_item.height));
            //标签和数值颜色
            COLORREF label_color = label_colors[iter->first];
            COLORREF value_color = value_colors[iter->first];
            //绘制文本
            DrawSkinText(draw, rect, iter->second.label, iter->second.value, label_color, value_color, layout_item.align);
        }
        index++;
    }

    //绘制插件项目
    for (const auto& iplugin_item : rPluginManager.GetAllIPluginItems())
    {
        const auto& layout_item = layoutInUse.GetItem(iplugin_item);
        if (layout_item.show)
        {
            //标签和数值颜色
            COLORREF label_color = label_colors[iplugin_item];
            COLORREF value_color = value_colors[iplugin_item];
            if (!iplugin_item->IsCustomDraw())
            {
                //矩形区域
                CPoint point;
                point.SetPoint(layout_item.x, layout_item.y);
//              point.Offset(pos.x, pos.y);
                CRect rect(point, CSize(layout_item.width, layout_item.height));
                //绘制标签和数值
                DrawStr draw_str;
                draw_str.label = rMainWnd_M_LayoutItems[iplugin_item].Prefix;
                draw_str.value = iplugin_item->GetItemValueText();
                DrawSkinText(draw, rect, draw_str.label, draw_str.value, label_color, value_color, layout_item.align);
            }
            else               //插件项目自绘
            {
                ITMPlugin* plugin = rPluginManager.GetITMPluginByIPlguinItem(iplugin_item);
                if (plugin != nullptr && plugin->GetAPIVersion() >= 2)
                {
                    plugin->OnExtenedInfo(ITMPlugin::EI_VALUE_TEXT_COLOR, std::to_wstring(value_color).c_str());
                    plugin->OnExtenedInfo(ITMPlugin::EI_DRAW_TASKBAR_WND, L"0");
                }
                draw.GetDC()->SetTextColor(value_color);
                int brightness{ (GetRValue(value_color) + GetGValue(value_color) + GetBValue(value_color)) / 2 };
                iplugin_item->DrawItem(draw.GetDC()->GetSafeHdc(), layout_item.x, layout_item.y, layout_item.width, layout_item.height, brightness >= 128);
            }
        }
    }
}

