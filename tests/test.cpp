//
// Created by ChaosChen on 2021/5/8.
//

#include "../mocker/log.h"
#include <iostream>

int main(int argc, char *argv[]) {
    mocker::Logger::ptr logger(new mocker::Logger);
    logger->addAppender(mocker::LogAppender::ptr (new mocker::StdoutLogAppender));
    mocker::LogEvent::ptr logEvent(new mocker::LogEvent(__FILE__, __LINE__, 0,
                                                        1, 2, time(0)));
    logEvent->getSS() << "hello mocker log";

    logger->log(mocker::LogLevel::DEBUG, logEvent);

    return 0;
}