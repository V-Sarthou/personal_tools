#!/usr/bin/env python

import urllib.request
import shutil
import os
import time

def init_dir(dir):
  try:
    os.stat(dir)
  except:
    os.makedirs(dir)

website = "cdn-1.matterport.com"
model = "dcd6ce22a2e5405db1c42590b0257d8a"
token = "2-680281b871ccc4318cdcffb568aa2531c5e4390c-1624020890-0"

url_pattern = "https://{website}/models/{model}/assets/~/tiles/{sweep}/{res}_face{face}_{i}_{j}.jpg?t={token}"

sweeps = [
  "20e2ff25f2ab46858caab730d97fb276",
  "63d6ce363f0440a4b9f9d15e284c3620",
  "b71f33fe3d3d4cac97920a4c06f7a254",
  "ff61f1ebb37e4065b297dd3d1cd009c4",
  "775a8558790a4d40818d06866abc68b9",
  "128335395b7b450f865fc140b8192f60",
  "147b649c59344ecca87b169334d539c8",
  "2ed3b1be759a451a908409d47783f3c3",
  "af1c992d71ed464b9faf8933c3b2870c",
  "672c25188f3343088ee1b0a27fb48ef0",
  "35e9596f87a74bbc851652a7188204a6",
  "d0565618f496423fa67a71cb0702aa1a",
  "bc8acd46177541b49dcbe011436c0f4d",
  "b05a8c0a0eb941cc8b452921124da6fc",
  "f4718db7aed442df90e445793484e5b9",
  "9ab568a7a09240df9cdc52c6b25dca92",
  "0962767fa3f740f0bb216ab7af73206d",
  "2067632c4913467181b9b3037732a688",
  "e2f6d2f6a491409e8835020feb00bbb0",
  "c0d8d57b255746e2990f9c9db7c4f82b",
  "829859b0081f4d0988b4b7f0476cde3e",
  "d27d55145e134d3c91d8b019b60a660d",
  "8dc5a79ea4cb4a3c838f0b82a353122e",
  "decb4af2ab9c4bb3b0f720e6f50f8e90",
  "ac6408fa4fb043f58e97b15e70007c79",
  "1ce4cae1ef5a4eab97cd262812d84a53",
  "afec63ff38fc4942a2a4178cfb6ef670",
  "863aed9ba8af4d8e9d9233eba3fd6eef",
  "b00f54ad39854df9b731b6e2c2c0c5e2",
  "fd7015188457439f9398f5b255dd98f0",
  "5fded0e8df7b4807a232fa0d45f705e5",
  "8db6d667959a41dc937c1388f6a3689e",
  "e0533b8b6dbb45aa86529e69e015787d",
  "6381bd180e014b2aac6522307902bc0b",
  "59fec5a895b1410c8633571f09e9a45c",
  "b78c4b3ba80c4daba9526bf41b959e7c",
  "d38cf06ee0a649529a0272fd379b8cea",
  "74f55c53c9604fa194c75cfecb84c9e5",
  "9209643f98264dbb80590417101ac91c",
  "7272ed2fe03a4acc8083f9f96e9510e9",
  "0273ad0ab3484e609405b8a5c93d35eb",
  "6a1ee33f7a87488eaa4d584dd29d8b66",
  "a8eb7ff9a5cf447ab92e240e7ab0e40a",
  "98b156845ea848a6b0239d3c137c08b1",
  "dd0251cb11a4412ca7ba68c15f246bcd",
  "ca375116b30d41e5a6f92e77ba0c16e5",
  "789a0af2e6a14a82975a3cf4b2dc8673",
  "9533e87a848a4e6e8935cfdd4d946401",
  "202e4ac75bba4e668ac06b570a56516d",
  "53df32808fb04917b81fdbb3302576f7",
  "bc5b803bb11548eea7a6c2a961540ec6",
  "3d9f568f19ab4aae841ec10e10d8bc70",
  "39b7ad3522674ecdab7c40f08855fe8d",
  "ccda9f2ac9d643fcabf8aeadbf4f3df7",
  "e12d105c200c45ef99773a34e68b6908",
  "eaa783a310924b3d9b7c9cf5a99d2310",
  "ebe0aeb564ec46a98002aaf6b121b0e3",
  "23e6ff4bc96041cca2074c57d6b4d1de",
  "5e9bbd3e11eb49aca9f9eea7b0c970d8",
  "445fdab3a0fc4c658797cf9cecc677df",
  "a0d355b77186440d9c450febaa8c9441"
]

faces = [0, 1, 2, 3, 4, 5]
resolutions = ["512", "1k", "2k"]
ranges = {
  "512": [0],
  "1k": [0, 1],
  "2k": [0, 1, 2, 3],
}

download_max_tries = 10

cwd = os.getcwd()

for sweep in sweeps:
  for res in resolutions:
    for face in faces:
      for i in ranges[res]:
        for j in ranges[res]:
          url = url_pattern.format(website=website, model=model, token=token, sweep=sweep, res=res, face=face, i=i, j=j)
          dir = os.path.join(cwd, "tiles", sweep)
          init_dir(dir)
          file_path = os.path.join(dir, "{res}_face{face}_{i}_{j}.jpg".format(res = res, face=face, i=i, j=j))
          print(url)
          print(file_path)
          download_ok = False
          download_num_tries = 0
          while not download_ok:
            try:
              with urllib.request.urlopen(url) as response, open(file_path, 'wb') as out_file:
                shutil.copyfileobj(response, out_file)
            except TimeoutError:
              print("Could not download file. Retrying...")
              time.sleep(1)
              download_num_tries += 1
              if download_num_tries > download_max_tries:
                raise
            else:
              download_ok = True
