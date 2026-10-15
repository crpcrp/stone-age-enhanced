# ymfm

The YM3812 (OPL2) emulator from [ymfm](https://github.com/aaronsgiles/ymfm)
by Aaron Giles, BSD 3-Clause licence
([../licenses/ymfm.txt](../licenses/ymfm.txt)). These are the files the
OPL2 needs, from commit
`81aec25ccbb98f4873a255f7551ac4dadac59b4a` (2026-07-27), with one change.

`ymfm_fm.ipp`: an attack whose rate is 60 or more is instant, and one that
reaches 60 later stops, where ymfm draws that line at 62. The OPL's rate is
4 * AR + KSR, so attack rate 15 is instant whatever the key scaling, as in
Nuked-OPL3 and DOSBox. Unchanged, a snare keyed at rate 60 rose slowly and
froze when the driver retuned its channel for the hi-hat (rate 62), which
made the drums of the original's songs harsh.
