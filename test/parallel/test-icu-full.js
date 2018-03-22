'use strict';
const common = require('../common');
const assert = require('assert');

if (!common.hasIntl)
  common.skip('missing Intl');

assert.strictEqual(Intl.NumberFormat('nl-NL').resolvedOptions().locale,
                   'nl-NL');
