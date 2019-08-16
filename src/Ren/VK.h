#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <vulkan.h>

#ifdef __VK_API_DEF__
#define EXTERN_FUNC
#else
#define EXTERN_FUNC extern
#endif

#define vkCreateInstance ren_vkCreateInstance
#define vkDestroyInstance ren_vkDestroyInstance
#define vkEnumerateInstanceLayerProperties ren_vkEnumerateInstanceLayerProperties
#define vkEnumerateInstanceExtensionProperties ren_vkEnumerateInstanceExtensionProperties
#define vkGetInstanceProcAddr ren_vkGetInstanceProcAddr

#define vkEnumeratePhysicalDevices ren_vkEnumeratePhysicalDevices
#define vkGetPhysicalDeviceProperties ren_vkGetPhysicalDeviceProperties
#define vkGetPhysicalDeviceQueueFamilyProperties ren_vkGetPhysicalDeviceQueueFamilyProperties

#define vkCreateDevice ren_vkCreateDevice
#define vkDestroyDevice ren_vkDestroyDevice

#define vkEnumerateDeviceExtensionProperties ren_vkEnumerateDeviceExtensionProperties

#define vkCreateDebugReportCallbackEXT ren_vkCreateDebugReportCallbackEXT
#define vkDestroyDebugReportCallbackEXT ren_vkDestroyDebugReportCallbackEXT
#define vkDebugReportMessageEXT ren_vkDebugReportMessageEXT

#define vkCreateWin32SurfaceKHR ren_vkCreateWin32SurfaceKHR
#define vkGetPhysicalDeviceSurfaceSupportKHR ren_vkGetPhysicalDeviceSurfaceSupportKHR
#define vkGetPhysicalDeviceSurfaceCapabilitiesKHR ren_vkGetPhysicalDeviceSurfaceCapabilitiesKHR
#define vkGetPhysicalDeviceSurfaceFormatsKHR ren_vkGetPhysicalDeviceSurfaceFormatsKHR

#define vkGetPhysicalDeviceSurfacePresentModesKHR ren_vkGetPhysicalDeviceSurfacePresentModesKHR
#define vkCreateSwapchainKHR ren_vkCreateSwapchainKHR
#define vkDestroySwapchainKHR ren_vkDestroySwapchainKHR
#define vkGetDeviceQueue ren_vkGetDeviceQueue
#define vkCreateCommandPool ren_vkCreateCommandPool
#define vkAllocateCommandBuffers ren_vkAllocateCommandBuffers

#define vkGetSwapchainImagesKHR ren_vkGetSwapchainImagesKHR

#define vkCreateFence ren_vkCreateFence
#define vkWaitForFences ren_vkWaitForFences
#define vkResetFences ren_vkResetFences
#define vkDestroyFence ren_vkDestroyFence

#define vkBeginCommandBuffer ren_vkBeginCommandBuffer
#define vkEndCommandBuffer ren_vkEndCommandBuffer
#define vkCmdPipelineBarrier ren_vkCmdPipelineBarrier

#define vkQueueSubmit ren_vkQueueSubmit
#define vkResetCommandBuffer ren_vkResetCommandBuffer
#define vkCreateImageView ren_vkCreateImageView
#define vkDestroyImageView ren_vkDestroyImageView

#define vkAcquireNextImageKHR ren_vkAcquireNextImageKHR
#define vkQueuePresentKHR ren_vkQueuePresentKHR

#define vkGetPhysicalDeviceMemoryProperties ren_vkGetPhysicalDeviceMemoryProperties
#define vkGetPhysicalDeviceFormatProperties ren_vkGetPhysicalDeviceFormatProperties

#define vkCreateImage ren_vkCreateImage

#define vkGetImageMemoryRequirements ren_vkGetImageMemoryRequirements
#define vkAllocateMemory ren_vkAllocateMemory
#define vkBindImageMemory ren_vkBindImageMemory

#define vkCreateRenderPass ren_vkCreateRenderPass
#define vkCreateFramebuffer ren_vkCreateFramebuffer

#define vkCreateBuffer ren_vkCreateBuffer
#define vkGetBufferMemoryRequirements ren_vkGetBufferMemoryRequirements
#define vkBindBufferMemory ren_vkBindBufferMemory

#define vkMapMemory ren_vkMapMemory
#define vkUnmapMemory ren_vkUnmapMemory

#define vkCreateShaderModule ren_vkCreateShaderModule
#define vkDestroyShaderModule ren_vkDestroyShaderModule

#define vkCreatePipelineLayout ren_vkCreatePipelineLayout
#define vkDestroyPipelineLayout ren_vkDestroyPipelineLayout

#define vkCreateGraphicsPipelines ren_vkCreateGraphicsPipelines
#define vkDestroyPipeline ren_vkDestroyPipeline

#define vkCreateSemaphore ren_vkCreateSemaphore
#define vkDestroySemaphore ren_vkDestroySemaphore

#define vkCmdBeginRenderPass ren_vkCmdBeginRenderPass
#define vkCmdBindPipeline ren_vkCmdBindPipeline
#define vkCmdSetViewport ren_vkCmdSetViewport
#define vkCmdSetScissor ren_vkCmdSetScissor
#define vkCmdBindVertexBuffers ren_vkCmdBindVertexBuffers
#define vkCmdDraw ren_vkCmdDraw
#define vkCmdEndRenderPass ren_vkCmdEndRenderPass

extern "C" {
    EXTERN_FUNC PFN_vkCreateInstance ren_vkCreateInstance;
    EXTERN_FUNC PFN_vkDestroyInstance ren_vkDestroyInstance;
    EXTERN_FUNC PFN_vkEnumerateInstanceLayerProperties ren_vkEnumerateInstanceLayerProperties;
    EXTERN_FUNC PFN_vkEnumerateInstanceExtensionProperties ren_vkEnumerateInstanceExtensionProperties;
    EXTERN_FUNC PFN_vkGetInstanceProcAddr ren_vkGetInstanceProcAddr;

    EXTERN_FUNC PFN_vkEnumeratePhysicalDevices ren_vkEnumeratePhysicalDevices;
    EXTERN_FUNC PFN_vkGetPhysicalDeviceProperties ren_vkGetPhysicalDeviceProperties;
    EXTERN_FUNC PFN_vkGetPhysicalDeviceQueueFamilyProperties ren_vkGetPhysicalDeviceQueueFamilyProperties;

    EXTERN_FUNC PFN_vkCreateDevice ren_vkCreateDevice;
    EXTERN_FUNC PFN_vkDestroyDevice ren_vkDestroyDevice;

    EXTERN_FUNC PFN_vkEnumerateDeviceExtensionProperties ren_vkEnumerateDeviceExtensionProperties;

    EXTERN_FUNC PFN_vkCreateDebugReportCallbackEXT ren_vkCreateDebugReportCallbackEXT;
    EXTERN_FUNC PFN_vkDestroyDebugReportCallbackEXT ren_vkDestroyDebugReportCallbackEXT;
    EXTERN_FUNC PFN_vkDebugReportMessageEXT ren_vkDebugReportMessageEXT;

    EXTERN_FUNC PFN_vkCreateWin32SurfaceKHR ren_vkCreateWin32SurfaceKHR;
    EXTERN_FUNC PFN_vkGetPhysicalDeviceSurfaceSupportKHR ren_vkGetPhysicalDeviceSurfaceSupportKHR;
    EXTERN_FUNC PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR ren_vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    EXTERN_FUNC PFN_vkGetPhysicalDeviceSurfaceFormatsKHR ren_vkGetPhysicalDeviceSurfaceFormatsKHR;

    EXTERN_FUNC PFN_vkGetPhysicalDeviceSurfacePresentModesKHR ren_vkGetPhysicalDeviceSurfacePresentModesKHR;
    EXTERN_FUNC PFN_vkCreateSwapchainKHR ren_vkCreateSwapchainKHR;
    EXTERN_FUNC PFN_vkDestroySwapchainKHR ren_vkDestroySwapchainKHR;
    EXTERN_FUNC PFN_vkGetDeviceQueue ren_vkGetDeviceQueue;
    EXTERN_FUNC PFN_vkCreateCommandPool ren_vkCreateCommandPool;
    EXTERN_FUNC PFN_vkAllocateCommandBuffers ren_vkAllocateCommandBuffers;

    EXTERN_FUNC PFN_vkGetSwapchainImagesKHR ren_vkGetSwapchainImagesKHR;

    EXTERN_FUNC PFN_vkCreateFence ren_vkCreateFence;
    EXTERN_FUNC PFN_vkWaitForFences ren_vkWaitForFences;
    EXTERN_FUNC PFN_vkResetFences ren_vkResetFences;
    EXTERN_FUNC PFN_vkDestroyFence ren_vkDestroyFence;

    EXTERN_FUNC PFN_vkBeginCommandBuffer ren_vkBeginCommandBuffer;
    EXTERN_FUNC PFN_vkEndCommandBuffer ren_vkEndCommandBuffer;
    EXTERN_FUNC PFN_vkCmdPipelineBarrier ren_vkCmdPipelineBarrier;

    EXTERN_FUNC PFN_vkQueueSubmit ren_vkQueueSubmit;
    EXTERN_FUNC PFN_vkResetCommandBuffer ren_vkResetCommandBuffer;
    EXTERN_FUNC PFN_vkCreateImageView ren_vkCreateImageView;
    EXTERN_FUNC PFN_vkDestroyImageView ren_vkDestroyImageView;

    EXTERN_FUNC PFN_vkAcquireNextImageKHR ren_vkAcquireNextImageKHR;
    EXTERN_FUNC PFN_vkQueuePresentKHR ren_vkQueuePresentKHR;

    EXTERN_FUNC PFN_vkGetPhysicalDeviceMemoryProperties ren_vkGetPhysicalDeviceMemoryProperties;
    EXTERN_FUNC PFN_vkGetPhysicalDeviceFormatProperties ren_vkGetPhysicalDeviceFormatProperties;

    EXTERN_FUNC PFN_vkCreateImage ren_vkCreateImage;

    EXTERN_FUNC PFN_vkGetImageMemoryRequirements ren_vkGetImageMemoryRequirements;
    EXTERN_FUNC PFN_vkAllocateMemory ren_vkAllocateMemory;
    EXTERN_FUNC PFN_vkBindImageMemory ren_vkBindImageMemory;

    EXTERN_FUNC PFN_vkCreateRenderPass ren_vkCreateRenderPass;
    EXTERN_FUNC PFN_vkCreateFramebuffer ren_vkCreateFramebuffer;

    EXTERN_FUNC PFN_vkCreateBuffer ren_vkCreateBuffer;
    EXTERN_FUNC PFN_vkGetBufferMemoryRequirements ren_vkGetBufferMemoryRequirements;
    EXTERN_FUNC PFN_vkBindBufferMemory ren_vkBindBufferMemory;

    EXTERN_FUNC PFN_vkMapMemory ren_vkMapMemory;
    EXTERN_FUNC PFN_vkUnmapMemory ren_vkUnmapMemory;

    EXTERN_FUNC PFN_vkCreateShaderModule ren_vkCreateShaderModule;
    EXTERN_FUNC PFN_vkDestroyShaderModule ren_vkDestroyShaderModule;

    EXTERN_FUNC PFN_vkCreatePipelineLayout ren_vkCreatePipelineLayout;
    EXTERN_FUNC PFN_vkDestroyPipelineLayout ren_vkDestroyPipelineLayout;

    EXTERN_FUNC PFN_vkCreateGraphicsPipelines ren_vkCreateGraphicsPipelines;
    EXTERN_FUNC PFN_vkDestroyPipeline ren_vkDestroyPipeline;

    EXTERN_FUNC PFN_vkCreateSemaphore ren_vkCreateSemaphore;
    EXTERN_FUNC PFN_vkDestroySemaphore ren_vkDestroySemaphore;

    EXTERN_FUNC PFN_vkCmdBeginRenderPass ren_vkCmdBeginRenderPass;
    EXTERN_FUNC PFN_vkCmdBindPipeline ren_vkCmdBindPipeline;
    EXTERN_FUNC PFN_vkCmdSetViewport ren_vkCmdSetViewport;
    EXTERN_FUNC PFN_vkCmdSetScissor ren_vkCmdSetScissor;
    EXTERN_FUNC PFN_vkCmdBindVertexBuffers ren_vkCmdBindVertexBuffers;
    EXTERN_FUNC PFN_vkCmdDraw ren_vkCmdDraw;
    EXTERN_FUNC PFN_vkCmdEndRenderPass ren_vkCmdEndRenderPass;
}

namespace Ren {
    bool InitVKExtensions(VkInstance instance);
}