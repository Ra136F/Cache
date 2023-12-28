##### 项目概述

本项目用于测试部署在多级混合存储机制的缓存策略的系统性能数据，项目结构如下：

```shell
├── Caching-Policy
│   ├── scripts
│   │   └── ycsb_kvtracer_process.py
│   ├── src
│   │   ├── cache
│   │   │   ├── 2q.hpp
│   │   │   ├── arc.h
│   │   │   ├── cache.hpp
│   │   │   ├── cache_policy.hpp
│   │   │   ├── clockpro.hpp
│   │   │   ├── fifo.hpp
│   │   │   ├── lfu.hpp
│   │   │   ├── lirs.h
│   │   │   ├── lru.hpp
│   │   │   └── random.h
│   │   ├── simulator
│   │   │   ├── 2qSl.h
│   │   │   ├── arcSl.h
│   │   │   ├── clockproSl.h
│   │   │   ├── fifoSl.h
│   │   │   ├── lfuSl.h
│   │   │   ├── lirsSl.h
│   │   │   ├── lruSl.h
│   │   │   ├── randomSl.h
│   │   │   ├── sl.h
│   │   │   └── tinylfuSl.h
│   │   ├── test.cpp
│   │   └── utils
│   │       ├── bitmap.h
│   │       ├── chunk.h
│   │       ├── config.h
│   │       ├── policy.h
│   │       └── statistic.h
│   └── test
│       └── cpu_mem_disk.sh
└── README.md
```

##### 项目配置

###### 挂载磁盘

1. 查看系统所有识别到的磁盘

```shell
fdisk -l
```

2. 将eMMC临时挂载到/mnt/eMMC

```shell
sudo mount /dev/mmcblk0p1 /mnt/eMMC
```

3. 检查磁盘挂载情况

```shell
lsblk
```

###### 配置trace

1. 修改 `TRACE_PATH`

```C++
const char *TRACE_PATH = "../trace/zipfian/zipfian_r100w_o15w_0.99/trace.txt";
```

2. 修改 `DISK_SIZE`和 `CHUNK_NUM`

```C++
const long long DISK_SIZE = 33668;
const long long CHUNK_NUM = 33668;
```

其中，`DISK_SIZE`表示disk的总存储容量，由trace访问的offset跨度决定；

`CHUNK_NUM`表示trace访问的不重复的chunk总size。

> 在生成的trace.txt文件第一行按顺序记录了统计的 `DISK_SIZE`、`CHUNK_NUM`和 `TRACE_SIZE`信息，我使用YCSB生成的trace控制了单次访问一个chunk，因此前两者数值一致，只记录了一个数字。

###### 配置cache和disk

1. 进入项目根目录

```shell'
cd Caching-Policy/Caching-Policy
```

2. 将用作缓存地址空间的文件复制进 `/mnt/eMMC`

```shell
sudo cp trace/zipfian/zipfian_r100w_o15w_0.99/storage/* /mnt/eMMC/
```

3. 为cache和disk文件赋予可写可读可执行权限

```shell
sudo chmod 777 /mnt/eMMC/cache*
sudo chmod 777 trace/zipfian/zipfian_r100w_o15w_0.99/storage/disk.bin
```

###### 修改地址，并确定缓存占比

1. 打开 `src/utils/config.h`，修改 `CACHE_PATH`和 `DISK_PATH`

```c++
const char *CACHE_PATH = "/mnt/eMMC/cache_0.1.bin";
const char *DISK_PATH = "../trace/zipfian/zipfian_r100w_o15w_0.99/storage/disk.bin";
```

2. 其中 `CACHE_PATH`中cache文件的选择请根据需要手动修改，并且同时修改求得 `CACHE_SIZE`的系数

```C++
const long long CACHE_SIZE = CHUNK_NUM * 0.1;
```

##### 运行

生成 `results`目录

```shell
mkdir results
```

运行结果均保存在 `results`中

###### Terminal 1

```shell
cd test
sh cpu_mem_disk.sh
```

`Terminal 2`中代码运行结束后 `Ctrl+C`终止

###### Terminal 2

1. 进入源代码目录

```shell
cd src
```

2. 编译test.cpp

```shell
g++ -std=c++17 -o test test.cpp # -std=c++17  for clock-pro
```

3. 执行

```shell
./test 3
```

这里的参数3用于指定缓存策略为LRU，其余对应关系如下表

| Number | Caching policy |
| :----: | :------------: |
|   0    |     RANDOM     |
|   1    |      FIFO      |
|   2    |      LFU       |
|   3    |      LRU       |
|   4    |      LIRS      |
|   5    |      ARC       |
|   6    |    CLOCKPRO    |
|   7    |       2Q       |
|   8    |    TINYLFU     |
