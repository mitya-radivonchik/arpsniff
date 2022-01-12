#ifndef __ARP_SNIFFER_H__
#define __ARP_SNIFFER_H__

#include <string>
#include <map>

#include "arpSocket.h"
#include "vendorMap.h"

class ArpSniffer {
private:
    ArpSocket _socket;
    bool _valid;
    VendorMap _vendors;
public:
    ArpSniffer& operator=(const ArpSniffer&) = delete;
    ArpSniffer(const ArpSniffer&) = delete;

    ArpSniffer(ArpSniffer&& other) : _socket(std::move(other._socket)),
                                     _valid(other._valid),
                                     _vendors(std::move(other._vendors))
    {
        other._valid = false;
    }

    ArpSniffer& operator=(ArpSniffer&& other) {
        this->~ArpSniffer();
        new (this) ArpSniffer(std::move(other));
        return *this;
    }

    ArpSniffer(const std::string& ouiPath, const std::string& ifaceName) : _socket(ifaceName),
                                                                           _valid(true),
                                                                           _vendors(ouiPath) { }

    ArpSniffer() : _valid(false) {}

    void Poll() {
        if (!_valid)
            throw std::runtime_error("Polling invalid ArpSniffer");

        _socket.Poll().Print(_vendors);
    }
};

#endif /* __ARP_SNIFFER_H__ */