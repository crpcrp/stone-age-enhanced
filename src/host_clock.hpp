#pragma once
#include <cstdint>
namespace stone {
// Host time only. Original VGA waits remain counts of 70-Hz logical ticks.
class HostTickClock {
public:
    void add(uint64_t elapsed_ns, bool one_test_tick = false) {
        if (one_test_tick) {
            units = 1000000000;
            return;
        }
        units += (elapsed_ns > 100000000 ? 100000000 : elapsed_ns) * 70;
    }
    bool next() {
        if (units < 1000000000)
            return false;
        units -= 1000000000;
        return true;
    }
    void reset() { units = 0; }

private:
    uint64_t units = 0;
};
} // namespace stone
