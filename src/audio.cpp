#include "audio.hpp"
#include "ymfm_opl.h"
#include <algorithm>
#include <stdexcept>
namespace stone {
// The AdLib's YM3812 on its 3.579545 MHz crystal, emulated by ymfm. It needs
// no timers or interrupts from the host: the driver runs on the PIT below.
struct MusicSynth::Chip {
    ymfm::ymfm_interface host;
    ymfm::ym3812 opl{host};
};
MusicSynth::MusicSynth(const Song& song) : chip(new Chip), driver(song) {
    chip->opl.reset();
    apply();
}
MusicSynth::~MusicSynth() = default;
void MusicSynth::replace_song(const Song& song) {
    driver.replace_song(song);
    apply();
}
void MusicSynth::reset() {
    driver.reset();
    apply();
}
int MusicSynth::sample_rate() const {
    // The chip's own rate, clock / 72; SDL resamples it for the device.
    return 49716;
}
void MusicSynth::apply() {
    queued.insert(queued.end(), driver.writes.begin(), driver.writes.end());
}
std::vector<int16_t> MusicSynth::generate(int count) {
    if (count < 0 || count > sample_rate() * 120)
        throw std::runtime_error("Invalid audio sample count");
    std::vector<int16_t> samples(count);
    // FXDRIVER chains the unmodified BIOS PIT channel-0 interrupt (18.2065 Hz).
    const int64_t period = int64_t(sample_rate()) * 65536 * 12;
    for (auto& sample : samples) {
        if (until_tick <= 0) {
            driver.tick();
            apply();
            until_tick += period;
        }
        // One register write a sample. An AdLib needs 23 microseconds after
        // each write, a little more than a sample, and ymfm, like the chip,
        // sees a key only when it clocks: a note keyed off and on again
        // between two samples would not start again.
        if (!queued.empty()) {
            chip->opl.write_address(queued.front().first);
            chip->opl.write_data(queued.front().second);
            queued.pop_front();
        }
        ymfm::ym3812::output_data output;
        chip->opl.generate(&output);
        // Twice ymfm's level: as loud as these songs were with DOSBox's OPL.
        sample = int16_t(std::max(-32768, std::min(32767, output.data[0] * 2)));
        until_tick -= 14318180;
    }
    return samples;
}
void write_wav(const std::string& path, int rate, const std::vector<int16_t>& samples) {
    Bytes b(44 + samples.size() * 2);
    auto word = [&](int p, unsigned v, int n) {
        for (int k = 0; k < n; ++k)
            b[p + k] = uint8_t(v >> (k * 8));
    };
    auto label = [&](int p, const char* s) {
        for (int k = 0; k < 4; ++k)
            b[p + k] = s[k];
    };
    label(0, "RIFF");
    word(4, b.size() - 8, 4);
    label(8, "WAVE");
    label(12, "fmt ");
    word(16, 16, 4);
    word(20, 1, 2);
    word(22, 1, 2);
    word(24, rate, 4);
    word(28, rate * 2, 4);
    word(32, 2, 2);
    word(34, 16, 2);
    label(36, "data");
    word(40, samples.size() * 2, 4);
    for (std::size_t k = 0; k < samples.size(); ++k)
        word(int(44 + k * 2), uint16_t(samples[k]), 2);
    write_file(path, b);
}
} // namespace stone
