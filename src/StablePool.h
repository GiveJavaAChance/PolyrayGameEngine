#ifndef STABLEPOOL_H_INCLUDED
#define STABLEPOOL_H_INCLUDED

template<typename T>
struct StablePool {
private:
    constexpr static uint32_t pageSize = 4096u / sizeof(T);
    constexpr static uint32_t wordCount = (pageSize + 63u) / 64u;
    struct Page {
        T* ptr;
        uint64_t words[wordCount];
    };

    std::vector<Page> pages;
    std::vector<uint32_t> counts;

    Page& allocatePage() {
        pages.emplace_back(reinterpret_cast<T*>(_mm_malloc(4096u, 4096u)));
        Page& page = pages[pages.size() - 1u];
        std::memset(page.words, 0, wordCount * sizeof(uint64_t));
        counts.push_back(0u);
        return page;
    }

public:
    template<typename... Args>
    T* allocate(Args... args) {
        uint32_t idx = 0u;
        while(idx < pages.size() && counts[idx] == pageSize) idx++;
        Page& page;
        if(idx == pages.size()) {
            page = allocatePage();
        } else {
            page = pages[idx];
        }
        uint64_t word = ~page.words[0u];
        uint32_t u = 0u;
        uint32_t index = 0u;
        while (true) {
            if(word != 0ull) {
                uint32_t off = __builtin_ctzll(word);
                page.words[u] |= 1ull << off;
                index = (u << 6u) + off;
                break;
            }
            u++;
            word = ~page.words[u];
        }
        counts[idx]++;
        T* e = page.ptr + index;
        new (e) T(std::forward<Args>(args)...);
        return e;
    }

    void free(const T* e) {
        for(uint32_t i = 0u; i < pages.size(); i++) {
            if(pages[i].ptr > e || pages[i].ptr + pageSize <= e) {
                continue;
            }
            uint32_t idx = (e - pages[i].ptr) / sizeof(T);
            pages[i].words[idx >> 6u] &= ~(1ull << (idx & 63u));
            e->~T();
            counts[i]--;
            return;
        }
    }
};

#endif
