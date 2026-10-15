// Saves in the browser (BUILDING.md, Web version). Linked into index.js with
// --pre-js, so FS and IDBFS are in scope. /save is the player's folder for
// STONEAGE.ENH, STONEAGE.HI, FREEMAPS.HI, original.dat and the folders
// levels/ (level packs) and scores/ (their score tables), kept in the
// browser's IndexedDB.
Module.preRun = Module.preRun || [];
Module.preRun.push(function () {
  // The game starts only once the saves are read in.
  FS.mkdir('/save');
  FS.mount(IDBFS, {}, '/save');
  addRunDependency('saves');
  FS.syncfs(true, function (error) {
    if (error)
      console.error('The saves could not be read:', error);
    removeRunDependency('saves');
  });
});

// Writes /save to IndexedDB; play.cpp calls it after each file it saves.
// One write at a time: a call during a write starts one more after it, and
// `done` is called once what was in /save at the call is stored.
Module.storeSaves = (function () {
  var writing = false, again = [], waiting = [];
  function write() {
    writing = true;
    var callbacks = again;
    again = [];
    FS.syncfs(false, function (error) {
      if (error)
        console.error('The saves could not be stored:', error);
      callbacks.forEach(function (done) { done(error); });
      writing = false;
      if (again.length)
        write();
    });
  }
  return function (done) {
    again.push(done || function () {});
    if (!writing)
      write();
  };
})();

// A zip of the files in /save, for the page's Export saves button. Throws
// an Error with the reason when it cannot be made.
Module.exportSaves = function () {
  var problem = ccall('stone_export_saves', 'string', [], []);
  if (problem)
    throw new Error(problem);
  var zip = FS.readFile('/tmp/saves.zip');
  FS.unlink('/tmp/saves.zip');
  return zip;
};

// Imports a zip made by Export saves: its files replace those in /save. On
// success `done` gets null once they are stored, and the page then reloads so
// the game reads them; otherwise it gets the reason.
Module.importSaves = function (bytes, done) {
  FS.writeFile('/tmp/saves.zip', bytes);
  var problem = ccall('stone_import_saves', 'string', [], []);
  FS.unlink('/tmp/saves.zip');
  if (problem)
    return done(problem);
  Module.storeSaves(function (error) {
    done(error ? 'The saves could not be stored in this browser.' : null);
  });
};
