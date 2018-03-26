/******************************************************************************
 * Copyright 2017 James Fitzpatrick <james_fitzpatrick@outlook.com>           *
 *                                                                            *
 * Permission is hereby granted, free of charge, to any person obtaining a    *
 * copy of this software and associated documentation files (the "Software"), *
 * to deal in the Software without restriction, including without limitation  *
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,   *
 * and/or sell copies of the Software, and to permit persons to whom the      *
 * Software is furnished to do so, subject to the following conditions:       *
 *                                                                            *
 * The above copyright notice and this permission notice shall be included in *
 * all copies or substantial portions of the Software.                        *
 *                                                                            *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR *
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   *
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    *
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER *
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    *
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        *
 * DEALINGS IN THE SOFTWARE.                                                  *
 ******************************************************************************/
#ifndef VKCRAFT_VK_EXT_INTERNAL
#define VKCRAFT_VK_EXT_INTERNAL

#include <vulkan/vulkan.h>

#define VK_EXT(ext) VkBool32 b##ext;
#define VK_EXT_PFN(cmd) PFN_vk##cmd pfn##cmd;

#define VK_SET_EXT_ENABLED(ext, func) g_ext_pfns.b##ext = g_ext_pfns.pfn##func != VK_NULL_HANDLE;
#define VK_IS_EXT_ENABLED(ext) g_ext_pfns.b##ext;

#define VK_GET_EXT_METHOD(type, func)                                                                                                      \
    g_ext_pfns.pfn##func = (PFN_vk##func) vkGet##type##ProcAddr(type, "vk" #func);                                                         \
    if (g_ext_pfns.pfn##func == VK_NULL_HANDLE)                                                                                            \
        return VK_ERROR_EXTENSION_NOT_PRESENT;

#define VK_CALL_EXT_METHOD(func, ...)                                                                                                      \
    if (g_ext_pfns.pfn##func == VK_NULL_HANDLE)                                                                                            \
        return;                                                                                                                            \
    g_ext_pfns.pfn##func(__VA_ARGS__);                                                                                                     \
    return;

#define VK_CALL_EXT_METHOD_RETURN(func, ...)                                                                                               \
    if (g_ext_pfns.pfn##func == VK_NULL_HANDLE)                                                                                            \
        return VK_ERROR_EXTENSION_NOT_PRESENT;                                                                                             \
    return g_ext_pfns.pfn##func(__VA_ARGS__);

#endif /* VKCRAFT_VK_EXT_INTERNAL */
