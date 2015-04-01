// Flags: --expose_internals --random_seed=42

'use strict';

var common = require('../common');
var assert = require('assert');
var Heap = require('internal/heap');

var numbers = [];
for (var n = 0; n < 1e3; n += 1)
  numbers.push(Heap.mixin({ n: n }));
shuffle(numbers);

var heap = new Heap;
for (var n = 0; n < numbers.length; n += 1) {
  heap.insert(numbers[n], compare);
  assert.equal(heap.size, n + 1);
  assert.equal(verify(heap.root), n + 1);
}

shuffle(numbers);

while (numbers.length > 0) {
  heap.remove(numbers.pop(), compare);
  assert.equal(heap.size, numbers.length);
  assert.equal(verify(heap.root), numbers.length);
}

assert.equal(heap.size, 0);

function shuffle(a) {
  for (var i = 0, n = a.length; i < n; i += 1) {
    var k = i + (Math.random() * (n - i)) | 0;
    var t = a[i];
    a[i] = a[k];
    a[k] = t;
  }
  return a;
}

function compare(a, b) {
  return (b.n < a.n) - (a.n < b.n);
}

function verify(node) {
  if (node === null)
    return 0;

  var left = node._heapLeft;
  var right = node._heapRight;
  if (left === null) {
    assert.equal(right, null);
  } else {
    assert(node.n < left.n);
    if (right !== null)
      assert(node.n < right.n);
  }

  return 1 + verify(left) + verify(right);
}
