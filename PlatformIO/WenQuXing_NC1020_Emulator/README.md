# Test

It's a PlatformIO project.

* https://github.com/quadpixels/banxian_cc800_java
* https://github.com/fwindpeak/wqxsim-nc1020
* https://github.com/hackwaly/NC1020
* https://github.com/fancyblock/GVBASIC
* https://github.com/Wang-Yue/NC1020
* https://github.com/wangyu-/NC2000
* 牛: http://yxts8888.ysepan.com/
* https://www.emsky.net/bbs/forum.php?mod=viewthread&tid=33474
* https://computer.retromuseum.org:86/player.html?machine=ggvnc1020
* https://www.bilibili.com/video/BV1kt421w7Pb/
* https://github.com/wangyu-/NC1020android -> This branch is 2 commits ahead of hackwaly/NC1020:master

## performance

```

ST: 0x10=26296,0x2c=26244,0x30=26160,0x48=206,0x85=464,0xa5=26475,0xbd=240,0xd0=26357,0xec=26157,
slice=50,cost=111227us
nc1020_loop,cost=167ms
ST: 0x10=26717,0x2c=26755,0x30=26691,0xa5=26834,0xd0=26742,0xec=26691,
slice=50,cost=107592us
nc1020_loop,cost=163ms
ST: 0x10=26715,0x2c=26754,0x30=26689,0xa5=26832,0xd0=26741,0xec=26688,
slice=50,cost=107652us
nc1020_loop,cost=163ms
ST: 0x10=26716,0x2c=26756,0x30=26690,0xa5=26833,0xd0=26743,0xec=26691,
slice=50,cost=107627us
nc1020_loop,cost=163ms

```


去掉handler 6个params以后:
```
slice=50,cost=100670us
nc1020_loop,cost=156ms
ST: 0x10=26733,0x2c=26768,0x30=26709,0xa5=26841,0xd0=26757,0xec=26709,
slice=50,cost=100519us
nc1020_loop,cost=156ms
ST: 0x10=26604,0x2c=26628,0x30=26550,0x85=224,0xa5=26748,0xd0=26632,0xec=26547,
slice=50,cost=101679us
nc1020_loop,cost=157ms
ST: 0x10=26715,0x2c=26754,0x30=26689,0xa5=26832,0xd0=26741,0xec=26688,
slice=50,cost=100629us
nc1020_loop,cost=156ms
ST: 0x10=26716,0x2c=26756,0x30=26690,0xa5=26833,0xd0=26743,0xec=26691,
slice=50,cost=100622us
nc1020_loop,cost=156ms
ST: 0x10=26715,0x2c=26754,0x30=26689,0xa5=26832,0xd0=26741,0xec=26689,
slice=50,cost=100626us
nc1020_loop,cost=156ms
```
带-O2以后
```
slice=50,cost=87018us
```

带 -O3 会有横纹 bug
```
754,0x30=26689,0xa5=26832,0xd0=26741,0xec=26689,
slice=50,cost=87268us
nc1020_loop,cost=134ms
ST: 0x10=26716,0x2c=26756,0x30=26690,0xa5=26833,0xd0=26743,0xec=26691,
slice=50,cost=87268us
```
```

#define OPTIMIZE_OFF _Pragma("GCC push_options") \
                     _Pragma("GCC optimize (\"O0\")")
#define OPTIMIZE_ON  _Pragma("GCC pop_options")

OPTIMIZE_OFF
__attribute__((used)) void mock_func() {
  // __asm__ volatile("");
}
OPTIMIZE_ON
```
加入上面以后
```
slice=50,cost=94113us
```
加两个:
```
slice=50,cost=101337us
```

恢复到加一个以后:
```
ST: 0x10=26605,0x2c=26629,0x30=26551,0x85=224,0xa5=26749,0xd0=26634,0xec=26548,
slice=50,cost=95126us
```

## Memory mapping

根据nc1020.cpp中的代码,主要的内存访问逻辑在Peek和PeekW函数中。我来整理一个详细的内存映射表格:


| 地址范围(十六进制) | 大小 | 类型 | 描述 | 访问方式 |
|------------------|------|------|------|---------|
| 0x0000 - 0x003F | 64字节 | IO空间 | I/O寄存器映射 | io_read/io_write数组 |
| 0x0040 - 0x007F | 64字节 | RAM | 特殊RAM区域 | ram_40指针 |
| 0x0080 - 0x00FF | 128字节 | RAM | 栈空间 | stack指针 |
| 0x0100 - 0x1FFF | 7936字节 | RAM | RAM页面0 | ram_page0指针 |
| 0x2000 - 0x3FFF | 8192字节 | RAM | RAM页面1 | ram_page1指针 |
| 0x4000 - 0x5FFF | 8192字节 | RAM | RAM页面2 | ram_page2指针 |
| 0x6000 - 0x7FFF | 8192字节 | RAM | RAM页面3 | ram_page3指针 |
| 0x8000 - 0x9FFF | 8192字节 | NOR Flash | NOR Bank 0-31 | nor_banks数组 |
| 0xA000 - 0xBFFF | 8192字节 | NOR Flash | NOR Bank 0-31 | nor_banks数组 |
| 0xC000 - 0xDFFF | 8192字节 | NOR Flash | NOR Bank 0-31 | nor_banks数组 |
| 0xE000 - 0xFFFF | 8192字节 | ROM | 系统ROM | rom_volume0/1/2数组 |

### 内存访问说明:

1. **IO空间 (0x0000-0x003F)**
   - 通过io_read和io_write函数指针数组访问
   - 每个地址都有对应的读写处理函数

2. **RAM空间 (0x0040-0x7FFF)**
   - 分为多个页面,通过memmap数组映射
   - 特殊区域如栈空间有专门指针

3. **NOR Flash (0x8000-0xDFFF)**
   - 通过nor_banks数组访问
   - 每个bank大小为0x8000
   - 最多支持32个bank

4. **ROM空间 (0xE000-0xFFFF)**
   - 通过rom_volume0/1/2数组访问
   - 支持多个ROM卷
   - 通过ram_io[0x0D]选择当前卷

### 内存映射切换:

1. **Bank切换**
   - 通过ram_io[0x00]选择当前bank
   - 影响0x8000-0xDFFF区域

2. **Volume切换**
   - 通过ram_io[0x0D]选择当前ROM卷
   - 影响0xE000-0xFFFF区域

3. **页面映射**
   - 通过memmap数组动态映射内存页面
   - 可以灵活配置RAM和ROM的映射关系


这个内存布局是基于代码中的以下关键部分:

1. Peek函数中的地址判断逻辑
2. memmap数组的使用
3. rom_volume和nor_banks数组的分配
4. IO空间的特殊处理

