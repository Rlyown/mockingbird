//
// Created by ChaosChen on 2021/5/8.
//

#ifndef MOCKER_LOG_H
#define MOCKER_LOG_H

#include <string>
#include <cstdint>
#include <memory>
#include <list>
#include <sstream>
#include <fstream>
#include <vector>
#include <ostream>
#include <cstdarg>
#include <map>
#include <mocker/util.h>
#include <mocker/singleton.h>


namespace mocker {

    class Logger;


    // 日志级别
    class LogLevel {
    public:
        enum Level {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };

        static const char * ToString(LogLevel::Level level);
    };


    // 日志事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(const char * file, int32_t line, uint32_t elapse,
                 uint32_t threadId, uint32_t fiberId, uint64_t time);
        ~LogEvent();

        const char * getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        size_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        uint64_t getTime() const { return m_time; }
        std::string getContent() const { return m_ss.str(); }
        std::stringstream& getSS() { return m_ss; }

        void format(const char* fmt, ...);
        void format(const char* fmt, va_list al);
    private:
        const char * m_file = nullptr;      // 文件名
        int32_t m_line = 0;                 // 行号
        uint32_t m_elapse = 0;              // 程序启动开始到现在的毫秒数
        size_t m_threadId = 0;              // 线程id
        uint32_t m_fiberId = 0;             // 协程id
        uint64_t m_time = 0;                // 时间戳
        std::stringstream m_ss;
    };


    // 日志格式器
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        LogFormatter(std::string pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<LogFormatter::FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;

    };




    // 日志输出地
    class LogAppender {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        LogAppender(LogLevel::Level level = LogLevel::DEBUG);
        virtual ~LogAppender() {}

        // 纯虚函数，方法交由子类实现
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        void setLevel(LogLevel::Level level) { m_level = level; }
        LogLevel::Level getLevel() { return m_level; }

    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };


    // 日志器
    class Logger : public std::enable_shared_from_this<Logger>{
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger(const std::string& name = "root");

        void log(LogLevel::Level level, LogEvent::ptr event);

        void debug(LogEvent::ptr event);
        void info(LogEvent::ptr event);
        void warn(LogEvent::ptr event);
        void error(LogEvent::ptr event);
        void fatal(LogEvent::ptr event);

        void addAppender(LogAppender::ptr appender);
        void delAppender(LogAppender::ptr appender);

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

        const std::string& getName() const { return m_name; }
    private:
        std::string m_name;                         // 日志名称
        LogLevel::Level m_level;                    // 日志级别
        std::list<LogAppender::ptr> m_appenders;    // 日志输出集合
        LogFormatter::ptr m_formatter;
    };


    // 输出到console
    class StdoutLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        StdoutLogAppender(LogLevel::Level level = LogLevel::DEBUG);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  // override指明是重载的方法
    };


    // 输出到文件
    class FileLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        FileLogAppender(const std::string& filename, LogLevel::Level level = LogLevel::DEBUG);
        ~FileLogAppender();
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };


    class LogEventWrapper {
    public:
        LogEventWrapper(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event);
        ~LogEventWrapper();
        std::stringstream& getSS() { return m_event->getSS(); }
        LogEvent::ptr getEvent() { return m_event; }

    private:
        Logger::ptr m_logger;
        LogLevel::Level m_level;
        LogEvent::ptr m_event;
    };


    // Log 管理器
    class LogManager {
    public:
        LogManager();
        Logger::ptr getLogger(const std::string& name);

        void init();
        Logger::ptr getRoot() const { return m_root; }
    private:
        std::map<std::string, Logger::ptr> m_loggers;
        Logger::ptr m_root;
    };

    typedef Singleton<LogManager> LoggerMgr;

}  /* namespace mocker */


#define MOCKER_LOG_LEVEL(logger, level) \
        mocker::LogEventWrapper(logger, level, \
                mocker::LogEvent::ptr(new mocker::LogEvent(__FILE__, __LINE__, 0, \
                                                        mocker::GetThreadId(), \
                                                        mocker::GetFiberId(), \
                                                        time(0)))).getSS()

#define MOCKER_LOG_DEBUG(logger) MOCKER_LOG_LEVEL(logger, mocker::LogLevel::DEBUG)
#define MOCKER_LOG_INFO(logger)  MOCKER_LOG_LEVEL(logger, mocker::LogLevel::INFO)
#define MOCKER_LOG_WARN(logger)  MOCKER_LOG_LEVEL(logger, mocker::LogLevel::WARN)
#define MOCKER_LOG_ERROR(logger) MOCKER_LOG_LEVEL(logger, mocker::LogLevel::ERROR)
#define MOCKER_LOG_FATAL(logger) MOCKER_LOG_LEVEL(logger, mocker::LogLevel::FATAL)


#define MOCKER_LOG_FMT_LEVEL(logger, level, fmt, ...) \
        mocker::LogEventWrapper(logger, level, \
                mocker::LogEvent::ptr(new mocker::LogEvent(__FILE__, __LINE__, 0, \
                                                        mocker::GetThreadId(), \
                                                        mocker::GetFiberId(), \
                                                        time(0)))).getEvent()->format(fmt, __VA_ARGS__)

#define MOCKER_LOG_FMT_DEBUG(logger, fmt, ...) MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_INFO(logger, fmt, ...)  MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::INFO, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_WARN(logger, fmt, ...)  MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::WARN, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_ERROR(logger, fmt, ...) MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::ERROR, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_FATAL(logger, fmt, ...) MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::FATAL, fmt, __VA_ARGS__)

#define MOCKER_LOG_ROOT() mocker::LoggerMgr::GetInstance()->getRoot()

#endif //MOCKER_LOG_H

