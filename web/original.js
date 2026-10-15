// The original game in the browser (BUILDING.md, Web version). Linked into
// index.js with --pre-js after saves.js. The player adds a zip of the
// original's files; play.cpp packs it into /save/original.dat, which stays in
// this browser like the saves, and the game then offers ORIG in Options F2.

// Whether this browser holds the original game.
Module.hasOriginal = function () {
  return FS.analyzePath('/save/original.dat').exists;
};

// Packs the zip's original game into /save/original.dat. `done` gets null once
// it is stored, and the page then reloads so the game finds it; otherwise it
// gets the reason. The packing runs at the game's next frame.
Module.addOriginal = function (bytes, done) {
  FS.writeFile('/tmp/original.zip', bytes);
  Module.originalPacked = function () {
    Module.originalPacked = null;
    var problem = ccall('stone_original_problem', 'string', [], []);
    if (problem)
      return done(problem);
    Module.storeSaves(function (error) {
      done(error ? 'The original game could not be stored in this browser.' : null);
    });
  };
  ccall('stone_add_original', null, [], []);
};

// Deletes /save/original.dat; the asset choices in STONEAGE.ENH fall back to
// the free set without it.
Module.removeOriginal = function (done) {
  if (Module.hasOriginal())
    FS.unlink('/save/original.dat');
  Module.storeSaves(function (error) {
    done(error ? 'The change could not be stored in this browser.' : null);
  });
};
