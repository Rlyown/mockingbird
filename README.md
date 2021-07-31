
## Develop Environment

* Dev Platform: arm64
* Ubuntu 20.04
* gcc 9.3.0
* cmake 3.16.3
* libboost-dev
* yaml-cpp

## Project Directory

```
.
├── CMakeLists.txt              -- cmake file
├── Makefile
├── README.md
├── bin                          
├── cmake                       -- cmake function file
├── conf                        -- configuration
├── docker-compose.yml          -- compile env
├── lib                         -- lib output
├── src                         -- project resource
└── tests                       -- test file
```

## Log System

### Log4J
```
Logger (a log object)
    |
    |-----------Formatter (format the output)
    |
Appender (output object)

LogEvent -> Logger -> Appender -> output
```

### LogFormatter:
    /**
     * %xxx %xxx{xxx} %%
     */
* %m -- message content
* %p -- level
* %r -- milliseconds after start
* %c -- log name
* %t -- thread id
* %n -- new line
* %d -- time. It can change datetime format by `%d{xxx}`. It uses the linux localhost format.
* %f -- filename
* %l -- line number
* %T -- tab
* %% -- output %

### LogAppender
Current LogAppender:
* StdoutLogAppender - Output the log event to the standard output stream
* FileLogAppender - Output the log event to the file

If you want to add a new LogAppender type:
1. Implement a derived class of `LogAppender`, and implement pure virtual functions `log` and `toYamlString`
   ```c++
    class StdoutLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        StdoutLogAppender(LogLevel::Level level = LogLevel::UNKNOWN);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  // override指明是重载的方法
        std::string toYamlString() override;
    };
   
   
    StdoutLogAppender::StdoutLogAppender(LogLevel::Level level) : LogAppender(level) {}

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            std::cout << m_formatter->format(logger, level, event);
        }
    }

    std::string StdoutLogAppender::toYamlString() {
        YAML::Node node;
        node["type"] = "StdoutLogAppender";
        if (m_level != LogLevel::UNKNOWN)
            node["level"] = LogLevel::ToString(m_level);
        if (m_formatter && m_hasFormatter) {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
   ```
2. Register the new LogAppender type in `log.cpp`
   ```c++
    // Add it in LogAppenderDefine::AppenderType 
    struct LogAppenderDefine {
     enum AppenderType {
         UNKNOWN = 0,
         StdLogAppender = 1,
         FileLogAppender = 2,
         MyLogAppender = 3
     };
   ```
3. Add the parse method in new LogAppender type in `class LexicalCast<std::string, LogDefine>`, 
   `class LexicalCast<LogDefine, std::string>`, `LogIniter()`


### LogManager
There are two default global Logger:
* Logger `root`. If it does not exist, it will be automatically created by LogManager.
All loggers except `root` have a pointer `m_root` pointing to `root`. When the sub-logger does not have any `LogAppender`,
then the `LogAppender` of `root` will be called for output.
* Logger `system`. 

If called `LogManager::getLogger(name)` or `MOCKER_LOG_NAME(name)` and the `name` has never been used, 
   it will create a new logger by this `name`.

### Set up Logger by YAML Config

Default configuration file path of the logging system is `bin/conf/log.yml`, and the configuration format is as follows:
```yaml
logs:
  - name: root
    level: (debug, info, warn, error, fatal)
    formatter: '%d%T%p%T%t%m%n'
    appenders:
      - type: (StdoutLogAppener, FileLogAppender)
        level: (debug, ...)
        file: /logs/xxx.log
        formatter: '%d%T%m%n'
```


### Example
1. Custom
```c++
mocker::Logger::ptr logger(new mocker::Logger);
logger->addAppender(mocker::LogAppender::ptr (new mocker::StdoutLogAppender(mocker::LogLevel::INFO)));
mocker::LogEvent::ptr logEvent(new mocker::LogEvent(__FILE__, __LINE__, 0, 0, 0, 0, "root"));
logEvent->getSS() << "Hello, World!";
logger->log(mocker::LogLevel::WARN, logEvent);
```

2. Use macro definition in log.h
```c++
// If you has setted up the logger config, 
// and register it to the LogManager.
// Also logger config can set up by yaml file.
mocker::Logger::ptr logger = MOCKER_LOG_NAME(logger_name);
MOCKER_LOG_INFO(logger) << "log message";
// or format the output
MOCKER_LOG_FMT_DEBUG(logger, "log message No.%d", 10); 
```

3. Serialization
```c++
Logger->toYamlString();
LogAppender->toYamlString();
LogManager->toYamlString();
```


## Configuration System

Current supported type: `vector`, `list`, `set`, `map`,`unordered_set`, 
   `unordered_map` and basic data type. (`(unordered_)map/set` only support the key type is `std::string`)

```c++
// Use it to store a config variable.
// ConfigVar name use a dot to seperate parent and child.
template<T, FromStr, ToStr>
class ConfigVar(name, default_value, description);

// use it to serialization/deserialization between ConfigVar and YAML.
template<F, T>
LexicalCast;  
```

### Custom Type
For custom type, you need to implement two special `mocker::LexicalCast` and override `operater()`.
* from string: `class LexicalCast<std::string, custom_type>`
* to string: `class LexicalCast<ustom_type, std::string>`

For example:
```c++
class Person {
public:
   std::string m_name = "null";
   int m_age = 0;
   bool m_sex = 0;

   std::string ToString() const {
   std::stringstream ss;
   ss << "[Person name=" << m_name
      << " age=" << m_age
      << " sex=" << m_sex
      << "]";
   return ss.str();
}

   bool operator== (const Person& oth) const {
      return m_name == oth.m_name
      && m_age == oth.m_age
      && m_sex == oth.m_sex;
   }
};

// Here are the two special `mocker::LexicalCast` you need to implement.
namespace mocker {
template<>
class LexicalCast<std::string, Person> {
public:
   Person operator() (const std::string& v) {
      YAML::Node node = YAML::Load(v);
      Person p;
      p.m_name = node["name"].as<std::string>();
      p.m_age = node["age"].as<int>();
      p.m_sex= node["sex"].as<bool>();
      return p;
   }
};

template<>
class LexicalCast<Person, std::string> {
public:
   std::string operator()(const Person &p) {
      YAML::Node node;
      node["name"] = p.m_name;
      node["age"] = p.m_age;
      node["sex"] = p.m_sex;
      std::stringstream ss;
      ss << node;
      return ss.str();
   }
};
}
```

### Config Event
Register a callback to catch the config event. It include: 
* `addListener` -- add a listener or modify a current listener
* `delListener` -- delete a listener
* `getListener` -- get a listener's callback function
* `clearListener` -- delete all listener

For example:
```c++
// Call Lookup to get the config from YAML
mocker::ConfigVar<Person>::ptr g_person =
      mocker::Config::Lookup("class.person", Person(), "system person");

// Set a listener to ConfigVar g_person.
// addListener([](old, new) -> void {})
g_person->addListener([](const Person& old_value, const Person& new_value){
      MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "old_value=" << old_value.ToString()
      << " new_value=" << new_value.ToString();
}
```

### Example
If I have `config.yml`
```yaml
system:
   ipaddr: localhost
   port: 123
```
and then
```c++
// Define a config variable `system port`. 
// The name "system.port" corresponds to the `port` in `system` table 
// in the yaml file 
mocker::ConfigVar<int>::ptr g_int_value_config =
      mocker::Config::Lookup("system.port", (int)8080, "system port");

// Call LoadFromYaml to parse yaml file.
// The yaml file will change the value in `g_int_value_config`
YAML::Node root = YAML::LoadFile("/path/to/config.yml");
mocker::Config::LoadFromYaml(root);

// Get value
g_int_value_config->getValue();
// Get value string
g_int_value_config->ToString();
```

## Thread
Encapsulates some thread and mutex functions in pthread. `Mutex`, `Spinlock`, `RWMutex`, `CASLock` are supported.

### Example
Use Thread class to create a thread task.
```c++
// Create and run
mocker::Thread::ptr thr(new mocker::Thread(fun2, "name_0");
// wait for finishing
thr.join();
```
Use a mutex lock
```c++
mocker::RWMutex s_mutex;
mocker::Mutex mutex;

// To lock a block
{
    // write lock 
    mocker::RWMutex::WriteLock lock(s_mutex);
    // or read lock
    mocker::RWMutex::ReadLock lock(s_mutex);
    // or just a lock
    mocker::Mutex::Lock lock(mutex);
}
```

## Coroutine


