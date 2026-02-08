#pragma once

#include <string>
#include <vector>
#include <memory>

namespace aether {

enum class ClipboardType {
    Text,
    Image,
    Video,
    Audio,
    Custom
};

struct ClipboardData {
    ClipboardType type = ClipboardType::Text;
    std::vector<uint8_t> data;
    std::string mimeType;
    size_t size = 0;
};

class Clipboard {
public:
    // Singleton access
    static Clipboard& getInstance();
    
    // Delete copy constructor and assignment operator
    Clipboard(const Clipboard&) = delete;
    Clipboard& operator=(const Clipboard&) = delete;

    // Clipboard operations
    bool setText(const std::string& text);
    bool setData(const ClipboardData& data);
    std::string getText() const;
    ClipboardData getData() const;
    
    // State queries
    bool hasText() const;
    bool hasData() const;
    ClipboardType getType() const { return m_data.type; }
    void clear();

private:
    Clipboard() = default;
    ~Clipboard() = default;

    ClipboardData m_data;
    std::string m_text;
};

} // namespace aether
