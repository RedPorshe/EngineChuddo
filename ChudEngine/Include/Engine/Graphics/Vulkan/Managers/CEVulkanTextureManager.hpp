// Graphics/Vulkan/Managers/CEVulkanTextureManager.hpp
#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>

namespace CE
    {
    class CEVulkanContext;
    class CEVulkanImage;

    class CEVulkanTexture
        {
        public:
            CEVulkanTexture ();
            ~CEVulkanTexture ();

            bool CreateFromFile ( CEVulkanContext * context, const std::string & filename );
            bool CreateFromData ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                  VkFormat format, const void * data, VkImageUsageFlags additionalUsage = 0 );
            bool CreateRenderTarget ( CEVulkanContext * context, uint32_t width, uint32_t height,
                                      VkFormat format, VkImageUsageFlags usage );
            void Destroy ();

            VkImage GetImage () const { return m_Image; }
            VkImageView GetImageView () const { return m_ImageView; }
            VkSampler GetSampler () const { return m_Sampler; }
            uint32_t GetWidth () const { return m_Width; }
            uint32_t GetHeight () const { return m_Height; }
            VkFormat GetFormat () const { return m_Format; }
            VkImageLayout GetLayout () const { return m_CurrentLayout; }

            void TransitionLayout ( VkCommandBuffer commandBuffer, VkImageLayout newLayout );
            void CopyFromBuffer ( VkCommandBuffer commandBuffer, VkBuffer buffer );

        private:
            bool CreateImage ();
            bool CreateImageView ();
            bool CreateSampler ();

            CEVulkanContext * m_Context = nullptr;
            VkImage m_Image = VK_NULL_HANDLE;
            VkDeviceMemory m_ImageMemory = VK_NULL_HANDLE;
            VkImageView m_ImageView = VK_NULL_HANDLE;
            VkSampler m_Sampler = VK_NULL_HANDLE;

            uint32_t m_Width = 0;
            uint32_t m_Height = 0;
            VkFormat m_Format = VK_FORMAT_R8G8B8A8_SRGB;
            VkImageLayout m_CurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkImageUsageFlags m_Usage = 0;
        };

    class CEVulkanTextureManager
        {
        public:
            CEVulkanTextureManager ( CEVulkanContext * context );
            ~CEVulkanTextureManager ();

            bool Initialize ();
            void Shutdown ();

            std::shared_ptr<CEVulkanTexture> LoadTexture ( const std::string & filename );
            std::shared_ptr<CEVulkanTexture> CreateTexture ( const std::string & name, uint32_t width, uint32_t height,
                                                             VkFormat format, const void * data = nullptr );
            std::shared_ptr<CEVulkanTexture> CreateRenderTarget ( const std::string & name, uint32_t width, uint32_t height,
                                                                  VkFormat format, VkImageUsageFlags usage );
            std::shared_ptr<CEVulkanTexture> GetTexture ( const std::string & name );
            void DestroyTexture ( const std::string & name );

            std::shared_ptr<CEVulkanTexture> GetDefaultTexture ();
            std::shared_ptr<CEVulkanTexture> GetNormalMapTexture ();
            std::shared_ptr<CEVulkanTexture> GetBlackTexture ();
            std::shared_ptr<CEVulkanTexture> GetWhiteTexture ();

        private:
            bool CreateDefaultTextures ();

            CEVulkanContext * m_Context = nullptr;
            std::unordered_map<std::string, std::shared_ptr<CEVulkanTexture>> m_Textures;

            std::shared_ptr<CEVulkanTexture> m_DefaultTexture;
            std::shared_ptr<CEVulkanTexture> m_NormalMapTexture;
            std::shared_ptr<CEVulkanTexture> m_BlackTexture;
            std::shared_ptr<CEVulkanTexture> m_WhiteTexture;
        };
    }