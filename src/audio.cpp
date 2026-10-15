#include "audio.hpp"
#include "dbopl.h"
#include <algorithm>
#include <stdexcept>
namespace DBOPL {
void InitTables();
}
namespace stone {
MusicSynth::MusicSynth(const Song& song) : chip(new DBOPL::Chip), driver(song) {
    // Match the user's reference: DOSBox 0.74-3, default DBOPL at 44100 Hz.
    // DBOPL global lookup tables are immutable after this one-time setup.
    static const bool tables_ready = []() {
        DBOPL::InitTables();
        return true;
    }();
    (void)tables_ready;
    chip->Setup(sample_rate());
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
    return 44100;
}
void MusicSynth::apply() {
    // DBOPL processes key edges when written, just as DOSBox does. Inserting
    // invented chip clocks changes attack/release and transient note pitches.
    for (auto w : driver.writes)
        chip->WriteReg(w.first, w.second);
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
        Bit32s output = 0;
        chip->GenerateBlock2(1, &output);
        // Original DOSBox 0.74-3 FM mixer uses SetScale(2.0).
        sample = int16_t(std::max(-32768, std::min(32767, output * 2)));
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
