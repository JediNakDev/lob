#ifndef LOB_OBJECT_POOL_HPP
#define LOB_OBJECT_POOL_HPP

#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace lob {

template <typename T, std::size_t BlockSize = 4096>
class ObjectPool {
public:
    ObjectPool() = default;
    ~ObjectPool() = default;

    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    ObjectPool(ObjectPool&&) = delete;
    ObjectPool& operator=(ObjectPool&&) = delete;

    void reserve(std::size_t object_count) {
        const std::size_t needed_blocks = (object_count + BlockSize - 1) / BlockSize;
        while (blocks_.size() < needed_blocks) {
            allocate_block();
        }
    }

    void set_allow_growth(bool allow_growth) noexcept {
        allow_growth_ = allow_growth;
    }

    [[nodiscard]] std::size_t growth_failures() const noexcept {
        return growth_failures_;
    }

    [[nodiscard]] std::size_t block_count() const noexcept {
        return blocks_.size();
    }

    template <typename... Args>
    T* create(Args&&... args) {
        if (!free_list_) {
            if (!allow_growth_) {
                ++growth_failures_;
                return nullptr;
            }
            allocate_block();
        }

        Node* node = free_list_;
        free_list_ = free_list_->next;

        T* object = reinterpret_cast<T*>(&node->storage);
        ::new (static_cast<void*>(object)) T(std::forward<Args>(args)...);
        return object;
    }

    void destroy(T* object) noexcept {
        if (!object) {
            return;
        }

        object->~T();
        Node* node = reinterpret_cast<Node*>(object);
        node->next = free_list_;
        free_list_ = node;
    }

private:
    union Node {
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
        Node* next;
    };

    void allocate_block() {
        blocks_.emplace_back(new Node[BlockSize]);
        Node* block = blocks_.back().get();

        for (std::size_t i = 0; i + 1 < BlockSize; ++i) {
            block[i].next = &block[i + 1];
        }
        block[BlockSize - 1].next = free_list_;
        free_list_ = block;
    }

    std::vector<std::unique_ptr<Node[]>> blocks_;
    Node* free_list_ = nullptr;
    bool allow_growth_ = true;
    std::size_t growth_failures_ = 0;
};

}  // namespace lob

#endif
