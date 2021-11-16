#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

# not stable. do not use it!!!!

import sys
import os
import zipfile

script_root = os.path.abspath(os.path.dirname(__file__))


def find_project_name(build_dir):
    files = os.listdir(build_dir)
    for f in files:
        if f.endswith(".wasm"):
            basename = os.path.basename(f)
            return os.path.splitext(basename)[0]
    return None


if __name__ == '__main__':
    current_dir = os.getcwd()

    publish_dir = sys.argv[1]
    project_name = find_project_name(publish_dir)
    if project_name:
        if os.path.exists(os.path.join(publish_dir, "%s.js" % project_name)):
            os.remove(os.path.join(publish_dir, "%s.js" % project_name))

        os.chdir(publish_dir)
        f = zipfile.ZipFile('%s.zip' % project_name, 'w', zipfile.ZIP_DEFLATED)
        for dirpath, dirnames, filesnames in os.walk(publish_dir):
            for filename in filesnames:
                f.write(os.path.join(dirpath, filename))
        f.close()
    else:
        print("cannot find .wasm file, something is error!")

    os.chdir(current_dir)


