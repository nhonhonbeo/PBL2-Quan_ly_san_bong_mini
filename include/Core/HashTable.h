#ifndef HASHTABLE_H
#define HASHTABLE_H

// Bang bam tu cai dat: dung de tra cuu khach hang theo SDT,
// tra cuu tinh trang dat san theo (maSan + ngay) voi do phuc tap O(1)
// TODO: em cai dat mang bucket + xu ly dung do (chaining) bang LinkedList

template <typename K, typename V>
class HashTable {
public:
    HashTable(int soBucket = 101);
    ~HashTable();

    void chenHoacCapNhat(const K& key, const V& value);
    bool tim(const K& key, V& out) const;
    bool xoa(const K& key);

private:
    int hamBam(const K& key) const;
    // TODO: mang cac bucket (moi bucket la 1 danh sach lien ket cap key-value)
    int soBucket;
};

#endif
