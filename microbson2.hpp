// microbson2.hpp

#pragma once

#include <algorithm>
#include <stdexcept>
#include <string_view>

namespace microbson {
class Node {
public:
  enum class NodeType {
    None      = 0x00,
    Double    = 0x01,
    String    = 0x02,
    Document  = 0x03,
    Array     = 0x04,
    Binary    = 0x05,
    Boolean   = 0x08,
    Null      = 0x0A,
    Int32     = 0x10,
    Timestamp = 0x11,
    Int64     = 0x12,
    Unknown   = 0xFF
  };

  Node(const void *bytes)
      : bytes_{reinterpret_cast<const uint8_t *>(bytes)} {
  }

  /**\return true if the node points to nullptr
   */
  bool empty() const {
    return !bytes_;
  }

  NodeType type() const {
    return bytes_ ? static_cast<NodeType>(bytes_[0]) : NodeType::None;
  }

  std::string_view key() const {
    return bytes_ ? std::string_view{reinterpret_cast<const char *>(bytes_ + 1)}
                  : std::string_view{};
  }

  const void *data() const {
    return bytes_ + 1 + this->key().size() + 1; // type + name + \0
  }

  size_t size() const {
    size_t retval = 1 + this->key().size() + 1; // type + name + \0

    switch (this->type()) {
    case NodeType::Double:
      retval += sizeof(double);
      break;
    case NodeType::String:
      retval +=
          (sizeof(int) +
           *reinterpret_cast<const int *>(
               bytes_ + retval)); // size of string + len of string (include \0)
      break;
    case NodeType::Document:
    case NodeType::Array:
      retval += *reinterpret_cast<const int *>(bytes_ + retval);
      break;
    case NodeType::Binary:
      retval +=
          (sizeof(int) + 1 +
           *reinterpret_cast<const int *>(
               bytes_ + retval)); // size of node + subtype + len of binary
      break;
    case NodeType::Boolean:
      retval += 1;
      break;
    case NodeType::Int32:
      retval += sizeof(int32_t);
      break;
    case NodeType::Int64:
    case NodeType::Timestamp:
      retval += sizeof(int64_t);
      break;
    default:
      retval = 0;
    }

    return retval;
  }

  operator std::string_view() const {
    return this->type() == NodeType::String
               ? std::string_view{reinterpret_cast<const char *>(this->data()) +
                                  sizeof(int32_t)}
               : std::string_view{};
  }

  explicit operator int32_t() const {
    return this->type() == NodeType::Int32
               ? *reinterpret_cast<const int32_t *>(this->data())
               : 0;
  }

  explicit operator int64_t() const {
    return this->type() == NodeType::Int64
               ? *reinterpret_cast<const int64_t *>(this->data())
               : 0;
  }

  explicit operator double() const {
    return this->type() == NodeType::Int64
               ? *reinterpret_cast<const double *>(this->data())
               : 0;
  }

  explicit operator bool() const {
    return this->type() == NodeType::Boolean
               ? *reinterpret_cast<const bool *>(this->data())
               : 0;
  }

  bool operator==(const Node &rhs) const {
    return this->bytes_ == rhs.bytes_;
  }

  bool operator!=(const Node &rhs) const {
    return this->bytes_ != rhs.bytes_;
  }

  const uint8_t *ptr() const {
    return bytes_;
  }

protected:
  const uint8_t *bytes_;
};

class Bson final : public Node {
public:
  using Node::Node;

  Bson(const Node &node)
      : Node{node.ptr()} {
  }

  /**\param size if set, then check serialized size with input size. If 0 then
   * not check. By default = 0
   */
  bool valid(size_t size = 0) const {
    if (!bytes_ || (size && size != this->size()) ||
        (bytes_[this->size() - 1] == 0)) {
      return false;
    }

    return true;
  }

  class ConstIterator final {
    friend Bson;

  public:
    ConstIterator &operator++() {
      node_ = Node{node_.ptr() + node_.size()};
      return *this;
    }
    ConstIterator &operator++(int) {
      node_ = Node{node_.ptr() + node_.size()};
      return *this;
    }

    const Node &operator*() const {
      return node_;
    }
    const Node *operator->() const {
      return &node_;
    }

    bool operator==(const ConstIterator &rhs) const {
      return this->node_ == rhs.node_;
    }

    bool operator!=(const ConstIterator &rhs) const {
      return this->node_ != rhs.node_;
    }

  private:
    ConstIterator(const uint8_t *data);

  private:
    Node node_;
  };

  ConstIterator begin() const {
    return ConstIterator(bytes_);
  }

  ConstIterator end() const {
    return ConstIterator(bytes_ + this->size());
  }

  bool contains(std::string_view key) const {
    auto found =
        std::find_if(this->begin(), this->end(), [key](const Node &node) {
          if (node.key() == key) {
            return true;
          }
          return false;
        });

    return found != this->end();
  }

  bool contains(std::string_view key, NodeType type) const {
    auto found =
        std::find_if(this->begin(), this->end(), [key, type](const Node &node) {
          if (node.key() == key && node.type() == type) {
            return true;
          }
          return false;
        });

    return found != this->end();
  }

  /**\return empty node if not founded in the document
   */
  Node operator[](std::string_view key) const {
    auto found =
        std::find_if(this->begin(), this->end(), [key](const Node &node) {
          if (node.key() == key) {
            return true;
          }
          return false;
        });

    return found != this->end() ? *found : Node{nullptr};
  }
};
} // namespace microbson

namespace std {
template <>
struct iterator_traits<microbson::Bson::ConstIterator> {
  using iterator_category = std::forward_iterator_tag;
  using value_type        = microbson::Node;
  using pointer           = microbson::Node *;
  using reference         = microbson::Node &;
  using difference_type   = std::ptrdiff_t;
};
} // namespace std
