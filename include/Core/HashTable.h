#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "LinkedList.h"
#include <string>
template <typename K, typename V>
struct Entry{
    K key;
    V value;
    Entry(){}
    Entry(const K& k, const V& v) : key(k), value(v) {}
    bool operator == (const Entry<K, V>& other) const{
            return key == other.key;
    }
};
template <typename K, typename V>
class HashTable {
private: 
    static const int bucket = 32;
    LinkedList<Entry<K,V>> buckets[bucket];

    int Hash(const K& key) const{
        unsigned long sum = 0;
        std::string s = to_string(key);
        for(size_t i = 0; i<s.size(); i++){
            sum = sum*31 + (unsigned char)s[i];
        }
        return (int)(sum%bucket);
    }
    std::string to_string(const K& key) const {return key;}

public:
    void insert(const K& key, const V& value){
        int index = Hash(key);
        Entry<K, V> temp(key, value);
        buckets[index].delete_val(temp);
        buckets[index].push_back(temp);
    }

    bool search(const K& key, V& out) const{
        int index = Hash(key);
        typename LinkedList<Entry<K, V>>::Node* cur = buckets[index].get_head();
        while(cur != nullptr){
            if(cur->data.key == key){
                out = cur->data.value;
                return true;
            }
            cur = cur->next;
        }
        return false;
    }
    bool remove(const K& key){
        int index = Hash(key);
        Entry<K, V>temp;
        temp.key = key;
        return buckets[index].delete_val(temp);
    }

    LinkedList<Entry<K, V>> get_all() const{
        LinkedList<Entry<K, V>>res;
        for(int i = 0; i<bucket; i++){
            typename LinkedList<Entry<K, V>>::Node* cur = buckets[i].get_head();
            while(cur != nullptr){
                res.push_back(cur->data);
                cur = cur->next;
            }
        }
        return res;
    }
};
#endif