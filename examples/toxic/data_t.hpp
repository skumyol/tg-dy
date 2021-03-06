//
// Created by YAN Yuchen on 11/12/2018.
//

#ifndef DYNET_WRAPPER_DATA_T_HPP
#define DYNET_WRAPPER_DATA_T_HPP
#include <string>
#include <unordered_set>
#include <vector>
#include <json/json.h>
#include <fstream>

struct datum_t {
  std::vector<std::string> input;
  std::unordered_set<std::string> oracle;
  void parse_json(const Json::Value& json) {
    const auto& _input = json["input"];
    for(unsigned i=0; i<_input.size(); i++) {
      input.push_back(_input[i].asString());
    }
    const auto& _oracle = json["oracle"];
    for(unsigned i=0; i<_oracle.size(); i++) {
      oracle.insert(_oracle[i].asString());
    }
  }
};

struct dataset_t {
  std::unordered_set<std::string> labels;
  std::vector<datum_t> data;
  void parse_json(const Json::Value& json) {
    const auto& _labels = json["labels"];
    for(unsigned i=0; i<_labels.size(); i++) {
      labels.insert(_labels[i].asString());
    }
    const auto& _data = json["data"];
    for(unsigned i=0; i<_data.size(); i++) {
      datum_t datum;
      datum.parse_json(_data[i]);
      data.push_back(datum);
    }
  }
};

dataset_t read_dataset(const std::string& path) {
  using namespace std;
  ifstream ifs(path);
  Json::Value json;
  Json::Reader().parse(ifs, json);
  dataset_t ret;
  ret.parse_json(json);
  return ret;
}

#endif //DYNET_WRAPPER_DATA_T_HPP
