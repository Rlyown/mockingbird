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

namespace mocker {

    // 日志事件
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;
        LogEvent();

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
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5
        };
    };


    // 日志格式器
    class LogFormatter {
    public:
        typedef std::shared_ptr<LogFormatter> ptr;

        std::string format(LogEvent::ptr event);
    };


    // 日志输出地
    class LogAppender {
    public:
        typedef std::shared_ptr<LogAppender> ptr;

        virtual ~LogAppender() {}

        // 纯虚函数，方法交由子类实现
        virtual void log(LogLevel::Level level, LogEvent::ptr event) = 0;

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

    private:
        LogLevel::Level m_level;                    // 日志级别
        std::string m_name;                         // 日志名称
        std::list<LogAppender::ptr> m_appenders;    // 日志输出集合
    };


    // 输出到console
    class StdoutLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<StdoutLogAppender> ptr;

        void log(LogLevel::Level level, LogEvent::ptr event) override;  // override指明是重载的方法
    };

    // 输出到文件
    class FileLogAppender: public LogAppender {
    public:
        typedef std::shared_ptr<FileLogAppender> ptr;

        FileLogAppender(const std::string& filename);
        void log(LogLevel::Level level, LogEvent::ptr event) override;

        bool reopen();
    private:
        std::string m_filename;
        std::ofstream m_filestream;
    };
}  /* namespace mocker */


#endif /* __MOCKER_LOG_H__ */

