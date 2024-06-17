import time
import rwkv_tokenizerc
import rwkv_tokenizer

from python_stuff.rwkv_tokenizer import TRIE_TOKENIZER

tokenizer = rwkv_tokenizerc.RWKVTokenizer(open("./python_stuff/rwkv_vocab_v20230424.bin", "rb").read())
rust = rwkv_tokenizer.RWKVTokenizer()
reference = TRIE_TOKENIZER("./python_stuff/rwkv_vocab_v20230424.txt")

src = '''起業家イーロン・マスク氏が創業した宇宙開発企業「スペースX（エックス）」の巨大新型ロケット「スターシップ」が20日朝、初めて打ち上げられたが、爆発した。
打ち上げは米テキサス州の東海岸で行われた。無人の試験で、負傷者はいなかった。
打ち上げから2～3分後、史上最大のロケットが制御不能になり、まもなく搭載された装置で破壊された。
マスク氏は、数カ月後に再挑戦すると表明した。
スペースXのエンジニアたちは、それでもこの日のミッションは成功だったとしている。「早期に頻繁に試験する」ことを好む人たちなので、破壊を恐れていない。次のフライトに向け、大量のデータを収集したはずだ。2機目のスターシップは、ほぼ飛行準備が整っている。
マスク氏は、「SpaceXチームの皆さん、スターシップのエキサイティングな試験打ち上げ、おめでとう！　数カ月後に行われる次の試験打ち上げに向けて、多くを学んだ」とツイートした。
アメリカでのロケット打ち上げを認可する米連邦航空局（NASA）は、事故調査を監督するとした。広報担当者は、飛行中に機体が失われた場合の通常の対応だと述べた。
マスク氏は打ち上げ前、期待値を下げようとしていた。発射台の設備を破壊せずに機体を打ち上げるだけでも「成功」だとしていた。
その願いはかなった。スターシップは打ち上げ施設からどんどん上昇し、メキシコ湾の上空へと向かっていった。しかし1分もしないうち、すべてが計画通りに進んでいるのではないことが明らかになった。'''

src = src*500
src_len = len(src.encode())

def benchmark(XXX):
    min_t = 1e100
    for i in range(100):
        t_begin = time.time_ns()
        tokens = XXX.encode(src)
        t = time.time_ns() - t_begin
        # print(t)
        min_t = min(t, min_t)
    # print(min_t)
    print('Encode', round(src_len / min_t * 1e3, 3), 'MB/s')
    lt = len(tokens)
    min_t = 1e100
    for i in range(100):
        t_begin = time.time_ns()
        sss = XXX.decode(tokens)
        min_t = min(time.time_ns() - t_begin, min_t)
    # print(min_t)
    print('Decode', round(lt / min_t * 1e3, 3), 'MT/s')

print(src_len)

print("C:")
benchmark(tokenizer) # 0.00321 + 0.00126
print("Rust:")
benchmark(rust)
print("Ref:")
benchmark(reference)