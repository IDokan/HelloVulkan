/******************************************************************************
Copyright (C) 2021 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Allocator.cpp
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 06.13.2021
	Source file for vulkan allocator.
******************************************************************************/

#include <malloc.h>
#include "Graphics/Allocator/Allocator.h"

void* allocator::Allocation(size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return _aligned_malloc(size, alignment);
}

void* VKAPI_CALL allocator::Allocation(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return static_cast<allocator*>(pUserData)->Allocation(size, alignment, allocationScope);
}

void* allocator::Reallocation(void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return _aligned_realloc(pOriginal, size, alignment);
}

void* VKAPI_CALL allocator::Reallocation(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope allocationScope)
{
	return static_cast<allocator*>(pUserData)->Reallocation(pOriginal, size, alignment, allocationScope);
}

void allocator::Free(void* pMemory)
{
	_aligned_free(pMemory);
}

void VKAPI_CALL allocator::Free(void* pUserData, void* pMemory)
{
	return static_cast<allocator*>(pUserData)->Free(pMemory);
}