
import argparse
import xml.etree.ElementTree as ET
from cgen import *

COPYWRITE = '''/******************************************************************************
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
'''

def parse_vk_spec(args):
    exts = {}
    tree = ET.parse(args.vk_spec)
    root = tree.getroot()
    vk_exts = root.find("extensions")
    vk_cmds = root.find("commands")

    for ext in vk_exts.findall("extension"):
        ext_name = ext.get("name")
        if ext_name in args.extensions:
            print ("Found extension %s" % ext_name)
            exts[ext_name] = {'commands':{},'type':ext.get("type")}
            for enum in ext.iter("enum"):
                if enum.get("name").endswith("_NAME"):
                    exts[ext_name]['name'] = enum.get("name")
            for cmd in ext.iter("command"):
                name = cmd.get("name")
                print ("\tFound command %s" % name)
                exts[ext_name]['commands'][name] = {}
            print ("\n")

    for ext_name,_ in exts.items():
        for cmd in vk_cmds.findall("command"):
            name = cmd.find("proto").find("name").text
            if name in exts[ext_name]['commands']:
                exts[ext_name]['commands'][name]['type'] = cmd.find("proto").find("type").text
                exts[ext_name]['commands'][name]['params'] = []
                for param in cmd.findall("param"):
                    type = ""
                    for t in param.itertext():
                        if t is param.find("name").text:
                            continue
                        type += t.strip() + " "
                    exts[ext_name]['commands'][name]['params'].append({'name': param.find("name").text, 'type': type.strip()})

    return exts

def gen_hdr(args, exts):
    with File(args.hdr, FileType.HEADER) as hdr:
        hdr.copywrite(COPYWRITE)
        hdr.comment_block("GENERATED FILE DO NOT EDIT BY HAND")
        hdr.include("<vulkan/vulkan.h>")

        hdr.function("vk_ext_init_instance", "VkResult").arg("VkInstance", "instance").end()
        hdr.function("vk_ext_init_device", "VkResult").arg("VkDevice", "device").end()
        hdr.function("vk_is_ext_present", "VkBool32").arg("const char *", "ext").end()

        for ext_name,ext in exts.items():
            hdr.comment_block(ext_name)
            for cmd_name,cmd in ext['commands'].items():
                func = hdr.function(cmd_name[2:], cmd["type"])
                for param in cmd['params']:
                    func.arg(param['type'], param['name'])
                func.end()

    with File(args.src, FileType.SRC) as src:
        src.copywrite(COPYWRITE)
        src.comment_block("GENERATED FILE DO NOT EDIT BY HAND")
        src.include("\"vk_ext.h\"").new_line()
        src.include("<string.h>").new_line()
        src.include("\"vk_ext_internal.h\"").new_line()

        struct = src.struct("vk_ext_pfns")
        for ext_name,ext in exts.items():
            src.new_line().comment_block(ext_name)
            struct.macro_call("VK_EXT", ext['name'])
            for cmd_name,cmd in ext['commands'].items():
                struct.macro_call("VK_EXT_PFN", cmd_name[2:])
        struct.end()

        src.variable('struct vk_ext_pfns', 'g_ext_pfns').end()

        for ext_name,ext in exts.items():
            func = src.function("vk_ext_init_%s" % ext_name, "VkResult").arg("Vk%s" % ext['type'].capitalize(), "%s" % ext['type'].capitalize()).end_args()
            last_cmd = ""
            for cmd_name,cmd in ext['commands'].items():
                func.insert_code("\tVK_GET_EXT_METHOD(%s, %s);" % (ext['type'].capitalize(), cmd_name[2:]))
                last_cmd = cmd_name
            func.insert_code("\tVK_SET_EXT_ENABLED(%s, %s)" % (ext['name'], last_cmd[2:]))
            func.new_line()
            func.func_return('VK_SUCCESS').end()

        func = src.function("vk_ext_init_instance", "VkResult").arg("VkInstance", "instance").end_args()
        for ext_name,ext in exts.items():
            if ext['type'] == 'instance':
                func.insert_code('\tvk_ext_init_%s(instance);' % ext_name)
        func.new_line().func_return('VK_SUCCESS').end()

        func = src.function("vk_ext_init_device", "VkResult").arg("VkDevice", "device").end_args()
        for ext_name,ext in exts.items():
            if ext['type'] == 'device':
                func.insert_code('\tvk_ext_init_%s(device);' % ext_name)
        func.new_line().func_return('VK_SUCCESS').end()

        func = src.function("vk_is_ext_present", "VkBool32").arg("const char *", "ext").end_args()
        for ext_name,ext in exts.items():
            func.insert_code('\tif(strcmp(ext, %s) == 0)' % ext['name'])
            func.insert_code('\t{')
            func.insert_code('\t\treturn VK_IS_EXT_ENABLED(%s)' % ext['name'])
            func.insert_code('\t}')
        func.func_return('VK_FALSE')
        func.end()

        for ext_name,ext in exts.items():
            src.comment_block(ext_name)
            for cmd_name,cmd in ext['commands'].items():
                func = src.function(cmd_name[2:], cmd["type"])

                for param in cmd['params']:
                    func.arg(param['type'], param['name'])

                func.end_args()
                code = "\t"

                if cmd['type'] == 'VkResult':
                    code += 'VK_CALL_EXT_METHOD_RETURN'
                else:
                    code += 'VK_CALL_EXT_METHOD'

                code += '(%s' % cmd_name[2:]

                for param in cmd['params']:
                    code += ', %s' % param['name']

                code += ');'
                func.insert_code(code)
                func.end()

def parse_arguments():
    parser = argparse.ArgumentParser(description='Generate vk_ext files')
    parser.add_argument('hdr',
        help='The output header file');
    parser.add_argument('src',
        help='The output src file');
    parser.add_argument('vk_spec',
        help='The current vk.xml file');
    parser.add_argument('extensions',
        help='The current vk.xml file',
        nargs='+');
    return parser.parse_args()

def main():
    args = parse_arguments()
    exts = parse_vk_spec(args)
    gen_hdr(args, exts)

if __name__ == "__main__":
    main()
