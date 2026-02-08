#include "../../include/aether/TilingRenderer.h"
#include <algorithm>
#include <cmath>

namespace aether {

TilingRenderer::TilingRenderer() {
}

bool TilingRenderer::initialize(uint32_t imageWidth, uint32_t imageHeight, uint32_t tileSize) {
    if (imageWidth == 0 || imageHeight == 0) {
        std::cerr << "Invalid image dimensions for tiling renderer" << std::endl;
        return false;
    }
    
    m_imageWidth = imageWidth;
    m_imageHeight = imageHeight;
    m_tileSize = tileSize;
    
    generateTiles();
    
    m_initialized = true;
    std::cout << "Tiling Renderer initialized: " << m_imageWidth << "x" << m_imageHeight 
              << " with " << m_allTiles.size() << " tiles (" << m_tileSize << "x" << m_tileSize << ")" << std::endl;
    return true;
}

void TilingRenderer::shutdown() {
    m_allTiles.clear();
    m_visibleTiles.clear();
    m_initialized = false;
}

void TilingRenderer::generateTiles() {
    m_allTiles.clear();
    
    uint32_t tilesX = static_cast<uint32_t>(std::ceil(static_cast<float>(m_imageWidth) / m_tileSize));
    uint32_t tilesY = static_cast<uint32_t>(std::ceil(static_cast<float>(m_imageHeight) / m_tileSize));
    
    for (uint32_t y = 0; y < tilesY; y++) {
        for (uint32_t x = 0; x < tilesX; x++) {
            Tile tile;
            tile.x = x * m_tileSize;
            tile.y = y * m_tileSize;
            tile.width = std::min(m_tileSize, m_imageWidth - tile.x);
            tile.height = std::min(m_tileSize, m_imageHeight - tile.y);
            tile.isVisible = false;
            
            m_allTiles.push_back(tile);
        }
    }
}

void TilingRenderer::updateViewport(uint32_t viewportX, uint32_t viewportY, uint32_t viewportWidth, uint32_t viewportHeight) {
    m_viewportX = viewportX;
    m_viewportY = viewportY;
    m_viewportWidth = viewportWidth;
    m_viewportHeight = viewportHeight;
    
    cullTiles();
}

void TilingRenderer::cullTiles() {
    m_visibleTiles.clear();
    
    for (auto& tile : m_allTiles) {
        tile.isVisible = isTileVisible(tile);
        if (tile.isVisible) {
            m_visibleTiles.push_back(tile);
        }
    }
}

bool TilingRenderer::isTileVisible(const Tile& tile) const {
    // Check if tile intersects with viewport
    uint32_t tileRight = tile.x + tile.width;
    uint32_t tileBottom = tile.y + tile.height;
    uint32_t viewportRight = m_viewportX + m_viewportWidth;
    uint32_t viewportBottom = m_viewportY + m_viewportHeight;
    
    // AABB intersection test
    if (tile.x >= viewportRight || tileRight <= m_viewportX ||
        tile.y >= viewportBottom || tileBottom <= m_viewportY) {
        return false;
    }
    
    return true;
}

Tile TilingRenderer::getTileAt(uint32_t x, uint32_t y) const {
    uint32_t tileX = (x / m_tileSize) * m_tileSize;
    uint32_t tileY = (y / m_tileSize) * m_tileSize;
    
    for (const auto& tile : m_allTiles) {
        if (tile.x == tileX && tile.y == tileY) {
            return tile;
        }
    }
    
    return Tile{}; // Return empty tile if not found
}

std::vector<Tile> TilingRenderer::getTilesInRegion(uint32_t x, uint32_t y, uint32_t width, uint32_t height) const {
    std::vector<Tile> tiles;
    
    uint32_t regionRight = x + width;
    uint32_t regionBottom = y + height;
    
    for (const auto& tile : m_allTiles) {
        uint32_t tileRight = tile.x + tile.width;
        uint32_t tileBottom = tile.y + tile.height;
        
        // Check intersection
        if (tile.x < regionRight && tileRight > x &&
            tile.y < regionBottom && tileBottom > y) {
            tiles.push_back(tile);
        }
    }
    
    return tiles;
}

} // namespace aether
