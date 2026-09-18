#include "core/processes/process-manager.hpp"

#include <cassert>
#include <sstream>

int main() {
    std::istringstream statusInput(
        "Name:\tbash\n"
        "State:\tS (sleeping)\n"
        "VmRSS:\t    4096 kB\n");
    const xenon::ProcessInfo process = xenon::ProcessManager::parseStatus(1234, statusInput);

    assert(process.pid == 1234);
    assert(process.name == "bash");
    assert(process.state == "S (sleeping)");
    assert(process.memoryBytes == 4194304ULL);

    return 0;
}