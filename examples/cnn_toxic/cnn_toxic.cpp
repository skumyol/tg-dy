//
// Created by YAN Yuchen on 11/12/2018.
//

#include "data_t.hpp"
#include "../../dy.hpp"
#include "../../dy_training_framework.hpp"
#include <fstream>

using namespace std;
using namespace tg;
data_t read_dataset(const string& path) {
  ifstream ifs(path);
  Json::Value json;
  Json::Reader().parse(ifs, json);
  data_t ret;
  ret.parse_json(json);
  return ret;
}
vector<string> collect_frequent_tokens(const data_t& data, unsigned top_x=20000) {
  tg::dy::frequent_token_collector collector;
  for(const auto& datum:data.data) {
    for(const auto& token:datum.input) {
      collector.add_occurence(token);
    }
  }
  return collector.list_frequent_tokens(top_x);
}

class my_model {
public:
  my_model(const vector<string>& labels, const vector<string>& vocab, const unordered_map<string, vector<float>>& init_embeddings, unsigned embedding_size)
  :emb(embedding_size, vocab, [&](const string& token){return init_embeddings.at(token);}), conv0(embedding_size,3,1,false,false), conv1(embedding_size,3,1,true,false), conv2(embedding_size,3,1,true,false), fc(128), ro(labels){
  }

  dynet::Expression forward(const vector<string>& sentence) {
    dynet::Expression x;
    x = dynet::concatenate(emb.read_sentence(sentence),1);
    x = dynet::rectify(conv0.forward(x));
    x = dy::maxpooling1d(x, 3, 1, false);
    x = dynet::rectify(conv1.forward(x));
    x = dy::maxpooling1d(x, 3, 1, false);
    x = dynet::rectify(conv2.forward(x));
    x = dynet::max_dim(x, 1);

    x = dynet::tanh(fc.forward(x));
    return x;
  }

  unordered_set<string> predict(const vector<string>& sentence) {
    return ro.readout(forward(sentence));
  }

  dynet::Expression compute_loss(const vector<string>& sentence, const unordered_set<string>& labels) {
    return ro.compute_loss(forward(sentence), labels);
  }

  EASY_SERIALZABLE(emb, conv0, conv1, conv2, fc, ro)
private:
  dy::embedding_lookup emb;
  dy::conv1d_layer conv0;
  dy::conv1d_layer conv1;
  dy::conv1d_layer conv2;
  dy::linear_layer fc;
  dy::multi_readout_layer ro;
};

template<class T>
void print_helper(const T& x) {
  for(const auto& t:x) {
    cout << t << " ";
  }
  cout <<endl;
}


int main() {
  const string TRAINING_DATA_PATH = "/hltc/0/cl/corpora/jigsaw-toxic-comment-classification-challenge/processed/train.json";
  const string TEST_DATA_PATH = "/hltc/0/cl/corpora/jigsaw-toxic-comment-classification-challenge/processed/test.json";
  const string PATH_TO_WORD2VEC_FILE = "/hltc/0/cl/tools/word_embeddings/w2vgw.d300.en.bin";
  cout << "read dataset" <<endl;
  const auto trainint_set = read_dataset(TRAINING_DATA_PATH);
  cout << "collect frequent tokens" <<endl;
  const auto vocab = collect_frequent_tokens(trainint_set);
  cout << "import word2vec" <<endl;
  const auto w2v = dy::import_word2vec(PATH_TO_WORD2VEC_FILE);
  cout << "initialze model" <<endl;
  dy::initialize();
  my_model model(vector<string>(trainint_set.labels.begin(), trainint_set.labels.end()), vocab, w2v, 128);
//  my_model model(vector<string>({"a","b"}), vector<string>({"a","b"}), unordered_map<string, vector<float>>(), 128);
//  model.forward({"a","b","a","b"});
  cout << "training" <<endl;
  for(unsigned epoch = 0; epoch<10; epoch++) {
    cout << "epoch:"<< epoch <<endl;
    dy::mp_train<datum_t>(8, trainint_set.data, [&](const datum_t& datum){
      return model.compute_loss(datum.input, datum.oracle);
    });
    for(const auto& datum:trainint_set.data) {
      dy::renew_cg();
      try {
        dy::train_on_loss(model.compute_loss(datum.input, datum.oracle));
      }
      catch(...) {
        cout << "nan for sentence:";
        print_helper(datum.input);
      }
    }
  }

  cout << "testing" <<endl;
  const auto test_set = read_dataset(TEST_DATA_PATH);
  for(unsigned i=0; i<5; i++) {
    const auto& datum = test_set.data[i];
    cout << "sentence:";
    print_helper(datum.input);
    cout << "oracle:";
    print_helper(datum.oracle);
    const auto result = model.predict(datum.input);
    cout << "predict:";
    print_helper(result);
    cout << endl;
  }

}
