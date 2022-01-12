#ifndef __ARP_SOCKET_H__
#define __ARP_SOCKET_H__

#include <string>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <memory>
#include <cerrno>

extern "C" {
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
}

#include "arpPacket.h"

class ArpSocket {
private:
    int _fd;
    static const size_t _bufSize = 512;
    std::shared_ptr<uint8_t> _buf;
public:
    ArpSocket() : _fd(-1), _buf(nullptr) {}
    ArpSocket(const std::string& ifaceName) : _buf(new uint8_t[_bufSize]) {
        unsigned ifIndex = if_nametoindex(ifaceName.c_str());
        if (ifIndex == 0)
            throw std::runtime_error(std::string("No interface with name ") + ifaceName);

        _fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if (_fd == -1)
            throw std::runtime_error(std::string("Failed to open a socket: ") + std::string(strerror(errno)));

        struct sockaddr_ll iface = {
            .sll_family = AF_PACKET,
            //.sll_protocol = htons(ETH_P_ALL),
            .sll_ifindex = (int)ifIndex,
        };
        if (bind(_fd, (const sockaddr*) &iface, sizeof(iface)) != 0)
            throw std::runtime_error(std::string("Failed to bind: ") + std::string(strerror(errno)));
    }

    ArpSocket(const ArpSocket&) = delete;
    ArpSocket& operator=(const ArpSocket&) = delete;

    ArpSocket(ArpSocket&& other) : _fd(other._fd), _buf(std::move(other._buf)) {
        other._fd = -1;
    }
    ArpSocket& operator=(ArpSocket&& other) {
        if (this != &other) {
            this->~ArpSocket();
            new (this) ArpSocket(std::move(other));
        }
        return *this;
    }

    ArpPacket Poll(void) {
        if (_fd == -1)
            throw std::runtime_error("Polling invalid socket");

        int rc = read(_fd, _buf.get(), _bufSize);
        if (rc <= 0)
            throw std::runtime_error(std::string("Socket poll failed: ") + std::string(strerror(errno)));

        return ArpPacket(_buf, rc);
    }

    ~ArpSocket() {
        if (_fd != -1) {
            close(_fd);
        }
    }
};

#endif /* __ARP_SOCKET_H__ */