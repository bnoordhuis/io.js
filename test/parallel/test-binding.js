// Flags: --allow-natives-syntax --expose-internals
'use strict';
require('../common');
const assert = require('assert');
assert(eval('%HasFastProperties(require("internal/binding"))'));
