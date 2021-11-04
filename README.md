# JenovaSDK

## TBJGameSDK

### install
- Enter JenovaSDK's dir
```sh
$ cd ${JenovaSDK}
```
- install offical emscripten
```sh
$ python install_offical_emscripten.py
```
- install jenova modifications:        (add --test if you want to compile test projects)
```sh
$ python publish_jenova.py [--test] 
```

### build your project (choose one of the following methods)
- CMakeLists.txt (recommend)
```sh
$ taomake ${CMakeLists.txt'dir} [--box2d] [--chipmunk] [--freetype]
```
- ${your project}.xcodeproj
```sh
$ taomake ${${your project}.xcodeproj'path} --override [--box2d] [--chipmunk] [--freetype]
```

### taomake all options
```sh
$ taomake --help
```


## StoreSDK
