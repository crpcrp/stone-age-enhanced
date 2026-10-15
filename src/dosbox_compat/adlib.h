#pragma once
#include "dosbox.h"
// DBOPL's optional DOSBox mixer adapter is unused by the standalone host.
struct MixerChannel {
    void AddSamples_m32(Bitu, const Bit32s*) {}
    void AddSamples_s32(Bitu, const Bit32s*) {}
};
namespace Adlib {
struct Handler {
    virtual ~Handler() = default;
    virtual Bit32u WriteAddr(Bit32u, Bit8u) = 0;
    virtual void WriteReg(Bit32u, Bit8u) = 0;
    virtual void Generate(MixerChannel*, Bitu) = 0;
    virtual void Init(Bitu) = 0;
};
} // namespace Adlib
