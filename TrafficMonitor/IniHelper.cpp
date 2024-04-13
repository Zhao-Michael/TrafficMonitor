#include "stdafx.h"
#include "IniHelper.h"
#include "TrafficMonitor.h"


CIniHelper::CIniHelper(const wstring& file_path)
{
    m_file_path = file_path;
    ifstream file_stream{ file_path };
    if (file_stream.fail())
    {
        return;
    }
    //读取文件内容
    string ini_str;
    while (!file_stream.eof())
    {
        ini_str.push_back(file_stream.get());
    }
    ini_str.pop_back();
    if (!ini_str.empty() && ini_str.back() != L'\n')        //确保文件末尾有回车符
        ini_str.push_back(L'\n');
    //判断文件是否是utf8编码
    bool is_utf8;
    if (ini_str.size() >= 3 && ini_str[0] == -17 && ini_str[1] == -69 && ini_str[2] == -65)
    {
        //如果有UTF8的BOM，则删除BOM
        is_utf8 = true;
        ini_str = ini_str.substr(3);
    }
    else
    {
        is_utf8 = false;
    }
    //转换成Unicode
    m_ini_str = CCommon::StrToUnicode(ini_str.c_str(), is_utf8);
}


CIniHelper::~CIniHelper()
{
}

void CIniHelper::SetSaveAsUTF8(bool utf8)
{
    m_save_as_utf8 = utf8;
}

void CIniHelper::RemoveSpecialChar(wstring& rtn)
{
    //如果读取的字符串前后有指定的字符，则删除它
    if (!rtn.empty() && (rtn.front() == L'$' || rtn.front() == DEF_CH))
        rtn = rtn.substr(1);
    if (!rtn.empty() && (rtn.back() == L'$' || rtn.back() == DEF_CH))
        rtn.pop_back();
}

BOOL CIniHelper::FindAppName(const wchar_t* AppName)
{
    wstring app_str{ L"[" };
    app_str.append(AppName).append(L"]");
    size_t app_pos{};
    app_pos = m_ini_str.find(app_str);
    if (app_pos == wstring::npos)       //找不到AppName，则在最后面添加
    {
        return false;
    }
    return true;
}

void CIniHelper::WriteString(const wchar_t * AppName, const wchar_t * KeyName, const wstring& str)
{
    wstring write_str{ str };
    if (!write_str.empty() && (write_str[0] == L' ' || write_str.back() == L' '))       //如果字符串前后含有空格，则在字符串前后添加引号
    {
        write_str = DEF_CH + write_str;
        write_str.push_back(DEF_CH);
    }
    _WriteString(AppName, KeyName, write_str);
}

wstring CIniHelper::GetString(const wchar_t * AppName, const wchar_t * KeyName, const wchar_t* default_str) const
{
    wstring rtn{_GetString(AppName, KeyName, default_str)};
//    return RemoveSpecialChar(rtn);
    //如果读取的字符串前后有指定的字符，则删除它
    if (!rtn.empty() && (rtn.front() == L'$' || rtn.front() == DEF_CH))
        rtn = rtn.substr(1);
    if (!rtn.empty() && (rtn.back() == L'$' || rtn.back() == DEF_CH))
        rtn.pop_back();
    return rtn;
}

void CIniHelper::WriteInt(const wchar_t * AppName, const wchar_t * KeyName, int value)
{
    _WriteString(AppName, KeyName, std::to_wstring(value));
}

void CIniHelper::WriteInt(const wchar_t* AppName, const wchar_t* KeyName, int value, int base)
{
    if (16 == base)
    {
        CString str;
        str.Format(_T("0x%x,"), value);
        _WriteString(AppName, KeyName, wstring(str));
    }
    else
        _WriteString(AppName, KeyName, std::to_wstring(value));
}

int CIniHelper::GetInt(const wchar_t * AppName, const wchar_t * KeyName, int default_value) const
{
    wstring rtn{ _GetString(AppName, KeyName, std::to_wstring(default_value).c_str()) };
    return _ttoi(rtn.c_str());
}

int CIniHelper::GetInt(const wchar_t* AppName, const wchar_t* KeyName, int default_value, int base) const
{
    wstring rtn{ _GetString(AppName, KeyName, std::to_wstring(default_value).c_str()) };
    if (16 == base)
    {
        const wchar_t* str = rtn.c_str();

        if (wcslen(str) >= 2 && '0' == str[0] && 'x' == str[1])
            return wcstol(str, nullptr, 16);
        else
            return _wtoi(str);
    }
    else
        return _ttoi(rtn.c_str());
}

void CIniHelper::WriteBool(const wchar_t * AppName, const wchar_t * KeyName, bool value)
{
    if(value)
        _WriteString(AppName, KeyName, wstring(L"true"));
    else
        _WriteString(AppName, KeyName, wstring(L"false"));
}

bool CIniHelper::GetBool(const wchar_t * AppName, const wchar_t * KeyName, bool default_value) const
{
    wstring rtn{ _GetString(AppName, KeyName, (default_value ? L"true" : L"false")) };
    if (rtn == L"true")
        return true;
    else if (rtn == L"false")
        return false;
    else
        return (_ttoi(rtn.c_str()) != 0);
}

void CIniHelper::WriteIntArray(const wchar_t * AppName, const wchar_t * KeyName, const int * values, int size)
{
    CString str, tmp;
    for (int i{}; i < size; i++)
    {
        tmp.Format(_T("%d,"), values[i]);
        str += tmp;
    }
    _WriteString(AppName, KeyName, wstring(str));
}

void CIniHelper::GetIntArray(const wchar_t * AppName, const wchar_t * KeyName, int * values, int size, int default_value) const
{
    CString default_str;
    default_str.Format(_T("%d"), default_value);
    wstring str;
    str = _GetString(AppName, KeyName, default_str);
    std::vector<wstring> split_result;
    CCommon::StringSplit(str, L',', split_result);
    for (int i = 0; i < size; i++)
    {
        if (i < split_result.size())
            values[i] = _wtoi(split_result[i].c_str());
        else if (i > 0)
            values[i] = values[i - 1];
        else
            values[i] = default_value;
    }
}

void CIniHelper::WriteBoolArray(const wchar_t * AppName, const wchar_t * KeyName, const bool * values, int size)
{
    int value{};
    for (int i{}; i < size; i++)
    {
        if (values[i])
            value |= (1 << i);
    }
    return WriteInt(AppName, KeyName, value);
}

void CIniHelper::GetBoolArray(const wchar_t * AppName, const wchar_t * KeyName, bool * values, int size, bool default_value) const
{
    int value = GetInt(AppName, KeyName, 0);
    for (int i{}; i < size; i++)
    {
        values[i] = ((value >> i) % 2 != 0);
    }
}

void CIniHelper::WriteStringList(const wchar_t* AppName, const wchar_t* KeyName, const vector<wstring>& values)
{
    wstring str_write = MergeStringList(values);
    _WriteString(AppName, KeyName, str_write);
}

void CIniHelper::GetStringList(const wchar_t* AppName, const wchar_t* KeyName, vector<wstring>& values, const vector<wstring>& default_value) const
{
    wstring default_str = MergeStringList(default_value);
    wstring str_value = _GetString(AppName, KeyName, default_str.c_str());
    SplitStringList(values, str_value);
}

bool CIniHelper::Save()
{
    ofstream file_stream{ m_file_path };
    if(file_stream.fail())
        return false;
    string ini_str{ CCommon::UnicodeToStr(m_ini_str.c_str(), m_save_as_utf8) };
    if (m_save_as_utf8)     //如果以UTF8编码保存，先插入BOM
    {
        string utf8_bom;
        utf8_bom.push_back(-17);
        utf8_bom.push_back(-69);
        utf8_bom.push_back(-65);
        file_stream << utf8_bom;
    }

    file_stream << ini_str;
    return true;
}

void CIniHelper::LoadFontData(const wchar_t * AppName, FontInfo & font, const FontInfo& default_font) const
{
    font.name = GetString(AppName, L"font_name", default_font.name).c_str();
    font.size = GetInt(AppName, L"font_size", default_font.size);
    bool style[4];
    GetBoolArray(AppName, L"font_style", style, 4);
    font.bold = style[0];
    font.italic = style[1];
    font.underline = style[2];
    font.strike_out = style[3];
}

void CIniHelper::SaveFontData(const wchar_t* AppName, const FontInfo& font)
{
    WriteString(AppName, L"font_name", wstring(font.name));
    WriteInt(AppName, L"font_size", font.size);
    bool style[4];
    style[0] = font.bold;
    style[1] = font.italic;
    style[2] = font.underline;
    style[3] = font.strike_out;
    WriteBoolArray(AppName, L"font_style", style, 4);
}

void CIniHelper::LoadLayoutItemAttributes(const wchar_t* AppName, const wchar_t* KeyName, std::map<CommonDisplayItem, LayoutItem>& M_layout_items,
                            EBuiltinDisplayItem item_type, IPluginItem* iplugin_item, const wchar_t* default_str, COLORREF default_color)
{
    LayoutItem* pLayoutItem = nullptr;
    if (nullptr == iplugin_item)
    {
        pLayoutItem         = &M_layout_items[item_type];
        pLayoutItem->id     = CCommon::StrToUnicode(CCommon::GetDisplayItemXmlNodeName(item_type).c_str());
    }
    else
    {
        pLayoutItem         = &M_layout_items[iplugin_item];
        pLayoutItem->id     = iplugin_item->GetItemId();
    }

    //每项的所有属性保存格式为： 标签，标签颜色，数值颜色
    wstring str = _GetString(AppName, KeyName, default_str);    //如果找不到AppName.KeyName =，则说明这项LayoutItem从来没自定义过，会返回default_str。
                                                                //如果找到了AppName.KeyName =，但后面都是空字符串，则返回空字符串。但这是人为修改引起的错误(因为软件写入时后面至少还有颜色数据)，我们兼容这种情况。
    std::vector<wstring>    ColorsStr_SplitResult;
    CCommon::StringSplit(str, L',', ColorsStr_SplitResult);
    size_t ColorsStr_num = ColorsStr_SplitResult.size();
    if (0 == ColorsStr_num)                                     //找到了AppName.KeyName =，但后面都是空字符串。我们兼容这种情况。
    {
        //标签值留空，颜色值使用default_color。
        pLayoutItem->PrefixColor = default_color;
        pLayoutItem->ValueColor = default_color;
        return;
    }
    //去除引号后保存标签
    RemoveSpecialChar(ColorsStr_SplitResult[0]);
    pLayoutItem->Prefix = ColorsStr_SplitResult[0].c_str();     //保存标签

    size_t index = 1;
    for (; index < LAYOUT_ITEM_ATTRIBUTE_NUM; index++)
    {
        const wchar_t* color_str = nullptr;
        if (index < ColorsStr_num)
        {
            if (index == 1)             //保存标签颜色
            {
                color_str = ColorsStr_SplitResult[index].c_str();
                //support Decimal data or Hex data from saved data
                if (wcslen(color_str) >= 2 && '0' == color_str[0] && 'x' == color_str[1])
                    pLayoutItem->PrefixColor = wcstol(color_str, nullptr, 16);
                else
                    pLayoutItem->PrefixColor = _wtoi(color_str);
            }
            else if (index == 2)        //保存数值颜色
            {
                color_str = ColorsStr_SplitResult[index].c_str();
                //support Decimal data or Hex data from saved data
                if (wcslen(color_str) >= 2 && '0' == color_str[0] && 'x' == color_str[1])
                    pLayoutItem->ValueColor = wcstol(color_str, nullptr, 16);
                else
                    pLayoutItem->ValueColor = _wtoi(color_str);
            }
            else;
        }
        else
        {
            if (index == 1)             //没找到标签颜色
            {
                pLayoutItem->PrefixColor = default_color;
                index++;
            }
            if (index == 2)             //没找到数值颜色
            {
                pLayoutItem->ValueColor = default_color;
                index++;
            }
        }
    }
}

void CIniHelper::SaveLayoutItemAttributes(const wchar_t* AppName, const wchar_t* KeyName, LayoutItem& layout_item)
{
    CString str;

    CString tmp;
    tmp.Format(_T("\"%s\",0x%x,0x%x"), layout_item.Prefix, layout_item.PrefixColor, layout_item.ValueColor);      //saved as Hex data
    str += tmp;

    _WriteString(AppName, KeyName, wstring(str));
}

void CIniHelper::LoadPluginItemsAttributes(const wchar_t* AppName, std::map<CommonDisplayItem, LayoutItem>& M_layout_items)
{
    for (const auto& plugin : theApp.m_plugin_manager.GetAllIPluginItems())
    {
        LoadLayoutItemAttributes(AppName, plugin->GetItemId(), M_layout_items, TDI_UP, plugin, plugin->GetItemLableText(), 0xffffff);
    }
}

void CIniHelper::SavePluginItemsAttributes(const wchar_t* AppName, std::map<CommonDisplayItem, LayoutItem>& M_layout_items)
{
    for (const auto& plugin : theApp.m_plugin_manager.GetAllIPluginItems())
    {
        SaveLayoutItemAttributes(AppName, plugin->GetItemId(), M_layout_items[plugin]);
    }
}

void CIniHelper::_WriteString(const wchar_t * AppName, const wchar_t * KeyName, const wstring & str)
{
    wstring app_str{ L"[" };
    app_str.append(AppName).append(L"]");
    size_t app_pos{}, app_end_pos, key_pos;
    app_pos = m_ini_str.find(app_str);
    if (app_pos == wstring::npos)       //找不到AppName，则在最后面添加
    {
        if (!m_ini_str.empty() && m_ini_str.back() != L'\n')
            m_ini_str += L"\n";
        app_pos = m_ini_str.size();
        m_ini_str += app_str;
        m_ini_str += L"\n";
    }
    app_end_pos = m_ini_str.find(L"\n[", app_pos + 2);
    if (app_end_pos != wstring::npos)
        app_end_pos++;

    key_pos     = m_ini_str.find(wstring(L"\n") + KeyName + L' ', app_pos);     //查找“\nkey_name ”
    if (key_pos >= app_end_pos)                    //如果找不到“\nkey_name ”，则查找“\nkey_name=”
        key_pos = m_ini_str.find(wstring(L"\n") + KeyName + L'=', app_pos);
    if (key_pos >= app_end_pos)             //找不到KeyName，则插入一个
    {
        //wchar_t buff[256];
        //swprintf_s(buff, L"%s = %s\n", KeyName, str.c_str());
        std::wstring str_temp = KeyName;
        str_temp += L" = ";
        str_temp += str;
        str_temp += L"\n";
        if (app_end_pos == wstring::npos)
            m_ini_str += str_temp;
        else
            m_ini_str.insert(app_end_pos, str_temp);
    }
    else    //找到了KeyName，将等号到换行符之间的文本替换
    {
        size_t str_pos      = m_ini_str.find(L'=',  key_pos + 2);
        size_t line_end_pos = m_ini_str.find(L'\n', key_pos + 2);
        if (str_pos > line_end_pos) //所在行没有等号，则插入一个等号
        {
            m_ini_str.insert(key_pos + wcslen(KeyName) + 1, L" =");
            str_pos = key_pos + wcslen(KeyName) + 2;
        }
        else
        {
            str_pos++;
        }
        size_t str_end_pos;
        str_end_pos = m_ini_str.find(L"\n", str_pos);
        m_ini_str.replace(str_pos, str_end_pos - str_pos, L" " + str);
    }
}

wstring CIniHelper::_GetString(const wchar_t * AppName, const wchar_t * KeyName, const wchar_t* default_str) const
{
    wstring app_str{ L"[" };
    app_str.append(AppName).append(L"]");
    size_t app_pos{}, app_end_pos, key_pos;
    app_pos = m_ini_str.find(app_str);
    if (app_pos == wstring::npos)       //找不到AppName，返回默认字符串
        return default_str;

    app_end_pos = m_ini_str.find(L"\n[", app_pos + 2);
    if (app_end_pos != wstring::npos)
        app_end_pos++;

    key_pos     = m_ini_str.find(wstring(L"\n") + KeyName + L' ', app_pos);     //查找“\nkey_name ”
    if (key_pos >= app_end_pos)                    //如果找不到“\nkey_name ”，则查找“\nkey_name=”
        key_pos = m_ini_str.find(wstring(L"\n") + KeyName + L'=', app_pos);
    if (key_pos >= app_end_pos)             //找不到KeyName，返回默认字符串
    {
        return default_str;
    }
    else    //找到了KeyName，获取等号到换行符之间的文本
    {
        size_t str_pos      = m_ini_str.find(L'=',  key_pos + 2);
        size_t line_end_pos = m_ini_str.find(L'\n', key_pos + 2);
        if (str_pos > line_end_pos) //所在行没有等号，返回默认字符串
        {
            return default_str;
        }
        else
        {
            str_pos++;
        }
        ////////////////////////////////////////////////////////////////////////////
        //  注意：如果等号后面是空的或者全是空格，则返回空字符串，而不是返回默认字符串。
        ////////////////////////////////////////////////////////////////////////////
        size_t str_end_pos;
        str_end_pos = m_ini_str.find(L"\n", str_pos);
        //获取文本
        wstring return_str{ m_ini_str.substr(str_pos, str_end_pos - str_pos) };
        //如果前后有空格，则将其删除
        CCommon::StringNormalize(return_str);
        return return_str;
    }
}

wstring CIniHelper::MergeStringList(const vector<wstring>& values)
{
    wstring str_merge;
    int index = 0;
    //在每个字符串前后加上引号，再将它们用逗号连接起来
    for (const wstring& str : values)
    {
        if (index > 0)
            str_merge.push_back(L',');
        str_merge.push_back(L'\"');
        str_merge += str;
        str_merge.push_back(L'\"');
        index++;
    }
    return str_merge;
}

void CIniHelper::SplitStringList(vector<wstring>& values, wstring str_value)
{
    CCommon::StringSplit(str_value, wstring(L"\",\""), values);
    if (!values.empty())
    {
        //结果中第一项前面和最后一项的后面各还有一个引号，将它们删除
        values.front() = values.front().substr(1);
        values.back().pop_back();
    }
}
