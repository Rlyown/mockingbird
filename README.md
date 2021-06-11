
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

配置系统原则：约定优于配置
```c++
template<T, FromStr, ToStr>
class ConfigVar;

template<F, T>
LexicalCast;

// STL支持
// 目前支持：vector, list, set, map, unordered_set, unordered_map
// (unordered_)map/set仅支持key = std::string
// Config::lookup key相同但类型不同的，会报错
// 所有的配置名需均为小写
```

自定义类型，需要实现mocker::LexicalCast特化
实现后，就可以支持Config解析自定义类型，自定义类型可以和常规STL容器一起使用

配置的事件机制
当一个配置项发生修改的时候，可以反向同志对应的代码，回调

## 协程库封装


## socket函数库


## http协议开发


## 分布协议


## 推荐系统


