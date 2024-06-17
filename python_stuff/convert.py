if __name__ == "__main__":
    import struct
    import rwkv_tokenizer

    tokenizer = rwkv_tokenizer.TRIE_TOKENIZER('./rwkv_vocab_v20230424.txt')
    st = "Привет мир _)\n\t\n\n\t\t"
    print('/*\n *', st, '\n *', list(st.encode('utf-8')), tokenizer.encode(st))
    print('*/')

    a = sorted(tokenizer.idx2token.items(), key = lambda x: x[0])
    print('//', a[:10], len(tokenizer.idx2token), max([len(i) for (_, i) in tokenizer.idx2token.items()]))
    with open("./rwkv_vocab_v20230424.bin", "wb") as f:
        # 65529 is really close to 65536
        f.write(struct.pack("<L", len(tokenizer.idx2token)))
        for (k, v) in a:
            # max length is 128
            f.write(struct.pack("<B", len(v)))
            f.write(v)
    
    # Now let's experiment with meta generation
    """
    string_table = b''
    string_table_offset = 0
    inner_struct = ''
    for (k, v) in a:
        string_table = string_table + v
        inner_struct = f'    {{ .id = {k}, .s = {{ .ptr = (u8*)(rwkv_token_string_table + {string_table_offset}), .size = {len(v)} }} }},\n'
        string_table_offset += len(v)
    #-
    print(f"const u8 rwkv_token_string_table[{len(string_table)}] = {{", end='')
    for (idx, i) in enumerate(string_table):
        if (idx % 40) == 0:
            print('\n   ', end='')
        print(f' 0x{i:02X},', end='')
    print("\n};\n")
    #-
    # """