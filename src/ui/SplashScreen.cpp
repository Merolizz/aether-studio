#define STB_IMAGE_IMPLEMENTATION
#include "../../include/aether/SplashScreen.h"
#include "../core/Window.h"
#include "../engine/render/VulkanRenderer.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "stb_image.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>
#include <stdexcept>


namespace aether {

// Helper function to find suitable memory type
static uint32_t findMemoryType(VkPhysicalDevice physicalDevice,
                               uint32_t typeFilter,
                               VkMemoryPropertyFlags properties) {
  VkPhysicalDeviceMemoryProperties memProperties;
  vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

  for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags &
                                    properties) == properties) {
      return i;
    }
  }

  throw std::runtime_error("failed to find suitable memory type!");
}

// Helper to create a one-time command buffer
static VkCommandBuffer beginSingleTimeCommands(VulkanRenderer *renderer) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = renderer->getCommandPool();
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(renderer->getDevice(), &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

static void endSingleTimeCommands(VulkanRenderer *renderer,
                                  VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(renderer->getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(renderer->getGraphicsQueue());

  vkFreeCommandBuffers(renderer->getDevice(), renderer->getCommandPool(), 1,
                       &commandBuffer);
}

static void transitionImageLayout(VulkanRenderer *renderer, VkImage image,
                                  VkFormat format, VkImageLayout oldLayout,
                                  VkImageLayout newLayout) {
  (void)format; // Unused parameter - needed for future format-specific
                // transitions
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
      newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                       nullptr, 0, nullptr, 1, &barrier);

  endSingleTimeCommands(renderer, commandBuffer);
}

static void copyBufferToImage(VulkanRenderer *renderer, VkBuffer buffer,
                              VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(renderer);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  endSingleTimeCommands(renderer, commandBuffer);
}

SplashScreen::SplashScreen() {}

SplashScreen::~SplashScreen() { shutdown(); }

bool SplashScreen::initialize(VulkanRenderer *renderer, Window *window) {
  if (m_initialized) {
    return true;
  }

  if (!renderer || !window) {
    std::cerr << "Invalid renderer or window for SplashScreen" << std::endl;
    return false;
  }

  m_renderer = renderer;
  m_window = window;

  if (loadSplashImage(SPLASH_IMAGE_PATH)) {
    std::cout << "Splash screen initialized with image: " << SPLASH_IMAGE_PATH
              << std::endl;
  } else {
    std::cerr << "Warning: Failed to load splash image, will use fallback "
                 "visualization"
              << std::endl;
  }

  m_initialized = true;
  return true;
}

void SplashScreen::shutdown() {
  cleanupVulkanResources();
  m_visible = false;
  m_initialized = false;
  m_imageLoaded = false;
  m_renderer = nullptr;
  m_window = nullptr;
}

void SplashScreen::cleanupVulkanResources() {
  VkDevice device = m_renderer ? m_renderer->getDevice() : VK_NULL_HANDLE;
  if (device == VK_NULL_HANDLE)
    return;

  // Use Graphics Queue Wait Idle to ensure resources aren't in use
  vkQueueWaitIdle(m_renderer->getGraphicsQueue());

  if (m_splashDS != VK_NULL_HANDLE) {
    ImGui_ImplVulkan_RemoveTexture(m_splashDS);
    m_splashDS = VK_NULL_HANDLE;
  }

  if (m_splashSampler != VK_NULL_HANDLE) {
    vkDestroySampler(device, m_splashSampler, nullptr);
    m_splashSampler = VK_NULL_HANDLE;
  }

  if (m_splashImageView != VK_NULL_HANDLE) {
    vkDestroyImageView(device, m_splashImageView, nullptr);
    m_splashImageView = VK_NULL_HANDLE;
  }

  if (m_splashImage != VK_NULL_HANDLE) {
    vkDestroyImage(device, m_splashImage, nullptr);
    m_splashImage = VK_NULL_HANDLE;
  }

  if (m_splashImageMemory != VK_NULL_HANDLE) {
    vkFreeMemory(device, m_splashImageMemory, nullptr);
    m_splashImageMemory = VK_NULL_HANDLE;
  }
}

void SplashScreen::show() {
  if (!m_initialized) {
    std::cerr << "SplashScreen not initialized" << std::endl;
    return;
  }

  m_visible = true;
  m_startTime = std::chrono::steady_clock::now();
}

void SplashScreen::hide() { m_visible = false; }

bool SplashScreen::shouldClose() const {
  if (!m_visible) {
    return false;
  }

  auto now = std::chrono::steady_clock::now();
  auto elapsed =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - m_startTime);
  double elapsedSeconds = elapsed.count() / 1000.0;

  return elapsedSeconds >= SPLASH_DURATION_SECONDS;
}

bool SplashScreen::loadSplashImage(const std::string &imagePath) {
  if (!std::ifstream(imagePath).good()) {
    std::cerr << "Splash image file not found: " << imagePath << std::endl;
    return false;
  }

  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(imagePath.c_str(), &texWidth, &texHeight,
                              &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    std::cerr << "Failed to load splash image pixels: " << stbi_failure_reason()
              << std::endl;
    return false;
  }

  m_imageWidth = texWidth;
  m_imageHeight = texHeight;

  VkDevice device = m_renderer->getDevice();
  VkPhysicalDevice physicalDevice = m_renderer->getPhysicalDevice();

  // Create Staging Buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = imageSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) !=
      VK_SUCCESS) {
    stbi_image_free(pixels);
    return false;
  }

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) !=
      VK_SUCCESS) {
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    stbi_image_free(pixels);
    return false;
  }

  vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

  void *data;
  vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(device, stagingBufferMemory);

  stbi_image_free(pixels);

  // Create Image
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = texWidth;
  imageInfo.extent.height = texHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage =
      VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0;

  if (vkCreateImage(device, &imageInfo, nullptr, &m_splashImage) !=
      VK_SUCCESS) {
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    return false;
  }

  vkGetImageMemoryRequirements(device, m_splashImage, &memRequirements);
  allocInfo.allocationSize = memRequirements.size;
  allocInfo.memoryTypeIndex =
      findMemoryType(physicalDevice, memRequirements.memoryTypeBits,
                     VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  if (vkAllocateMemory(device, &allocInfo, nullptr, &m_splashImageMemory) !=
      VK_SUCCESS) {
    vkDestroyImage(device, m_splashImage, nullptr);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
    return false;
  }

  vkBindImageMemory(device, m_splashImage, m_splashImageMemory, 0);

  // Copy to image
  transitionImageLayout(m_renderer, m_splashImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_UNDEFINED,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  copyBufferToImage(m_renderer, stagingBuffer, m_splashImage,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  transitionImageLayout(m_renderer, m_splashImage, VK_FORMAT_R8G8B8A8_SRGB,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  vkDestroyBuffer(device, stagingBuffer, nullptr);
  vkFreeMemory(device, stagingBufferMemory, nullptr);

  // Create Image View
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = m_splashImage;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  if (vkCreateImageView(device, &viewInfo, nullptr, &m_splashImageView) !=
      VK_SUCCESS) {
    return false;
  }

  // Create Sampler
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
  samplerInfo.anisotropyEnable = VK_FALSE;
  samplerInfo.maxAnisotropy = 1.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.mipLodBias = 0.0f;
  samplerInfo.minLod = 0.0f;
  samplerInfo.maxLod = 0.0f;

  if (vkCreateSampler(device, &samplerInfo, nullptr, &m_splashSampler) !=
      VK_SUCCESS) {
    return false;
  }

  // Register with ImGui
  m_splashDS =
      ImGui_ImplVulkan_AddTexture(m_splashSampler, m_splashImageView,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

  m_imageLoaded = true;
  return true;
}

void SplashScreen::render() {
  if (!m_visible || !m_initialized) {
    return;
  }

  // Check timeout
  if (shouldClose()) {
    hide();
    return;
  }

  ImGuiIO &io = ImGui::GetIO();
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(io.DisplaySize);

  ImGuiWindowFlags windowFlags =
      ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
      ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNav;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  if (ImGui::Begin("SplashScreen", nullptr, windowFlags)) {
    if (m_imageLoaded && m_splashDS) {
      // Render full screen image
      ImGui::Image((ImTextureID)m_splashDS, io.DisplaySize);
    } else {
      // Fallback: Black background with text
      ImDrawList *drawList = ImGui::GetWindowDrawList();
      drawList->AddRectFilled(ImVec2(0, 0), io.DisplaySize,
                              IM_COL32(20, 20, 20, 255));

      ImVec2 textSize = ImGui::CalcTextSize("Aether Studio");
      ImGui::SetCursorPos(ImVec2((io.DisplaySize.x - textSize.x) * 0.5f,
                                 (io.DisplaySize.y - textSize.y) * 0.5f));
      ImGui::Text("Aether Studio");
    }
  }
  ImGui::End();

  ImGui::PopStyleVar(2);
}

} // namespace aether
