from distutils.sysconfig import get_python_inc
import platform
import os
import ycm_core

# These are the compilation flags that will be used in case there's no
# compilation database set (by default, one is not set).
# CHANGE THIS LIST OF FLAGS. YES, THIS IS THE DROID YOU HAVE BEEN LOOKING FOR.
flags = [
        '-Wall',
        '-Wextra',
        '-Werror',
        '-Wno-long-long',
        '-Wno-variadic-macros',
        '-fexceptions',
        # THIS IS IMPORTANT! Without the '-x' flag, Clang won't know which language to
        # use when compiling headers. So it will guess. Badly. So C++ headers will be
        # compiled as C headers. You don't want that so ALWAYS specify the '-x' flag.
        # For a C project, you would set this to 'c' instead of 'c++'.
        '-x',
        'c++',
        ]

build_folders = [
        'build-debug',
        'build-relese',
]

SOURCE_EXTENSIONS = ['.cpp', '.cxx', '.cc', '.c', '.m', '.mm']

def DirectoryOfThisScript():
    return os.path.dirname(os.path.abspath(__file__))

def PathInProjectRoot(path):
    return os.path.join(DirectoryOfThisScript(), path)


database = None

for bf in build_folders:
    full = PathInProjectRoot(bf)
    if os.path.exists(full):
        print "using database in ", full
        database = ycm_core.CompilationDatabase(full)
        print "database successfully loaded: ", database.DatabaseSuccessfullyLoaded()


def IsHeaderFile(filename):
    extension = os.path.splitext(filename)[1]
    return extension in ['.h', '.hxx', '.hpp', '.hh', ".inc", ".impl"]


def IsSourceFile(filename):
    ext = os.path.splitext(filename)[1]
    return ext in SOURCE_EXTENSIONS


def GetSrcDirForInclude(filename):
    while filename != DirectoryOfThisScript():
        filename = os.path.dirname(filename)
        srcdir = os.path.join(filename, "src")
        if os.path.exists(srcdir):
            return srcdir
    return None


def TryFilesInFolder(folder, check):
    for root, _, files in os.walk(folder):
        for f in files:
            full = os.path.join(root, f)
            print "trying file: ", full
            if not check(f):
                print "file not fit: ", full
                continue

            ci = database.GetCompilationInfoForFile(full)
            print list(ci.compiler_flags_)
            if ci.compiler_flags_:
                print "found ci for file: ", full
                return ci
    return None



def GetFlagsForSourceForInclude(filename):
    base = os.path.splitext(os.path.basename(filename))[0]
    posiblenames = list(map(lambda ext: base + ext, SOURCE_EXTENSIONS))
    # looking for file near header
    ci = TryFilesInFolder(os.path.dirname(filename), lambda f: (f in posiblenames) or IsSourceFile(f))
    if ci:
        print "found flags in same dir: ", os.path.dirname(filename)
        return ci

    srcdir = GetSrcDirForInclude(filename)
    if srcdir is None:
        print "source dir not found"
        return None

    return TryFilesInFolder(srcdir, lambda f: (f in posiblenames) or IsSourceFile(f))


def GetCompilationInfoForFile(filename):
    ci = database.GetCompilationInfoForFile(filename)
    if ci.compiler_flags_:
        print "file found in db: ", filename
        return ci

    if IsHeaderFile(filename):
        return GetFlagsForSourceForInclude(filename)

    return GetFlagsForSourceForInclude(filename)


def FlagsForFile(filename, **kwargs):
    print "looking for flags for file:", filename
    if not database:
        return {'flags': flags, 'include_paths_relative_to_dir': DirectoryOfThisScript()}

    compilation_info = GetCompilationInfoForFile(filename)

    if not compilation_info:
        return None

    # Bear in mind that compilation_info.compiler_flags_ does NOT return a
    # python list, but a "list-like" StringVec object.
    final_flags = list(compilation_info.compiler_flags_)

    return {'flags': final_flags, 'include_paths_relative_to_dir': compilation_info.compiler_working_dir_}
