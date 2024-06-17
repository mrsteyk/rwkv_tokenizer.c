from setuptools import setup, Extension

module = Extension(
    "rwkv_tokenizerc",
    sources=["py.c"],
    depends=["tokenizer.c"],
)

setup(
    name="rwkv_tokenizerc",
    version="0.1",
    description="RWKV tokenizer C module",
    ext_modules=[module],
)