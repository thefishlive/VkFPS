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

#include "u_io.h"

#include <iostream>

#define ASSET_PATH "../../resources/"
#define FILENAME_TO_PATH(name) ASSET_PATH + name

bool read_file(std::string filename, file_data *data)
{
	FILE *file;
	bool result;

	std::string path = FILENAME_TO_PATH(filename);
	void *buffer;
	size_t length, read_size;

	file = fopen(path.c_str(), "r");
	if (!file)
	{
		std::cerr << "Error reading file " << path << " " << std::strerror(errno) << std::endl;
		result = false;
		goto err_out;
	}

	fseek(file, 0, SEEK_END);
	length = ftell(file);
	rewind(file);

	buffer = malloc(length);

	if (!buffer)
	{
		std::cerr << "Error allocating buffer "  << std::endl;
		result = false;
		goto err_close;
	}

	read_size = fread(buffer, sizeof(char), length, file);

	if (read_size != length)
	{
		std::cerr << "Error reading file " << path << " " << std::strerror(errno) << std::endl;
		result = false;
		goto err_out;
	}

	data->size = length;
	data->data = buffer;
	result = true;

err_close:
	if (fclose(file) != 0)
	{
		std::cerr << "Error closing file " << path << " " << std::strerror(errno) << std::endl;
		result = false;
		goto err_out;
	}

err_out:
	return result;
}
