#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

extern VkAllocationCallbacks *gAllocation;

VkDevice InitForVkDevice();
