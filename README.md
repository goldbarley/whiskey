## Description

**Whiskey** is a windowing and input library. The library currently only supports the 
[X Windoy System](https://en.wikipedia.org/wiki/X_Window_System) (using the 
[XCB API](https://xcb.freedesktop.org/XcbApi/)) and [Wayland](https://wayland.freedesktop.org/) 
(via XWayland). Support for Windows operating systems is absent but is planned for the future. 
Further documentation for the library will be written as soon as possible.

The following is a list of things this library is capable of:
1. **Creating and destroying windows**: Graphical windows can be created by using the
`wh_create_window` function. This function returns a pointer to the window handle, otherwise
returns `NULL` or `0` on failure.

2. **Getting events**: The library features two ways of getting events from a window: (a)
using `wh_poll_event` (non-blocking and high CPU usage) and (b) `wh_wait_event` (blocking and
low CPU usage).

3. **The event structure**: It is a non-opaque structure defined as `struct wh_event`. The structure
contains the pointer to the window from which the event comes from, the type of event, and other
event details depending on the event.

4. **Key and button mappings**: The library maps the keys as in the
[HID Usage Table for USB](https://www.usb.org/sites/default/files/hut1_21.pdf). Button mappings
are custom and do not follow any standard convention.

5. **Vulkan compatibility**: This library is primarily meant to be used alongside
[Vulkan](https://en.wikipedia.org/wiki/Vulkan) applications. Therefore, it directly provides a 
function to create Vulkan surfaces depending on the platform using the `wh_create_vulkan_surface`
function.

## Obtaining sources
This library uses no submodules hence a simple `git clone` command from either mirrors will
fetch the files from the repository. For example:
```
git clone https://codeberg.org/nilabhro/whiskey
```
Alternatively, if you want to use the GitHub mirror, then:
```
git clone https://github.com/goldbarley/whiskey
```
Then change your currently directory to the newly fetched one with:
```
cd whiskey
```
Now you can freely browse the source files.

## Building
The library uses [CMake](https://cmake.org/) as its build system.
### Dependencies (required!)
- [Vulkan SDK](https://vulkan.lunarg.com/).
- [pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config)
- [XCB developer API and headers](https://xcb.freedesktop.org/dist/).
- ISO C11 or higher.
- [CMake version 3.15.7](https://cmake.org/files/v3.15/) or [higher](https://cmake.org/download/).

For compilation, either [GCC](https://gcc.gnu.org/) or [Clang](https://clang.llvm.org/) is
preferred. Otherwise any other standard C compiler will also work.

### Build options
|Option|Type|Description|
|:--:|:--:|:--|
|[`CMAKE_BUILD_TYPE`](https://cmake.org/cmake/help/latest/variable/CMAKE_BUILD_TYPE.html)|String|Defines the type of build. If a build type is not set then it will default to `Release`.
|`WHIS_PLATFORM`|String|This is a mandatory option to set. If left blank, the build will fail. This defines the platform for which one's building the project for. The available options being: `X11`, `WAYLAND`, and `WIN32`.|
|`WHIS_LIB_TYPE`|String|This defines the type of library that the project compiles to. It can either be a shared library, a static library, or an executable. The valid values for this option are: `STATIC`, `SHARED`, or `EXEC`. If a value for this option is not set, it will default to either `STATIC` if the project is **not** at the top level or `EXEC` if the project **is** at the top level.
|`WHIS_INCLUDE_TESTS`|Boolean|This value decides whether the build should include the test files for this project or not. It has two values, either `ON` or `OFF`. If the project is at the top level or is compiled to an executable, the test files will be included regardless of the value for this option.|

Having gone through all the options, you can choose to either leave it at their default values
for convenience or set each value explicitly. 

**Build command**:
```
cmake -S . -B build -DWHIS_PLATFORM=X11 -DWHIS_LIB_TYPE=STATIC
```
**Compile command**:
This can vary from platform to platform depending on what kind of compiler you have. But
you can let CMake do the compilation for you using:
```
cmake --build build
```
You will have your static library inside the newly created `build` directory.

## API usage
The following is a small code snippet showing a basic and one of many ways to use this library.
```
int main(void)
{
	/* Define the properties of the window. */
	const uint32_t width = 1280;
	const uint32_t height = 720;
	const char *title = "My window";

	/* Create the window. */
	wh_window *window = wh_create_window(width, height, title);
	/* Check if it returns a valid pointer */
	if (!window)
		return -1;

	/* Declare an event structure to store the events. */
	struct wh_event event = {0};

	/* Run the loop while the program still has active windows. */
	while (wh_get_nwin())
	{
		/* Poll or wait for events. This example uses polling. */
		while (wh_poll_event(&event))
		{
			/* Handle the event */
		}
	}

	/* Destroy the window when done. */
	wh_destroy_window(window);

	/* Shutdown and free any and all resources. */
	wh_shutdown();

	return 0;
}
```
> **Note** `wh_shutdown` will automatically destroy all active windows if called anywhere
mid-execution. So, whle it is not explicitly necessary to destroy each window one by one
right before shutdown, it is recommend that you do so if you want no hidden behaviours.

## Repositories
- [**Codeberg**](https://codeberg.org/nilabhro/whiskey)
- [**GitHub**](https://github.com/goldbarley/whiskey)

### *This project is licensed under the terms of the [MIT license](https://en.wikipedia.org/wiki/MIT_License).*
