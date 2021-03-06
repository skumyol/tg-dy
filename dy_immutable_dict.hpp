//
// Created by YAN Yuchen on 6/29/2018.
//

#ifndef DYNET_WRAPPER_DY_IMMUTABLE_DICT_HPP
#define DYNET_WRAPPER_DY_IMMUTABLE_DICT_HPP

#include <dynet/dict.h>
#include <dynet/expr.h>
#include "dy_common.hpp"
#include <memory>

namespace tg {
  namespace dy {
    class immutable_dict {
    public:
      immutable_dict() = default;

      immutable_dict(const immutable_dict &) = default;

      immutable_dict(immutable_dict &&) = default;

      immutable_dict &operator=(const immutable_dict &) = default;

      immutable_dict &operator=(immutable_dict &&) = default;

      explicit immutable_dict(const std::unordered_set<std::string> &entries) :
        dict(std::make_shared<dynet::Dict>()) {
        for (const auto &entry:entries) {
          dict->convert(entry);
        }
        dict->freeze();
      }

      explicit immutable_dict(const std::unordered_set<std::string> &entries, const std::unordered_set<std::string> &more_entries) :
        dict(std::make_shared<dynet::Dict>()) {
        for (const auto &entry:entries) {
          dict->convert(entry);
        }
        for (const auto &entry:more_entries) {
          dict->convert(entry);
        }
        dict->freeze();
      }

      /**
      * get the size of label dictionary
      * this might be different from the size of labels it constructs with
      * because the labels it constructs might can have duplicates
      * \return the size of label dictionary
      */
      unsigned size() const {
        return dict->size();
      }

      unsigned convert(const std::string &label) const {
        return (unsigned) dict->convert(label);
      }

      std::string convert(unsigned id) const {
        return dict->convert(id);
      }

      /**
       * get the one-hot representation of the label, according to the internal label ID
       * \param entry the label to represent
       * \return a dim(#-of-labels) tensor, all values are 0 except 1 at the position of internal label ID
       */
      dy::Expression one_hot(const std::string &entry) const {
        std::vector<float> ret(dict->size());
        fill(ret.begin(), ret.end(), 0);
        ret[dict->convert(entry)] = 1;
        return dynet::input(dy::cg(), {dict->size()}, ret);
      }

      const std::vector<std::string> &list_entries() const {
        return dict->get_words();
      }

      template<class Archive>
      void save(Archive &a) const {
        a(list_entries());
      }

      template<class Archive>
      void load(Archive &a) {
        std::vector<std::string> entries;
        a(entries);
        dict = std::make_shared<dynet::Dict>();
        for (const auto &entry:entries) {
          dict->convert(entry);
        }
        dict->freeze();
      }

    private:
      std::shared_ptr<dynet::Dict> dict;
    };
  }
}

#endif //DYNET_WRAPPER_DY_IMMUTABLE_DICT_HPP
