#! /usr/bin/python

import argparse
import string

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

HDR = '''{copywrite}
#ifndef SHADER_{guard}_H
#define SHADER_{guard}_H

/******************************************************************************
 * This file is generated from glsl source code, do not edit this file       *
 * Your changes will be lost upon regeneration of this file.                  *
 ******************************************************************************/

/*
 * The shader source code length
 */
const size_t g_shader_{shader}_size = {shader_size};

/*
 * The compiled shaders source code
 */
const char * g_shader_{shader}_code = {shader_code};

#endif /* SHADER_{guard}_H */
'''

def gen_hdr(args):
    code = ""
    count = 0
    max_len = 0

    with open(args.src) as src:
        for line in src:
            if len(line) > max_len:
                max_len = len(line)

        max_len += 8

    with open(args.src) as src:
        for line in src:
            line = line.rstrip()
            code += '\\\n\t"%s\\n"%s' % (line, ' ' * (max_len - len(line)))
            count += len(line) + 1

    with open(args.dst, 'w+') as dst:
        dst.write(HDR.format(copywrite=COPYWRITE,
                         shader=args.name,
                         shader_size=count,
                         shader_code=code,
                         guard=args.name.upper()))

def parse_arguments():
    parser = argparse.ArgumentParser(description='Convert a glsl file to a c header file')
    parser.add_argument('src',
                        help='The input glsl file');
    parser.add_argument('dst',
                        help='The output header file');
    parser.add_argument('name',
                        help='The shader name, for use in the c header');
    return parser.parse_args()

def main():
    args = parse_arguments()
    gen_hdr(args)
    print("Converted spir-v {src} to header {dst}".format(src=args.src, dst=args.dst))

if __name__ == "__main__":
    main()
