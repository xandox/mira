#!/usr/bin/env python3

import sys
import os
import argparse
import subprocess
import shutil
import logging

BUILD_DIRS = {
        "release": "build-release",
        "debug": "build-debug",
}

CXX_FLAGS = [
        "-stdlib=libc++",
        "-fcoroutines-ts",
        "-std=c++17",
        ]

COMMON_CMAKE_ARGS = [
        "-GNinja",
        "-DCMAKE_C_COMPILER=clang-6.0",
        "-DCMAKE_LINKER=ld.gold",
        "-DCMAKE_CXX_COMPILER=clang++-6.0",
        "-DCMAKE_CXX_FLAGS=-stdlib=libc++ -fcoroutines-ts -std=c++17",
        "-DCMAKE_EXE_LINKER_FLAGS=-lc++abi -stdlib=libc++",
        "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
]

CMAKE_ARGS = {
        "relase": [
                "-DCMAKE_INSTALL_PREFIX=release-install",
                "-DCMAKE_BUILD_TYPE=Release",
                "-DCMAKE_CXX_FLAGS=-ggdb3 -O0 " + " ".join(CXX_FLAGS),
            ],
        "debug": [
                "-DCMAKE_INSTALL_PREFIX=debug-install",
                "-DCMAKE_BUILD_TYPE=Debug",
                "-DCMAKE_CXX_FLAGS=-O3 " + " ".join(CXX_FLAGS),
            ],
}


def run_ninja(cwd):
    logging.debug("running ninja")
    subprocess.check_call(["ninja"], cwd=cwd)


def disable_build(directory):
    logging.debug("disabling build directory {directory}".format(directory=directory))
    if not os.path.exists(directory):
        return

    shutil.move(directory, "." + directory)


def enable_build(directory):
    logging.debug("enabling build directory {directory}".format(directory=directory))
    if not os.path.exists("." + directory):
        return

    shutil.move("." + directory, directory)


def do_configure(directory, cmake_args):
    enable_build(directory)
    if not os.path.exists(directory):
        os.mkdir(directory)

    args = ["cmake"] + cmake_args + [os.path.dirname(os.path.abspath(__file__))]
    logging.debug("running cmake with args: {}".format(" ".join(args)))
    subprocess.check_call(args, cwd=directory)



def configure(build_type):
    cmake_args = COMMON_CMAKE_ARGS + CMAKE_ARGS[build_type]
    do_configure(BUILD_DIRS[build_type], cmake_args)


def build_debug(args):
    logging.info("building debug")
    disable_build(BUILD_DIRS["release"])
    configure("debug")
    run_ninja(BUILD_DIRS["debug"])


def build_release(args):
    logging.info("building debug")
    disable_build(BUILD_DIRS["debug"])
    configure("release")
    run_ninja(BUILD_DIRS["release"])


def parse_arguments():
    parser = argparse.ArgumentParser(description="Build arguments")
    subparsers = parser.add_subparsers(help="variants")

    debug = subparsers.add_parser("debug", aliases=['d'], )
    debug.set_defaults(func=build_debug)
    release = subparsers.add_parser("release", aliases=['r'])
    release.set_defaults(func=build_release)

    return parser.parse_args(sys.argv[1:])


def main():
    args = parse_arguments()
    args.func(args)


if __name__ == '__main__':
    main()
