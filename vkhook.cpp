#include <funchook.h>
#include <stdexcept>
#include <thread>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
VkAllocationCallbacks *gAllocation = nullptr;

VkInstance GetInstance() {
  VkInstanceCreateInfo createInfo = {};
  VkInstance instance;
  constexpr const char *extensions = "VK_KHR_surface"; // used by imgui

  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.enabledExtensionCount = 1;
  createInfo.ppEnabledExtensionNames = &extensions;

  auto err = vkCreateInstance(&createInfo, gAllocation, &instance);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Create Instnace err");
  }
  return instance;
}

VkPhysicalDevice GetPhysicalDevice(VkInstance instance) {
  uint32_t gpuCount;
  auto err = vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr);
  VkPhysicalDevice devices[gpuCount];
  err = vkEnumeratePhysicalDevices(instance, &gpuCount, devices);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("EnumPhysicalDevices");
  }
  for (int i = 0; i < gpuCount; i++) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(devices[i], &properties);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
      return devices[i];
    }
  }

  if (gpuCount > 0) {
    return devices[0];
  }

  throw std::runtime_error("No physical device");
  return VK_NULL_HANDLE; // this wont be ran;
}

uint32_t GetQueueFamily(VkPhysicalDevice pDevice) {
  uint32_t count;
  uint32_t queue;
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, nullptr);
  VkQueueFamilyProperties *queues = (VkQueueFamilyProperties *)malloc(
      sizeof(VkQueueFamilyProperties) * count);
  vkGetPhysicalDeviceQueueFamilyProperties(pDevice, &count, queues);
  for (uint32_t i = 0; i < count; i++) {
    if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      queue = i;
      break;
    }
  }
  free(queues);
  return queue;
}

VkDevice GetLogicalDevice(VkPhysicalDevice pDevice, uint32_t queueFamily) {
  VkDevice device;
  const char *extension = "VK_KHR_swapchain";
  uint32_t properties_count;
  std::vector<VkExtensionProperties> properties;
  vkEnumerateDeviceExtensionProperties(pDevice, nullptr, &properties_count,
                                       nullptr);
  properties.resize(properties_count);
  auto err = vkEnumerateDeviceExtensionProperties(
      pDevice, nullptr, &properties_count, properties.data());
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Could not enumerate device extension properties");
  }
  const float queue_priority[] = {1.0f};
  VkDeviceQueueCreateInfo queue_info[1] = {};
  queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info[0].queueFamilyIndex = queueFamily;
  queue_info[0].queueCount = 1;
  queue_info[0].pQueuePriorities = queue_priority;
  VkDeviceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  create_info.queueCreateInfoCount = sizeof(queue_info) / sizeof(queue_info[0]);
  create_info.pQueueCreateInfos = queue_info;
  create_info.enabledExtensionCount = 1;
  create_info.ppEnabledExtensionNames = &extension;
  err = vkCreateDevice(pDevice, &create_info, gAllocation, &device);
  if (err != VK_SUCCESS) {
    throw std::runtime_error("Could not create logical device");
  }
  return device;
}

PFN_vkQueuePresentKHR oQueuePresent;
VkResult VKAPI_CALL queuePresentHook(VkQueue queue,
                                     const VkPresentInfoKHR *pPresentInfo) {
  asm("int3");
  return oQueuePresent(queue, pPresentInfo);
}

VkDevice InitForVkDevice() {
  VkInstance instance = GetInstance();
  VkPhysicalDevice physicalDevice = GetPhysicalDevice(instance);
  uint32_t queueFamily = GetQueueFamily(physicalDevice);
  return GetLogicalDevice(physicalDevice, queueFamily);
}

void main_thread() {
  VkDevice device = InitForVkDevice();

  auto oQueuePresent =
      (PFN_vkQueuePresentKHR)vkGetDeviceProcAddr(device, "vkQueuePresentKHR");

  auto funchook = funchook_create();
  auto rv = funchook_prepare(funchook, (void **)&oQueuePresent,
                             (void *)queuePresentHook);
  if (rv != 0) {
    throw std::runtime_error("Could not prepare queuePresentHook");
  }

  rv = funchook_install(funchook, 0);
  if (rv != 0) {
    throw std::runtime_error("Could not install hooks");
  }
}

__attribute__((constructor)) void entry() {
  std::thread t(main_thread);
  t.detach();
}
