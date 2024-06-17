# RWKV Trie Tokenizer in C

Initially started this because Blink posted a [Rust tokenizer][rust] in his channel, I looked at it and saw that it wasn't the most optimal thing (the decode). Without [that project][rust] I would've not done this.

RIIC.

## Usage

Run `./python_stuff/convert.py` to get the required `.bin` file.

```py3
import rwkv_tokenizerc
tokenizer = rwkv_tokenizerc.RWKVTokenizer(open("./python_stuff/rwkv_vocab_v20230424.bin", "rb").read())
```

## Rough Speed estimates

MB - megabytes (power of 1000).

MT - megatokens.

```
py -3 .\bench.py
926500
C:
Encode 369.418 MB/s
Decode 259.552 MT/s
Rust:
Encode 308.916 MB/s (sometimes gets to 370, but C is faster in that case as well)
Decode 22.554 MT/s
Ref:
Encode 7.792 MB/s
Decode 14.407 MT/s
```

[rust]: <https://github.com/cahya-wirawan/rwkv-tokenizer> "Rust RWKV Tokenizer by gh:cahya-wirawan"
