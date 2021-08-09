//
// Created by ChaosChen on 2021/8/9.
//

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <mocker/mocker.h>
#include <mocker/iomanager.h>
#include <netinet/in.h>

mocker::Logger::ptr g_logger = MOCKER_LOG_ROOT();

void test_coroutine() {
    MOCKER_LOG_INFO(g_logger) << "test_coroutine";
}

void test1() {
    mocker::IOManager iom;
    iom.schedule(&test_coroutine);

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "110.242.68.4", &addr.sin_addr.s_addr);

    iom.addEvent(sock, mocker::IOManager::READ, [](){
        MOCKER_LOG_INFO(g_logger) << "read connected";
    });
    iom.addEvent(sock, mocker::IOManager::WRITE, [=](){
        MOCKER_LOG_INFO(g_logger) << "write connected";
        mocker::IOManager::GetCurrent()->cancelEvent(sock, mocker::IOManager::READ);
        close(sock);
    });
    int rt = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    fcntl(sock, F_SETFL, O_NONBLOCK);
    if (rt)
        MOCKER_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno << " (" << strerror(errno) << ")";
}

int main(int argc, char *argv[]) {
    test1();
    return 0;
}