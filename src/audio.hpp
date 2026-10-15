#pragma once
#include "music.hpp"
#include <memory>
namespace DBOPL {
struct Chip;
}
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
    std::unique_ptr<DBOPL::Chip> chip;
    MusicDriver driver;
    int64_t until_tick = 0;
    void apply();
};
void write_wav(const std::string& path, int rate, const std::vector<int16_t>& samples);
} // namespace stone
