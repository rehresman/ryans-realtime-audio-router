#include "SimDriver.h"


int main() {
    const int sampleRate = 48000;
    const int runDuration = 6;
    const bool simulateOverflow = false;
    const bool simulateUnderrun = false;
    auto driver = SimDriver(sampleRate, runDuration, simulateOverflow, simulateUnderrun);

    driver.run();
    return 0;
}