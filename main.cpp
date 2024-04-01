#include <map>
#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string text;
    getline(cin, text);
    return text;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;

    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double tf = 1./words.size();

        for (const string& word : words) { 
            word_to_documents_freqs_[word][document_id] += tf;
        }

        ++document_count_;

    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

private:
    struct Query {
        set<string> minus_words;
        set<string> plus_words;
    };

    struct QueryWord {
        string query_word;
        bool is_minus;
        bool is_stop;
    };

    map<string, map<int, double>> word_to_documents_freqs_;
    set<string> stop_words_;
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        
        return words;
    }

    QueryWord ParseQueryWord(string word) const {
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        return {word, is_minus, IsStopWord(word)};
    }

    Query ParseQuery(const string& text) const {
        Query query_words;

        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query = ParseQueryWord(word);
            if (!query.is_stop) {
                if (query.is_minus) {
                    query_words.minus_words.insert(query.query_word);
                } else {
                    query_words.plus_words.insert(query.query_word);
                }
            }
        }

        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        vector<Document> matched_documents;
        map<int, double> document_to_relevance; 

        for (const auto& query_plus : query_words.plus_words) { 
            if (word_to_documents_freqs_.count(query_plus)) {
                double idf = log(static_cast<double>(document_count_)
                /word_to_documents_freqs_.at(query_plus).size());
                for (const auto& [id, tf] : word_to_documents_freqs_.at(query_plus)) {
                    document_to_relevance[id] += tf * idf;
                }
            }
        }
        for (const auto& query_minus : query_words.minus_words) {
            if (word_to_documents_freqs_.count(query_minus)) {
                for (const auto& [id, tf] : word_to_documents_freqs_.at(query_minus)) {
                    document_to_relevance.erase(id);
                }
            }
        }

        for (const auto& document : document_to_relevance) {
                matched_documents.push_back({document.first, document.second});
        }

        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());
    const int document_count = ReadLineWithNumber();

    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();
    const string raw_query = ReadLine();

    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(raw_query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
    
}


