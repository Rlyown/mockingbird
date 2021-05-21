
## 开发环境

* Ubuntu 20.04
* gcc 9.3.0
* cmake 3.16.3

相关软件包：
* libboost-dev
* yaml-cpp

## 项目路径

```
.
├── CMakeLists.txt              -- cmake定义文件
├── Makefile
├── README.md
├── bin                         -- 二进制
├── cmake-build-debug           -- 中间文件路径
├── cmake                       -- cmake函数文件夹
├── docker-compose.yml          -- 编译环境
├── lib                         -- 库的输出路径
├── src                         -- 项目源码
└── tests                       -- 测试代码路径
```

## 日志系统

1. Log4J
```
Logger (定义日志级别)
    |
    |-----------Formatter (日志格式)
    |
Appender (日志输出)
```

## 配置系统

Config --> Yaml

## 协程库封装


## socket函数库


## http协议开发


## 分布协议


## 推荐系统


