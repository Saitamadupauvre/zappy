#pragma once

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include <stdexcept>
#include <string>

constexpr int SOCKET_FAILED = -1;
constexpr int EMPTY_SOCKET = -1;

class Socket
{
public:
    Socket(): _fd(EMPTY_SOCKET) {}
    Socket(int domain, int type);
    ~Socket();

    Socket(Socket&&) noexcept;
    Socket& operator=(Socket&&) noexcept;

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;

    explicit operator bool() const noexcept { return _fd != EMPTY_SOCKET; }

    void setReuseAddr();

    void bind(const sockaddr_in& addr);
    static sockaddr_in makeAddress(uint16_t port, const std::string* ip = nullptr);

    void listen(int backlog = SOMAXCONN);

    Socket accept();

    void connect(sockaddr_in serverAddr);
    std::string read(size_t maxBytes = 1024);

    ssize_t writeSome(const void* data, size_t size);
    void writeAll(const void* data, size_t size);
    void writeAll(std::string_view data);

    template<typename... T>
    void writeMulti(const T&... parts)
    {
        (writeAll(std::string_view(parts)), ...);
    }

    int fd() const { return _fd; }

    sockaddr_in peerAddress() const;
    std::string peerIP() const;
    uint16_t peerPort() const;

    void close();

    class SocketException : public std::exception
    {
    public:
        SocketException(const std::string& msg): _msg(msg) {}
        const char* what() const noexcept override { return _msg.c_str(); }
    private:
        std::string _msg;
    };

private:
    int _fd;

    Socket(int fd): _fd(fd) {}
};