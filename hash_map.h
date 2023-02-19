#pragma once

#include <iostream>
#include <vector>
#include <list>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {       // Алгоритм Робин Гуда
    using MapValue = std::pair<const KeyType, ValueType>;
public:
    class Node {
    public:
        Node() : it(nullptr), dist(-1) {
        }

        MapValue &operator*() {
            return &(*it);
        }

        typename std::list<MapValue>::iterator it; // указатель на list_
        int dist;  // расстояние до нужной ячейки, если равна -1, то ячейка пуста
    };

    class iterator {
        typename std::list<MapValue>::iterator iterator_;
    public:

        iterator() : iterator_(nullptr) {
        }

        explicit iterator(typename std::list<MapValue>::iterator it) : iterator_(it) {
        }

        iterator &operator=(typename std::list<MapValue>::iterator it) {
            iterator_ = it;
            return *this;
        }

        typename std::list<MapValue>::iterator operator->() {
            return iterator_;
        }

        MapValue &operator*() {
            return *iterator_;
        }

        bool operator==(iterator other) const {
            return iterator_ == other.iterator_;
        }

        bool operator!=(iterator other) const {
            return iterator_ != other.iterator_;
        }

        iterator &operator++() {
            ++iterator_;
            return *this;
        }

        iterator operator++(int) {
            iterator result = *this;
            ++iterator_;
            return result;
        }
    };

    class const_iterator {
        typename std::list<MapValue>::const_iterator iterator_;
    public:

        const_iterator() : iterator_(nullptr) {
        }

        explicit const_iterator(typename std::list<MapValue>::const_iterator it) : iterator_(it) {
        }

        const_iterator &operator=(typename std::list<MapValue>::const_iterator it) {
            iterator_ = it;
            return *this;
        }

        typename std::list<MapValue>::const_iterator operator->() const {
            return iterator_;
        }

        const MapValue &operator*() const {
            return *iterator_;
        }

        bool operator==(const_iterator other) const {
            return iterator_ == other.iterator_;
        }

        bool operator!=(const_iterator other) const {
            return iterator_ != other.iterator_;
        }

        const_iterator &operator++() {
            ++iterator_;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator result = *this;
            ++iterator_;
            return result;
        }
    };

    iterator begin() {
        return iterator(list_.begin());
    }

    iterator end() {
        return iterator(list_.end());
    }

    const_iterator begin() const {
        return const_iterator(list_.begin());
    }

    const_iterator end() const {
        return const_iterator(list_.end());
    }


    explicit HashMap(const Hash hasher = Hash()) : size_(0), h(hasher) {
        capacity_ = entry_capacity;
        map_.resize(capacity_);
    };

    HashMap(iterator begin, iterator end, const Hash hasher = Hash()) : HashMap(hasher) {
        while (begin != end) {
            (*this).insert(*begin);
            ++begin;
        }
    }

    HashMap(const std::initializer_list<MapValue> list_in, const Hash hasher = Hash())
            : HashMap(hasher) {
        for (const MapValue &value: list_in) {
            (*this).insert(value);
        }
    }

    HashMap &operator=(HashMap other) {
        capacity_ = other.capacity_;
        size_ = other.size_;
        map_.clear();
        map_.resize(capacity_);
        list_.clear();
        for (MapValue c: other.list_) {
            insert(c);
        }
        return *this;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    Hash hash_function() const {
        return h;
    }

    void rebuild() {
        std::list<MapValue> old_list = list_;
        capacity_ *= 10;
        size_ = 0;
        map_.clear();
        list_.clear();

        map_.resize(capacity_);

        for (typename std::list<MapValue>::iterator it = old_list.begin(); it != old_list.end(); ++it) {
            insert(*it);
        }
    }

    void insert(MapValue value) {
        if (find(value.first) != end()) {
            return;
        }

        size_++;
        int global_dist = 0;
        list_.push_back(value);
        typename std::list<MapValue>::iterator it = (--list_.end());

        for (size_t i = h(value.first) % capacity_;; i = (i + 1) % capacity_) {
            if (map_[i].dist < global_dist) {
                std::swap(map_[i].dist, global_dist);
                if (global_dist != -1) {
                    std::swap(map_[i].it, it);
                } else {
                    map_[i].it = it;
                }
            }
            if (global_dist == -1) {
                break;
            }
            ++global_dist;
        }
        if (3 * size_ >= 2 * capacity_) {
            rebuild();
        }
    }


    void erase(const KeyType key) {
        if (find(key) == end()) {
            return;
        }

        for (size_t i = h(key) % capacity_;; i = (i + 1) % capacity_) {

            if (map_[i].it->first == key) {
                list_.erase(map_[i].it);
                map_[i].dist = -1;
                --size_;
                for (size_t j = (i + 1) % capacity_; map_[j].dist > 0; j = (j + 1) % capacity_) {
                    size_t j_past = j;
                    if (j == 0) {
                        j_past = capacity_ - 1;
                    } else {
                        j_past--;
                    }
                    map_[j].dist--;
                    std::swap(map_[j].dist, map_[j_past].dist);
                    std::swap(map_[j_past].it, map_[j].it);
                }
                break;
            }
        }
    }

    iterator find(const KeyType key) {
        for (size_t i = h(key) % capacity_;; i = (i + 1) % capacity_) {
            if (map_[i].dist == -1) {
                break;
            }
            if (map_[i].it->first == key) {
                return iterator(map_[i].it);
            }
        }
        return iterator(list_.end());
    }

    const_iterator find(const KeyType key) const {
        for (size_t i = h(key) % capacity_;; i = (i != capacity_ - 1 ? i + 1 : 0)) {
            if (map_[i].dist == -1) {
                break;
            }
            if (map_[i].it->first == key) {
                return const_iterator(map_[i].it);
            }
        }
        return const_iterator(list_.end());
    }

    ValueType &operator[](const KeyType key) {
        iterator it = find(key);
        if (it == end()) {
            MapValue push = std::make_pair(key, ValueType());
            insert(push);
            it = find(key);
        }
        return (it->second);
    }

    const ValueType &at(const KeyType key) const {
        const_iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("out_of_range");
        }
        return (it->second);
    }


    void clear() {
        capacity_ = entry_capacity;
        size_ = 0;
        map_.clear();
        map_.resize(capacity_);
        list_.clear();
    }

private:
    size_t entry_capacity = 1;  // изначальный размер map_
    size_t capacity_; // рфзмер map_
    size_t size_; // количество элементов в HashMap
    std::vector<Node> map_; // ячейки с сылками в список со всеми элементами
    std::list<MapValue> list_; // список всех элементов
    Hash h; // хэш функция
};