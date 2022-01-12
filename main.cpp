#include "arpSniffer.h"

bool checkArgs(int argc, char** argv) {
    return argc == 3;
}

void printUsage(const char* argv0) {
    std::cout << "usage: " << argv0 << " <ouiPath> <iface>" << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        printUsage(argv[0]);
        return 0;
    }

    ArpSniffer sniffer(argv[1], argv[2]);

    while (true)
        sniffer.Poll();
    
    return 0;
}