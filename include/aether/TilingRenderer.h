#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

namespace aether {

struct Tile {
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t width = 512;
    uint32_t height = 512;
    bool isVisible = false;
};

class TilingRenderer {
public:
    TilingRenderer();
    ~TilingRenderer() = default;

    TilingRenderer(const TilingRenderer&) = delete;
    TilingRenderer& operator=(const TilingRenderer&) = delete;

    // Initialization
    bool initialize(uint32_t imageWidth, uint32_t imageHeight, uint32_t tileSize = 512);
    void shutdown();

    // Tile management
    void updateViewport(uint32_t viewportX, uint32_t viewportY, uint32_t viewportWidth, uint32_t viewportHeight);
    const std::vector<Tile>& getVisibleTiles() const { return m_visibleTiles; }
    const std::vector<Tile>& getAllTiles() const { return m_allTiles; }
    
    // Tile queries
    Tile getTileAt(uint32_t x, uint32_t y) const;
    std::vector<Tile> getTilesInRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const;
    
    // Statistics
    uint32_t getTotalTileCount() const { return static_cast<uint32_t>(m_allTiles.size()); }
    uint32_t getVisibleTileCount() const { return static_cast<uint32_t>(m_visibleTiles.size()); }
    uint32_t getTileSize() const { return m_tileSize; }

private:
    void generateTiles();
    void cullTiles();
    bool isTileVisible(const Tile& tile) const;

    uint32_t m_imageWidth = 0;
    uint32_t m_imageHeight = 0;
    uint32_t m_tileSize = 512;
    
    uint32_t m_viewportX = 0;
    uint32_t m_viewportY = 0;
    uint32_t m_viewportWidth = 0;
    uint32_t m_viewportHeight = 0;
    
    std::vector<Tile> m_allTiles;
    std::vector<Tile> m_visibleTiles;
    
    bool m_initialized = false;
};

} // namespace aether
