#ifndef WHIS_VULKAN_H_
#define WHIS_VULKAN_H_ 1

#include "attr.h"
#include "types.h"

#include <vulkan/vulkan.h>

WHIS_EXPORT
wh_fnresult wh_create_vulkan_surface(VkSurfaceKHR *surface, wh_window *window,
				     VkInstance instance,
				     VkAllocationCallbacks *allocator);

#endif /* WHIS_VULKAN_H_ */
