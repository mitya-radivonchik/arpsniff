#ifndef __VENDOR_MAP_H__
#define __VENDOR_MAP_H__

#include <map>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

class VendorMap {
public:
    static const size_t MacPrefixLen = 3;
    typedef std::array<uint8_t, MacPrefixLen> MacPrefix;
private:
    std::map<MacPrefix, std::string> _vendors;

    static std::pair<MacPrefix, std::string> parse(const std::string& s) {
        std::stringstream ss(s);
        std::string byte0, byte1, byte2, tmp, vendor;
        std::getline(ss, byte0, '-');
        std::getline(ss, byte1, '-');
        ss >> byte2 >> tmp >> std::ws;
        std::getline(ss, vendor);

        MacPrefix prefix = { (uint8_t)std::stoi(byte0, nullptr, 16),
                             (uint8_t)std::stoi(byte1, nullptr, 16),
                             (uint8_t)std::stoi(byte2, nullptr, 16) };

        return std::make_pair(std::move(prefix), std::move(vendor));
    }
public:
    VendorMap(const std::string& ouiPath) {
        std::fstream s(ouiPath);
        if (!s.is_open())
            throw std::runtime_error(std::string("Could not open oui file: ") + ouiPath);
        
        std::string line;
        // Skip header
        std::getline(s, line);
        std::getline(s, line);
        std::getline(s, line);
        std::getline(s, line);

        while (!s.eof()) {
            std::getline(s, line);
            if (s.eof())
                return;

            if (line.find("(hex)") != std::string::npos)
                _vendors.insert(parse(line));
        }
    }

    std::string Lookup(const MacPrefix& mac) const {
        auto it = _vendors.find(mac);
        if (it == _vendors.end())
            return std::string("<unknown vendor>");

        return it->second;
    }

    VendorMap() = default;
    VendorMap(VendorMap&&) = default;
    VendorMap(const VendorMap&) = default;
    VendorMap& operator=(VendorMap&&) = default;
    VendorMap& operator=(const VendorMap&) = default;
};

#endif /* __VENDOR_MAP_H__ */