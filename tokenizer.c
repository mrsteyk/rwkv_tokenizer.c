#include <stdint.h>

typedef uint32_t U32;
typedef uint16_t U16;
typedef int32_t  B32;
typedef uint8_t  U8;

typedef U16 token_t;

// NOTE(mrsteyk): normal people's string type
typedef struct RWKVS8 RWKVS8;
struct RWKVS8 {
    const U8* ptr;
    size_t len;
};

// NOTE(mrsteyk): think of this as some sort of trie hashmap.
typedef struct RWKVTrie RWKVTrie;
struct RWKVTrie {
    U32 id;
    // U8 b; // I decided against this.
    //RWKVS8 s;
    //
    const RWKVTrie* next[256];
};

typedef struct RWKVTokenizer RWKVTokenizer;
struct RWKVTokenizer {
    RWKVS8* idx2token;
    RWKVTrie* root[256];
    U32 num_tokens;
};

static void
rwkv_trie_init(RWKVTrie* trie) {
    memset(trie, 0, sizeof(*trie));
}

// NOTE(mrsteyk): libc dep.
static RWKVTrie*
rwkv_trie_alloc() {
    return (RWKVTrie*)memset(malloc(sizeof(RWKVTrie)), 0, sizeof(RWKVTrie));
}

// INTERNAL FUNCTION
static B32
rwkv_trie_find_longest(const RWKVTrie** root, const U8* ptr, size_t available, token_t* ptoken, size_t* pskip) {
    if (!root || !ptr || !available) return 0;
    
    const RWKVTrie* curr = root[ptr[0]];
    
    if (!curr) {
        return 0;
    }
    
#if 0
    //U32 max_t = curr->id;
    const RWKVTrie* max_n = curr;
    size_t max_s = curr->s.len;
    
    for (size_t i = 1; i < available; i++) {
        if (!curr->next[ptr[i]]) {
            break;
        }
        
        curr = curr->next[ptr[i]];
        
        if (curr->s.len) {
            max_n = curr;
            max_s = curr->s.len;
        }
    }
    
    if (ptoken)
        *ptoken = max_n->id;
    if (pskip)
        *pskip = max_s;
    return 1;
#else
    
    const RWKVTrie* old_node = curr;
    size_t old_index = 0;
    size_t i;
    
    for (i = 1; i < available; i++) {
        const RWKVTrie* next = curr->next[ptr[i]];
        
        if (!next) {
            break;
        }
        
        if (curr->id) {
            old_node = curr;
            old_index = i;
        }
        curr = next;
    }
    
    if (!curr->id) {
        if (ptoken)
            *ptoken = old_node->id;
        if (pskip)
            *pskip = old_index;
    } else {
        if (ptoken)
            *ptoken = curr->id;
        if (pskip)
            *pskip = i;
    }
    
    return 1;
#endif
}

static void
rwkv_tokenizer_init(RWKVTokenizer* tokenizer) {
    memset(tokenizer, 0, sizeof(*tokenizer));
}

static RWKVTokenizer*
rwkv_tokenizer_alloc(U32 num_tokens) {
    // Increase C++ validity
    RWKVTokenizer* tokenizer = (RWKVTokenizer*)memset(malloc(sizeof(RWKVTokenizer)), 0, sizeof(RWKVTokenizer));
    tokenizer->num_tokens = num_tokens;
    tokenizer->idx2token = calloc(num_tokens + 1, sizeof(RWKVS8));
    
    return tokenizer;
}

// NOTE(mrsteyk): has libc dep due to allocate call
static B32
rwkv_tokenizer_add(RWKVTokenizer* tokenizer, RWKVS8 token, token_t id) {
    if (!tokenizer || !token.ptr || !token.len) return 0;
    
    if (id > tokenizer->num_tokens) return 0;
    
    if (tokenizer->idx2token[id].ptr) return 0;
    
    tokenizer->idx2token[id] = token;
    
    if (!tokenizer->root[token.ptr[0]]) {
        RWKVTrie* trie = rwkv_trie_alloc();
        tokenizer->root[token.ptr[0]] = trie;
    }
    
    RWKVTrie* t = tokenizer->root[token.ptr[0]];
    
    for (size_t i = 1; i < token.len; i++) {
        if (!t->next[token.ptr[i]]) {
            t->next[token.ptr[i]] = rwkv_trie_alloc();
        }
        
        t = t->next[token.ptr[i]];
    }
    
    t->id = id;
    //t->s = token;
    
    return 1;
}

static B32
rwkv_tokenize(const RWKVTokenizer* tokenizer, RWKVS8 data, token_t* out, size_t* size) {
    if (!tokenizer || !data.ptr || !data.len) return 0;
    
    if (!out) {
        // NOTE(mrsteyk): counter mode
        size_t offset = 0;
        size_t skip = 0;
        size_t counter = 0;
        
        const RWKVTrie** root = tokenizer->root;
        while(rwkv_trie_find_longest(root, data.ptr + offset, data.len - offset, 0, &skip)) {
            counter++;
            if((data.len - offset) <= skip) {
                *size = counter;
                return 1;
            }
            
            offset += skip;
        }
        
        // TODO(mrsteyk): REMOVE
        __debugbreak();
        // TODO(mrsteyk): broken?
        *size = counter;
        return 0;
    } else {
        // NOTE(mrsteyk): normal tokenizing mode
        size_t offset = 0;
        size_t counter = 0;
        size_t skip = 0;
        size_t sz = *size;
        token_t token = 0;
        
        const RWKVTrie** root = tokenizer->root;
        while(rwkv_trie_find_longest(root, data.ptr + offset, data.len - offset, &token, &skip)) {
            if (counter >= sz) {
                *size = counter + 1;
                return 0;
            }
            
            out[counter] = token;
            counter++;
            
            if((data.len - offset) <= skip) {
                *size = counter;
                return 1;
            }
            
            offset += skip;
        }
        
        // TODO(mrsteyk): REMOVE
        __debugbreak();
        *size = counter;
        // TODO(mrsteyk): ???
        return 0;
    }
}

static B32
rwkv_tokenizer_decode(const RWKVTokenizer* tokenizer, token_t* tokens, size_t len, U8* out, size_t* size) {
    if (!tokenizer || !tokens || !len || !size) return 0;
    
    if (!out) {
        // NOTE(mrsteyk): counting mode
        size_t counter = 0;
        for (size_t i = 0; i < len; i++) {
            U32 token = tokens[i];
            if (token >= tokenizer->num_tokens)
                return 0;
            RWKVS8 b = tokenizer->idx2token[token];
            counter += b.len;
        }
        
        *size = counter;
        
        return 1;
    } else {
        // NOTE(mrsteyk): normal mode
        size_t counter = 0;
        size_t sz = *size;
        
        for (size_t i = 0; i < len; i++) {
            U32 token = tokens[i];
            if (token >= tokenizer->num_tokens) {
                return 0;
            }
            RWKVS8 b = tokenizer->idx2token[token];
            if ((counter + b.len) > sz) {
                return 0;
            }
            memcpy(out + counter, b.ptr, b.len * sizeof(*b.ptr));
            counter += b.len;
        }
        
        *size = counter;
        
        return 1;
    }
}