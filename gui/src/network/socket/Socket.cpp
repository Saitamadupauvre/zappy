#include "./Socket.hpp"

#include <cerrno>
#include <vector>

Socket::Socket(int domain, int type)
{
    _fd = socket(domain, type, 0);

    if (_fd == EMPTY_SOCKET) {
        throw SocketException("Failed to create socket");
    }
}

Socket::~Socket()
{
    close();
}

void Socket::close()
{
    if (_fd != EMPTY_SOCKET) {
        ::close(_fd);
        _fd = EMPTY_SOCKET;
    }
}

Socket::Socket(Socket&& other) noexcept : _fd(other._fd)
{
    other._fd = SOCKET_FAILED;
}

Socket& Socket::operator=(Socket&& other) noexcept
{
    if (this != &other) {
        if (_fd != EMPTY_SOCKET)
            ::close(_fd);

        _fd = other._fd;
        other._fd = SOCKET_FAILED;
    }
    return *this;
}

void Socket::setReuseAddr()
{
    if (_fd == SOCKET_FAILED)
        throw SocketException("Socket is closed");

    int opt = 1;
    if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw SocketException("setsockopt(SO_REUSEADDR) failed");
}

void Socket::bind(const sockaddr_in& addr)
{
    if (::bind(_fd, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw SocketException("Bind failed");
    }
}

sockaddr_in Socket::makeAddress(uint16_t port, const std::string* ip)
{
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (ip && *ip != "0.0.0.0")
        inet_pton(AF_INET, ip->c_str(), &addr.sin_addr);
    else
        addr.sin_addr.s_addr = INADDR_ANY;
    return addr;
}

void Socket::listen(int backlog)
{
    if (::listen(_fd, backlog) == SOCKET_FAILED) {
        throw SocketException("Listen failed");
    }
}

Socket Socket::accept()
{
    sockaddr_in clientAddr{};
    socklen_t len = sizeof(clientAddr);

    int clientFd = ::accept(_fd, reinterpret_cast<sockaddr*>(&clientAddr), &len);
    if (clientFd < 0) {
        throw SocketException("Accept failed");
    }
    return Socket(clientFd);
}

void Socket::connect(sockaddr_in serverAddr)
{
    if (::connect(_fd, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
        throw SocketException("Connect failed");
    }
}

std::string Socket::read(size_t maxBytes)
{
    if (_fd == SOCKET_FAILED) {
        throw SocketException("Can't read on a closed socket");
    }

    std::vector<char> buffer(maxBytes);
    ssize_t n = -1;
    do {
        n = ::read(_fd, buffer.data(), maxBytes);
    } while (n < 0 && errno == EINTR);

    if (n < 0) {
        throw SocketException("Read failed");
    }
    return std::string(buffer.data(), n);
}

ssize_t Socket::writeSome(const void* data, size_t size)
{
    if (_fd == SOCKET_FAILED)
        throw SocketException("Write on closed socket");

    ssize_t sent = -1;
    do {
        sent = ::write(_fd, data, size);
    } while (sent < 0 && errno == EINTR);

    if (sent < 0)
        throw SocketException("Write failed");

    return sent;
}

void Socket::writeAll(const void* data, size_t size)
{
    const char* ptr = static_cast<const char*>(data);
    size_t total = 0;

    while (total < size) {
        ssize_t sent = writeSome(ptr + total, size - total);
        total += static_cast<size_t>(sent);
    }
}

void Socket::writeAll(std::string_view data)
{
    writeAll(data.data(), data.size());
}

sockaddr_in Socket::peerAddress() const
{
    if (_fd == SOCKET_FAILED)
        throw SocketException("Socket closed");

    sockaddr_in addr{};
    socklen_t len = sizeof(addr);
    if (getpeername(_fd, reinterpret_cast<sockaddr*>(&addr), &len) < 0)
        throw SocketException("getpeername failed");

    return addr;
}

std::string Socket::peerIP() const
{
    sockaddr_in addr = peerAddress();
    char ip[INET_ADDRSTRLEN];

    if (!inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)))
        throw SocketException("inet_ntop failed");
    return std::string(ip);
}

uint16_t Socket::peerPort() const
{
    sockaddr_in addr = peerAddress();

    return ntohs(addr.sin_port);
}
