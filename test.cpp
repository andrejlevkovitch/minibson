#include "microbson.hpp"
#include "microbson2.hpp"
#include "minibson.hpp"
#include <cassert>

void test_minibson();
void test_microbson();
void test_microbson2();

int main() {
  test_minibson();
  test_microbson();
  test_microbson2();
  return 0;
}

void test_minibson() {
  using namespace minibson;
  using namespace std;

  document d;

  d.set("int32", 1);
  d.set("int64", 140737488355328LL);
  d.set("float", 30.20);
  d.set("string", "text");
  d.set("binary", binary::buffer(&d, sizeof(d)));
  d.set("boolean", true);
  d.set("document", document().set("a", 3).set("b", 4));
  d.set("null");

  assert(d.contains<int>("int32"));
  assert(d.contains<long long int>("int64"));
  assert(d.contains<double>("float"));
  assert(d.contains<std::string>("string"));
  assert(d.contains<binary::buffer>("binary"));
  assert(d.contains<bool>("boolean"));
  assert(d.contains<document>("document"));
  assert(d.contains("null"));

  assert(d.get("int32", 0) == 1);
  assert(d.get("int64", 0LL) == 140737488355328LL);
  assert(d.get("float", 0.0) == 30.20);
  assert(d.get("string", "") == "text");
  assert(d.get("binary", binary::buffer{}).data.size() == sizeof(d));
  assert(d.get("boolean", false) == true);
  assert(d.get("document", document()).contains("a") &&
         d.get("document", document()).contains("b"));

  size_t size   = d.get_serialized_size();
  char * buffer = new char[size];
  d.serialize(buffer, size);

  document d1(buffer, size);

  delete[] buffer;

  assert(d1.get("int32", 0) == 1);
  assert(d1.get("int64", 0LL) == 140737488355328LL);
  assert(d1.get("float", 0.0) == 30.20);
  assert(d1.get("string", "") == "text");
  assert(d1.get("binary", binary::buffer{}).data.size() == sizeof(d));
  assert(d1.get("boolean", false) == true);
  assert(d1.get("document", document()).contains("a") &&
         d.get("document", document()).contains("b"));
  assert(d1.contains("null"));
}

void test_microbson() {
  using namespace std;

  minibson::document d;

  d.set("int32", 1);
  d.set("int64", 140737488355328LL);
  d.set("float", 30.20);
  d.set("string", "text");
  d.set("binary", minibson::binary::buffer(&d, sizeof(d)));
  d.set("boolean", true);
  d.set("document", minibson::document().set("a", 3).set("b", 4));
  d.set("some_other_string", "some_other_text");
  d.set("null");

  size_t size   = d.get_serialized_size();
  char * buffer = new char[size];
  d.serialize(buffer, size);

  const microbson::document m(buffer, size);

  assert(m.contains<int>("int32"));
  assert(m.contains<long long int>("int64"));
  assert(m.contains<double>("float"));
  assert(m.contains<std::string>("string"));
  assert(m.contains<void *>("binary"));
  assert(m.contains<bool>("boolean"));
  assert(m.contains<microbson::document>("document"));
  assert(m.contains("null"));

  assert(m.get("int32", 0) == 1);
  assert(m.get("int64", 0LL) == 140737488355328LL);
  assert(m.get("float", 0.0) == 30.20);
  assert(m.get("string", string("")) == "text");
  assert(m.get("binary").second == sizeof(d));
  assert(m.get("boolean", false) == true);
  assert(m.get("document", microbson::document()).contains("a") &&
         m.get("document", microbson::document()).contains("b"));

  delete[] buffer;
}

void test_microbson2() {
  using namespace std;

  minibson::document d;

  d.set("int32", 1);
  d.set("int64", 140737488355328LL);
  d.set("float", 30.20);
  d.set("string", "text");
  d.set("binary", minibson::binary::buffer(&d, sizeof(d)));
  d.set("boolean", true);
  d.set("document", minibson::document().set("a", 3).set("b", 4));
  d.set("some_other_string", "some_other_text");
  d.set("null");

  size_t size   = d.get_serialized_size();
  char * buffer = new char[size];
  d.serialize(buffer, size);

  using RBson = microbson::Bson;

  const microbson::Bson m(buffer);

  assert(m.contains("int32", RBson::NodeType::Int32));
  assert(m.contains("int64", RBson::NodeType::Int64));
  assert(m.contains("float", RBson::NodeType::Double));
  assert(m.contains("string", RBson::NodeType::String));
  assert(m.contains("binary", RBson::NodeType::Binary));
  assert(m.contains("boolean", RBson::NodeType::Boolean));
  assert(m.contains("document", RBson::NodeType::Document));
  assert(m.contains("null"));

  assert((int32_t)m["int32"] == 1);
  assert((int64_t)m["int64"] == 140737488355328LL);
  assert((double)m["float"] == 30.20);
  assert(m["string"] == "text");
  assert(m["binary"].size() == sizeof(d));
  assert((bool)m["boolean"] == true);
  assert(RBson{m["document"]}.contains("a") &&
         RBson{m["document"]}.contains("b"));

  delete[] buffer;
}
