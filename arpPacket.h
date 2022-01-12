#ifndef __ARP_PACKET_H__
#define __ARP_PACKET_H__

#include <array>
#include <memory>
#include <cstring>
#include <iostream>
#include <map>

extern "C" {
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
}

#include "vendorMap.h"

class ArpPacket {
public:
    enum class Type { Request, Reply };
    using Mac = std::array<uint8_t, ETH_ALEN>;
private:
    bool _valid;
    Type _type;
    Mac _srcMac;
    Mac _dstMac;
    static std::string mac2str(const Mac& mac) {
        const int len = 2 * ETH_ALEN + (ETH_ALEN - 1);
        char str[len + 1];
        std::snprintf(str, len + 1, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(str, len);
    }
    static VendorMap::MacPrefix mac2prefix(const Mac& mac) {
        return { mac[0], mac[1], mac[2] };
    }
public:
    ArpPacket() : _valid(false) {}
    ArpPacket(const uint8_t* buf, size_t len) : _valid(false) {
        if (len < sizeof(struct ethhdr) + sizeof(struct arphdr))
            return;

        const struct ethhdr* eth = (struct ethhdr*) buf;
        if (ntohs(eth->h_proto) != ETH_P_ARP)
            return;

        const struct arphdr* hdr = (struct arphdr*) (eth + 1);
        if (ntohs(hdr->ar_hrd) != ARPHRD_ETHER)
            return;

        if (len < sizeof(struct ethhdr) + sizeof(struct arphdr) + 2 * hdr->ar_pln + 2 * hdr->ar_hln)
            return;

        switch (ntohs(hdr->ar_op)) {
        case ARPOP_REQUEST:
            _type = Type::Request;
            break;
        case ARPOP_REPLY:
            _type = Type::Reply;
            break;
        default:
            return;
        }

        uint8_t* data = (uint8_t*) (hdr + 1);
        memcpy(_srcMac.data(), data, ETH_ALEN);
        if (_type == Type::Reply)
            memcpy(_dstMac.data(), data + ETH_ALEN + hdr->ar_pln, ETH_ALEN);
        
        _valid = true;
    }

    void Print(const VendorMap& vendors) const {
        if (!_valid)
            return;

        std::cout << "Type: " << ((_type == Type::Reply) ? "Reply" : "Request") << "\n";
        std::cout << "Source MAC: " << mac2str(_srcMac) << "\n";
        std::cout << "Source vendor: " << vendors.Lookup(mac2prefix(_srcMac)) << "\n";
        if (_type == Type::Reply) {
            std::cout << "Destination MAC: " << mac2str(_dstMac) << "\n";
            std::cout << "Destination vendor: " << vendors.Lookup(mac2prefix(_dstMac)) << "\n";
        }
        std::cout << std::endl;
    }
};

#endif /* __ARP_PACKET_H__ */