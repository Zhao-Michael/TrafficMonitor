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

void CSkinFile::LoadLayoutItemFromXmlNode(LayoutItem& layout_item, tinyxml2::XMLElement* ele)
{
    layout_item.x               =               theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "x")));
    layout_item.y               =               theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "y")));
    layout_item.width           =               theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "width")));
    layout_item.align           =   static_cast<Alignment>(atoi(CTinyXml2Helper::ElementAttribute(ele, "align")));
    layout_item.show            = CTinyXml2Helper::StringToBool(CTinyXml2Helper::ElementAttribute(ele, "show"));
//    layout_item.LabelValueStr.label.Format(_T("%s"),            CTinyXml2Helper::ElementAttribute(ele, "lable"));
//    layout_item.LabelColor      =                          atoi(CTinyXml2Helper::ElementAttribute(ele, "lable_color"));
//    layout_item.ValueColor      =                          atoi(CTinyXml2Helper::ElementAttribute(ele, "value_color"));
}

void CSkinFile::LoadLayoutFromXmlNode(CSkinFile::Layout& layout, tinyxml2::XMLElement* ele)
{
    layout.width    = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "width")));
    layout.height   = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(ele, "height")));
    CTinyXml2Helper::IterateChildNode(ele, [&](tinyxml2::XMLElement* ele_layout_item)
        {
            string layout_item_cfg_name = CTinyXml2Helper::ElementName(ele_layout_item);
            for (auto builtin_item : gS_AllBuiltinDisplayItems)
            {
                if (layout_item_cfg_name == CCommon::GetDisplayItemXmlNodeName(builtin_item))
                {
                    //如果是内置的"UP"等，就找到了。
                    //插件item不能map到内置的"UP"等，否则当内置项处理了。
                    LoadLayoutItemFromXmlNode(layout.M_LayoutItems[builtin_item], ele_layout_item);
//                  layout.M_LayoutItems[builtin_item].height       = layout.height;
                    layout.M_LayoutItems[builtin_item].id           = CCommon::StrToUnicode(layout_item_cfg_name.c_str(), true);
                    break;
                }
            }
            wstring plugin_id = CCommon::StrToUnicode(m_plugin_map[layout_item_cfg_name].c_str(), true);
            if (!plugin_id.empty())
            {
                for (const auto& iplugin_item : theApp.m_plugin_manager.GetAllIPluginItems())
                {
                    if (plugin_id == iplugin_item->GetItemId())
                    {
                        LoadLayoutItemFromXmlNode(layout.M_LayoutItems[iplugin_item], ele_layout_item);
//                      layout.M_LayoutItems[iplugin_item].height   = layout.height;
                        layout.M_LayoutItems[iplugin_item].id       = plugin_id;
                        break;
                    }
                }
            }
        });
}

void CSkinFile::DrawSkinText(CDrawCommon drawer, CRect rect, DrawStr draw_str, COLORREF label_color, COLORREF value_color, Alignment align)
{
    int whole_width = drawer.GetDC()->GetTextExtent(draw_str.GetStr()).cx;
    int label_width = drawer.GetDC()->GetTextExtent(draw_str.label).cx;
    if (label_width == 0)                   //只画数值
    {
        //绘制数值
        drawer.DrawWindowText(rect, draw_str.value, value_color, align);
        return;
    }

    if (align == Alignment::SIDE)               //标签和数值两端对齐
    {
        if (whole_width <= rect.Width())      //只有标签和数值总宽度小于矩形的宽度时才使用两端对齐
        {
            //绘制标签
            drawer.DrawWindowText(rect, draw_str.label, label_color, Alignment::LEFT);
            //绘制数值
            drawer.DrawWindowText(rect, draw_str.value, value_color, Alignment::RIGHT);
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
            drawer.DrawWindowText(rect_label, draw_str.label, label_color, align);
            //画数值
            drawer.DrawWindowText(rect_value, draw_str.value, value_color, align);
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
        drawer.DrawWindowText(rect_label, draw_str.label, label_color, align);
        //画数值
        drawer.DrawWindowText(rect_value, draw_str.value, value_color, align);
        
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
        drawer.DrawWindowText(rect_label, draw_str.label, label_color, align);
        //画数值
        drawer.DrawWindowText(rect_value, draw_str.value, value_color, align);
    }
    else;
}

void CSkinFile::LoadCfgAndBGImage(const wstring& file_path)
{
    CFilePathHelper file_path_helper{ file_path };
    wstring ext = file_path_helper.GetFileExtension();
    if (ext == L"ini")
        LoadFromIni(file_path);
    else
        LoadFromXml(file_path);

    if (m_font.m_hObject)
        m_font.DeleteObject();

    //创建字体对象
    m_skin_info.font_info.Create(m_font);

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

    tinyxml2::XMLDocument doc;
    if (CTinyXml2Helper::LoadXmlFile(doc, file_path.c_str()))
    {
        CTinyXml2Helper::IterateChildNode(doc.FirstChildElement(), [this](tinyxml2::XMLElement* child)
            {
                string ele_name = CTinyXml2Helper::ElementName(child);
                //读取皮肤信息
                if (ele_name == "skin")
                {
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* skin_item)
                        {
                            string skin_item_name = CTinyXml2Helper::ElementName(skin_item);
                            //文本颜色
                            if (skin_item_name == "text_color")
                            {
                                wstring str_text_color = CCommon::StrToUnicode(CTinyXml2Helper::ElementText(skin_item));
                                CCommon::LoadValueColorsFromColorStr(m_layout_manager.layout_l.M_LayoutItems, str_text_color);
                                CCommon::LoadValueColorsFromColorStr(m_layout_manager.layout_s.M_LayoutItems, str_text_color);
                            }
                            //指定每个项目的颜色
                            else if (skin_item_name == "specify_each_item_color")
                            {
                                m_skin_info.specify_each_item_color = CTinyXml2Helper::StringToBool(CTinyXml2Helper::ElementText(skin_item));
                            }
                            //皮肤作者
                            else if (skin_item_name == "skin_author")
                            {
                                m_skin_info.skin_author = CCommon::StrToUnicode(CTinyXml2Helper::ElementText(skin_item), true);
                            }
                            //字体
                            else if (skin_item_name == "font")
                            {
                                m_skin_info.font_info.name = CTinyXml2Helper::ElementAttribute(skin_item, "name");
                                m_skin_info.font_info.size = atoi(CTinyXml2Helper::ElementAttribute(skin_item, "size"));
                                int font_style = atoi(CTinyXml2Helper::ElementAttribute(skin_item, "style"));
                                m_skin_info.font_info.bold = CCommon::GetNumberBit(font_style, 0);
                                m_skin_info.font_info.italic = CCommon::GetNumberBit(font_style, 1);
                                m_skin_info.font_info.underline = CCommon::GetNumberBit(font_style, 2);
                                m_skin_info.font_info.strike_out = CCommon::GetNumberBit(font_style, 3);
                            }
                            else if (skin_item_name == "display_text")      //定义在这里说明是整个皮肤通用，与layout无关。
                            {
                                CTinyXml2Helper::IterateChildNode(skin_item, [this](tinyxml2::XMLElement* display_text_item)
                                    {
                                        string item_name = CTinyXml2Helper::ElementName(display_text_item);
                                        wstring item_text = CCommon::StrToUnicode(CTinyXml2Helper::ElementText(display_text_item), true);
                                        for (auto display_item : gS_AllBuiltinDisplayItems)     //只支持内置项标签，暂时不修改，因为以后标签将作为各监控项的属性来设置。
                                        {
                                            if (item_name == CCommon::GetDisplayItemXmlNodeName(display_item))
                                            {
                                                /////////目前只针对皮肤范围配置，所以都复制。
                                                m_layout_manager.layout_l.M_LayoutItems[display_item].LabelValueStr.label = item_text.c_str();
                                                m_layout_manager.layout_s.M_LayoutItems[display_item].LabelValueStr.label = item_text.c_str();
                                                break;
                                            }
                                        }
                                    }
                                                                );
                            }
                        });
                }
                //布局信息
                else if (ele_name == "layout")
                {
                    m_layout_manager.text_height = theApp.DPI(atoi(CTinyXml2Helper::ElementAttribute(child, "text_height")));
                    m_layout_manager.no_label = CTinyXml2Helper::StringToBool(CTinyXml2Helper::ElementAttribute(child, "no_label"));
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* ele_layout)
                        {
                            string str_layout = CTinyXml2Helper::ElementName(ele_layout);
                            if (str_layout == "layout_l")
                                LoadLayoutFromXmlNode(m_layout_manager.layout_l, ele_layout);
                            else if (str_layout == "layout_s")
                                LoadLayoutFromXmlNode(m_layout_manager.layout_s, ele_layout);
                        });
                }
                //预览图
                else if (ele_name == "preview")
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
                        });
                }
                //插件名称映射
                else if (ele_name == "plugin_map")
                {
                    CTinyXml2Helper::IterateChildNode(child, [this](tinyxml2::XMLElement* plugin_item)
                        {
                            string ele_name = CTinyXml2Helper::ElementName(plugin_item);
                            string ele_text = CTinyXml2Helper::ElementText(plugin_item);
                            m_plugin_map[ele_name] = ele_text;
                        });
                }
            });
    }
}

void CSkinFile::LoadFromIni(const wstring& file_path)
{
    m_skin_info         = SkinInfo();
    m_layout_manager    = LayoutManager();
    m_preview_info      = PreviewInfo();

    CSkinFile::Layout& rLayout_L = m_layout_manager.layout_l;
    CSkinFile::Layout& rLayout_S = m_layout_manager.layout_s;

    //获取皮肤信息
    CIniHelper ini(file_path);
    //获取皮肤作者
    m_skin_info.skin_author             = ini.GetString(_T("skin"),   _T("skin_author"),    _T("unknow"));
    //获取当前皮肤的字体
    FontInfo default_font{};
    ini.LoadFontData(L"skin", m_skin_info.font_info, default_font);
    //获取显示标签
    rLayout_L.M_LayoutItems[TDI_UP].LabelValueStr.label     = ini.GetString(_T("skin"), _T("up_string"),                NONE_STR).c_str();
    rLayout_L.M_LayoutItems[TDI_DOWN].LabelValueStr.label   = ini.GetString(_T("skin"), _T("down_string"),              NONE_STR).c_str();
    rLayout_L.M_LayoutItems[TDI_CPU].LabelValueStr.label    = ini.GetString(_T("skin"), _T("cpu_string"),               NONE_STR).c_str();
    rLayout_L.M_LayoutItems[TDI_CPU].LabelValueStr.label    = ini.GetString(_T("skin"), _T("memory_string"),            NONE_STR).c_str();
    /////////目前只针对皮肤范围配置，所以复制一份。
    rLayout_S.M_LayoutItems[TDI_UP].LabelValueStr.label     = ini.GetString(_T("skin"), _T("up_string"),                NONE_STR).c_str();
    rLayout_S.M_LayoutItems[TDI_DOWN].LabelValueStr.label   = ini.GetString(_T("skin"), _T("down_string"),              NONE_STR).c_str();
    rLayout_S.M_LayoutItems[TDI_CPU].LabelValueStr.label    = ini.GetString(_T("skin"), _T("cpu_string"),               NONE_STR).c_str();
    rLayout_S.M_LayoutItems[TDI_CPU].LabelValueStr.label    = ini.GetString(_T("skin"), _T("memory_string"),            NONE_STR).c_str();
    //获取当前皮肤的文字颜色
    m_skin_info.specify_each_item_color                     = ini.GetBool  (_T("skin"), _T("specify_each_item_color"),  false);
    wstring str_text_color                                  = ini.GetString(_T("skin"), _T("text_color"),               _T(""));
    CCommon::LoadValueColorsFromColorStr(rLayout_L.M_LayoutItems, str_text_color);
    CCommon::LoadValueColorsFromColorStr(rLayout_S.M_LayoutItems, str_text_color);

    //从ini文件读取皮肤布局，并根据DPI进行缩放
    m_layout_manager.text_height                            = theApp.DPI(ini.GetInt(_T("layout"), _T("text_height"),    20));
    m_layout_manager.no_label                                       = ini.GetBool(  _T("layout"), _T("no_text"),        false);

    rLayout_L.width                             =             theApp.DPI(ini.GetInt(_T("layout"), _T("width_l"),        220));
    rLayout_L.height                            =             theApp.DPI(ini.GetInt(_T("layout"), _T("height_l"),       43));
    rLayout_L.M_LayoutItems[TDI_UP].x           =             theApp.DPI(ini.GetInt(_T("layout"), _T("up_x_l"),         6));
    rLayout_L.M_LayoutItems[TDI_UP].y           =             theApp.DPI(ini.GetInt(_T("layout"), _T("up_y_l"),         2));
    rLayout_L.M_LayoutItems[TDI_UP].width       =             theApp.DPI(ini.GetInt(_T("layout"), _T("up_width_l"),     108));
    rLayout_L.M_LayoutItems[TDI_UP].align       = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("up_align_l"),     0));
    rLayout_L.M_LayoutItems[TDI_DOWN].x         =             theApp.DPI(ini.GetInt(_T("layout"), _T("down_x_l"),       114));
    rLayout_L.M_LayoutItems[TDI_DOWN].y         =             theApp.DPI(ini.GetInt(_T("layout"), _T("down_y_l"),       2));
    rLayout_L.M_LayoutItems[TDI_DOWN].width     =             theApp.DPI(ini.GetInt(_T("layout"), _T("down_width_l"),   110));
    rLayout_L.M_LayoutItems[TDI_DOWN].align     = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("down_align_l"),   0));
    rLayout_L.M_LayoutItems[TDI_CPU].x =                       theApp.DPI(ini.GetInt(_T("layout"), _T("cpu_x_l"),       6));
    rLayout_L.M_LayoutItems[TDI_CPU].y =                       theApp.DPI(ini.GetInt(_T("layout"), _T("cpu_y_l"),       21));
    rLayout_L.M_LayoutItems[TDI_CPU].width      =             theApp.DPI(ini.GetInt(_T("layout"), _T("cpu_width_l"),    108));
    rLayout_L.M_LayoutItems[TDI_CPU].align      = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("cpu_align_l"),    0));
    rLayout_L.M_LayoutItems[TDI_MEMORY].x       =             theApp.DPI(ini.GetInt(_T("layout"), _T("memory_x_l"),     114));
    rLayout_L.M_LayoutItems[TDI_MEMORY].y       =             theApp.DPI(ini.GetInt(_T("layout"), _T("memory_y_l"),     21));
    rLayout_L.M_LayoutItems[TDI_MEMORY].width   =             theApp.DPI(ini.GetInt(_T("layout"), _T("memory_width_l"), 110));
    rLayout_L.M_LayoutItems[TDI_MEMORY].align   = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("memory_align_l"), 0));
    rLayout_L.M_LayoutItems[TDI_UP].show        =                        ini.GetBool(_T("layout"), _T("show_up_l"),     true);
    rLayout_L.M_LayoutItems[TDI_DOWN].show      =                        ini.GetBool(_T("layout"), _T("show_down_l"),   true);
    rLayout_L.M_LayoutItems[TDI_CPU].show       =                        ini.GetBool(_T("layout"), _T("show_cpu_l"),    true);
    rLayout_L.M_LayoutItems[TDI_MEMORY].show    =                        ini.GetBool(_T("layout"), _T("show_memory_l"), true);

    rLayout_S.width                             =             theApp.DPI(ini.GetInt(_T("layout"), _T("width_s"), 220));
    rLayout_S.height                            =             theApp.DPI(ini.GetInt(_T("layout"), _T("height_s"), 28));
    rLayout_S.M_LayoutItems[TDI_UP].x           =             theApp.DPI(ini.GetInt(_T("layout"), _T("up_x_s"), 6));
    rLayout_S.M_LayoutItems[TDI_UP].y           =             theApp.DPI(ini.GetInt(_T("layout"), _T("up_y_s"), 4));
    rLayout_S.M_LayoutItems[TDI_UP].width       =             theApp.DPI(ini.GetInt(_T("layout"), _T("up_width_s"), 108));
    rLayout_S.M_LayoutItems[TDI_UP].align       = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("up_align_s"), 0));
    rLayout_S.M_LayoutItems[TDI_UP].show        =                       ini.GetBool(_T("layout"), _T("show_up_s"), true);
    rLayout_S.M_LayoutItems[TDI_DOWN].x         =             theApp.DPI(ini.GetInt(_T("layout"), _T("down_x_s"), 114));
    rLayout_S.M_LayoutItems[TDI_DOWN].y         =             theApp.DPI(ini.GetInt(_T("layout"), _T("down_y_s"), 4));
    rLayout_S.M_LayoutItems[TDI_DOWN].width     =             theApp.DPI(ini.GetInt(_T("layout"), _T("down_width_s"), 110));
    rLayout_S.M_LayoutItems[TDI_DOWN].align     = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("down_align_s"), 0));
    rLayout_S.M_LayoutItems[TDI_DOWN].show      =                       ini.GetBool(_T("layout"), _T("show_down_s"), true);
    rLayout_S.M_LayoutItems[TDI_CPU].x          =             theApp.DPI(ini.GetInt(_T("layout"), _T("cpu_x_s"), 0));
    rLayout_S.M_LayoutItems[TDI_CPU].y          =             theApp.DPI(ini.GetInt(_T("layout"), _T("cpu_y_s"), 0));
    rLayout_S.M_LayoutItems[TDI_CPU].width      =             theApp.DPI(ini.GetInt(_T("layout"), _T("cpu_width_s"), 0));
    rLayout_S.M_LayoutItems[TDI_CPU].align      = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("cpu_align_s"), 0));
    rLayout_S.M_LayoutItems[TDI_CPU].show       =                       ini.GetBool(_T("layout"), _T("show_cpu_s"), false);
    rLayout_S.M_LayoutItems[TDI_MEMORY].x       =             theApp.DPI(ini.GetInt(_T("layout"), _T("memory_x_s"), 0));
    rLayout_S.M_LayoutItems[TDI_MEMORY].y       =             theApp.DPI(ini.GetInt(_T("layout"), _T("memory_y_s"), 0));
    rLayout_S.M_LayoutItems[TDI_MEMORY].width   =             theApp.DPI(ini.GetInt(_T("layout"), _T("memory_width_s"), 0));
    rLayout_S.M_LayoutItems[TDI_MEMORY].align   = static_cast<Alignment>(ini.GetInt(_T("layout"), _T("memory_align_s"), 0));
    rLayout_S.M_LayoutItems[TDI_MEMORY].show    =                       ini.GetBool(_T("layout"), _T("show_memory_s"), false);

    //获取预览区配置
    m_preview_info.width                        =             theApp.DPI(ini.GetInt(_T("layout"), _T("preview_width"), 238));
    m_preview_info.height                       =             theApp.DPI(ini.GetInt(_T("layout"), _T("preview_height"), 105));
    m_preview_info.l_pos.x                      =             theApp.DPI(ini.GetInt(_T("layout"), _T("preview_x_l"), 0));
    m_preview_info.l_pos.y                      =             theApp.DPI(ini.GetInt(_T("layout"), _T("preview_y_l"), 47));
    m_preview_info.s_pos.x                      =             theApp.DPI(ini.GetInt(_T("layout"), _T("preview_x_s"), 0));
    m_preview_info.s_pos.y                      =             theApp.DPI(ini.GetInt(_T("layout"), _T("preview_y_s"), 0));
}

//只使用皮肤配置文件中的配置数据来显示预览画面(包括大小两个layout)
void CSkinFile::DrawPreview(CDC* pDC, CRect rect)
{
    CDrawCommon draw;
    draw.Create(pDC, nullptr);
    if (!m_skin_info.font_info.name.IsEmpty() && m_skin_info.font_info.size > 0)
        draw.SetFont(&m_font);
    else
        draw.SetFont(theApp.m_pMainWnd->GetFont());
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
    std::map<EBuiltinDisplayItem, DrawStr>  map_builtin_str;    //存放所有内置项目的显示标签和数值
    std::map<CommonDisplayItem, COLORREF>   label_colors{};     //存放所有项目的显示标签的颜色
    std::map<CommonDisplayItem, COLORREF>   value_colors{};     //存放所有项目的显示数值的颜色

    ////////////////////////////////////////////////////////////////////////////////////////
    //              (1)所有内置项目的显示标签数值
    ////////////////////////////////////////////////////////////////////////////////////////    
    for (auto iter = gS_AllBuiltinDisplayItems.begin(); iter != gS_AllBuiltinDisplayItems.end(); ++iter)
    {
        //wstring disp_text = m_skin_info.display_text.Get(*iter);
        //if (disp_text == NONE_STR)
        //    disp_text = rMainWndData.disp_str.Get(*iter);
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
        case TDI_CPU_TEMP: case TDI_GPU_TEMP: case TDI_HDD_TEMP: case TDI_MAIN_BOARD_TEMP:
            draw_str.value = _T("40 °C");
            break;
        case TDI_CPU_FREQ:
            draw_str.value = _T("1.0 GHz");
            break;
        default:
            draw_str.value = _T("99");
            break;
        }
        if (m_layout_manager.layout_l.M_LayoutItems[*iter].LabelValueStr.label == NONE_STR)
            m_layout_manager.layout_l.M_LayoutItems[*iter].LabelValueStr.label = rMainWndData.M_LayoutItems[*iter].LabelValueStr.label;
        if (m_layout_manager.layout_s.M_LayoutItems[*iter].LabelValueStr.label == NONE_STR)
            m_layout_manager.layout_s.M_LayoutItems[*iter].LabelValueStr.label = rMainWndData.M_LayoutItems[*iter].LabelValueStr.label;
        if (!m_layout_manager.no_label)
        {
            /////////皮肤的xml配置中目前只针对皮肤范围配置，layout_l和layout_s两个都复制了，所以使用哪个都行。
            draw_str.label = m_layout_manager.layout_l.M_LayoutItems[*iter].LabelValueStr.label;
        }
        map_builtin_str[*iter] = draw_str;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //              (2)所有显示项目的标签颜色和数值颜色
    ////////////////////////////////////////////////////////////////////////////////////////
    if (m_skin_info.specify_each_item_color)
    {
        for (const auto& item : rPluginManager.AllDisplayItemsWithPlugins())
        {
            /////////皮肤的xml配置中目前只针对皮肤范围配置，layout_l和layout_s两个都复制了，所以使用哪个都行。
            label_colors[item]  = m_layout_manager.layout_l.M_LayoutItems[item].ValueColor;      //目前皮肤配置文件中不能配置标签颜色
            value_colors[item]  = m_layout_manager.layout_l.M_LayoutItems[item].ValueColor;
        }
    }
    else
    {
        if (1)               //if (!m_skin_info.text_color.empty())
        {
            for (const auto& item : rPluginManager.AllDisplayItemsWithPlugins())
            {
                if (!m_layout_manager.layout_l.M_LayoutItems.empty())
                {
                    /////////皮肤的xml配置中目前只针对皮肤范围配置，layout_l和layout_s两个都复制了，所以使用哪个都行。
                    label_colors[item]  = m_layout_manager.layout_l.M_LayoutItems.begin()->second.ValueColor;
                    value_colors[item]  = m_layout_manager.layout_l.M_LayoutItems.begin()->second.ValueColor;
                }
            }
        }
        else                //目前xml或ini格式的皮肤配置文件里都设置了color，所以不会走这里。这里暂时不加代码，以后补充。
        {
        }
    }

    //绘制预览图文本
    auto drawPreviewText = [&](Layout& layout, const PreviewInfo::Pos& pos)
    {
        for (auto iter = map_builtin_str.begin(); iter != map_builtin_str.end(); ++iter)
        {
            if (layout.M_LayoutItems[iter->first].show)
            {
                //矩形区域
                CPoint point;
                point.SetPoint(layout.M_LayoutItems[iter->first].x, layout.M_LayoutItems[iter->first].y);
                point.Offset(pos.x, pos.y);
                CRect rect(point, CSize(layout.M_LayoutItems[iter->first].width, m_layout_manager.text_height));
                //标签和数值颜色
                COLORREF label_color    = label_colors[iter->first];
                COLORREF text_color     = value_colors[iter->first];
                DrawSkinText(draw, rect, iter->second, label_color, text_color, layout.M_LayoutItems[iter->first].align);
            }
        }

        //绘制插件项目
        for (const auto& iplugin_item : rPluginManager.GetAllIPluginItems())
        {
            LayoutItem layout_item = layout.GetItem(iplugin_item);
            if (layout_item.show)
            {
                //矩形区域
                CPoint point;
                point.SetPoint(layout_item.x, layout_item.y);
                point.Offset(pos.x, pos.y);
                CRect rect(point, CSize(layout_item.width, m_layout_manager.text_height));
                //标签和数值颜色
                COLORREF label_color    = label_colors[iplugin_item];
                COLORREF value_color    = value_colors[iplugin_item];
                if (!iplugin_item->IsCustomDraw())
                {
                    //绘制文本
                    DrawStr draw_str;
                    draw_str.label = iplugin_item->GetItemLableText();              //这里没有使用皮肤配置文件设置的标签，可能是因为当前的皮肤配置文件中都没有设置插件标签。   //以后还是要使用皮肤配置文件设置的标签，而不是系统缺省值。
                    draw_str.value = iplugin_item->GetItemValueSampleText();
                    DrawSkinText(draw, rect, draw_str, label_color, value_color, layout_item.align);
                }
                else           //插件项目自绘
                {
                    int brightness{ (GetRValue(value_color) + GetGValue(value_color) + GetBValue(value_color)) / 2 };
                    ITMPlugin* plugin = rPluginManager.GetITMPluginByIPlguinItem(iplugin_item);
                    if (plugin != nullptr && plugin->GetAPIVersion() >= 2)
                    {
                        plugin->OnExtenedInfo(ITMPlugin::EI_VALUE_TEXT_COLOR, std::to_wstring(value_color).c_str());
                        plugin->OnExtenedInfo(ITMPlugin::EI_DRAW_TASKBAR_WND, L"0");
                    }
                    draw.GetDC()->SetTextColor(value_color);
                    iplugin_item->DrawItem(draw.GetDC()->GetSafeHdc(), point.x, point.y, layout_item.width, m_layout_manager.text_height, brightness >= 128);
                }
            }
        }
    };

    //绘制小预览图文本
    drawPreviewText(m_layout_manager.layout_s, m_preview_info.s_pos);
    //绘制大预览图文本
    drawPreviewText(m_layout_manager.layout_l, m_preview_info.l_pos);
}

void CSkinFile::DrawInfo(CDC* pDC, bool show_more_info, CFont& font)
{
    //绘制背景图
    CImage& background_image{ show_more_info ? m_background_l : m_background_s };
    Layout& layoutInUse{ show_more_info ? m_layout_manager.layout_l : m_layout_manager.layout_s };

    CRect rect(CPoint(0, 0), CSize(layoutInUse.width, layoutInUse.height));
    CDrawDoubleBuffer draw_double_buffer(pDC, rect);
    CDrawCommon draw;
    draw.Create(draw_double_buffer.GetMemDC(), nullptr);
    draw.DrawBitmap(background_image, CPoint(0, 0), CSize(layoutInUse.width, layoutInUse.height));

    MainWndSettingData&                     rMainWndData     = theApp.m_main_wnd_data;
    CPluginManager&                         rPluginManager   = theApp.m_plugin_manager;
    std::map<EBuiltinDisplayItem, DrawStr>  map_builtin_str;    //存放所有内置项目的显示标签和数值
    std::map<CommonDisplayItem, COLORREF>   label_colors{};     //存放所有项目的显示标签的颜色
    std::map<CommonDisplayItem, COLORREF>   value_colors{};     //存放所有项目的显示数值的颜色

    ////////////////////////////////////////////////////////////////////////////////////////
    //              (1)获取所有内置项目的显示标签
    ////////////////////////////////////////////////////////////////////////////////////////
    if (!m_layout_manager.no_label)
    {
        map_builtin_str[TDI_UP].label               = rMainWndData.M_LayoutItems[TDI_UP].LabelValueStr.label;
        map_builtin_str[TDI_DOWN].label             = rMainWndData.M_LayoutItems[TDI_DOWN].LabelValueStr.label;
        map_builtin_str[TDI_CPU].label              = rMainWndData.M_LayoutItems[TDI_CPU].LabelValueStr.label;
        map_builtin_str[TDI_MEMORY].label           = rMainWndData.M_LayoutItems[TDI_MEMORY].LabelValueStr.label;
        map_builtin_str[TDI_GPU_USAGE].label        = rMainWndData.M_LayoutItems[TDI_GPU_USAGE].LabelValueStr.label;
        map_builtin_str[TDI_CPU_TEMP].label         = rMainWndData.M_LayoutItems[TDI_CPU_TEMP].LabelValueStr.label;
        map_builtin_str[TDI_GPU_TEMP].label         = rMainWndData.M_LayoutItems[TDI_GPU_TEMP].LabelValueStr.label;
        map_builtin_str[TDI_HDD_TEMP].label         = rMainWndData.M_LayoutItems[TDI_HDD_TEMP].LabelValueStr.label;
        map_builtin_str[TDI_MAIN_BOARD_TEMP].label  = rMainWndData.M_LayoutItems[TDI_MAIN_BOARD_TEMP].LabelValueStr.label;
        map_builtin_str[TDI_HDD_USAGE].label        = rMainWndData.M_LayoutItems[TDI_HDD_USAGE].LabelValueStr.label;
        map_builtin_str[TDI_TOTAL_SPEED].label      = rMainWndData.M_LayoutItems[TDI_TOTAL_SPEED].LabelValueStr.label;
        map_builtin_str[TDI_CPU_FREQ].label         = rMainWndData.M_LayoutItems[TDI_CPU_FREQ].LabelValueStr.label;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    //              (2)获取所有内置项目的显示数值
    ////////////////////////////////////////////////////////////////////////////////////////
    //上传/下载
    CString in_speed    = CCommon::DataSizeToString(theApp.m_in_speed,                      rMainWndData);
    CString out_speed   = CCommon::DataSizeToString(theApp.m_out_speed,                     rMainWndData);
    CString total_speed = CCommon::DataSizeToString(theApp.m_in_speed + theApp.m_out_speed, rMainWndData);
    if (!rMainWndData.hide_unit || rMainWndData.speed_unit == SpeedUnit::AUTO)
    {
        in_speed += _T("/s");
        out_speed += _T("/s");
        total_speed += _T("/s");
    }
    map_builtin_str[TDI_UP].value           = out_speed.GetString();
    map_builtin_str[TDI_DOWN].value         = in_speed.GetString();
    map_builtin_str[TDI_TOTAL_SPEED].value  = total_speed.GetString();
    if (rMainWndData.swap_up_down) //交换上传和下载位置
    {
        std::swap(map_builtin_str[TDI_UP], map_builtin_str[TDI_DOWN]);
    }
    //CPU/内存/显卡利用率
    map_builtin_str[TDI_CPU].value          = CCommon::UsageToString(theApp.m_cpu_usage, rMainWndData);
    map_builtin_str[TDI_CPU_FREQ].value     = CCommon::FreqToString (theApp.m_cpu_freq,  rMainWndData);
    CString str_memory_value;
    if (rMainWndData.memory_display == MemoryDisplay::MEMORY_USED)
        str_memory_value            = CCommon::DataSizeToString(static_cast<unsigned long long>(theApp.m_used_memory) * 1024, rMainWndData.separate_value_unit_with_space);
    else if (rMainWndData.memory_display == MemoryDisplay::MEMORY_AVAILABLE)
        str_memory_value            = CCommon::DataSizeToString((static_cast<unsigned long long>(theApp.m_total_memory) - static_cast<unsigned long long>(theApp.m_used_memory)) * 1024, 
                                                                    rMainWndData.separate_value_unit_with_space);
    else
        str_memory_value            = CCommon::UsageToString(theApp.m_memory_usage, rMainWndData);
    map_builtin_str[TDI_MEMORY].value       = str_memory_value;
    map_builtin_str[TDI_GPU_USAGE].value    = CCommon::UsageToString(theApp.m_gpu_usage, rMainWndData);
    map_builtin_str[TDI_HDD_USAGE].value    = CCommon::UsageToString(theApp.m_hdd_usage, rMainWndData);
    //4个温度
    auto getTemperatureStr = [&](EBuiltinDisplayItem display_item, float temperature)
    {
        map_builtin_str[display_item].value = CCommon::TemperatureToString(temperature, rMainWndData);
    };
    getTemperatureStr(TDI_CPU_TEMP,         theApp.m_cpu_temperature);
    getTemperatureStr(TDI_GPU_TEMP,         theApp.m_gpu_temperature);
    getTemperatureStr(TDI_HDD_TEMP,         theApp.m_hdd_temperature);
    getTemperatureStr(TDI_MAIN_BOARD_TEMP,  theApp.m_main_board_temperature);
    ////////////////////////////////////////////////////////////////////////////////////////
    //              (3)获取所有显示项目的标签颜色和数值颜色
    ////////////////////////////////////////////////////////////////////////////////////////
    if (rMainWndData.specify_each_item_color)
    {
        for (const auto& item : rPluginManager.AllDisplayItemsWithPlugins())
        {
            label_colors[item] = rMainWndData.M_LayoutItems[item].LabelColor;
            value_colors[item] = rMainWndData.M_LayoutItems[item].ValueColor;
        }
    }
    else if (!rMainWndData.M_LayoutItems.empty())
    {
        for (const auto& item : rPluginManager.AllDisplayItemsWithPlugins())
        {
            label_colors[item] = rMainWndData.M_LayoutItems.begin()->second.ValueColor;     //标签颜色保存在数值颜色里
            value_colors[item] = rMainWndData.M_LayoutItems.begin()->second.ValueColor;
        }
    }

    //设置字体。目前不支持每个显示项单独设置字体。
    draw.SetFont(&font);
    //绘制标签和数值
    int index{};
    for (auto iter = map_builtin_str.begin(); iter != map_builtin_str.end(); ++iter)
    {
        const auto& layout_item = layoutInUse.GetItem(iter->first);
        if (layout_item.show)
        {
            //矩形区域
            CRect rect(CPoint(layout_item.x, layout_item.y), CSize(layout_item.width, m_layout_manager.text_height));
            //标签和数值颜色
            COLORREF label_color = label_colors[iter->first];
            COLORREF value_color = value_colors[iter->first];
            //绘制文本
            DrawSkinText(draw, rect, iter->second, label_color, value_color, layout_item.align);
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
                CRect rect(CPoint(layout_item.x, layout_item.y), CSize(layout_item.width, m_layout_manager.text_height));
                //绘制标签和数值
                DrawStr draw_str;
                draw_str.label = rMainWndData.M_LayoutItems[iplugin_item].LabelValueStr.label;
                draw_str.value = iplugin_item->GetItemValueText();
                DrawSkinText(draw, rect, draw_str, label_color, value_color, layout_item.align);
            }
            else               //插件项目自绘
            {
                int brightness{ (GetRValue(value_color) + GetGValue(value_color) + GetBValue(value_color)) / 2 };
                ITMPlugin* plugin = rPluginManager.GetITMPluginByIPlguinItem(iplugin_item);
                if (plugin != nullptr && plugin->GetAPIVersion() >= 2)
                {
                    plugin->OnExtenedInfo(ITMPlugin::EI_VALUE_TEXT_COLOR, std::to_wstring(value_color).c_str());
                    plugin->OnExtenedInfo(ITMPlugin::EI_DRAW_TASKBAR_WND, L"0");
                }
                draw.GetDC()->SetTextColor(value_color);
                iplugin_item->DrawItem(draw.GetDC()->GetSafeHdc(), layout_item.x, layout_item.y, layout_item.width, m_layout_manager.text_height, brightness >= 128);
            }
        }
    }
}

