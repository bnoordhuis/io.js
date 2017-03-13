'use strict';

const fs = require('fs');
const os = require('os');
const { spawn, spawnSync } = require('child_process');
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
let failures = 0;
process.on('exit', () => process.exit(failures > 0));

const jobs = [];

for (const basedir of fs.readdirSync(kAddonsDirectory)) {
  const path = resolve(kAddonsDirectory, basedir);
  if (!fs.statSync(path).isDirectory()) continue;

  const gypfile = resolve(path, 'binding.gyp');
  if (!fs.existsSync(gypfile)) continue;

  exec(path, 'configure', () => exec(path, 'build'));
}

for (const _ of os.cpus()) next();

function next() {
  const job = jobs.shift();
  if (job) job();
}

function exec(path, command, done) {
  jobs.push(() => {
    if (failures > 0) return;

    const args = [kNodeGyp,
                  '--directory=' + path,
                  '--loglevel=silent',
                  '--nodedir=' + kAddonsDirectory,
                  '--python=' + kPython,
                  command];

    const env = Object.assign({}, process.env);
    env.MAKEFLAGS = '-j1';

    const options = { env, stdio: 'inherit' };
    const proc = spawn(process.execPath, args, options);

    proc.on('exit', (exitCode) => {
      if (exitCode !== 0) ++failures;
      if (done) done();
      next();
    });

    console.log(command, path);
  });
}
