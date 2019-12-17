#ifndef SRC_HEAP_UTILS_H_
#define SRC_HEAP_UTILS_H_

#if defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#include "util.h"
#include "v8.h"
#include "v8-profiler.h"

namespace node {
namespace heap {

class JSGraphJSNode : public v8::EmbedderGraph::Node {
 public:
  const char* Name() override { return "<JS Node>"; }
  size_t SizeInBytes() override { return 0; }
  bool IsEmbedderNode() override { return false; }

  v8::Local<v8::Value> JSValue() {
    return PersistentToLocal::Strong(persistent_);
  }

  int IdentityHash();

  JSGraphJSNode(v8::Isolate* isolate, v8::Local<v8::Value> val)
      : persistent_(isolate, val) {
    CHECK(!val.IsEmpty());
  }

  struct Hash {
    inline size_t operator()(JSGraphJSNode* n) const {
      return static_cast<size_t>(n->IdentityHash());
    }
  };

  struct Equal {
    inline bool operator()(JSGraphJSNode* a, JSGraphJSNode* b) const {
      return a->JSValue()->SameValue(b->JSValue());
    }
  };

 private:
  v8::Global<v8::Value> persistent_;
};

class JSGraph : public v8::EmbedderGraph {
 public:
  using Node = v8::EmbedderGraph::Node;
  using Nodes = std::unordered_set<std::unique_ptr<Node>>;
  using EngineNodes =
      std::unordered_set<JSGraphJSNode*,
                         JSGraphJSNode::Hash,
                         JSGraphJSNode::Equal>;
  using Edges =
      std::unordered_map<Node*, std::set<std::pair<const char*, Node*>>>;

  explicit JSGraph(v8::Isolate* isolate) : isolate_(isolate) {}

  const Nodes& nodes() const { return nodes_; }
  const Edges& edges() const { return edges_; }
  const EngineNodes& engine_nodes() const { return engine_nodes_; }

  Node* V8Node(const v8::Local<v8::Value>& value) override {
    std::unique_ptr<JSGraphJSNode> n { new JSGraphJSNode(isolate_, value) };
    auto it = engine_nodes_.find(n.get());
    if (it != engine_nodes_.end())
      return *it;
    engine_nodes_.insert(n.get());
    return AddNode(std::unique_ptr<Node>(n.release()));
  }

  Node* AddNode(std::unique_ptr<Node> node) override {
    Node* n = node.get();
    nodes_.emplace(std::move(node));
    return n;
  }

  void AddEdge(Node* from, Node* to, const char* name = nullptr) override {
    edges_[from].insert(std::make_pair(name, to));
  }

  v8::MaybeLocal<v8::Array> CreateObject() const;

 private:
  v8::Isolate* isolate_;
  Nodes nodes_;
  EngineNodes engine_nodes_;
  Edges edges_;
};

}  // namespace heap
}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_HEAP_UTILS_H_
