// Refs: https://github.com/nodejs/node/issues/14757
'use strict';
const common = require('../common');
const assert = require('assert');
const vm = require('vm');

{
  const s = '(function() { return [this, runInThisContext("this")] })';
  const f = vm.runInNewContext(s, vm);
  const [that0, that1] = f();
  assert.strictEqual(that0, that1);
  assert.notEqual(that0, global);
}
