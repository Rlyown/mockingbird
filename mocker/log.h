//
// Created by ChaosChen on 2021/5/8.
//

#ifndef __MOCKER_LOG_H__
#define __MOCKER_LOG_H__

#include <string>
#include <stdint.h>
#include <memory>
#include <list>
#include <sstream>  // stringstream
#include <fstream>
#include <vector>
#include <ostream>

namespace mocker {

    class Logger;


    // 日志事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();

        const char * getFile() const { return m_file; }
        int32_t getLine() const { return m_line; }
        uint32_t getElapse() const { return m_elapse; }
        uint32_t getThreadId() const { return m_threadId; }
        uint32_t getFiberId() const { return m_time; }
        uint64_t getTime() const { return m_time; }
        const std::string& getContent() const { return m_content; }

    private:
        const char * m_file = nullptr;      // 文件名
        int32_t m_line = 0;                 // 行号
        uint32_t m_elapse = 0;              // 程序启动开始到现在的毫秒数
        uint32_t m_threadId = 0;            // 线程id
        uint32_t m_fiberId = 0;             // 协程id
        uint64_t m_time = 0;                // 时间戳
        std::string m_content;

    };


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


    // 日志格式器
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        LogFormatter(std::string& pattern);

        std::string format(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event);

    public:
        class FormatItem {
        public:
            typedef std::shared_ptr<LogFormatter::FormatItem> ptr;
            FormatItem(const std::string& fmt = "") {}
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;
        };

        void init();

    private:
        std::string m_pattern;
        std::vector<FormatItem::ptr> m_items;

    };


    class MessageFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getContent();
        }
    };


    class LevelFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << LogLevel::ToString(level);
        }
    };


    class ElapseFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getElapse();
        }
    };


    class NameFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << logger->getName();
        }
    };


    class ThreadIdFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getThreadId();
        }
    };


    class FiberIdFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFiberId();
        }
    };


    class DateTimeFormatItem: public LogFormatter::FormatItem {
    public:
        DateTimeFormatItem(const std::string& format = "%Y:%m:%d %H:%M:%S") : m_format(format) {

        }

        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getTime();
        }

    private:
        std::string m_format;

    };


    class FilenameFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getFile();
        }
    };


    class LineFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << event->getLine();
        }
    };


    class NewLineFormatItem: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << std::endl;
        }
    };


    class StringFormatItem: public LogFormatter::FormatItem {
    public:
        StringFormatItem(const std:: string& str) : m_string(str) {}

        void format(std::ostream& os, std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override {
            os << m_string;
        }

    private:
        std::string m_string;
    };


    // 日志输出地
    class LogAppender {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        virtual ~LogAppender() {}

        // 纯虚函数，方法交由子类实现
        virtual void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) = 0;

        void setFormatter(LogFormatter::ptr val) { m_formatter = val; }
        LogFormatter::ptr getFormatter() const { return m_formatter; }
    protected:
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
    };


    // 日志器
    class Logger {
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
        LogLevel::Level m_level;                    // 日志级别
        std::string m_name;                         // 日志名称
        std::list<LogAppender::ptr> m_appenders;    // 日志输出集合
    };


    // 输出到console
    class StdoutLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;  // override指明是重载的方法
    };


    // 输出到文件
    class FileLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        FileLogAppender(const std::string& filename);
        void log(std::shared_ptr<Logger> logger, LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}  /* namespace mocker */


#endif /* __MOCKER_LOG_H__ */

