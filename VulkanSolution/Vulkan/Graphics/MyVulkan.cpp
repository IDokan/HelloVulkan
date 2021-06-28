/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   MyVulkan.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.12.2021
	Source file for my vulkan.
******************************************************************************/
#include <iostream>
#include <fstream>
#include <vector>
#include "Graphics/MyVulkan.h"
#include "Vulkan/vulkan.h"
#include "Helper/VulkanHelper.h"
#include "Graphics/Allocator/Allocator.h"
#include "GLMath.h"

namespace MyVulkan
{
	VkInstance instance{};
	std::vector<VkPhysicalDevice> physicalDevices;
	std::vector<VkDevice> logicalDevices;
	allocator allocatorForm;
	const VkAllocationCallbacks myAllocator = (VkAllocationCallbacks)allocatorForm;

	double linearTosRGB(double cl);
	double sRGBToLinear(double cs);

	// When required or preferredFlags are not,
	uint32_t ChooseHeapFromFlags(
		const VkMemoryRequirements& memoryRequirements);
	// returns ~0u if matched memory does not exist.
	uint32_t ChooseHeapFromFlags(
		const VkMemoryRequirements& memoryRequirements,
		VkMemoryPropertyFlags requiredFlags,
		VkMemoryPropertyFlags preferredFlags);

	void FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize length, const float value);

	/*
	void CreateSwapChain();

	void TransitionImageLayout();

	void SavingPipelineCacheDataToFile(VkDevice device, VkPipelineCache cache, const char* fileName);

	void CreateDescriptorSetLayout();

	void CreatePipelineLayout();
	*/

	void CreateSimpleRenderpass();

	void CreateSimpleGraphicsPipeline();

	void DescribeVertexInputData();
}

void MyVulkan::InitVulkan(const char* appName, uint32_t appVersion)
{
	std::ofstream resultFile;

	//A generic application info structure
	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = appName;
	applicationInfo.applicationVersion = appVersion;
	applicationInfo.pEngineName = "Sinil Engine";
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VK_API_VERSION_1_0;
	// application.Info.apiVersion = ; it contains the version of Vulkan API that my application is expecting to run on. This should be set to the absolute minimum version


	// Create an instance
	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &applicationInfo;
	VulkanHelper::VkCheck(vkCreateInstance(&instanceCreateInfo, &myAllocator, &instance), "Could not create instance");


	void* getHowMany = nullptr;

	// Get Physical devices
	uint32_t numOfPhysicalDevices = 0;
	std::vector<VkPhysicalDevice> arrayOfPhysicalDevices;
	// second parameter of VK physical devices function works both input & output.
	// As an output, the parameter get how many physical devices I can use.
	// As an input, The maximum number of devices I can control in this application.
	// Once we want to know how many devices available in the system, give final parameter nullptr.(second parameter still should be valid pointer)
	// Then call the same function again with the final parameter set to an array that has been appropriately sized for the number what we have known.
	VulkanHelper::VkCheck(vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, reinterpret_cast<VkPhysicalDevice*>(getHowMany)), "The first Procedure with physical devices is failed! (Originally, it might be failed)");
	arrayOfPhysicalDevices.resize(numOfPhysicalDevices);
	VulkanHelper::VkCheck(vkEnumeratePhysicalDevices(instance, &numOfPhysicalDevices, arrayOfPhysicalDevices.data()), "The second Procedure with physical devices is failed!MSVC\14.29.30037\include\cmath         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\yvals.h         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\crtdbg.h         K   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt.h         H   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\vcruntime_new_debug.h         B   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\vcruntime_new.h         <   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\crtdefs.h         =   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\use_ansi.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cstdlib         H   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\math.h         P   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_math.h         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\stdlib.h         R   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_malloc.h         R   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_search.h         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\stddef.h         S   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wstdlib.h         =   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xtr1common         <   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\intrin0.h         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xutility         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cstring         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\string.h         R   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_memory.h         T   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_memcpy_s.h         I   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\errno.h         E   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\vcruntime_string.h         S   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wstring.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\utility         >   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\type_traits         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cstdint         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\stdint.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xstddef         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cstddef         C   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\initializer_list         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\concepts         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\compare         6   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\bit         E   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\isa_availability.h         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\limits         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cfloat         I   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\float.h         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cwchar         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cstdio         I   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\stdio.h         R   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wstdio.h         X   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_stdio_config.h         I   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\wchar.h         R   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wconio.h         R   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wctype.h         S   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wdirect.h         O   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wio.h         Q   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_share.h         T   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wprocess.h         Q   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_wtime.h         L   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\sys/stat.h         M   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\sys/types.h         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\iterator         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\iosfwd         <   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\streambuf         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xiosbase         I   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\share.h         ?   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\system_error         N   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\__msvc_system_error_abi.hpp         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cerrno         <   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\stdexcept         <   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\exception         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\malloc.h         H   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\vcruntime_exception.h         7   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\eh.h         U   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\corecrt_terminate.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xstring         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xmemory         6   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\new         <   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xatomic.h         8   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\tuple         K   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xpolymorphic_allocator.h         ?   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xcall_once.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xerrc.h         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\atomic         A   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xatomic_wait.h         =   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xthreads.h         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xtimec.h         8   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\ctime         H   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\time.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xlocale         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\memory         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\typeinfo         G   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\vcruntime_typeinfo.h         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xfacet         ;   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xlocinfo         =   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\xlocinfo.h         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cctype         I   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\ctype.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\clocale         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\locale.h         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\fstream         9   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\vector         A   D:\Vulkan\FancyVulkan\VulkanSolution\\Vulkan\Graphics/MyVulkan.h �K�`    N   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\Vulkan\Include\Vulkan/vulkan.h         S   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\Vulkan\Include\Vulkan/vk_platform.h         S   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\Vulkan\Include\Vulkan/vulkan_core.h         C   D:\Vulkan\FancyVulkan\VulkanSolution\\Vulkan\Helper/VulkanHelper.h         L   D:\Vulkan\FancyVulkan\VulkanSolution\\Vulkan\Graphics/Allocator/Allocator.h         6   D:\Vulkan\FancyVulkan\VulkanSolution\\Vulkan\GLMath.h �I�`    <   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/vec2.hpp         J   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_bool2.hpp         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/type_vec2.hpp         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/qualifier.hpp         M   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/setup.hpp         :   D:\VisualStudio\VC\Tools\MSVC\14.29.30037\include\cassert         J   C:\Program Files (x86)\Windows Kits\10\Include\10.0.19041.0\ucrt\assert.h         V   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/../simd/platform.h         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/type_vec2.inl         c   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/./compute_vector_relational.hpp         O   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/./setup.hpp         T   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_bool2_precision.hpp         K   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_float2.hpp         U   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_float2_precision.hpp         L   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_double2.hpp         V   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_double2_precision.hpp         I   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_int2.hpp         O   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_int2_sized.hpp         U   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../ext/scalar_int_sized.hpp         T   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../ext/../detail/setup.hpp         J   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_uint2.hpp         P   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_uint2_sized.hpp         V   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../ext/scalar_uint_sized.hpp         T   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../ext/../detail/setup.hpp         <   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/vec3.hpp         J   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_bool3.hpp         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/type_vec3.hpp         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/type_vec3.inl         T   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_bool3_precision.hpp         K   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_float3.hpp         U   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_float3_precision.hpp         L   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_double3.hpp         V   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_double3_precision.hpp         I   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_int3.hpp         O   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_int3_sized.hpp         J   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_uint3.hpp         P   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_uint3_sized.hpp         <   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/vec4.hpp         J   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_bool4.hpp         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/type_vec4.hpp         Q   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/../detail/type_vec4.inl         T   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_bool4_precision.hpp         K   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_float4.hpp         U   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_float4_precision.hpp         L   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_double4.hpp         V   D:\Vulkan\FancyVulkan\VulkanSolution\Libraries\glm/./ext/vector_double4_precision.hpp         M   D:\Vulkan\Fan0)
	{
		cl = 1.0;
	}
	else if (cs <= 0.0)
	{
		cl = 0.0;
	}
	else if (cs <= 0.04045)
	{
		cl = cs / 12.92;
	}
	else
	{
		cl = pow(((cs + 0.0555) / 1.055), 2.4);
	}

	return cl;
}

uint32_t MyVulkan::ChooseHeapFromFlags(const VkMemoryRequirements& memoryRequirements)
{
	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if ((memoryRequirements.memoryTypeBits & memoryType) != 0)
		{
			return memoryType;
		}
	}
}

uint32_t MyVulkan::ChooseHeapFromFlags(const VkMemoryRequirements& memoryRequirements, VkMemoryPropertyFlags requiredFlags, VkMemoryPropertyFlags preferredFlags)
{
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevices.front(), &deviceMemoryProperties);

	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if (memoryRequirements.memoryTypeBits & (1 << memoryType))
		{
			const VkMemoryType& type = deviceMemoryProperties.memoryTypes[memoryType];

			// If it exactly matches my preffered properties, grab it.
			if ((type.propertyFlags & preferredFlags) == preferredFlags)
			{
				return memoryType;
			}
		}
	}

	for (uint32_t memoryType = 0; memoryType < VK_MAX_MEMORY_TYPES; memoryType++)
	{
		if (memoryRequirements.memoryTypeBits & (1 << memoryType))
		{
			const VkMemoryType& type = deviceMemoryProperties.memoryTypes[memoryType];

			// If it has all my required properties, it'll do.
			if ((type.propertyFlags & requiredFlags) == requiredFlags)
			{
				return memoryType;
			}
		}
	}

	return ~0U;
}

void MyVulkan::FillBufferWithFloats(VkCommandBuffer cmdBuffer, VkBuffer dstBuffer, VkDeviceSize offset, VkDeviceSize size, const float value)
{
	vkCmdFillBuffer(cmdBuffer, dstBuffer, offset, size, *(const uint32_t*)&value);
}
/*
void MyVulkan::CreateSwapChain()
{
	// First, we create the swap chain.
	VkSwapchainCreateInfoKHR swapchainCreateInfo =
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,		// sType
		nullptr,																									// pNext
		0,																											// flags
		m_mainSurface,																					// surface
		2, 																										// minImageCount
		VK_FORMAT_R8G8B8A8_UNORM,													// imageFormat
		VK_COLORSPACE_SRGB_NONLINEAR_KHR,								// imageColorSpace
		{1024, 768},																							// imageExtent
		1,																											// imageArrayLayers
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,							// imageUsage
		VK_SHARING_MODE_EXCLUSIVE,													// imageSharingMode
		0,																											// queueFamilyIndexCount
		nullptr,																									// pQueueFamilyIndices
		VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR,							// preTransform
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,								// compositeAlpha
		VK_PRESENT_MODE_FIFO_KHR,													// presentMode
		VK_TRUE,																							// clipped
		m_swapChain																						// oldSwapchain
	};

	VulkanHelper::VkCheck(
		vkCreateSwapchainKHR(
		logicalDevices.front(),
			&swapchainCreateInfo,
			&myAllocator,
			&m_swapChain
		),
		"vkCreateSwapchainKHR is failed!");

	// Next, we query the swap chain for the number of images it actaully contains.
	uint32_t swapchainImageCount = 0;
	VulkanHelper::VkCheck(
		vkGetSwapchainImagesKHR(
			logicalDevices.front(),
			m_swapChain,
			&swapchainImageCount,
			nullptr
		),
		"vkGetSwapchainImagesKHR is failed!"
	);
	// Now we resize our image array and retrieve the image handles from the swap chain.
	m_swapChainImages.resize(swapchainImageCount);
	VulkanHelper::VkCheck(
		vkGetSwapchainImagesKHR(
			logicalDevices.front(),
			m_swapChain,
			&swapchainImageCount,
			m_swapChainImages.data()
		),
		"vkGetSwapchainImagesKHR is failed!"
	);
}

void MyVulkan::TransitionImageLayout()
{
	const VkImageMemoryBarrier barrier =
	{
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,			//	sType
		nullptr, 																							//	pNext
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,				//	srcAccessMask
		VK_ACCESS_MEMORY_READ_BIT,										//	dstAccessMask
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		//	oldLayout
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,							//	newLayout
		0,																									//	srcQueueFamilyIndex
		0,																									//	dstQueueFamilyIndex
		sourceImage,																				//	image
		{																									//	subresourceRange
			VK_IMAGE_ASPECT_COLOR_BIT,										//	aspectMask
			0,																								//	baseMipLevel
			1,																								//	levelCount
			0,																								//	baseArrayLayer
			1																								//	layerCount
		}
	};

	vkCmdPipelineBarrier(
		cmdBuffer,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
		);
}

void MyVulkan::SavingPipelineCacheDataToFile(VkDevice device, VkPipelineCache cache, const char* fileName)
{
	size_t cacheDataSize;
	
	// Determine the size of the cache data.
	VulkanHelper::VkCheck(
		vkGetPipelineCacheData(device, cache, &cacheDataSize, nullptr),
		"first call to vkGetPipelineCacheData failed!"
	);

	if (cacheDataSize == 0)
	{
		// failed!
		// Handle to fail needed
		return;
	}

	FILE* pOutputFile;
	void* pData;

	// Allocate a temporary store for the cache data.
	pData = malloc(cacheDataSize);

	if (pData == nullptr)
	{
		return;
	}

	// Retrieve the actual data from the cache.
	VulkanHelper::VkCheck(
		vkGetPipelineCacheData(device, cache, &cacheDataSize, pData),
		"Second call to vkGetPipelineCacheData is failed!"
	);

	// Open the file as a binary file and write the data to it.
	pOutputFile = fopen(fileName, "wb");

	if (pOutputFile == nullptr)
	{
		free(pData);
		return;
	}

	fwrite(pData, 1, cacheDataSize, pOutputFile);

	fclose(pOutputFile);

	free(pData);
}

void MyVulkan::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutCreateInfo createInfo;
	VkDescriptorSetLayout layout;
	vkCreateDescriptorSetLayout(logicalDevices.front(),
		&createInfo,
		&myAllocator,
		&layout);
}

void MyVulkan::CreatePipelineLayout()
{
	// This describes our combined image-samplers.
	// One set, two disjoint binding

	static const VkDescriptorSetLayoutBinding samplers[] =
	{
		{
			0,																										// Start from binding 0
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// Combined image-sampler
			1, 																									// Create One binding
			VK_SHADER_STAGE_ALL,															// Usable in all stages
			nullptr																								// No static samplers
		},

		{
			2,																										// Start from binding 2
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,		// Combined image-sampler
			1,																										// Create one binding
			VK_SHADER_STAGE_ALL,															// Usable in all stages
			nullptr																								// No static samplers
		}
	};

	static const VkDescriptorSetLayoutBinding uniformBlock =
	{
		0, 																										// Start from binding 0
		VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,								// Uniform block
		1,																											// One binding
		VK_SHADER_STAGE_ALL,																// All stages
		nullptr																									// No static samplers
	};

	// Now create the two descriptor set layouts
	static const VkDescriptorSetLayoutCreateInfo createInfoSamplers =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		2,
		&samplers[0]
	};

	static const VkDescriptorSetLayoutCreateInfo createInfoUniforms =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		nullptr,
		0,
		1,
		&uniformBlock
	};

	// This array holds the two set layouts
	VkDescriptorSetLayout setLayouts[2];
	vkCreateDescriptorSetLayout(
		device,
		&createInfoSamplers,
		nullptr,
		&setLayouts[0]
	);
	vkCreateDescriptorSetLayout(
		device,
		&createInfoUniforms,
		nullptr,
		&setLayouts[1]
	);

	// Now create the pipeline layout.
	const VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,		// sType
		nullptr,																									// pNext
		0, 																										// flags
		2,																											// setLayoutCount
		setLayouts,																							// pSetLayouts
		0,																											// pushConstantRangeCount
		nullptr																									// pPushConstantRanges
	};

	VkPipelineLayout pipelineLayout;

	vkCreatePipelineLayout(
		device,
		&pipelineLayoutCreateInfo,
		nullptr,
		pipelineLayout
		)
}
*/

void MyVulkan::CreateSimpleRenderpass()
{
	// This is our color attachment. It's an R8G8B8A8_UNORM single sample image.
	// We want to clear it at the start of the renderpass and save the contents when we are done.
	// It starts in UNDEFINED layout, which is a key to Vulkan that it is allowed to throw the old content away,
	// and we want to leave it in COLOR_ATTACHMENT_OPTIMAL state when we are done.	

	static const VkAttachmentDescription attachments[] =
	{
		{
			0,																										// flags
			VK_FORMAT_R8G8B8A8_UNORM,												// format
			VK_SAMPLE_COUNT_1_BIT,														// samples
			VK_ATTACHMENT_LOAD_OP_CLEAR,										// loadOp
			VK_ATTACHMENT_STORE_OP_STORE,										// storeOp
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,								// stencilLoadOp
			VK_ATTACHMENT_STORE_OP_DONT_CARE,							// stencilStoreOp
			VK_IMAGE_LAYOUT_UNDEFINED,												// initialLayout
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL			// finalLayout
		}
	};

	// This is the single reference to our single attachment.
	static const VkAttachmentReference attachmentReferences[] =
	{
		{
			0,																									// attachment
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL		// layout
		}
	};

	// There is one subpass in this renderpass, with only a reference to the single output attachment.
	static const VkSubpassDescription subpasses[] =
	{
		{
			0,																		// flags
			VK_PIPELINE_BIND_POINT_GRAPHICS,	// pipelineBindPoint
			0,																		// inputAttachmentCount
			nullptr,																// pInputAttachments
			1,																		// colorAttachmentCount
			&attachmentReferences[0],								// pColorAttachments
			nullptr,																// pResolveAttachments
			nullptr,																// pDepthStencilAttachment
			0,																		// preserveAttachmentCount
			nullptr																// pPreserveAttachments
		}
	};

	// Finally, this is the information that Vulkan needs to create the render pass object.
	static VkRenderPassCreateInfo renderpassCreateInfo =
	{
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,			// sType
		nullptr,																									// pNext
		0,																											// flags
		1,																											// attachmentCount
		&attachments[0],																					// pAttachments
		1,																											// subpassCount
		&subpasses[0],																					// pSubpasses
		0,																											// dependencyCount
		nullptr																									// pDependencies
	};

	VkRenderPass renderPass = VK_NULL_HANDLE;

	// The only code that actually executes is this single call, which creates the renderpass object.
	vkCreateRenderPass(
		logicalDevices.front(),
		&renderpassCreateInfo,
		&myAllocator,
		&renderPass);
}

void MyVulkan::CreateSimpleGraphicsPipeline()
{
	VkPipelineShaderStageCreateInfo shaderStageCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,		// sType
		nullptr,																													// pNext
		0,																															// flags
		VK_SHADER_STAGE_VERTEX_BIT,																// stage
		module,																												// module
		"main",																													// pName
		nullptr																													// pSpecializationInfo
	};
	
	static const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO	,		// sType
		nullptr,																																// pNext
		0,																																		// flags
		0,																																		// vertexBindingDescriptionCount
		nullptr,																																// pVertexBindingDescriptions
		0,																																		// vertexAttributeDescriptionCount
		nullptr																																// pVertexAttributeDescriptions
	};

	static const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,		// sType
		nullptr,																																	// pNext
		0,																																			// flags
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST,																		// topology
		VK_FALSE																															// primitiveRestartEnable
	};

	static const VkViewport dummyViewport =
	{
		0.f, 0.f,						// x, y
		1.f, 1.f,						// width, height
		0.1f, 1000.f				// minDepth, maxDepth
	};

	static const VkRect2D dummyScissor =
	{
		{0, 0},						  // offset
		{1, 1}						  // extent
	};

	static const VkPipelineViewportStateCreateInfo viewportStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,			// sType
		nullptr,																															// pNext
		0,																																	// flags
		1,																																	// viewportCount
		&dummyViewport,																										// pViewports
		1,																																	// scissorCount
		&dummyScissor																											// pScissors
	};

	static const VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo =
	{
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,			// sType
		nullptr,																																	// pNext
		0,																																			// flags
		VK_FALSE,																															// depthClampEnable
		VK_TRUE,																															// rasterizerDiscardEnable
		VK_POLYGON_MODE_FILL,																								// polygonMode
		VK_CULL_MODE_NONE,																									// cullMode
		VK_FRONT_FACE_COUNTER_CLOCKWISE,																	// frontFace
		VK_FALSE,																															// depthBiasEnable
		0.f,																																		// depthBiasConstantFactor
		0.f,																																		// depthBiasClamp
		0.f,																																		// depthBiasSlopeFactor
		0.f																																		// lineWidth
	};

	static const VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo =
	{
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		nullptr,
		0,
		1,
		&shaderStageCreateInfo,
		&vertexInputStateCreateInfo,
		&inputAssemblyStateCreateInfo,
		nullptr,
		&viewportStateCreateInfo,
		&rasterizationStateCreateInfo,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		VK_NULL_HANDLE,
		renderpass,
		0,
		VK_NULL_HANDLE,
		0
	};

	VkPipeline pipeline;

	VulkanHelper::VkCheck(
		vkCreateGraphicsPipelines(logicalDevices.front(),
			VK_NULL_HANDLE,
			1,
			&graphicsPipelineCreateInfo,
			&myAllocator,
			&pipeline),
		"vkCreateGraphicsPipelines is failed!"
	);
}

void MyVulkan::DescribeVertexInputData()
{
	typedef struct vertex_t
	{
		glm::vec4 position;
		glm::vec3 normal;
		glm::vec2 texcoord;
	} vertex;

	static const VkVertexInputBindingDescription vertexInputBindings[] =
	{
		{ 0, sizeof(vertex), VK_VERTEX_INPUT_RATE_VERTEX }		// Buffer
	};

	// need to improve to use comfortably
	static const VkVertexInputAttributeDescription vertexAttributes[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },																	// position
		{ 1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, normal) },								// normal
		{ 2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex, texcoord) }								// texcoord
	};

	static const VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = 
	{
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		nullptr,
		0,
		sizeof(vertexInputBindings) / sizeof(vertexInputBindings[0]),
		vertexInputBindings,
		sizeof(vertexAttributes) / sizeof(vertexAttributes[0]),
		vertexAttributes
	}
}
