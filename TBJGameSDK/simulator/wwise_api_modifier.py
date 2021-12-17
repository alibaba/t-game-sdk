#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

import os
import sys
import shutil


def find_and_insert(filepath, contents):
    if not os.path.exist(filepath):
        return

    filepath_copy = filepath + ".wwise_oringle_include"
    if not os.path.exist(filepath_copy):
        os.copyfile(filepath, filepath_copy)

    f = open(filepath_copy, "r")
    lines = f.readlines()
    f.close()

    for insert_content_wrap in contents:
        search_content = insert_content_wrap["search"]
        insert_content = insert_content_wrap["content"]
        ingore_offset = insert_content_wrap["offset"]
        for i in range(0, len(lines)):
            line = lines[i]
            find_idx = line.find(search_content)
            if find_idx >= 0:
                lines[i] = search_content[0:find_idx] + insert_content + search_content[find_idx + ingore_offset:]
                break

    f = open(filepath_copy, "w")
    f.writelines(lines)
    f.flush()
    f.close()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("usage: python3 wwise_api_modifier.py ${WWise_Include_dir}")
        exit(1)
    wwise_include_dir = sys.argv[1]
    files = [
        {
            "path": "AK/AkPlatforms.h",
            "contents": [
                {
                    "search": "#if defined( NN_PLATFORM_CTR )",
                    "content": '#if defined(__EMSCRIPTEN__) || defined(JENOVA_SDK)\n'
                               '	#include <AK/SoundEngine/Platforms/Android/AkTypes.h>\n'
                               '#el',
                    "offset": 1
                }
            ]
        }
    ]

    for to_modified_file in files:
        file_path = to_modified_file["path"]
        abs_path = os.path.abspath(os.path.join(wwise_include_dir, file_path))
        find_and_insert(abs_path, to_modified_file["contents"])
