#include "../../include/aether/Clipboard.h"
#ifdef _WIN32
#include <windows.h>
#include <cstring>
#endif
#include <iostream>

namespace aether {

Clipboard& Clipboard::getInstance() {
    static Clipboard instance;
    return instance;
}

bool Clipboard::setText(const std::string& text) {
    m_text = text;
    
#ifdef _WIN32
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
        if (hMem) {
            memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
            CloseClipboard();
            return true;
        }
        CloseClipboard();
    }
#endif
    
    return false;
}

bool Clipboard::setData(const ClipboardData& data) {
    m_data = data;
    return true;
}

std::string Clipboard::getText() const {
#ifdef _WIN32
    if (OpenClipboard(NULL)) {
        HANDLE hData = GetClipboardData(CF_TEXT);
        if (hData) {
            char* pszText = static_cast<char*>(GlobalLock(hData));
            if (pszText) {
                std::string text(pszText);
                GlobalUnlock(hData);
                CloseClipboard();
                return text;
            }
        }
        CloseClipboard();
    }
#endif
    
    return m_text;
}

ClipboardData Clipboard::getData() const {
    return m_data;
}

bool Clipboard::hasText() const {
    return !m_text.empty() || !getText().empty();
}

bool Clipboard::hasData() const {
    return m_data.size > 0;
}

void Clipboard::clear() {
    m_text.clear();
    m_data = ClipboardData{};
    
#ifdef _WIN32
    if (OpenClipboard(NULL)) {
        EmptyClipboard();
        CloseClipboard();
    }
#endif
}

} // namespace aether
