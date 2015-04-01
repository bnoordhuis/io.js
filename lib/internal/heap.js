'use strict';

module.exports = Heap;

// TODO(bnoordhuis) Node manipulations should be monomorphic for maximum
// performance but the insert and remove methods are shared between heap
// instances, they become polymorphic or megamorphic when objects with
// different shapes are added or removed.  Perhaps it's best to export
// a factory function that generates new methods with `new Function(...)`.
//
// Note: using eval() for that is a bad idea because the generated code is
// exempt from optimizations but that was fixed for `new Function(...)` in
// https://codereview.chromium.org/821553003.
function Heap() {
  this.root = null;
  this.size = 0;
}

Heap.prototype = {
  insert: insert,
  remove: remove,
};

Heap.mixin = function(object) {
  // Use named properties rather than ES6 symbols.  Keyed property lookups
  // are 5-10x slower than named property lookups at the time of this writing.
  object._heapLeft = null;
  object._heapRight = null;
  object._heapParent = null;
  return object;
};

function insert(node, compare) {
  var size = (this.size += 1);  // Append position if the heap were an array.

  var parent = this.root;
  if (parent !== null) {
    // Traverse the tree to the append position.  Each bit of |size| below
    // the highest set bit indicates the leaf to take, left or right.
    for (var mask = 1 << log2(size) >>> 1; mask > 1; mask >>>= 1)
      parent = (size & mask) ? parent._heapRight : parent._heapLeft;

    if (size & 1)
      parent._heapRight = node;
    else
      parent._heapLeft = node;

    node._heapParent = parent;

    // Restore the heap property.
    while (parent !== null && compare(node, parent) < 0)
      parent = swap(parent, node);
  }

  if (parent === null)
    this.root = node;
}

function remove(node, compare) {
  var root = this.root;
  var size = this.size;
  this.size -= 1;

  // Traverse the tree to the append position.  Each bit of |size| below
  // the highest set bit indicates the leaf to take, left or right.
  var parent = root;
  for (var mask = 1 << log2(size) >>> 1; mask > 1; mask >>>= 1)
    parent = (size & mask) ? parent._heapRight : parent._heapLeft;

  if (size & 1) {
    var max = parent._heapRight;
    parent._heapRight = null;
  } else {
    var max = parent._heapLeft;
    parent._heapLeft = null;
  }

  if (max === null || max === node) {
    // We're removing either the last or the max node.
    if (node === root)
      this.root = null;
    else
      node._heapParent = null;
    return;
  }

  var left = node._heapLeft;
  var right = node._heapRight;
  var parent = node._heapParent;

  // It's not really necessary to null the fields of the node that is about
  // to be removed but it's fairly cheap to do so and it helps catch bugs.
  node._heapLeft = null;
  node._heapRight = null;
  node._heapParent = null;

  max._heapLeft = left;
  max._heapRight = right;
  max._heapParent = parent;

  if (parent === null)
    this.root = max;
  else if (parent._heapLeft === node)
    parent._heapLeft = max;
  else
    parent._heapRight = max;

  // Because a heap is a complete binary tree it follows that:
  // 1. Only nodes at the bottom level have null child nodes, and
  // 2. When the right node is null, the left node is null as well.
  if (left !== null) {
    left._heapParent = max;

    if (right !== null)
      right._heapParent = max;

    // Bubble down until the heap property is restored.
    // Swap the max node with its smallest child each time.
    do {
      var replacement = max;
      if (compare(replacement, left) > 0)
        replacement = left;

      if (right !== null && compare(replacement, right) > 0)
        replacement = right;

      if (replacement === max)
        break;  // Both children are larger, heap property restored.

      swap(max, replacement);

      if (parent === null)
        this.root = replacement;

      parent = replacement;
      left = max._heapLeft;
      right = max._heapRight;
    } while (left !== null);
  }

  // When removing a node that is not the root node, we need to bubble
  // down and up again to restore the heap property.  A separate check
  // for the root node is not needed because its parent is null.
  while (parent !== null && compare(max, parent) < 0)
    parent = swap(parent, max);

  if (parent === null)
    this.root = max;
}

function swap(parent, child) {
  var grandparent = parent._heapParent;

  if (grandparent !== null) {
    if (grandparent._heapLeft === parent)
      grandparent._heapLeft = child;
    else
      grandparent._heapRight = child;
  }

  var parentleft = parent._heapLeft;
  var parentright = parent._heapRight;
  var childleft = child._heapLeft;
  var childright = child._heapRight;

  parent._heapLeft = childleft;
  parent._heapRight = childright;
  parent._heapParent = child;

  // The left child is null when the right child is null.  The reverse
  // is not true: a null right child does not imply a null left child.
  if (childleft !== null) {
    childleft._heapParent = parent;
    if (childright !== null)
      childright._heapParent = parent;
  }

  if (child === parentleft) {
    child._heapLeft = parent;
    child._heapRight = parentright;
    if (parentright !== null)
      parentright._heapParent = child;
  } else {
    child._heapLeft = parentleft;
    child._heapRight = parent;
    parentleft._heapParent = child;
  }
  child._heapParent = grandparent;

  return grandparent;
}

// Returns the highest set bit.  Equivalent to 31 - Math.clz32(v | 1) but
// slightly faster.
//
// TODO(bnoordhuis) Benchmark again in the future.  Upstream V8 has support
// for lowering Math.clz32() calls to LZCNT instructions.  It should be able
// to wipe the floor with our log2() function.
function log2(v) {
  var r, s;

  v = v | 0;
  r = (v > 65535) << 4;
  v = v >>> r;

  s = (v > 255) << 3;
  v = v >>> s;
  r = r | s;

  s = (v > 15) << 2;
  v = v >>> s;
  r = r | s;

  s = (v > 3) << 1;
  v = v >>> s;
  r = r | s;

  return r | (v >>> 1);
}
