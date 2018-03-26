from enum import Enum

class FileType(Enum):
    HEADER = 0
    SRC = 1

class File:
    def __init__(self, file, file_type):
        self.file = file
        self.file_type = file_type

    def __enter__(self):
        self.fd = open(self.file, 'w+')
        return self

    def __exit__(self, type, value, traceback):
        self.close()
        return

    def copywrite(self, text):
        self.fd.write(text)
        self.fd.write("\n")
        return self

    def comment_block(self, text):
        self.fd.write("/%s\n" % ("*" * (len(text) + 6)))
        self.fd.write(" *  %s  *\n" % text)
        self.fd.write(" %s/\n\n" % ("*" * (len(text) + 6)))
        return self

    def include(self, file):
        self.fd.write("#include %s\n" % file)
        return self

    def macro(self, name):
        return Macro(self.fd, self.file_type, name)

    def function(self, name, type):
        return Function(self.fd, self.file_type, name, type)

    def struct(self, name):
        return Struct(self.fd, self.file_type, name)

    def variable(self, type, name):
        return Variable(self.fd, self.file_type, type, name)

    def new_line(self):
        self.fd.write("\n")
        return self

    def close(self):
        self.fd.close()
        return self

class Struct:
    def __init__(self, fd, file_type, name):
        self.fd = fd
        self.file_type = file_type

        self.fd.write("struct %s {\n" % name)

    def variable(self, name, type):
        self.fd.write("\t%s %s;\n" % (type, name))

    def macro_call(self, macro, params):
        self.fd.write("\t%s(%s)\n" % (macro, params))

    def new_line(self):
        self.fd.write("\n")

    def end(self):
        self.fd.write("};\n\n")

class Function:
    def __init__(self, fd, file_type, name, type):
        self.fd = fd
        self.param_count = 0
        self.file_type = file_type
        self.args_completed = False

        self.fd.write("%s %s(\n" % (type, name))

    def arg(self, type, name):
        if (self.param_count != 0):
            self.fd.write(",\n")
        self.fd.write("\t%s %s" % (type, name))
        self.param_count += 1
        return self

    def comment(self, comment):
        if self.file_type is FileType.HEADER or not self.args_completed:
            return None
        self.fd.write("\t// %s\n" % comment)
        return self

    def new_line(self):
        self.fd.write("\n")
        return self

    def insert_code(self, code):
        if self.file_type is FileType.HEADER or not self.args_completed:
            return None

        self.fd.write("%s\n" % code)
        return self

    def func_return(self, code):
        if self.file_type is FileType.HEADER or not self.args_completed:
            return None

        self.fd.write("\treturn %s;\n" % code)
        return self

    def end_args(self):
        self.fd.write("\n)")
        if self.file_type is FileType.SRC:
            self.fd.write("\n{\n")
        self.args_completed = True
        return self

    def end(self):
        if self.file_type is FileType.SRC:
            self.fd.write("}\n\n")
        else:
            self.end_args()
            self.fd.write(";\n\n")
        return self

class Macro:
    def __init__(self, fd, file_type, name):
        self.fd = fd
        self.param_count = 0
        self.file_type = file_type

        self.fd.write("#define %s(" % (name))

    def arg(self, name):
        if (self.param_count != 0):
            self.fd.write(", ")
        self.fd.write("%s" % (name))
        self.param_count += 1
        return self

    def new_line(self):
        self.fd.write("\n")
        return self

    def end_args(self):
        self.fd.write(") ")
        return self

    def end(self):
        self.fd.write("\n\n")
        return self

class Variable:
    def __init__(self, fd, file_type, type, name):
        self.fd = fd
        self.file_type = file_type
        self.fd.write('%s %s' % (type, name))

    def end(self):
        self.fd.write(';\n\n')
        return self
