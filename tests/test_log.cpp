//
// Created by ChaosChen on 2021/5/8.
//

#include <iostream>
#include "mocker/log.h"
#include "mocker/util.h"



int main(int argc, char *argv[]) {

//    mocker::Logger::ptr logger(new mocker::Logger);
//    logger->addAppender(mocker::LogAppender::ptr (new mocker::StdoutLogAppender(mocker::LogLevel::INFO)));
//    mocker::LogEvent::ptr logEvent(new mocker::LogEvent(__FILE__, __LINE__, 0, 0, "main", 0, 0, "root"));
//    logEvent->getSS() << "Hello, World!";
//    logger->log(mocker::LogLevel::WARN, logEvent);

//    mocker::LogAppender::ptr file_appender(new mocker::FileLogAppender("./log.txt"));
//    mocker::LogFormatter::ptr formatter(new mocker::LogFormatter("%d%T%m%n"));
//    file_appender->setFormatter(formatter);
//    logger->addAppender(file_appender);
//
//
//    MOCKER_LOG_DEBUG(logger) << "Hello mocker log";
//
    MOCKER_LOG_DEBUG(MOCKER_LOG_NAME("root")) << "THis a debug";
    MOCKER_LOG_INFO(MOCKER_LOG_NAME("root")) << "THis a info";
    MOCKER_LOG_WARN(MOCKER_LOG_NAME("root")) << "THis a warning";
    MOCKER_LOG_ERROR(MOCKER_LOG_NAME("root")) << "THis a error";
    MOCKER_LOG_FATAL(MOCKER_LOG_NAME("root")) << "THis a fatal";
//
//    MOCKER_LOG_FMT_WARN(logger, "WARN happened in %d 0x%X", 12, 15);
//
//    auto l = mocker::LoggerMgr::GetInstance()->getLogger("xx");
//    MOCKER_LOG_INFO(l) << "It comes from LoggerManager";

    return 0;
}