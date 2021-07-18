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
    class LogManager;

    class LogLevel {
    public:
        enum Level {
            UNKNOWN = 0,
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,
            REMOVED = 100
        };

        static const char * toString(LogLevel::Level level);
        static LogLevel::Level fromString(std::string str);
    };


    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent(const char * file, int32_t line, uint32_t elapse,
                 uint32_t threadId, uint32_t fiberId, uint64_t time,
                 const std::string& logger_real_name = "");
        ~LogEvent();

        const char * getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        size_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_fiberId; }
        uint64_t getTime() const { return m_time; }
        std::string getContent() const { return m_ss.str(); }
        std::stringstream& getSS() { return m_ss; }
        std::string getLoggerRealName() { return m_logger_real_name; }

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

        std::string m_logger_real_name;
    };



    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        LogFormatter(std::string pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

        std::string getPattern() const { return m_pattern; }

    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<LogFormatter::FormatItem> ptr;
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

        bool isError() const { return m_error; }
    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;
        bool m_error = false;
    };


    class LogAppender {
    public:
        friend class Logger;
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        LogAppender(LogLevel::Level level = LogLevel::UNKNOWN);
        virtual ~LogAppender() {}

        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        virtual std::string toYamlString() = 0;

        void setFormatter(LogFormatter::ptr val);
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        void setLevel(LogLevel::Level level) { m_level = level; }
        LogLevel::Level getLevel() { return m_level; }

    protected:
        LogLevel::Level m_level;
        bool m_hasFormatter = false;
        LogFormatter::ptr m_formatter;
    };


    class Logger : public std::enable_shared_from_this<Logger>{
        friend class LogManager;
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
        void clearAppender();

        LogLevel::Level getLevel() const { return m_level; }
        void setLevel(LogLevel::Level val) { m_level = val; }

        const std::string& getName() const { return m_name; }

        void setFormatter(LogFormatter::ptr val);
        void setFormatter(const std::string& val);
        LogFormatter::ptr getFormatter() const { return m_formatter; }

        std::string toYamlString();
    private:
        std::string m_name;
        LogLevel::Level m_level;
        std::list<LogAppender::ptr> m_appenders;
        LogFormatter::ptr m_formatter;

        ptr m_root;
    };


    class StdoutLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        StdoutLogAppender(LogLevel::Level level = LogLevel::UNKNOWN);
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;  // override指明是重载的方法
        std::string toYamlString() override;
    };


    class FileLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        FileLogAppender(const std::string& filename, LogLevel::Level level = LogLevel::UNKNOWN);
        ~FileLogAppender();
        void log(Logger::ptr logger, LogLevel::Level level, LogEvent::ptr event) override;
        std::string toYamlString() override;

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


    class LogManager {
    public:
        LogManager();
        Logger::ptr getLogger(const std::string& name);

        void init();
        Logger::ptr getRoot() const { return m_root; }

        std::string toYamlString();
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
                                                        time(0), \
                                                        (logger)->getName()))).getSS()

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
                                                        time(0), \
                                                        (logger)->getName()))).getEvent()->format(fmt, __VA_ARGS__)

#define MOCKER_LOG_FMT_DEBUG(logger, fmt, ...) MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_INFO(logger, fmt, ...)  MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::INFO, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_WARN(logger, fmt, ...)  MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::WARN, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_ERROR(logger, fmt, ...) MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::ERROR, fmt, __VA_ARGS__)
#define MOCKER_LOG_FMT_FATAL(logger, fmt, ...) MOCKER_LOG_FMT_LEVEL(logger, mocker::LogLevel::FATAL, fmt, __VA_ARGS__)

#define MOCKER_LOG_ROOT() mocker::LoggerMgr::GetInstance()->getRoot()
#define MOCKER_LOG_NAME(name) mocker::LoggerMgr::GetInstance()->getLogger(name)

#endif //MOCKER_LOG_H

