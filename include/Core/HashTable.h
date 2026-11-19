#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "LinkedList.h"
#include <cmath>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <utility>

// LÝ DO CHỌN: Bảng băm tra cứu theo Separate Chaining (danh sách liên kết giải quyết xung đột),
// Rule of Five quản lý mảng bucket động, tự động rehash khi load factor vượt 0.75.
// ponytail: mảng bucket nhân đôi kích thước khi resize; nâng cấp sang danh sách kích thước số nguyên tố (prime numbers) nếu cần phân bố đồng đều hơn khi hàm băm kém ngẫu nhiên.

// THUẬT TOÁN BẢNG BĂM (HASHTABLE):
// 1. Công thức băm: hashValue = Hash{}(key) % capacity. Với string: phân tán đa thức; với int/con trỏ: xáo trộn bit.
// 2. Xử lý va chạm: Separate Chaining - các cặp key-value va chạm cùng bucket được lưu vào LinkedList tương ứng.
// 3. Điều kiện resize: loadFactor = count / capacity vượt maxLoadFactor (0.75). Tăng gấp đôi capacity và rehash lại toàn bộ.
// 4. Độ phức tạp: Trung bình O(1) cho insert, search, remove; Trường hợp xấu nhất O(N) khi toàn bộ key va chạm vào 1 bucket duy nhất (không O(1) tuyệt đối).

template <typename K, typename V>
struct Entry {
    K key;
    V value;

    Entry(const K& k, const V& v) : key(k), value(v) {}
    Entry(K&& k, V&& v) : key(std::move(k)), value(std::move(v)) {}
    Entry(const K& k, V&& v) : key(k), value(std::move(v)) {}

    bool operator==(const Entry<K, V>& other) const {
        return key == other.key;
    }
};

template <typename K, typename V, typename Hash = std::hash<K>>
class HashTable {
public:
    static constexpr std::size_t DEFAULT_CAPACITY = 16;
    static constexpr float DEFAULT_MAX_LOAD_FACTOR = 0.75f;

private:
    std::size_t capacity_;
    std::size_t count_;
    float maxLoadFactor_;
    Hash hasher_;
    LinkedList<Entry<K, V>>* buckets;

    std::size_t getBucketIndex(const K& key, std::size_t cap) const {
        if (cap == 0) return 0;
        return hasher_(key) % cap;
    }

    static float validateMaxLoadFactor(float value) {
        if (!std::isfinite(value) || value <= 0.0f) {
            throw std::invalid_argument("HashTable maxLoadFactor must be finite and positive");
        }
        return value;
    }

public:
    explicit HashTable(std::size_t initialCapacity = DEFAULT_CAPACITY,
                       float maxLoadFactor = DEFAULT_MAX_LOAD_FACTOR)
        : capacity_(initialCapacity < 4 ? 4 : initialCapacity),
          count_(0),
          maxLoadFactor_(validateMaxLoadFactor(maxLoadFactor)),
          hasher_(Hash()),
          buckets(new LinkedList<Entry<K, V>>[capacity_]) {}

    // Rule of Five: 1. Destructor
    ~HashTable() noexcept {
        delete[] buckets;
    }

    // Rule of Five: 2. Copy Constructor (Deep copy, Exception-safe)
    // LÝ DO CHỌN: Cấp phát sâu mảng bucket và sao chép từng phần tử, giải phóng sạch nếu gặp ngoại lệ.
    HashTable(const HashTable& other)
        : capacity_(other.capacity_),
          count_(0),
          maxLoadFactor_(other.maxLoadFactor_),
          hasher_(other.hasher_),
          buckets(other.capacity_ > 0 ? new LinkedList<Entry<K, V>>[other.capacity_] : nullptr) {
        try {
            if (other.buckets != nullptr) {
                for (std::size_t i = 0; i < other.capacity_; ++i) {
                    for (auto* cur = other.buckets[i].get_head(); cur != nullptr; cur = cur->next) {
                        buckets[i].push_back(cur->data);
                        count_++;
                    }
                }
            }
        } catch (...) {
            delete[] buckets;
            buckets = nullptr;
            capacity_ = 0;
            count_ = 0;
            throw;
        }
    }

    // Rule of Five: 3. Copy Assignment Operator (Strong Exception Guarantee & Self-assignment safe)
    // LÝ DO CHỌN: Copy-and-swap idiom bảo đảm an toàn tự gán và giữ nguyên dữ liệu cũ nếu cấp phát thất bại.
    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            HashTable temp(other);
            swap(temp);
        }
        return *this;
    }

    // Rule of Five: 4. Move Constructor
    // LÝ DO CHỌN: Tiếp quản trực tiếp con trỏ mảng bucket O(1), đưa đối tượng nguồn về rỗng an toàn, noexcept.
    HashTable(HashTable&& other) noexcept(std::is_nothrow_move_constructible<Hash>::value)
        : capacity_(other.capacity_),
          count_(other.count_),
          maxLoadFactor_(other.maxLoadFactor_),
          hasher_(std::move(other.hasher_)),
          buckets(other.buckets) {
        other.buckets = nullptr;
        other.capacity_ = 0;
        other.count_ = 0;
    }

    // Rule of Five: 5. Move Assignment Operator
    // LÝ DO CHỌN: Dọn sạch tài nguyên hiện tại và tiếp quản con trỏ đối tượng nguồn, an toàn khi tự move.
    HashTable& operator=(HashTable&& other) noexcept(std::is_nothrow_move_assignable<Hash>::value) {
        if (this != &other) {
            hasher_ = std::move(other.hasher_);
            delete[] buckets;
            buckets = other.buckets;
            capacity_ = other.capacity_;
            count_ = other.count_;
            maxLoadFactor_ = other.maxLoadFactor_;
            other.buckets = nullptr;
            other.capacity_ = 0;
            other.count_ = 0;
        }
        return *this;
    }

    void swap(HashTable& other) noexcept(std::is_nothrow_swappable<Hash>::value) {
        std::swap(buckets, other.buckets);
        std::swap(capacity_, other.capacity_);
        std::swap(count_, other.count_);
        std::swap(maxLoadFactor_, other.maxLoadFactor_);
        std::swap(hasher_, other.hasher_);
    }

    friend void swap(HashTable& a, HashTable& b) noexcept(std::is_nothrow_swappable<Hash>::value) {
        a.swap(b);
    }

    // Chèn hoặc cập nhật giá trị theo key.
    // LÝ DO CHỌN: Duyệt danh sách liên kết tại bucket; nếu trùng key thì update in-place, nếu chưa có thì chèn mới.
    bool insert(const K& key, const V& value) {
        if (capacity_ == 0) {
            rehash(DEFAULT_CAPACITY);
        }
        std::size_t index = getBucketIndex(key, capacity_);
        Entry<K, V>* existing = buckets[index].findIf(
            [&key](const Entry<K, V>& entry) { return entry.key == key; });
        if (existing != nullptr) {
            existing->value = value;
            return false;
        }
        buckets[index].push_back(Entry<K, V>(key, value));
        count_++;

        if (loadFactor() > maxLoadFactor_) {
            if (capacity_ > static_cast<std::size_t>(-1) / 2) {
                throw std::length_error("HashTable capacity overflow during insertion");
            }
            rehash(capacity_ * 2);
        }
        return true; // Chèn mới thành công
    }

    bool insert(const K& key, V&& value) {
        if (capacity_ == 0) {
            rehash(DEFAULT_CAPACITY);
        }
        std::size_t index = getBucketIndex(key, capacity_);
        Entry<K, V>* existing = buckets[index].findIf(
            [&key](const Entry<K, V>& entry) { return entry.key == key; });
        if (existing != nullptr) {
            existing->value = std::move(value);
            return false;
        }
        buckets[index].push_back(Entry<K, V>(key, std::move(value)));
        count_++;

        if (loadFactor() > maxLoadFactor_) {
            if (capacity_ > static_cast<std::size_t>(-1) / 2) {
                throw std::length_error("HashTable capacity overflow during insertion");
            }
            rehash(capacity_ * 2);
        }
        return true;
    }

    // Cập nhật giá trị cho key đã tồn tại.
    bool update(const K& key, const V& value) {
        if (capacity_ == 0 || count_ == 0 || buckets == nullptr) return false;
        std::size_t index = getBucketIndex(key, capacity_);
        Entry<K, V>* existing = buckets[index].findIf(
            [&key](const Entry<K, V>& entry) { return entry.key == key; });
        if (existing == nullptr) return false;
        existing->value = value;
        return true;
    }

    // Tìm kiếm giá trị theo key, trả về qua tham chiếu out.
    bool search(const K& key, V& out) const {
        if (capacity_ == 0 || count_ == 0 || buckets == nullptr) return false;
        std::size_t index = getBucketIndex(key, capacity_);
        for (auto* cur = buckets[index].get_head(); cur != nullptr; cur = cur->next) {
            if (cur->data.key == key) {
                out = cur->data.value;
                return true;
            }
        }
        return false;
    }

    // Tìm kiếm con trỏ tới giá trị (cho phép đọc/ghi trực tiếp).
    V* find(const K& key) {
        if (capacity_ == 0 || count_ == 0 || buckets == nullptr) return nullptr;
        std::size_t index = getBucketIndex(key, capacity_);
        Entry<K, V>* found = buckets[index].findIf(
            [&key](const Entry<K, V>& entry) { return entry.key == key; });
        return found != nullptr ? &found->value : nullptr;
    }

    // LÝ DO CHỌN: Con trỏ find() chỉ hợp lệ đến khi phần tử bị xóa, bảng bị sửa đổi làm
    // rehash, bị xóa toàn bộ, gán hoặc hủy; không giữ con trỏ qua các thao tác này.
    const V* find(const K& key) const {
        if (capacity_ == 0 || count_ == 0 || buckets == nullptr) return nullptr;
        std::size_t index = getBucketIndex(key, capacity_);
        for (auto* cur = buckets[index].get_head(); cur != nullptr; cur = cur->next) {
            if (cur->data.key == key) {
                return &(cur->data.value);
            }
        }
        return nullptr;
    }

    bool contains(const K& key) const {
        if (capacity_ == 0 || count_ == 0 || buckets == nullptr) return false;
        std::size_t index = getBucketIndex(key, capacity_);
        for (const auto* cur = buckets[index].get_head();
             cur != nullptr; cur = cur->next) {
            if (cur->data.key == key) return true;
        }
        return false;
    }

    // Xóa key khỏi bảng băm.
    // LÝ DO CHỌN: Sử dụng removeIf của LinkedList để xóa đúng Entry có key khớp, cập nhật count_ an toàn.
    bool remove(const K& key) {
        if (capacity_ == 0 || count_ == 0 || buckets == nullptr) return false;
        std::size_t index = getBucketIndex(key, capacity_);
        bool removed = buckets[index].removeIf([&key](const Entry<K, V>& entry) {
            return entry.key == key;
        });
        if (removed) {
            count_--;
        }
        return removed;
    }

    template <typename Predicate>
    std::size_t removeIf(Predicate pred) {
        // LÝ DO CHỌN: Quét từng bucket để xóa các bí danh theo giá trị, chi phí O(B + N).
        std::size_t removedCount = 0;
        if (buckets == nullptr) return removedCount;
        for (std::size_t i = 0; i < capacity_; ++i) {
            const int previousSize = buckets[i].size();
            try {
                buckets[i].removeIf(pred);
            } catch (...) {
                const int removedFromBucket = previousSize - buckets[i].size();
                count_ -= static_cast<std::size_t>(removedFromBucket);
                throw;
            }
            removedCount += static_cast<std::size_t>(
                previousSize - buckets[i].size());
        }
        count_ -= removedCount;
        return removedCount;
    }

    // LÝ DO CHỌN: Sao chép N phần tử, đồng thời quét B bucket: O(B + N) thời gian, O(N) bộ nhớ.
    LinkedList<Entry<K, V>> get_all() const {
        LinkedList<Entry<K, V>> res;
        if (buckets != nullptr) {
            for (std::size_t i = 0; i < capacity_; ++i) {
                for (auto* cur = buckets[i].get_head(); cur != nullptr; cur = cur->next) {
                    res.push_back(cur->data);
                }
            }
        }
        return res;
    }

    void clear() noexcept {
        if (buckets != nullptr) {
            for (std::size_t i = 0; i < capacity_; ++i) {
                buckets[i].clear();
            }
        }
        count_ = 0;
    }

    // Tái phân băm và mở rộng dung lượng bảng khi load factor vượt ngưỡng.
    // LÝ DO CHỌN: Cấp phát mảng bucket mới, chuyển toàn bộ phần tử và chỉ tráo đổi con trỏ khi hoàn tất, đảm bảo Strong Exception Guarantee.
    void rehash(std::size_t newCapacity) {
        if (newCapacity <= count_) {
            newCapacity = count_ + 1;
        }
        if (newCapacity < 4) {
            newCapacity = 4;
        }
        while (static_cast<float>(count_) / static_cast<float>(newCapacity) >
               maxLoadFactor_) {
            if (newCapacity > static_cast<std::size_t>(-1) / 2) {
                throw std::length_error("HashTable capacity overflow during rehash");
            }
            newCapacity *= 2;
        }

        LinkedList<Entry<K, V>>* newBuckets = new LinkedList<Entry<K, V>>[newCapacity];
        try {
            if (buckets != nullptr) {
                for (std::size_t i = 0; i < capacity_; ++i) {
                    for (auto* cur = buckets[i].get_head(); cur != nullptr; cur = cur->next) {
                        std::size_t newIdx = getBucketIndex(cur->data.key, newCapacity);
                        newBuckets[newIdx].push_back(cur->data);
                    }
                }
            }
        } catch (...) {
            delete[] newBuckets;
            throw;
        }

        delete[] buckets;
        buckets = newBuckets;
        capacity_ = newCapacity;
    }

    float loadFactor() const noexcept {
        return capacity_ > 0 ? static_cast<float>(count_) / static_cast<float>(capacity_) : 0.0f;
    }

    std::size_t capacity() const noexcept {
        return capacity_;
    }

    std::size_t size() const noexcept {
        return count_;
    }

    bool isEmpty() const noexcept {
        return count_ == 0;
    }
};

#endif