#pragma once
#include "music.hpp"
#include <deque>
#include <memory>
namespace stone {
class MusicSynth {
public:
    explicit MusicSynth(const Song& song);
    ~MusicSynth();
    int sample_rate() const;
    std::vector<int16_t> generate(int samples);
    void replace_song(const Song& song);
    // As MusicDriver's.
    void fade_step(unsigned step) { driver.fade_step(step); }
    void fade_out() { driver.fade_out(); }
    void reset();

private:
    struct Chip;
    std::unique_ptr<Chip> chip;
    MusicDriver driver;
    std::deque<std::pair<uint8_t, uint8_t>> queued; // register writes not yet made
    int64_t until_tick = 0;
    void apply();
};
void write_wav(const std::string& path, int rate, const std::vector<int16_t>& samples);
} // namespace stone
