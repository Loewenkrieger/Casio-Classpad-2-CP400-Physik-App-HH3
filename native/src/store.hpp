// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace physik {
struct Value {
  enum Type { Integer, Text, Tuple } type = Integer;
  int64_t number = 0;
  std::string text;
  std::vector<Value> items;
};
bool decodeRecord(const std::string& text, Value& result, std::string& error);
std::string encodeRecord(const Value& value);

struct Table { int file; uint32_t count, start, width; };
inline constexpr Table modes{0,3,33,40}, calcCategories{0,17,153,48},
  chapters{0,514,969,48}, sections{0,171,25641,48}, calculations{0,167,33849,152},
  calcNotes{0,81,59233,208}, articleMedia{0,32,76081,48}, entries{0,840,77617,72},
  mediaIndex{1,39,34,224};
inline constexpr Table tables[]{modes,calcCategories,chapters,sections,calculations,
                              calcNotes,articleMedia,entries,mediaIndex};

struct Media {
  std::string id, title, kind, caption;
  uint32_t start=0, length=0, width=0, height=0, checksum=0;
};

class Store {
public:
  explicit Store(std::string directory):directory_(std::move(directory)){}
  bool row(Table table, uint32_t index, Value& result);
  bool chapter(uint32_t index, std::string& result);
  bool media(const std::string& id, Media& result);
  bool images(uint32_t chapter, std::vector<std::string>& result);
  bool imageData(const Media& media, std::vector<uint8_t>& result);
  const std::string& error() const { return error_; }
  bool read(int kind, uint32_t offset, uint32_t length, std::vector<uint8_t>& result);
private:
  std::string directory_,error_;
};
}
