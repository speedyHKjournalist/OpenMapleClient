# NoLifeNx

A C++17 library that can read NX files.

Originally authored by retep998 and licensed (along with other NoLifeStory
software) under the GNU AGPL v3+.

## Dependencies

None.

## Using NoLifeNx

In order to use NoLifeNx you need to compile NoLifeNx, and link the resulting
library into your program. You also need to `#include` the relevant \*.hpp
files. Here follows a brief overview of the various classes:

### nl::file

* Does not allow copying or moving, so the recommended way of creating an
  `nl::file` is either as a static/global variable or by allocating it on the
  heap.
* Comes with only one constructor: `nl::file::file(char const *)`. Just pass
  this constructor a C-style string containing the name of an \*.nx file with
  the ".nx" extension included.
* Feel free to open multiple `nl::file`s to the same \*.nx file. The
  `nl::node`s will still be compatible. Just keep in mind OS handle limits.
* When an `nl::file` is destroyed, all `nl::node`s originating from that
  `nl::file` are now invalid and it is UB to use them.
* `nl::file::base()` returns the root node of an `nl::file`.
* The four count functions are there just in case you want to know how big the
  \*.nx file is.

### nl::node

* `nl::node` is nothing more than a fancy pointer wrapper, so feel free to copy
  and move them around. Don't waste your time allocating them on the heap.
* To get a child node, use `operator[]`. It can take C-style strings:
  `nl::node node = other_node["Hello"];`. It can take C++-style strings:
  `std::string str = "Hello"; nl::node node = other_node[str];`. It can even
  take other nodes, in which case it uses the string value of the node, not the
  name: `nl::node node = other_node[third_node];`.
* To get the name of the node as a C++ string, use `nl::node::name()`.
* To get the number of children that a node has, use `nl::node::size()`.
* `nl::node` implements `begin()` and `end()` along with several other
  operators in order to make it a proper iterator. This allows you to use
  `nl::node` in various C++ algorithms that take iterators. This also lets you
  conveniently recurse through the children of an `nl::node` using
  `for (nl::node child : parent) { /* ... */ }`.
* To get the type of an `nl::node`, use `nl::node::data_type()`, which returns
  a value of the enum `nl::node::type`.
* To get the value of an `nl::node`, you can either use the operators which
  allow easy implicit or explicit casting, or you can use the manual `get_*`
  functions.
* Some operator examples:
    * `void foo(int n); foo(some_node);`
    * `std::string s = some_node;`
    * `double d = (double)some_node;`
* Some `get_*` examples:
    * `std::int64_t n = some_node.get_integer();`
    * `nl::vector v = some_node.get_vector();`
* The `operator bool` does **not** return the value as a `bool`. Instead, it
  returns whether or not the `nl::node` exists. If you want to get the `bool`
  value, you'll need to use `nl::node::get_bool()` instead, or use
  `nl::node::get_bool(bool def)`, which takes an optional `bool` parameter to
  specify the default value.
* You can get the individual values of a vector-type `nl::node` using
  `nl::node::x()` and `nl::node::y()`.
* All methods do null checks, have default values, and do not return anything.
  This ensures that your program will not crash, but beware of unexpected
  consequences of "soft failure".

### nl::audio

* One of the different value types that `nl::node` can have:
  `nl::audio a = some_node;`.
* `nl::audio` is nothing more than a fancy pointer wrapper, so feel free to
  copy and move them around.
* To get the length of the data, use `nl::audio::length()`.
* To get a pointer to the data itself, use `nl::audio::data()`. Do **not**
  attempt to free or modify this data.
* Feel free to cache the pointer, because the data at that pointer will never
  go away or change.

### nl::bitmap

* One of the different value types that `nl::node` can have:
  `nl::bitmap b = some_node;`.
* `nl::bitmap` is nothing more than a fancy pointer wrapper, so feel free to
  copy and move them around.
* To get the width and height of the image use `nl::bitmap::width()` and
  `nl::bitmap::height()`.
* To get the data itself, use `nl::bitmap::data()`. Note that this function
  decompresses the data entirely each time that you call it, so try not to call
  it more than once when you don't have to. Also, the data at that pointer is
  **volatile** and will change with your next call to `nl::bitmap::data()`, or
  the pointer may even become invalid, so just copy the data to whatever
  internal texture you need and use it that way. The returned data is standard
  raw 32-bit BGRA pixel data.
* `nl::bitmap::length()` provides the length of the uncompressed pixel data in
  case you're too lazy to calculate it yourself from the width and height.
* `nl::bitmap::id()` returns a unique ID for that bitmap, useful as the index
  in a cache of textures.
