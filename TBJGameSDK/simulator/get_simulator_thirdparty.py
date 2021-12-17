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
	install_dir = os.path.join(script_root)
	local_zip = os.path.join(install_dir, "thirdparty.zip")
	os.chdir(install_dir)
	download("http://jenova-public.oss-cn-zhangjiakou.aliyuncs.com/ios_framework_dep/thirdparty.zip", local_zip)
	os.system("unzip thirdparty.zip")
	if os.path.exists(os.path.join(install_dir, "__MACOSX")):
		shutil.rmtree(os.path.join(install_dir, "__MACOSX"))
	os.remove(local_zip)


