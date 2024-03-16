#pragma once
#include "CommonData.h"
#include "TinyXml2Helper.h"
#include "DrawCommon.h"

class CSkinFile
{
public:
    CSkinFile();
    ~CSkinFile();

    //皮肤信息
    struct SkinInfo
    {
        wstring                 skin_author;                //皮肤的作者
        FontInfo                font_info;                  //字体信息
        DispStrings             display_text;               //每一项的显示文本
        bool                    specify_each_item_color{};  //是否指定每个项目的颜色
        std::vector<COLORREF>   text_color;                 //文本颜色

        COLORREF TextColor(size_t i) const
        {
            if (i < text_color.size())
                return text_color[i];
            else if (!text_color.empty())
                return text_color.front();
            else
                return 0;
        }
    };

    //皮肤布局
    struct Layout
    {
        int                     width{}, height{};                  //宽度、高度

        std::map<CommonDisplayItem, LayoutItem> layout_items;       //每一项的布局信息
        LayoutItem GetItem(CommonDisplayItem display_item) const
        {
            auto iter = layout_items.find(display_item);
            if (iter != layout_items.end())
                return iter->second;
            return LayoutItem();
        }
    };

    //皮肤布局信息
    struct LayoutManager
    {
        int     text_height{};      //皮肤文本的高度
        bool    no_label{};         //是否不显示标签
        Layout  layout_l;           //“显示更多信息”时的布局
        Layout  layout_s;           //不“显示更多信息”时的布局
    };

    //皮肤预览图信息
    struct PreviewInfo
    {
        struct Pos
        {
            int x{};
            int y{};
        };
        int width{};    //预览图的宽度
        int height{};   //预览图的高度
        Pos l_pos;      //“显示更多信息”时的窗口在预览图中的位置
        Pos s_pos;      //不“显示更多信息”时的窗口在预览图中的位置
    };

    const CImage&           GetBackgroundL()    const { return m_background_l;      }
    const CImage&           GetBackgroundS()    const { return m_background_s;      }
    const SkinInfo&         GetSkinInfo()       const { return m_skin_info;         }
    const LayoutManager&    GetLayoutManager()  const { return m_layout_manager;    }
    const PreviewInfo&      GetPreviewInfo()    const { return m_preview_info;      }

    static string GetDisplayItemXmlNodeName(EBuiltinDisplayItem display_item);
    //从文件载入皮肤信息
    void LoadCfgAndBGImage(const wstring& file_path);

    //绘制预览图
    //pDC: 绘图的CDC
    //rect: 绘图区域
    void DrawPreview(CDC* pDC, CRect rect);
    //绘制主界面
    void DrawInfo(CDC* pDC, bool show_more_info, CFont& font);
private:
    void LoadFromXml(const wstring& file_path);     //从xml文件读取皮肤数据
    void LoadFromIni(const wstring& file_path);     //从ini文件读取皮肤数据（用于兼容旧版皮肤）

    //新增功能代码
    void CSkinFile::InitLayoutItemAttributes(LayoutItem&   layout_item);

    CSkinFile::Layout GetLayoutFromXmlNode(tinyxml2::XMLElement* ele);

    static void DrawSkinText(CDrawCommon drawer, DrawStr draw_str, CRect rect, COLORREF color, Alignment align);

private:
    SkinInfo        m_skin_info;
    LayoutManager   m_layout_manager;
    PreviewInfo     m_preview_info;
    std::map<std::string, std::string> m_plugin_map;  //插件名称与xml节点名称的映射关系。key是xml节点名称，value是插件ID

    CFont m_font;
    CImage m_background_s;
    CImage m_background_l;

};
