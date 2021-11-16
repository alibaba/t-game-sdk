#!/usr/bin/python

# Copyright (C) 2021-2014 Alibaba Group Holding Limited

import os
import shutil
try :
	import requests
except:
	os.system("pip3 install requests")
	import requests

# curl -o file url is very unstable!!!! replace it
def download(url, file):
	# pass URL as first argument
	response = requests.head(url, allow_redirects=True)
	size = int(response.headers.get('content-length', -1))

	if os.path.exists(file):
		os.remove(file)

	f = open(file, "wb")

	per_chunck = 500 * 1024
	current_byte = 0
	while current_byte < size:
		load_from = current_byte
		load_to = load_from + per_chunck
		if load_to > size:
			load_to = size

		headers = {'Range': 'bytes={}-{}'.format(load_from, load_to - 1)}
		response = requests.get(url, headers=headers, timeout=30000)

		while response.status_code != 200 and response.status_code != 206:
			response = requests.get(url, headers=headers, timeout=30000)
		f.write(response.content)
		current_byte = load_to

		if current_byte < 10240:
			print("%s loaded %db" % (url, current_byte))
		else:
			print("%s loaded %dk" % (url, current_byte / 1024))

	f.flush()
	f.close()


script_root = os.path.abspath(os.path.dirname(__file__))

if __name__ == '__main__':
	grand_parent_dir = os.path.basename(os.path.dirname(script_root))
	if grand_parent_dir != "toolchain":
		exit(1)

	install_dir = os.path.join(script_root, "_install_")
	if not os.path.exists(install_dir):
		os.makedirs(install_dir)

	# freetype
	os.chdir(install_dir)

	if not os.path.exists(os.path.join(script_root, "libz", "include")):
		chipmunk_local = os.path.join(install_dir, "libz")
		if os.path.exists(chipmunk_local):
			shutil.rmtree(chipmunk_local)
		zlib_local = os.path.abspath(os.path.join(install_dir, "zlib-1.2.11.tar.gz"))
		if os.path.exists(zlib_local):
			os.remove(zlib_local)
		download("http://www.zlib.net/fossils/zlib-1.2.11.tar.gz", zlib_local)
		os.system("gunzip -c zlib-1.2.11.tar.gz | tar xopf -")

		include_dir = os.path.join(script_root, "libz", "include")
		os.makedirs(include_dir)

		zlib_uncompress_local = os.path.abspath(os.path.join(install_dir, "zlib-1.2.11"))
		shutil.copyfile(os.path.join(zlib_uncompress_local, "zconf.h"), os.path.join(include_dir, "zconf.h"))
		shutil.copyfile(os.path.join(zlib_uncompress_local, "zlib.h"), os.path.join(include_dir, "zlib.h"))

	os.chdir(install_dir)
	if not os.path.exists(os.path.join(script_root, "libjpg", "include")):
		chipmunk_local = os.path.join(install_dir, "libjpg")
		if os.path.exists(chipmunk_local):
			shutil.rmtree(chipmunk_local)
		jpeg_local = os.path.abspath(os.path.join(install_dir, "libjpg.zip"))
		if os.path.exists(jpeg_local):
			os.remove(jpeg_local)
		download("http://jenova-public.cn-hangzhou.oss-cdn.aliyun-inc.com/thirdparty/libjpg.zip", jpeg_local)
		os.system("unzip libjpg.zip")

		lib_dir = os.path.join(script_root, "libjpg")
		os.makedirs(lib_dir)

		jpeg_uncompress_local = os.path.abspath(os.path.join(install_dir, "libjpg"))
		shutil.copytree(os.path.join(jpeg_uncompress_local, "include"), os.path.join(lib_dir, "include"))

	os.chdir(install_dir)
	if not os.path.exists(os.path.join(script_root, "freetype", "include")):
		freetype_gz_local = os.path.abspath(os.path.join(install_dir, "freetype-2.10.0.tar.gz"))
		if os.path.exists(freetype_gz_local):
			os.remove(freetype_gz_local)
		download("http://mirror.yongbok.net/nongnu/freetype/freetype-2.10.0.tar.gz", freetype_gz_local)
		os.system("gunzip -c freetype-2.10.0.tar.gz | tar xopf -")
		freetype_uncompress_local = os.path.abspath(os.path.join(install_dir, "freetype-2.10.0"))
		if os.path.exists(freetype_uncompress_local):
			shutil.copytree(os.path.join(freetype_uncompress_local, "include"), os.path.join(script_root, "freetype", "include"))

	### os.system("git clone XXX") is super unstable!! why???
	os.chdir(install_dir)
	if not os.path.exists(os.path.join(script_root, "chipmunk", "include")):
		chipmunk_local = os.path.join(install_dir, "Chipmunk2D")
		if os.path.exists(chipmunk_local):
			shutil.rmtree(chipmunk_local)
		os.system("git clone https://github.com/slembcke/Chipmunk2D.git")
		os.chdir(chipmunk_local)
		os.system("git checkout -b Chipmunk-7.0.3")
		if os.path.exists(os.path.join(chipmunk_local, "include")):
			shutil.copytree(os.path.join(chipmunk_local, "include"), os.path.join(script_root, "chipmunk", "include"))

	os.chdir(install_dir)
	if not os.path.exists(os.path.join(script_root, "box2d", "include")):
		chipmunk_local = os.path.join(install_dir, "box2d")
		if os.path.exists(chipmunk_local):
			shutil.rmtree(chipmunk_local)
		os.system("git clone https://github.com/erincatto/box2d.git")
		os.chdir(chipmunk_local)
		os.system("git checkout -b v2.4.1")
		if os.path.exists(os.path.join(chipmunk_local, "include")):
			shutil.copytree(os.path.join(chipmunk_local, "include"), os.path.join(script_root, "box2d", "include"))

	os.chdir(install_dir)
	if not os.path.exists(os.path.join(script_root, "libpng", "include")):
		chipmunk_local = os.path.join(install_dir, "libpng")
		if os.path.exists(chipmunk_local):
			shutil.rmtree(chipmunk_local)
		os.system("git clone https://github.com/glennrp/libpng.git")
		os.chdir(chipmunk_local)
		os.system("git checkout -b libpng16")
		os.system("./configure")
		os.system("make")

		include_dir = os.path.join(script_root, "libpng", "include")
		os.makedirs(include_dir)
		shutil.copyfile(os.path.join(chipmunk_local, "png.h"), os.path.join(include_dir, "png.h"))
		shutil.copyfile(os.path.join(chipmunk_local, "pngconf.h"), os.path.join(include_dir, "pngconf.h"))
		shutil.copyfile(os.path.join(chipmunk_local, "pnglibconf.h"), os.path.join(include_dir, "pnglibconf.h"))

	# if not os.path.exists(os.path.join(script_root, "chipmunk", "include")):
	# 	chipmunk_gz_local_tmp = os.path.abspath(os.path.join(install_dir, "Chipmunk2D-Chipmunk-7.0.3.tar.gz.tmp"))
	# 	chipmunk_gz_local = os.path.abspath(os.path.join(install_dir, "Chipmunk2D-Chipmunk-7.0.3.tar.gz"))
	# 	if os.path.exists(chipmunk_gz_local):
	# 		os.remove(chipmunk_gz_local)
	# 	download("https://codeload.github.com/slembcke/Chipmunk2D/tar.gz/refs/tags/Chipmunk-7.0.3", chipmunk_gz_local_tmp)
	# 	download("https://github.com/slembcke/Chipmunk2D/archive/refs/tags/Chipmunk-7.0.3.tar.gz", chipmunk_gz_local)
	# 	os.system("gunzip -c Chipmunk2D-Chipmunk-7.0.3.tar.gz | tar xopf -")
	#
	# 	chipmunk_uncompress_local = os.path.abspath(os.path.join(install_dir, "Chipmunk2D-Chipmunk-7.0.3"))
	# 	if os.path.exists(os.path.join(chipmunk_uncompress_local, "include")):
	# 		shutil.copytree(os.path.join(chipmunk_uncompress_local, "include"), os.path.join(script_root, "chipmunk", "include"))
	#
	# if not os.path.exists(os.path.join(script_root, "box2d", "include")):
	# 	chipmunk_gz_local = os.path.abspath(os.path.join(install_dir, "box2d-2.4.1.tar.gz"))
	# 	if os.path.exists(chipmunk_gz_local):
	# 		os.remove(chipmunk_gz_local)
	# 	download("http://github.com/erincatto/box2d/archive/refs/tags/v2.4.1.tar.gz", chipmunk_gz_local)
	# 	os.system("gunzip -c box2d-2.4.1.tar.gz | tar xopf -")
	#
	# 	chipmunk_uncompress_local = os.path.abspath(os.path.join(install_dir, "box2d-2.4.1"))
	# 	if os.path.exists(os.path.join(chipmunk_uncompress_local, "include")):
	# 		shutil.copytree(os.path.join(chipmunk_uncompress_local, "include"), os.path.join(script_root, "box2d", "include"))
	#
	# if not os.path.exists(os.path.join(script_root, "libpng", "include")):
	# 	chipmunk_gz_local = os.path.abspath(os.path.join(install_dir, "libpng-1.6.37.tar.gz"))
	# 	if os.path.exists(chipmunk_gz_local):
	# 		os.remove(chipmunk_gz_local)
	# 	download("http://github.com/glennrp/libpng/archive/refs/tags/v1.6.37.tar.gz", chipmunk_gz_local)
	# 	os.system("gunzip -c libpng-1.6.37.tar.gz | tar xopf -")
	# 	chipmunk_uncompress_local = os.path.abspath(os.path.join(install_dir, "libpng-1.6.37"))
	# 	os.chdir(chipmunk_uncompress_local)
	# 	os.system("./configure")
	# 	os.system("make")
	# 	os.chdir(install_dir)
	#
	# 	include_dir = os.path.join(script_root, "libpng", "include")
	# 	os.makedirs(include_dir)
	# 	shutil.copyfile(os.path.join(chipmunk_uncompress_local, "png.h"), os.path.join(include_dir, "png.h"))
	# 	shutil.copyfile(os.path.join(chipmunk_uncompress_local, "pngconf.h"), os.path.join(include_dir, "pngconf.h"))
	# 	shutil.copyfile(os.path.join(chipmunk_uncompress_local, "pnglibconf.h"), os.path.join(include_dir, "pnglibconf.h"))

	if os.path.exists(install_dir):
		shutil.rmtree(install_dir)

