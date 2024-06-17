#define PY_SSIZE_T_CLEAN
#include <Python.h>


#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

HANDLE G_HEAP = 0;

static void*
realloc_stk(void* ptr, size_t size) {
    return HeapReAlloc(G_HEAP, 0, ptr, size);
}

static void*
malloc_stk(size_t size) {
    return HeapAlloc(G_HEAP, 0, size);
}

static void*
calloc_stk(size_t count, size_t size) {
    return HeapAlloc(G_HEAP, HEAP_ZERO_MEMORY, count*size);
}

#define realloc realloc_stk
#define malloc malloc_stk
#define calloc calloc_stk
#endif

#include "tokenizer.c"

typedef struct PyRWKVTokenizer PyRWKVTokenizer;
struct PyRWKVTokenizer {
    PyObject_HEAD
        RWKVTokenizer tokenizer;
};

static int
pytokenizer_init(PyRWKVTokenizer* self, PyObject* args, PyObject* kwds) {
    const char* bytes;
    Py_ssize_t bytes_size;
    
    if (!PyArg_ParseTuple(args, "s#", &bytes, &bytes_size))
        return -1;
    
    if (bytes_size < 4) {
        PyErr_SetString(PyExc_EOFError, "Failed to read num_tokens");
        return -1;
    }
    
    memset(&self->tokenizer, 0, sizeof(self->tokenizer));
    U32 num_tokens = *(U32*)bytes;
    RWKVS8* idx2token = calloc(num_tokens + 1, sizeof(RWKVS8));
    self->tokenizer.num_tokens = num_tokens;
    self->tokenizer.idx2token = idx2token;
    
    bytes += sizeof(U32);
    bytes_size -= sizeof(U32);
    
    for (size_t i = 0; i < num_tokens; i++) {
        if (bytes_size < 1) {
            PyErr_SetString(PyExc_EOFError, "Failed to read token length");
            free(idx2token);
            self->tokenizer.idx2token = 0;
            return -1;
        }
        RWKVS8 token;
        token.len = *(U8*)bytes;
        
        bytes++;
        bytes_size--;
        if (bytes_size <= 0 || bytes_size < token.len) {
            PyErr_SetString(PyExc_EOFError, "Failed to read token bytes");
            free(idx2token);
            self->tokenizer.idx2token = 0;
            return -1;
        }
        
        token.ptr = malloc(token.len);
        memcpy(token.ptr, bytes, token.len);
        
        if(!rwkv_tokenizer_add(&self->tokenizer, token, i + 1)) {
            PyErr_SetString(PyExc_Exception, "Failed to add token");
            free(idx2token);
            self->tokenizer.idx2token = 0;
            return -1;
        }
        
        bytes += token.len;
        bytes_size -= token.len;
    }
    
    return 0;
}

static void
pytokenizer_dealloc_rec(RWKVTrie* trie) {
    for (size_t i = 0; i < 256; i++) {
        if (trie->next[i]) {
            pytokenizer_dealloc_rec(trie->next[i]);
        }
    }
    free(trie);
}

static void
pytokenizer_dealloc(PyRWKVTokenizer *self) {
    if (self->tokenizer.idx2token) {
        free(self->tokenizer.idx2token);
    }
    for (size_t i = 0; i < 256; i++) {
        if (self->tokenizer.root[i]) {
            pytokenizer_dealloc_rec(self->tokenizer.root[i]);
        }
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject*
pytokenizer_encode(PyRWKVTokenizer *self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1 || !PyUnicode_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a single string");
        return NULL;
    }
    
    Py_ssize_t bytes_size;
    const char* bytes = PyUnicode_AsUTF8AndSize(args[0], &bytes_size);
    
    RWKVS8 data = { .ptr = (const U8*)bytes, .len = bytes_size };
    size_t counter = 0;
    size_t offset = 0;
    size_t skip = 0;
    token_t token = 0;
    
    size_t out_len = 64;
    token_t* out = malloc(sizeof(*out)*out_len);
    
    const RWKVTrie** root = self->tokenizer.root;
    while (bytes_size - offset) {
        rwkv_trie_find_longest(root, bytes + offset, bytes_size - offset, &token, &skip);
        
        if (counter == out_len) {
            out_len *= 2;
            token_t* out_new = realloc(out, sizeof(*out)*out_len);
            out = out_new;
        }
        
        out[counter] = token;
        counter++;
        
        offset += skip;
    }
    
    PyObject* list = PyList_New(counter);
    if (!list) {
        return NULL;
    }
    
    for (size_t i = 0; i < counter; i++) {
        PyObject *integer = PyLong_FromLong(out[i]);
        if (!integer) {
            Py_DECREF(list);
            return NULL;
        }
        PyList_SET_ITEM(list, i, integer);
    }
    
    free(out);
    return list;
}

static PyObject*
pytokenizer_decode(PyRWKVTokenizer* self, PyObject *const *args, Py_ssize_t nargs) {
    if (nargs != 1 || !PyList_Check(args[0])) {
        PyErr_SetString(PyExc_TypeError, "Argument must be a single list");
        return NULL;
    }
    
    PyObject* list = args[0];
    Py_ssize_t size = PyList_Size(list);
    
    size_t string_size = 0;
    // NOTE(mrsteyk): code duplication
    for (size_t i = 0; i < size; i++) {
        PyObject *item = PyList_GetItem(list, i);
        if (!PyLong_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List items must be integers");
            return NULL;
        }
        
        U32 token = PyLong_AsLong(item);
        if (token == (U32)-1 && PyErr_Occurred()) {
            return NULL;
        }
        
        if (token > self->tokenizer.num_tokens) {
            PyErr_SetString(PyExc_IndexError, "Token out of range");
            return NULL;
        }
        
        string_size += self->tokenizer.idx2token[token].len;
    }
    
    PyObject *decoded_string = PyUnicode_New(string_size, 127);
    
    if (!decoded_string) {
        return NULL;
    }
    
    size_t string_idx = 0;
    for (size_t i = 0; i < size; i++) {
        // NOTE(mrsteyk): we already checked all types and tokens
        PyObject *item = PyList_GetItem(list, i);
        
        if (!PyLong_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "List items must be integers");
            return NULL;
        }
        
        U32 token_id = PyLong_AsLong(item);
        if (token_id == (U32)-1 && PyErr_Occurred()) {
            return NULL;
        }
        
        if (token_id > self->tokenizer.num_tokens) {
            PyErr_SetString(PyExc_IndexError, "Token out of range");
            return NULL;
        }
        
        
        RWKVS8 token = self->tokenizer.idx2token[token_id];
        for (size_t j = 0; j < token.len; j++) {
            PyUnicode_WRITE(PyUnicode_1BYTE_KIND, PyUnicode_DATA(decoded_string), string_idx, (Py_UCS1)token.ptr[j]);
            string_idx++;
        }
    }
    
    return decoded_string;
}

static PyMethodDef PyRWKVTokenizer_methods[] = {
    {"decode", (PyCFunction)pytokenizer_decode, METH_FASTCALL, "Decode a list of tokens to a string"},
    {"encode", (PyCFunction)pytokenizer_encode, METH_FASTCALL, "Encode a string or readonly bytes object to a list of tokens"},
    {NULL}  // Sentinel
};

static PyTypeObject PyRWKVTokenizerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "rwkv_tokenizerc.RWKVTokenizer",
    .tp_doc = "RWKV Tokenizer object",
    .tp_basicsize = sizeof(PyRWKVTokenizer),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_init = (initproc)pytokenizer_init,
    .tp_dealloc = (destructor)pytokenizer_dealloc,
    .tp_methods = PyRWKVTokenizer_methods,
};

static struct PyModuleDef rwkv_tokenizermodule = {
    PyModuleDef_HEAD_INIT,
    .m_name = "rwkv_tokenizerc",
    .m_doc = "RWKVTokenizer C module",
    .m_size = -1,
};

PyMODINIT_FUNC PyInit_rwkv_tokenizerc(void) {
#if defined(_WIN32)
    G_HEAP = GetProcessHeap();
#endif
    
    PyObject *m;
    if (PyType_Ready(&PyRWKVTokenizerType) < 0) {
        return NULL;
    }
    
    m = PyModule_Create(&rwkv_tokenizermodule);
    if (m == NULL) {
        return NULL;
    }
    
    Py_INCREF(&PyRWKVTokenizerType);
    if (PyModule_AddObject(m, "RWKVTokenizer", (PyObject *)&PyRWKVTokenizerType) < 0) {
        Py_DECREF(&PyRWKVTokenizerType);
        Py_DECREF(m);
        return NULL;
    }
    
    return m;
}