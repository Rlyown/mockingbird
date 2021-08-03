//
// Created by ChaosChen on 2021/5/8.
//

#include <tuple>
#include <functional>
#include <iostream>
#include <ctime>
#include <cstring>
#include <utility>

#include <mocker/log.h>
#include <mocker/config.h>

namespace mocker {
    ////////////////////////////////////////////////////////////////////
    /// LogEvent
    ////////////////////////////////////////////////////////////////////
    LogEvent::LogEvent(const char *file, int32_t line, uint32_t elapse,
                       uint32_t threadId, const std::string& thread_name,
                       uint32_t fiberId, uint64_t time,
                       const std::string& logger_real_name)
            : m_file(file), m_line(line), m_elapse(elapse),
              m_threadId(threadId), m_threadName(thread_name),
              m_coroutineId(fiberId), m_time(time),
              m_logger_real_name(logger_real_name) {
            if (m_logger_real_name.empty()) {
                m_logger_real_name = "root";
            }
    }

    LogEvent::~LogEvent() {

    }

    void LogEvent::format(const char *fmt, ...) {
        va_list al;
        va_start(al, fmt);
        format(fmt, al);
        va_end(al);
    }

    void LogEvent::format(const char *fmt, va_list al) {
        char *buf = nullptr;
        int len = vasprintf(&buf, fmt, al);
        if (len != -1) {
            m_ss << std::string(buf, len);
            free(buf);
        }

    }

    LogEventWrapper::LogEventWrapper(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event)
            : m_logger(std::move(logger)), m_level(level), m_event(std::move(event)){

    }

    LogEventWrapper::~LogEventWrapper() {
            m_logger->log(m_level, m_event);
    }


    ////////////////////////////////////////////////////////////////////
    /// LogLevel
    ////////////////////////////////////////////////////////////////////
    const char * LogLevel::ToString(LogLevel::Level level) {
        switch (level) {
#define XX(name) \
        case LogLevel::name: \
            return #name;    \
            break;

            XX(DEBUG);
            XX(INFO);
            XX(WARN);
            XX(ERROR);
            XX(FATAL);
#undef XX
        default:
            return "UNKNOWN";
        }

        return "UNKNOWN";

    }

    LogLevel::Level LogLevel::FromString(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(), toupper);
#define XX(name) \
        if (str == #name) { \
            return LogLevel::name; \
        }

        XX(DEBUG);
        XX(INFO);
        XX(WARN);
        XX(ERROR);
        XX(FATAL);

        return UNKNOWN;
#undef XX
    }

    ////////////////////////////////////////////////////////////////////
    /// LogFormatter
    ////////////////////////////////////////////////////////////////////
    class MessageFormatItem: public LogFormatter::FormatItem {
    public:
        MessageFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };


    class LevelFormatItem: public LogFormatter::FormatItem {
    public:
        LevelFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::ToString(level);
        }
    };


    class ElapseFormatItem: public LogFormatter::FormatItem {
    public:
        ElapseFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };


    class NameFormatItem: public LogFormatter::FormatItem {
    public:
        NameFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLoggerRealName();
        }
    };


    class ThreadIdFormatItem: public LogFormatter::FormatItem {
    public:
        ThreadIdFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadId();
        }
    };


    class ThreadNameFormatItem: public LogFormatter::FormatItem {
    public:
        ThreadNameFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadName();
        }
    };


    class FiberIdFormatItem: public LogFormatter::FormatItem {
    public:
        FiberIdFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getCoroutineId();
        }
    };


    class DateTimeFormatItem: public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(const std::string& format = "%Y-%m-%d %H:%M:%S") : m_format(format) {
            if (m_format.empty()) {
                m_format = "%Y-%m-%d %H:%M:%S";
            }
        }

        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            struct tm tp{};
            time_t timer = event->getTime();
            localtime_r(&timer, &tp);
            char buf[64];
            strftime(buf, sizeof(buf), m_format.c_str(), &tp);

            os << buf;
        }

    private:
        std::string m_format;
    };


    class FilenameFormatItem: public LogFormatter::FormatItem {
    public:
        FilenameFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };


    class LineFormatItem: public LogFormatter::FormatItem {
    public:
        LineFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };


    class NewLineFormatItem: public LogFormatter::FormatItem {
    public:
        NewLineFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };


    class TabFormatItem: public LogFormatter::FormatItem {
    public:
        TabFormatItem(const std::string&str = "") {}
        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << "\t";
        }
    };


    class StringFormatItem: public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std:: string& str) : m_string(str) {}

        void format(std::ostream& os, Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }

    private:
        std::string m_string;
    };


    LogFormatter::LogFormatter(std::string pattern) : m_pattern(std::move(pattern)) {
        init();
    }

    std::string LogFormatter::format(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        std::stringstream ss;
        for (auto& i : m_items) {
            i->format(ss, logger, level, event);
        }
        return ss.str();
    }

    /**
     * %xxx %xxx{xxx} %%
     */
    void LogFormatter::init() {
        // str, format, type
        std::vector<std::tuple<std::string, std::string, int>> vec;

        std::string nstr;
        for (size_t i = 0; i < m_pattern.size(); ++i) {
            if (m_pattern[i] != '%') {
                nstr.append(1, m_pattern[i]);
                continue;
            }

            if (i + 1 < m_pattern.size()) {
                if (m_pattern[i + 1] == '%') {
                    nstr.append(1, '%');
                    continue;
                }
            }

            size_t j = i + 1;
            int fmt_status = 0;  // init status -- no status
            size_t fmt_begin = 0;

            std::string fmt_str;
            std::string fmt;

            bool break_while_flag = false;
            while (j < m_pattern.size()) {
                switch (fmt_status) {
                    /*
                     * char        xxx{yyy}
                     * cur_status  01112333
                     * next_status 11123334
                     */
                    case 0:
                    case 1: {
                        if (isalpha(m_pattern[j])) {
                            fmt_status = 1;
                            if (j + 1 == m_pattern.size()) {
                                // process the last char in m_pattern
                                fmt_str = m_pattern.substr(i + 1, j - i + 1);
                            }
                        } else if (m_pattern[j] == '{') {
                            fmt_status = 2;
                            fmt_str = m_pattern.substr(i + 1, j - i - 1);
                        } else {
                            // Terminate if it encounters a non-letter
                            fmt_str = m_pattern.substr(i + 1, j - i - 1);
                            break_while_flag = true;
                            --j;
                        }
                        break;
                    }
                    case 2: {
                        fmt_begin = j;
                        fmt_status = 3;
                        break;
                    }
                    case 3: {
                        if (m_pattern[j] == '}') {
                            fmt_status = 4;
                            fmt = m_pattern.substr(fmt_begin, j - fmt_begin);
                            break_while_flag = true;
                        }
                        break;
                    }
                    default:
                        break;
                }

                ++j;

                // break the loop
                if (break_while_flag) {
                    break;
                }
            }

            if (fmt_status == 1 || fmt_status == 4) {
                if (!nstr.empty()) {
                    vec.push_back(std::make_tuple(nstr, "", 0));
                    nstr.clear();
                }
                vec.push_back(std::make_tuple(fmt_str, fmt, 1));
                i = j - 1;  // Since the for loop will execute i++, -1 is needed here
            } else {
//                std::cout << "\033[31m" << "[MOCKER ERROR] pattern parse error: {" <<
//                        m_pattern << "} - {" << m_pattern.substr(i) << "}" << "\033[0m" << std::endl;
                vec.push_back(std::make_tuple("<<pattern_error>>", fmt, 1));
                m_error = true;
            }
        }

        if (!nstr.empty()) {
            vec.push_back(std::make_tuple(nstr, "", 0));
        }

        /*
         * %m -- message content
         * %p -- level
         * %r -- milliseconds after start
         * %c -- log name
         * %t -- thread id
         * %N -- thread name
         * %n -- new line
         * %d -- time
         * %f -- filename
         * %l -- line number
         * %T -- tab
         */
        static std::map<std::string, std::function<FormatItem::ptr(const std::string& str)>> s_format_items = {
#define XX(str, C) \
                {#str, [](const std::string& fmt) { return FormatItem::ptr(new C(fmt)); }}

        XX(m, MessageFormatItem),
        XX(p, LevelFormatItem),
        XX(r, ElapseFormatItem),
        XX(c, NameFormatItem),
        XX(t, ThreadIdFormatItem),
        XX(N, ThreadNameFormatItem),
        XX(n, NewLineFormatItem),
        XX(d, DateTimeFormatItem),
        XX(f, FilenameFormatItem),
        XX(l, LineFormatItem),
        XX(T, TabFormatItem),
        XX(F, FiberIdFormatItem)

#undef XX
        };

        for (auto& i : vec) {
            if (std::get<2>(i) == 0) {
                m_items.push_back(FormatItem::ptr(new StringFormatItem(std::get<0>(i))));
            } else {
                auto it = s_format_items.find(std::get<0>(i));
                if (it == s_format_items.end()) {
                    m_items.push_back(FormatItem::ptr(new StringFormatItem("<<error_format %" + std::get<0>(i) + ">>")));
                    m_error = true;
                } else {
                    m_items.push_back(it->second(std::get<1>(i)));
                }
            }

//            std::cout << "\033[31m" << "[MOCKER ERROR] {" << std::get<0>(i) << "} - {"
//                    << std::get<1>(i) << "} - {" << std::get<2>(i) << "}" << "\033[0m" << std::endl;
        }
    }


    ////////////////////////////////////////////////////////////////////
    /// Logger
    ////////////////////////////////////////////////////////////////////
    Logger::Logger(const std::string &name) : m_name(name), m_level(LogLevel::DEBUG) {
        m_formatter.reset(new LogFormatter("%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"));
    }

    void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            auto self = shared_from_this();
            MutexType::Lock lock(m_mutex);
            if (!m_appenders.empty()) {
                for (auto& i : m_appenders) {
                    i->log(self, level, event);
                }
            } else if (m_root) {
                m_root->log(level, event);
            }
        }
    }

    void Logger::debug(LogEvent::ptr event) {
        log(LogLevel::DEBUG, event);
    }

    void Logger::info(LogEvent::ptr event) {
        log(LogLevel::INFO, event);
    }

    void Logger::warn(LogEvent::ptr event) {
        log(LogLevel::WARN, event);
    }

    void Logger::error(LogEvent::ptr event) {
        log(LogLevel::ERROR, event);
    }

    void Logger::fatal(LogEvent::ptr event) {
        log(LogLevel::FATAL, event);
    }

    void Logger::addAppender(LogAppender::ptr appender) {
        MutexType::Lock lock(m_mutex);
        // Modify the m_formatter directly inside the Logger without
        // calling the setFormatter function. This way hasFormatter
        // will not become true due to the default initialization of
        // logger.
        if (!appender->getFormatter()) {
            MutexType::Lock sub_lock(appender->m_mutex);
            appender->m_formatter = m_formatter;
        }
        m_appenders.push_back(appender);
    }

    void Logger::delAppender(LogAppender::ptr appender) {
        MutexType::Lock lock(m_mutex);
        for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
            if (*it == appender) {
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::clearAppender() {
        MutexType::Lock lock(m_mutex);
        m_appenders.clear();
    }

    void Logger::setFormatter(LogFormatter::ptr val) {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;

        for (auto& i : m_appenders) {
            MutexType::Lock sub_lock(i->m_mutex);
            if (!i->m_hasFormatter) {
                i->m_formatter = m_formatter;
            }
        }
    }

    void Logger::setFormatter(const std::string &val) {
        LogFormatter::ptr new_val(new LogFormatter(val));
        if (new_val->isError()) {
            std::cout << "\033[31m" << "[MOCKER ERROR] Logger setFormatter name="
                    << m_name << " value=" << val << "invalid formatter" << "\033[0m" << std::endl;
            return;
        }
        setFormatter(new_val);
    }

    LogFormatter::ptr Logger::getFormatter() {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    std::string Logger::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["name"] = m_name;
        node["level"] = LogLevel::ToString(m_level);

        if (m_formatter) {
            node["formatter"] = m_formatter->getPattern();
        }

        for (auto& i : m_appenders) {
            node["appenders"].push_back(YAML::Load(i->toYamlString()));
        }

        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    ////////////////////////////////////////////////////////////////////
    /// LogAppender
    ////////////////////////////////////////////////////////////////////
    LogAppender::LogAppender(LogLevel::Level level) : m_level(level) {

    }

    void LogAppender::setFormatter(LogFormatter::ptr val) {
        MutexType::Lock lock(m_mutex);
        m_formatter = val;
        if (m_formatter)
            m_hasFormatter = true;
        else
            m_hasFormatter = false;
    }

    LogFormatter::ptr LogAppender::getFormatter() {
        MutexType::Lock lock(m_mutex);
        return m_formatter;
    }

    StdoutLogAppender::StdoutLogAppender(LogLevel::Level level) : LogAppender(level) {

    }

    void StdoutLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            MutexType::Lock lock(m_mutex);
            std::cout << GetColorMap()[level] << m_formatter->format(logger, level, event) << "\033[0m";
        }
    }

    std::string StdoutLogAppender::toYamlString() {
        MutexType::Lock lock(m_mutex);
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


    StdoutLogAppender::ColorMap StdoutLogAppender::GetColorMap() {
        static ColorMap m_colors = {
            {LogLevel::UNKNOWN, "\033[0m"},
            {LogLevel::DEBUG, "\033[32m"},
            {LogLevel::INFO, "\033[0m"},
            {LogLevel::WARN, "\033[33m"},
            {LogLevel::ERROR, "\033[31m"},
            {LogLevel::FATAL, "\033[41m"},
            {LogLevel::REMOVED, "\033[2m"},
        };
        return m_colors;
    }


    FileLogAppender::FileLogAppender(const std::string &filename, LogLevel::Level level)
            : LogAppender(level), m_filename(filename) {
        reopen();
    }

    FileLogAppender::~FileLogAppender() {
        m_filestream.close();
    }

    void FileLogAppender::log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) {
        if (level >= m_level) {
            uint64_t now = time(nullptr);
            if (now != m_lastTime) {
                reopen();
                m_lastTime = now;
            }
            MutexType::Lock lock(m_mutex);
            m_filestream << m_formatter->format(logger, level, event);
        }
    }

    std::string FileLogAppender::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        node["type"] = "FileLogAppender";
        node["file"] = m_filename;
        if (m_level != LogLevel::UNKNOWN) {
            node["level"] = LogLevel::ToString(m_level);
        }
        if (m_formatter && m_hasFormatter) {
            node["formatter"] = m_formatter->getPattern();
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    /**
     * Reopen the file
     * @return
     */
    bool FileLogAppender::reopen() {
        MutexType::Lock lock(m_mutex);

        // 给日志的文件名加上日期
        struct tm tp{};
        time_t timer = time(nullptr);
        localtime_r(&timer, &tp);
        char buf[64];
        strftime(buf, sizeof(buf), ".%Y-%m-%d", &tp);

        if (m_filestream) {
            m_filestream.close();
        }
        m_filestream.open(m_filename + std::string(buf), std::ios::app);
        return !!m_filestream;
    }


    ////////////////////////////////////////////////////////////////////
    /// LogManager
    ////////////////////////////////////////////////////////////////////
    LogManager::LogManager() {
        m_root.reset(new Logger);
        m_root->addAppender(LogAppender::ptr(new StdoutLogAppender(m_root->getLevel())));
        m_loggers[m_root->getName()] = m_root;
        init();
    }

    Logger::ptr LogManager::getLogger(const std::string &name) {
        MutexType::Lock lock(m_mutex);
        auto it = m_loggers.find(name);
        if (it != m_loggers.end()) {
            return it->second;
        }

        Logger::ptr logger(new Logger(name));
        logger->m_root = m_root;
        m_loggers[name] = logger;
        return logger;
    }

    std::string LogManager::toYamlString() {
        MutexType::Lock lock(m_mutex);
        YAML::Node node;
        for (auto& i : m_loggers) {
            node.push_back(YAML::Load(i.second->toYamlString()));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }

    void LogManager::init() {

    }


    ////////////////////////////////////////////////////////////////////
    /// Load config file to the LogManager
    ////////////////////////////////////////////////////////////////////
    // used to parse LogAppender from config file
    struct LogAppenderDefine {
        enum AppenderType {
            UNKNOWN = 0,
            StdLogAppender = 1,
            FileLogAppender = 2
        };

        AppenderType type = UNKNOWN;
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::string formatter;
        std::string file;

        bool operator== (const LogAppenderDefine& oth) const {
            return type == oth.type
                   && level == oth.level
                   && formatter == oth.formatter
                   && file == oth.file;
        }
    };

    // parse logger from config file
    struct LogDefine {
        std::string name;
        LogLevel::Level level = LogLevel::UNKNOWN;
        std::string formatter;
        std::vector<LogAppenderDefine> appenders;

        bool operator== (const LogDefine& oth) const {
            return name == oth.name
                   && level == oth.level
                   && formatter == oth.formatter
                   && appenders == appenders;
        }

        bool operator< (const LogDefine& oth) const {
            return name < oth.name;
        }
    };

    template<>
    class LexicalCast<std::string, LogDefine> {
    public:
        LogDefine operator() (const std::string& v) {
            YAML::Node node = YAML::Load(v);
            std::stringstream ss;
            LogDefine logDefine;
            if (!node["name"].IsDefined()) {
                std::cout << "\033[31m" << "[MOCKER ERROR] log config error: name is null, "
                        << node << "\033[0m" << std::endl;
            }
            logDefine.name = node["name"].as<std::string>();
            logDefine.level = LogLevel::FromString(node["level"].IsDefined() ? node["level"].as<std::string>() : "");
            if (node["formatter"].IsDefined()) {
                logDefine.formatter = node["formatter"].as<std::string>();
            }
            if (node["appenders"].IsDefined()) {
                for (size_t i = 0; i < node["appenders"].size(); ++i) {
                    auto ap = node["appenders"][i];
                    if (!ap["type"].IsDefined()) {
                        std::cout << "\033[31m" << "[MOCKER ERROR] log config error: appender type is null, "
                                << node << "\033[0m" << std::endl;
                        continue;
                    }
                    std::string type = ap["type"].as<std::string>();
                    LogAppenderDefine lad;

                    if (ap["level"].IsDefined()) {
                        lad.level = LogLevel::FromString(ap["level"].as<std::string>());
                    }

                    if (type == "FileLogAppender") {
                        lad.type = LogAppenderDefine::FileLogAppender;
                        if (!ap["file"].IsDefined()) {
                            std::cout << "\033[31m" << "[MOCKER ERROR] log config error: FileAppender file is null, "
                                    << ap << "\033[0m" << std::endl;
                            continue;
                        }
                        lad.file = ap["file"].as<std::string>();
                        if (ap["formatter"].IsDefined()) {
                            lad.formatter = ap["formatter"].as<std::string>();
                        }
                    } else if (type == "StdoutLogAppender") {
                        lad.type = LogAppenderDefine::StdLogAppender;
                        if (ap["formatter"].IsDefined()) {
                            lad.formatter = ap["formatter"].as<std::string>();
                        }
                    } else {
                        std::cout << "\033[31m" << "[MOCKER ERROR] log config error: appender type is invalid, "
                                << ap << "\033[0m" << std::endl;
                    }

                    logDefine.appenders.push_back(lad);
                }
            }
            return logDefine;
        }
    };

    template<>
    class LexicalCast<LogDefine, std::string> {
    public:
        std::string operator()(const LogDefine &v) {
            YAML::Node node;
            std::stringstream ss;

            node["name"] = v.name;
            node["level"] = LogLevel::ToString(v.level);
            if (!v.formatter.empty()) {
                node["formatter"] = v.formatter;
            }

            for (auto& ap : v.appenders) {
                YAML::Node nap;
                if (ap.type == LogAppenderDefine::FileLogAppender) {
                    nap["type"] = "FileLogAppender";
                    nap["file"] = ap.file;
                } else if (ap.type == LogAppenderDefine::StdLogAppender) {
                    nap["type"] = "StdoutLogAppender";
                }
                if (ap.level != LogLevel::UNKNOWN) {
                    nap["level"] = LogLevel::ToString(ap.level);
                }
                if (!ap.formatter.empty()) {
                    nap["formatter"] = ap.formatter;
                }
                node["appenders"].push_back(nap);
            }

            ss << node;
            return ss.str();
        }
    };

    // log global config variable
    ConfigVar<std::set<LogDefine>>::ptr g_log_defines =
            Config::Lookup("logs", std::set<LogDefine>(), "logs config");

    struct LogIniter {
        LogIniter() {
            g_log_defines->addListener([](const std::set<LogDefine>& old_value,
                                          const std::set<LogDefine>& new_value){
                                           MOCKER_LOG_INFO(MOCKER_LOG_ROOT()) << "on logger configuration changed";
                                           for (auto& i : new_value) {
                                               auto it = old_value.find(i);
                                               Logger::ptr logger;
                                               if (it == old_value.end()) {
                                                   // 新增logger
                                                   logger = MOCKER_LOG_NAME(i.name);
                                               } else {
                                                   if (!(i == *it)) {
                                                       // 修改logger
                                                       logger = MOCKER_LOG_NAME(i.name);
                                                   }
                                               }
                                               logger->setLevel(i.level);

                                               if (!i.formatter.empty()) {
                                                   logger->setFormatter(i.formatter);
                                               }

                                               logger->clearAppender();
                                               for (auto& ad : i.appenders) {
                                                   LogAppender::ptr ap;
                                                   if (ad.type == LogAppenderDefine::StdLogAppender) {
                                                       ap.reset(new StdoutLogAppender);
                                                   } else if (ad.type == LogAppenderDefine::FileLogAppender) {
                                                       ap.reset(new FileLogAppender(ad.file));
                                                   }
                                                   ap->setLevel(ad.level);

                                                   if (!ad.formatter.empty()) {
                                                       LogFormatter::ptr fmt(new LogFormatter(ad.formatter));
                                                       if (!fmt->isError()) {
                                                           ap->setFormatter(fmt);
                                                       } else {
                                                           std::cout << "\033[31m" << "[MOCKER ERROR]" << "logger name=" << i.name
                                                               << " appender type=" << ad.type
                                                               << " formatter=" << ad.formatter << " is invalid"
                                                               << "\033[0m" << std::endl;
                                                       }
                                                   }

                                                   logger->addAppender(ap);
                                               }
                                           }

                                           for (auto& i : old_value) {
                                               auto it = new_value.find(i);
                                               if (it == new_value.end()) {
                                                   // 删除logger
                                                   auto logger = MOCKER_LOG_NAME(i.name);
                                                   logger->setLevel(LogLevel::REMOVED);
                                                   logger->clearAppender();
                                               }
                                           }
                                       });
        }
    };

    // call the log init before main()
    static LogIniter __log_init;




} /* namespace mocker */