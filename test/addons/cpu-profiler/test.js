// Flags: --expose_gc --max_old_space_size=32 --single_threaded
'use strict';

const common = require('../../common');
const assert = require('assert');
const { start, stop } = require(`./build/${common.buildType}/binding`);

const before = process.memoryUsage().rss;
for (let i = 0; i < 1e3; i += 1) {
  start(String(i));
  for (const t = Date.now(); Date.now() < t + 1; /* empty */);
  stop(String(i));
}
global.gc();
const after = process.memoryUsage().rss;

// Memory usage is not an exact science but it seems reasonable to assume that,
// given the flags the process is started with, RSS should not grow by more
// than a few MB.  The memory leak this test originally exposed would grow RSS
// by about 460 MB, see https://github.com/nodejs/node/issues/14894.
assert(after - before < 25 << 20);
