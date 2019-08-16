#define __VK_API_DEF__
#include "VK.h"
#undef __VK_API_DEF__

#include <cassert>

bool Ren::InitVKExtensions(VkInstance instance) {
#define LOAD_VK_FUN(x) \
    *(void **)&(x) = vkGetInstanceProcAddr(instance, #x);   \
    assert((x));

    LOAD_VK_FUN(vkCreateDebugReportCallbackEXT)
    LOAD_VK_FUN(vkDestroyDebugReportCallbackEXT)
    LOAD_VK_FUN(vkDebugReportMessageEXT)
    LOAD_VK_FUN(vkDebugReportMessageEXT)
    LOAD_VK_FUN(vkCreateWin32SurfaceKHR)
    LOAD_VK_FUN(vkGetPhysicalDeviceSurfaceSupportKHR)
    LOAD_VK_FUN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
    LOAD_VK_FUN(vkGetPhysicalDeviceSurfaceFormatsKHR)
    LOAD_VK_FUN(vkGetPhysicalDeviceSurfacePresentModesKHR)
    LOAD_VK_FUN(vkCreateSwapchainKHR)
    LOAD_VK_FUN(vkDestroySwapchainKHR)
    LOAD_VK_FUN(vkGetDeviceQueue)
    LOAD_VK_FUN(vkCreateCommandPool)
    LOAD_VK_FUN(vkAllocateCommandBuffers)
    LOAD_VK_FUN(vkGetSwapchainImagesKHR)
    LOAD_VK_FUN(vkCreateFence)
    LOAD_VK_FUN(vkWaitForFences)
    LOAD_VK_FUN(vkResetFences)
    LOAD_VK_FUN(vkDestroyFence)
    LOAD_VK_FUN(vkBeginCommandBuffer)
    LOAD_VK_FUN(vkEndCommandBuffer)
    LOAD_VK_FUN(vkCmdPipelineBarrier)
    LOAD_VK_FUN(vkQueueSubmit)
    LOAD_VK_FUN(vkResetCommandBuffer)
    LOAD_VK_FUN(vkCreateImageView)
    LOAD_VK_FUN(vkDestroyImageView)
    LOAD_VK_FUN(vkAcquireNextImageKHR)
    LOAD_VK_FUN(vkQueuePresentKHR)
    LOAD_VK_FUN(vkGetPhysicalDeviceMemoryProperties)
    LOAD_VK_FUN(vkGetPhysicalDeviceFormatProperties)
    LOAD_VK_FUN(vkCreateImage)
    LOAD_VK_FUN(vkGetImageMemoryRequirements)
    LOAD_VK_FUN(vkAllocateMemory)
    LOAD_VK_FUN(vkBindImageMemory)
    LOAD_VK_FUN(vkCreateRenderPass)
    LOAD_VK_FUN(vkCreateFramebuffer)
    LOAD_VK_FUN(vkCreateBuffer)
    LOAD_VK_FUN(vkGetBufferMemoryRequirements)
    LOAD_VK_FUN(vkBindBufferMemory)
    LOAD_VK_FUN(vkMapMemory)
    LOAD_VK_FUN(vkUnmapMemory)
    LOAD_VK_FUN(vkCreateShaderModule)
    LOAD_VK_FUN(vkDestroyShaderModule)
    LOAD_VK_FUN(vkCreatePipelineLayout)
    LOAD_VK_FUN(vkDestroyPipelineLayout)
    LOAD_VK_FUN(vkCreateGraphicsPipelines)
    LOAD_VK_FUN(vkDestroyPipeline)
    LOAD_VK_FUN(vkCreateSemaphore)
    LOAD_VK_FUN(vkDestroySemaphore)

    LOAD_VK_FUN(vkCmdBeginRenderPass)
    LOAD_VK_FUN(vkCmdBindPipeline)
    LOAD_VK_FUN(vkCmdSetViewport)
    LOAD_VK_FUN(vkCmdSetScissor)
    LOAD_VK_FUN(vkCmdBindVertexBuffers)
    LOAD_VK_FUN(vkCmdDraw)
    LOAD_VK_FUN(vkCmdEndRenderPass)

#undef LOAD_VK_FUN

    return true;
}