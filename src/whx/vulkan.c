#define VK_USE_PLATFORM_XCB_KHR

#include "handle.h"
#include "whis/attr.h"

#include <vulkan/vulkan.h>

WHIS_EXPORT
wh_fnresult wh_create_vulkan_surface(VkSurfaceKHR *surface, wh_window *window,
				     VkInstance instance,
				     PFN_vkCreateXcbSurfaceKHR fp,
				     VkAllocationCallbacks *allocator)
{
	if (!surface || !window)
		return WHIS_INVARG;

	VkResult vkres = VK_SUCCESS;

	VkXcbSurfaceCreateInfoKHR createinfo = {
		.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
		.pNext = NULL,
		.connection = window->connection,
		.window = window->window
	};

	vkres = fp(instance, &createinfo, allocator, surface);
	if (vkres != VK_SUCCESS)
		return WHIS_FAILURE;

	return WHIS_SUCCESS;
}
