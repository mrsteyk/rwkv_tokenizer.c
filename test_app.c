#if defined(_WIN32)
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#else
#define __debugbreak() __builtin_trap()
#endif

#include <stdio.h>
#include <stdlib.h>

#include "tokenizer.c"

int
main() {
    FILE* f = fopen("python_stuff/rwkv_vocab_v20230424.bin", "rb");
    if (!f) {
        return 1;
    }
    
    B32 status;
    
    RWKVTokenizer tokenizer = {0};
    fread(&tokenizer.num_tokens, 4, 1, f);
    tokenizer.idx2token = calloc(tokenizer.num_tokens + 1, sizeof(RWKVS8));
    for (size_t i = 0; i < tokenizer.num_tokens; i++) {
        uint8_t size;
        fread(&size, 1, 1, f);
        RWKVS8 token;
        token.len = size;
        // NOTE(mrsteyk): this is valid in C _)
        token.ptr = malloc(size);
        fread((U8*)token.ptr, 1, size, f);
        
        status = rwkv_tokenizer_add(&tokenizer, token, i + 1);
        if (!status) __debugbreak();
    }
    
    puts("Tokenizer create OK!");
    
    // "Привет мир _)\n\t\n\n\t\t"
    static const U8 st_[] = {208, 159, 209, 128, 208, 184, 208, 178, 208, 181, 209, 130, 32, 208, 188, 208, 184, 209, 128, 32, 95, 41, 10, 9, 10, 10, 9, 9};
    static const RWKVS8 st = { .ptr = st_, .len = sizeof(st_)/sizeof(st_[0]) };
    
    size_t st_count_size = 0;
    status = rwkv_tokenize(&tokenizer, st, 0, &st_count_size);
    if (!status) __debugbreak();
    puts("Tokenize count OK!");
    
    token_t out[16];
    size_t out_size = 16;
    status = rwkv_tokenize(&tokenizer, st, out, &out_size);
    if (!status) __debugbreak();
    puts("Tokenize OK!");
    
    static const token_t st_expected[] = {27858, 27950, 27930, 32732, 2810, 4391, 3325, 3324};
    if (out_size != (sizeof(st_expected)/sizeof(st_expected[0])) || memcmp(st_expected, out, sizeof(st_expected))) {
        __debugbreak();
    }
    puts("Tokenize match!");
    
    U8 out_st[32];
    size_t out_st_size = 32;
    status = rwkv_tokenizer_decode(&tokenizer, st_expected, out_size, out_st, &out_st_size);
    if (!status) __debugbreak();
    puts("Decode OK!");
    
    if (sizeof(st_) != out_st_size || memcmp(st_, out_st, sizeof(st))) {
        __debugbreak();
    }
    puts("Decode match!");
    
    puts("Pass!\n");
    
    {
#if defined(_WIN32)
        LARGE_INTEGER freq;
        LARGE_INTEGER start, end;
        QueryPerformanceFrequency(&freq);
        //-
        
        static const U8 chinese[] = {232, 181, 183, 230, 165, 173, 229, 174, 182, 227, 130, 164, 227, 131, 188, 227, 131, 173, 227, 131, 179, 227, 131, 187, 227, 131, 158, 227, 130, 185, 227, 130, 175, 230, 176, 143, 227, 129, 140, 229, 137, 181, 230, 165, 173, 227, 129, 151, 227, 129, 159, 229, 174, 135, 229, 174, 153, 233, 150, 139, 231, 153, 186, 228, 188, 129, 230, 165, 173, 227, 128, 140, 227, 130, 185, 227, 131, 154, 227, 131, 188, 227, 130, 185, 88, 239, 188, 136, 227, 130, 168, 227, 131, 131, 227, 130, 175, 227, 130, 185, 239, 188, 137, 227, 128, 141, 227, 129, 174, 229, 183, 168, 229, 164, 167, 230, 150, 176, 229, 158, 139, 227, 131, 173, 227, 130, 177, 227, 131, 131, 227, 131, 136, 227, 128, 140, 227, 130, 185, 227, 130, 191, 227, 131, 188, 227, 130, 183, 227, 131, 131, 227, 131, 151, 227, 128, 141, 227, 129, 140, 50, 48, 230, 151, 165, 230, 156, 157, 227, 128, 129, 229, 136, 157, 227, 130, 129, 227, 129, 166, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 130, 137, 227, 130, 140, 227, 129, 159, 227, 129, 140, 227, 128, 129, 231, 136, 134, 231, 153, 186, 227, 129, 151, 227, 129, 159, 227, 128, 130, 10, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 129, 175, 231, 177, 179, 227, 131, 134, 227, 130, 173, 227, 130, 181, 227, 130, 185, 229, 183, 158, 227, 129, 174, 230, 157, 177, 230, 181, 183, 229, 178, 184, 227, 129, 167, 232, 161, 140, 227, 130, 143, 227, 130, 140, 227, 129, 159, 227, 128, 130, 231, 132, 161, 228, 186, 186, 227, 129, 174, 232, 169, 166, 233, 168, 147, 227, 129, 167, 227, 128, 129, 232, 178, 160, 229, 130, 183, 232, 128, 133, 227, 129, 175, 227, 129, 132, 227, 129, 170, 227, 129, 139, 227, 129, 163, 227, 129, 159, 227, 128, 130, 10, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 129, 139, 227, 130, 137, 50, 239, 189, 158, 51, 229, 136, 134, 229, 190, 140, 227, 128, 129, 229, 143, 178, 228, 184, 138, 230, 156, 128, 229, 164, 167, 227, 129, 174, 227, 131, 173, 227, 130, 177, 227, 131, 131, 227, 131, 136, 227, 129, 140, 229, 136, 182, 229, 190, 161, 228, 184, 141, 232, 131, 189, 227, 129, 171, 227, 129, 170, 227, 130, 138, 227, 128, 129, 227, 129, 190, 227, 130, 130, 227, 129, 170, 227, 129, 143, 230, 144, 173, 232, 188, 137, 227, 129, 149, 227, 130, 140, 227, 129, 159, 232, 163, 133, 231, 189, 174, 227, 129, 167, 231, 160, 180, 229, 163, 138, 227, 129, 149, 227, 130, 140, 227, 129, 159, 227, 128, 130, 10, 227, 131, 158, 227, 130, 185, 227, 130, 175, 230, 176, 143, 227, 129, 175, 227, 128, 129, 230, 149, 176, 227, 130, 171, 230, 156, 136, 229, 190, 140, 227, 129, 171, 229, 134, 141, 230, 140, 145, 230, 136, 166, 227, 129, 153, 227, 130, 139, 227, 129, 168, 232, 161, 168, 230, 152, 142, 227, 129, 151, 227, 129, 159, 227, 128, 130, 10, 227, 130, 185, 227, 131, 154, 227, 131, 188, 227, 130, 185, 88, 227, 129, 174, 227, 130, 168, 227, 131, 179, 227, 130, 184, 227, 131, 139, 227, 130, 162, 227, 129, 159, 227, 129, 161, 227, 129, 175, 227, 128, 129, 227, 129, 157, 227, 130, 140, 227, 129, 167, 227, 130, 130, 227, 129, 147, 227, 129, 174, 230, 151, 165, 227, 129, 174, 227, 131, 159, 227, 131, 131, 227, 130, 183, 227, 131, 167, 227, 131, 179, 227, 129, 175, 230, 136, 144, 229, 138, 159, 227, 129, 160, 227, 129, 163, 227, 129, 159, 227, 129, 168, 227, 129, 151, 227, 129, 166, 227, 129, 132, 227, 130, 139, 227, 128, 130, 227, 128, 140, 230, 151, 169, 230, 156, 159, 227, 129, 171, 233, 160, 187, 231, 185, 129, 227, 129, 171, 232, 169, 166, 233, 168, 147, 227, 129, 153, 227, 130, 139, 227, 128, 141, 227, 129, 147, 227, 129, 168, 227, 130, 146, 229, 165, 189, 227, 130, 128, 228, 186, 186, 227, 129, 159, 227, 129, 161, 227, 129, 170, 227, 129, 174, 227, 129, 167, 227, 128, 129, 231, 160, 180, 229, 163, 138, 227, 130, 146, 230, 129, 144, 227, 130, 140, 227, 129, 166, 227, 129, 132, 227, 129, 170, 227, 129, 132, 227, 128, 130, 230, 172, 161, 227, 129, 174, 227, 131, 149, 227, 131, 169, 227, 130, 164, 227, 131, 136, 227, 129, 171, 229, 144, 145, 227, 129, 145, 227, 128, 129, 229, 164, 167, 233, 135, 143, 227, 129, 174, 227, 131, 135, 227, 131, 188, 227, 130, 191, 227, 130, 146, 229, 143, 142, 233, 155, 134, 227, 129, 151, 227, 129, 159, 227, 129, 175, 227, 129, 154, 227, 129, 160, 227, 128, 130, 50, 230, 169, 159, 231, 155, 174, 227, 129, 174, 227, 130, 185, 227, 130, 191, 227, 131, 188, 227, 130, 183, 227, 131, 131, 227, 131, 151, 227, 129, 175, 227, 128, 129, 227, 129, 187, 227, 129, 188, 233, 163, 155, 232, 161, 140, 230, 186, 150, 229, 130, 153, 227, 129, 140, 230, 149, 180, 227, 129, 163, 227, 129, 166, 227, 129, 132, 227, 130, 139, 227, 128, 130, 10, 227, 131, 158, 227, 130, 185, 227, 130, 175, 230, 176, 143, 227, 129, 175, 227, 128, 129, 227, 128, 140, 83, 112, 97, 99, 101, 88, 227, 131, 129, 227, 131, 188, 227, 131, 160, 227, 129, 174, 231, 154, 134, 227, 129, 149, 227, 130, 147, 227, 128, 129, 227, 130, 185, 227, 130, 191, 227, 131, 188, 227, 130, 183, 227, 131, 131, 227, 131, 151, 227, 129, 174, 227, 130, 168, 227, 130, 173, 227, 130, 181, 227, 130, 164, 227, 131, 134, 227, 130, 163, 227, 131, 179, 227, 130, 176, 227, 129, 170, 232, 169, 166, 233, 168, 147, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 128, 129, 227, 129, 138, 227, 130, 129, 227, 129, 167, 227, 129, 168, 227, 129, 134, 239, 188, 129, 227, 128, 128, 230, 149, 176, 227, 130, 171, 230, 156, 136, 229, 190, 140, 227, 129, 171, 232, 161, 140, 227, 130, 143, 227, 130, 140, 227, 130, 139, 230, 172, 161, 227, 129, 174, 232, 169, 166, 233, 168, 147, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 129, 171, 229, 144, 145, 227, 129, 145, 227, 129, 166, 227, 128, 129, 229, 164, 154, 227, 129, 143, 227, 130, 146, 229, 173, 166, 227, 130, 147, 227, 129, 160, 227, 128, 141, 227, 129, 168, 227, 131, 132, 227, 130, 164, 227, 131, 188, 227, 131, 136, 227, 129, 151, 227, 129, 159, 227, 128, 130, 10, 227, 130, 162, 227, 131, 161, 227, 131, 170, 227, 130, 171, 227, 129, 167, 227, 129, 174, 227, 131, 173, 227, 130, 177, 227, 131, 131, 227, 131, 136, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 130, 146, 232, 170, 141, 229, 143, 175, 227, 129, 153, 227, 130, 139, 231, 177, 179, 233, 128, 163, 233, 130, 166, 232, 136, 170, 231, 169, 186, 229, 177, 128, 239, 188, 136, 78, 65, 83, 65, 239, 188, 137, 227, 129, 175, 227, 128, 129, 228, 186, 139, 230, 149, 133, 232, 170, 191, 230, 159, 187, 227, 130, 146, 231, 155, 163, 231, 157, 163, 227, 129, 153, 227, 130, 139, 227, 129, 168, 227, 129, 151, 227, 129, 159, 227, 128, 130, 229, 186, 131, 229, 160, 177, 230, 139, 133, 229, 189, 147, 232, 128, 133, 227, 129, 175, 227, 128, 129, 233, 163, 155, 232, 161, 140, 228, 184, 173, 227, 129, 171, 230, 169, 159, 228, 189, 147, 227, 129, 140, 229, 164, 177, 227, 130, 143, 227, 130, 140, 227, 129, 159, 229, 160, 180, 229, 144, 136, 227, 129, 174, 233, 128, 154, 229, 184, 184, 227, 129, 174, 229, 175, 190, 229, 191, 156, 227, 129, 160, 227, 129, 168, 232, 191, 176, 227, 129, 185, 227, 129, 159, 227, 128, 130, 10, 227, 131, 158, 227, 130, 185, 227, 130, 175, 230, 176, 143, 227, 129, 175, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 229, 137, 141, 227, 128, 129, 230, 156, 159, 229, 190, 133, 229, 128, 164, 227, 130, 146, 228, 184, 139, 227, 129, 146, 227, 130, 136, 227, 129, 134, 227, 129, 168, 227, 129, 151, 227, 129, 166, 227, 129, 132, 227, 129, 159, 227, 128, 130, 231, 153, 186, 229, 176, 132, 229, 143, 176, 227, 129, 174, 232, 168, 173, 229, 130, 153, 227, 130, 146, 231, 160, 180, 229, 163, 138, 227, 129, 155, 227, 129, 154, 227, 129, 171, 230, 169, 159, 228, 189, 147, 227, 130, 146, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 227, 130, 139, 227, 129, 160, 227, 129, 145, 227, 129, 167, 227, 130, 130, 227, 128, 140, 230, 136, 144, 229, 138, 159, 227, 128, 141, 227, 129, 160, 227, 129, 168, 227, 129, 151, 227, 129, 166, 227, 129, 132, 227, 129, 159, 227, 128, 130, 10, 227, 129, 157, 227, 129, 174, 233, 161, 152, 227, 129, 132, 227, 129, 175, 227, 129, 139, 227, 129, 170, 227, 129, 163, 227, 129, 159, 227, 128, 130, 227, 130, 185, 227, 130, 191, 227, 131, 188, 227, 130, 183, 227, 131, 131, 227, 131, 151, 227, 129, 175, 230, 137, 147, 227, 129, 161, 228, 184, 138, 227, 129, 146, 230, 150, 189, 232, 168, 173, 227, 129, 139, 227, 130, 137, 227, 129, 169, 227, 130, 147, 227, 129, 169, 227, 130, 147, 228, 184, 138, 230, 152, 135, 227, 129, 151, 227, 128, 129, 227, 131, 161, 227, 130, 173, 227, 130, 183, 227, 130, 179, 230, 185, 190, 227, 129, 174, 228, 184, 138, 231, 169, 186, 227, 129, 184, 227, 129, 168, 229, 144, 145, 227, 129, 139, 227, 129, 163, 227, 129, 166, 227, 129, 132, 227, 129, 163, 227, 129, 159, 227, 128, 130, 227, 129, 151, 227, 129, 139, 227, 129, 151, 49, 229, 136, 134, 227, 130, 130, 227, 129, 151, 227, 129, 170, 227, 129, 132, 227, 129, 134, 227, 129, 161, 227, 128, 129, 227, 129, 153, 227, 129, 185, 227, 129, 166, 227, 129, 140, 232, 168, 136, 231, 148, 187, 233, 128, 154, 227, 130, 138, 227, 129, 171, 233, 128, 178, 227, 130, 147, 227, 129, 167, 227, 129, 132, 227, 130, 139, 227, 129, 174, 227, 129, 167, 227, 129, 175, 227, 129, 170, 227, 129, 132, 227, 129, 147, 227, 129, 168, 227, 129, 140, 230, 152, 142, 227, 130, 137, 227, 129, 139, 227, 129, 171, 227, 129, 170, 227, 129, 163, 227, 129, 159, 227, 128, 130};
        const size_t chinese_count = sizeof(chinese);
        RWKVS8 chinese_s = { .ptr = chinese, .len = chinese_count };
        
        static const token_t chinese_tokens[] = {16944, 13436, 11920, 10169, 10242, 10237, 10239, 10241, 10222, 10189, 10179, 13651, 10108, 10827, 13436, 43347, 11887, 11898, 17693, 14728, 10412, 13436, 10088, 10189, 10218, 43528, 89, 19133, 10172, 43484, 10189, 19134, 10089, 10138, 12140, 11638, 13034, 11496, 10237, 10181, 43485, 10088, 58583, 10187, 43486, 10089, 10108, 640, 13053, 13198, 10079, 10782, 43425, 12642, 10127, 10257, 10114, 43435, 43358, 10079, 14288, 14728, 43347, 28329, 12642, 10127, 10257, 10114, 10139, 15260, 10199, 10177, 10185, 10189, 12133, 10138, 13241, 13849, 12061, 10132, 16403, 43448, 10125, 10080, 14215, 10370, 10138, 16572, 18142, 10132, 10079, 16821, 10616, 15643, 10139, 43314, 58526, 28329, 12642, 10127, 10257, 10114, 43328, 51, 19219, 52, 10768, 12341, 10079, 11024, 10257, 13188, 11638, 10138, 10237, 10181, 43485, 10108, 10794, 12354, 10260, 15752, 43410, 10159, 10079, 10148, 10152, 43399, 12883, 17058, 58534, 16451, 15586, 10132, 14894, 11595, 58534, 28329, 10222, 10189, 10179, 13651, 10139, 10079, 13004, 10175, 13190, 12341, 10136, 10703, 12752, 12616, 43352, 10133, 16417, 13078, 43347, 28329, 10189, 10218, 43528, 89, 10138, 10172, 10239, 10188, 10204, 10167, 10125, 10127, 10139, 10079, 43356, 43385, 43340, 13053, 10138, 10223, 10197, 58582, 10139, 12604, 10841, 58542, 58558, 43317, 10080, 10088, 13057, 13199, 10136, 17979, 15414, 10136, 16572, 18142, 43352, 10089, 43339, 10164, 11685, 10150, 10370, 10125, 10127, 43402, 10132, 10079, 14894, 11595, 10164, 12430, 58580, 43397, 10080, 13557, 10138, 10213, 43505, 10201, 10136, 11047, 10113, 10079, 11638, 17387, 10138, 10200, 43530, 10164, 10996, 17848, 43347, 10139, 10122, 10126, 10080, 51, 13521, 14778, 10138, 58583, 10187, 43486, 10139, 10079, 10147, 3022, 189, 18051, 16403, 13982, 10605, 10108, 13006, 64128, 28329, 10222, 10189, 10179, 13651, 10139, 10079, 10088, 33450, 89, 10196, 43534, 10138, 14735, 43344, 10079, 58583, 10187, 43486, 10138, 10172, 10177, 43468, 43487, 43519, 10135, 16572, 18142, 12642, 10127, 10257, 10114, 10079, 10106, 10151, 10132, 10133, 10104, 19126, 10078, 13004, 10175, 13190, 12341, 10136, 16403, 43448, 10160, 13557, 10138, 16572, 18142, 12642, 10127, 10257, 10114, 10136, 11047, 43337, 10079, 11632, 10111, 10164, 11871, 10165, 10126, 10089, 10133, 10198, 10169, 43531, 43347, 28329, 64137, 43383, 10237, 10181, 43485, 12642, 10127, 10257, 10114, 10164, 16587, 11021, 43352, 15260, 17197, 17269, 15878, 15090, 12005, 19133, 23940, 19134, 10139, 10079, 10336, 12985, 16605, 13299, 10164, 14773, 14819, 43352, 43388, 10125, 10080, 12212, 11549, 12701, 12307, 15643, 10139, 10079, 18051, 16403, 10285, 10136, 13521, 10452, 10108, 11645, 43448, 10125, 11550, 11038, 10138, 17189, 12182, 10138, 11961, 12380, 10126, 10133, 17162, 10146, 10125, 28329, 10222, 10189, 10179, 13651, 10139, 12642, 10127, 10257, 10114, 10808, 10079, 13199, 12335, 10567, 10164, 10258, 10114, 43429, 58558, 43310, 10080, 14728, 11965, 11022, 10138, 16552, 10605, 10164, 14894, 11595, 10123, 10122, 10136, 13521, 10452, 10164, 12642, 10127, 10257, 10114, 10160, 43366, 43385, 10088, 12604, 10841, 10089, 10126, 58558, 43310, 28329, 43355, 17988, 10103, 10139, 43327, 43369, 10080, 58583, 10187, 43486, 10139, 12642, 10127, 10257, 10114, 13038, 16552, 43328, 10134, 10165, 10134, 10165, 10257, 13074, 10119, 10079, 10225, 10177, 10187, 10183, 13974, 10138, 10257, 15090, 10145, 10133, 11047, 43326, 43372, 43369, 10080, 43346, 10119, 50, 10768, 10152, 10119, 43397, 10104, 10127, 10079, 10121, 10146, 10131, 10108, 16540, 14604, 17189, 10159, 10136, 17202, 43450, 43317, 43416, 43419, 10103, 58532, 13078, 10158, 10107, 58565, 10125, 10080};
        const size_t chinese_tokens_count = sizeof(chinese_tokens)/sizeof(chinese_tokens[0]);
        
        static token_t out_big[65536];
        size_t out_big_size = 65536;
        
        status = rwkv_tokenize(&tokenizer, chinese_s, out_big, &out_big_size);
        if (!status) __debugbreak();
        puts("Chinese tokenize OK!");
        
        if (out_big_size != chinese_tokens_count || memcmp(chinese_tokens, out_big, chinese_tokens_count)) {
            __debugbreak();
        }
        puts("Chinese tokenize match!");
        
        static U8 out_big_s[65536];
        size_t out_big_s_size = 65536;
        
        status = rwkv_tokenizer_decode(&tokenizer, out_big, chinese_tokens_count, out_big_s, &out_big_s_size);
        if (!status) __debugbreak();
        puts("Chinese decode OK!");
        
        if (out_big_s_size != chinese_count || memcmp(chinese, out_big_s, chinese_count)) {
            __debugbreak();
        }
        puts("Chinese decode match!");
        
        QueryPerformanceCounter(&start);
        for (size_t i = 0; i < 500; i++) {
            // Reset
            out_big_size = sizeof(out_big)/sizeof(out_big[0]);
            
            rwkv_tokenize(&tokenizer, chinese_s, out_big, &out_big_size);
        }
        QueryPerformanceCounter(&end);
        
        printf("Rough tokenizing speed estimate is: %.02f MB/s\n", (chinese_count*500.f/1024.f/1024.f)/((end.QuadPart - start.QuadPart)/(float)freq.QuadPart));
        
        
        QueryPerformanceCounter(&start);
        for (size_t i = 0; i < 500; i++) {
            // Reset
            out_big_s_size = sizeof(out_big_s);
            
            rwkv_tokenizer_decode(&tokenizer, chinese_tokens, chinese_tokens_count, out_big_s, &out_big_s_size);
        }
        QueryPerformanceCounter(&end);
        
        printf("Rough decoding speed estimate is: %.02f MT/s\n", (chinese_tokens_count*500.f/1024.f/1024.f)/((end.QuadPart - start.QuadPart)/(float)freq.QuadPart));
        
        //-
        
        FILE* fb = fopen("", "rb");
        
        // Reset
        out_big_size = sizeof(out_big)/sizeof(out_big[0]);
        // Reset
        out_big_s_size = sizeof(out_big_s);
        
#endif
    }
    return 0;
}