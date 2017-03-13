'use strict';

const fs = require('fs');
const { spawnSync } = require('child_process');
const { resolve } = require('path');

const kTopLevelDirectory = resolve(__dirname, '..');
const kAddonsDirectory = resolve(kTopLevelDirectory, 'test/addons');

const kPython = process.env.PYTHON || 'python';
const kNodeGyp =
    resolve(kTopLevelDirectory, 'deps/npm/node_modules/node-gyp/bin/node-gyp');

process.chdir(kTopLevelDirectory);

// Copy headers to test/addons/include.  install.py preserves timestamps.
{
  const args = [ 'tools/install.py', 'install', kAddonsDirectory, '/' ];
  const env = Object.assign({}, process.env);
  env.HEADERS_ONLY = 'yes, please';  // Ask nicely.
  env.LOGLEVEL = 'WARNING';

  const options = { env, stdio: 'inherit' };
  spawnSync(kPython, args, options);
}

// Scrape embedded add-ons from doc/api/addons.md.
require('./doc/addon-verify.js');

// Regenerate build files and rebuild if necessary.
for (const basedir of fs.readdirSync(kAddonsDirectory)) {
  const path = resolve(kAddonsDirectory, basedir);
  if (!fs.statSync(path).isDirectory()) continue;

  const gypfile = resolve(path, 'binding.gyp');
  if (!fs.existsSync(gypfile)) continue;

  const args = [ kNodeGyp,
                 '--directory=' + path,
                 '--loglevel=silent',
                 '--nodedir=' + kAddonsDirectory,
                 '--python=' + kPython ];
  const env = Object.assign({}, process.env);
  env.MAKEFLAGS = '-j1';

  const options = { env, stdio: 'inherit' };
  {
    const proc = spawnSync(process.execPath, args.concat('configure'), options);
    if (proc.status !== 0) process.exit(1);
  }
  {
    const proc = spawnSync(process.execPath, args.concat('build'), options);
    if (proc.status !== 0) process.exit(1);
  }
}
