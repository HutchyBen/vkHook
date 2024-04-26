#include <funchook.h>
#include "vulkaninit.h"
#include <stdexcept>
#include <thread>

PFN_vkQueuePresentKHR oQueuePresent;
VkResult VKAPI_CALL queuePresentHook(VkQueue queue,
                                     const VkPresentInfoKHR *pPresentInfo) {
  asm("int3");
  return oQueuePresent(queue, pPresentInfo);
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
