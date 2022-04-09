//
//  jserdes.h
//  jserdes
//
//  Created by Loy Kyle Wong on 2022/4/5.

// this code can be used to serialize and deserialize c/c++ struct to/from c-type json like string(char array),
// it's intended to be used in embedded systems, so no dynamic thing is used except the std::function for ease of using lambda.
// this is not simple enough yet, hard coding lines may be more than the struct definition, see jserdes_test.cc for examples.

// ==== test structs in jserdes_test.cc ====
// struct Complex
// {
//     float real = 0.0f;
//     float imag = 0.0f;
// };
// struct CplxCoef
// {
//     int order = 1;
//     Complex coef;
// };
// struct TestStruct
// {
//     char testStrs[2][64] = {"This is the 1st string of a string array.", "This is the 2nd string of a string array."};
//     char testStr[64]     = "This is a test string.";
//     CplxCoef coefs[4] = {
//             {1, { 1.23456789f, -2.34567890f}},
//             {2, { 3.14159265f, -2.71828183f}},
//             {3, {-1.41421356f,  1.73205081f}},
//             {4, { 2.23606798f,  2.64575131f}}
//     };
//     struct Seqs
//     {
//         int idxs[8] = {2, 3, 5, 7, 11, 13, 17, 19};
//         int length = 4;
//     } seqs;
//     float corrMat[3][4] = {
//             { 0.123456f,  1.012345f, -0.012345f,  0.001234f},
//             {-0.123456f,  0.013579f, -1.009876f,  0.013579f},
//             { 0.098765f,  0.001234f, -0.012345f, -0.991234f}
//     };
// };
// ==== output ====
// {
//     "testStrs" : [
//         "This is the 1st string of a string array.",
//         "This is the 2nd string of a string array."
//     ],
//     "testStr" : "This is a test string.",
//     "coefs" : [
//         { "order" :  1, "coef" : { "real" :  1.234568, "imag" : -2.345679 } },
//         { "order" :  2, "coef" : { "real" :  3.141593, "imag" : -2.718282 } },
//         { "order" :  3, "coef" : { "real" : -1.414214, "imag" :  1.732051 } },
//         { "order" :  4, "coef" : { "real" :  2.236068, "imag" :  2.645751 } }
//     ],
//     "seqs" : {
//         "idxs" : [  2,  3,  5,  7, 11, 13, 17, 19 ],
//         "length" :  4
//     },
//     "corrMat" : [
//         [  0.123456,  1.012345, -0.012345,  0.001234 ],
//         [ -0.123456,  0.013579, -1.009876,  0.013579 ],
//         [  0.098765,  0.001234, -0.012345, -0.991234 ]
//     ]
// }

#pragma once

#include <stdio.h>
#include <functional>
#include <stdarg.h>
#include <string.h>

namespace jserdes
{

// iw: indent width, spaces will be used for intdents.
template<int iw = 4>
class jser
{
private:
    int ind;            // current indent
    int l;              // working index in str
    char *s;            // string
    char ls[5]{0};      // line separation chars
    bool begin = true;
    bool nc = true;     // no comma
    int ln = 0;         // lines
    bool nl = false;    // str or elem new line
    bool nnl = false;   // array or struct no new line
    template<typename T>
    using epfunc = std::function<void(const T &elem)>;  // element print function
    inline void line_sep()
    {
        if(begin) begin = false;
        else for(int i = 0; '\0' != (s[l] = ls[i]); i++, l++);
    }
    inline void comma()
    {
        if(nc) nc = false;
        else { s[l++] = ','; s[l++] = ' '; }
    }
    inline void indent()
    {
        for(int i = 0; i < iw * ind; i++) s[l++] = ' ';
    }
    void next_line()
    {
        ln++;
        line_sep();
        indent();
    }
    inline void naming(const char *name)
    {
        if(*name)
            l += sprintf(s + l, "\"%s\" : ", name);
    }
public:
    jser() = delete;
    jser(const jser&) = delete;
    // str   : char buffer for storing the serializing result, make sure it's larger enough,
    //         or you must frequently pool len() to see if it's nearing the buffer size.
    // indent: initial indent.
    // ls    : chars for seperating lines, commonly "\r\n" of window, "\n" for unix and "\r" for mac.
    jser(char *str, int indent = 0, const char *ls = "\r\n")
    {
        this->s = str;
        this->ind = indent;
        this->l = 0;
        strcpy(this->ls, ls);
    }
    // length of current serialized string, without the tailing '\0'.
    int len() { return l; }
    // lines of current serialized string.
    int lines() { return ln; }
    // current indent level.
    int curr_indent() { return ind; }
    // increase and decrease indent, not recommended using since jser dealing with indents.
    void inc_indent() { nc = true; ind++; }
    void dec_indent() { nc = false; if(ind > 0) ind--; }
    // str and elem are start without new line by default,
    // if a new line needed, use newline() before next str or elem.
    void newline()    { nl = true; }
    // array and struct are start with new line by default,
    // if no new line needed, use no_newline() before next array or struct.
    void no_newline() { nnl = true; }
//    void printf(const char *fmt, ...)
//    {
//        va_list va;
//        va_start(va, fmt);
//        l += vsprintf(s + l, fmt, va);
//        va_end(va);
//    }
    // serialize a string, if name == "" it will be anonymous.
    void str(const char *name, const char *str)
    {
        comma();
        if(nl) { nl = false; next_line(); }
        naming(name);
        l += sprintf(s + l, "\"%s\"", str);
    }
    // serialize an simple typed element by using "fmt" just as in printf(), if name == "" it will be anonymous.
    template<typename T>
    void elem(const char *name, const char *fmt, const T &elem)
    {
        comma();
        if(nl) { nl = false; next_line(); }
        naming(name);
        l += sprintf(s + l, fmt, elem);
    }
    // serialize a 2D array by providing a print function which serialize each element.
    // if name == "" it will be anonymous.
    template<typename T>
    void array2d(const char *name, const T *array, size_t size1, size_t size0, const epfunc<T> &print)
    {
        this->array<T>(name, array, size0, size1, [&](const T &it){
            this->array<T>("", &it, size0, [&](const T &it){
                print(it);
            });
        });
    }
    // serialize an array by providing a print function which serialize each element.
    // if name == "" it will be anonymous.
    template<typename T>
    void array(const char *name, const T *array, size_t len, const epfunc<T> &print)
    {
        this->array<T>(name, array, 1, len, print);
    }
    // serialize an array by providing a print function which serialize each element.
    // if name == "" it will be anonymous, step is intended for dealing multi-dimensional array.
    template<typename T>
    void array(const char *name, const T *array, size_t step, size_t len, const epfunc<T> &print)
    {
        comma();
        if(nnl) nnl = false;
        else next_line();
        int ln = this->ln;
        naming(name);
        s[l++] = '['; s[l++] = ' ';
        inc_indent();
        size_t i;
        for(i = 0; i < len - 1; i++)
        {
            print(array[i * step]);
            s[l++] = ',';
            s[l++] = ' ';
            nc = true;
        }
        print(array[i * step]);
        s[l++] = ' ';
        dec_indent();
        if(ln != this->ln) next_line();
        s[l++] = ']';
        s[l] = '\0';
    }
    // serialize a struct by providing a print function which serialize elements in it.
    // if name == "" it will be anonymous.
    template<typename T>
    void structure(const char *name, const T &item, const epfunc<T> &print)
    {
        comma();
        if(nnl) nnl = false;
        else next_line();
        int ln = this->ln;
        naming(name);
        s[l++] = '{'; s[l++] = ' ';
        inc_indent();
        print(item);
        s[l++] = ' ';
        dec_indent();
        if(ln != this->ln) next_line();
        s[l++] = '}';
        s[l] = '\0';
    }
};

class jdes
{
private:
    bool err = false;
    int ln = 1;
    int l;
    const char *s;
    template<typename T>
    using epfunc = std::function<void(T &elem)>;  // element parser function
    void discard_next(char c)
    {
        if(err) return;
        for(; s[l] != c; l++)
        {
            if(s[l] == '\0') { err = true; return; }
            else if((s[l  ] == '\r' || s[l  ] == '\n') &&
                    (s[l+1] != '\r' && s[l+1] != '\n')) { ln++; }
        }
        ++l;
    }
    void scanstr(char *str)
    {
        discard_next('\"');
        int m = l;
        discard_next('\"');
        if(!err && l - m - 1 >= 0)
        {
            memcpy(str, s + m, l - m - 1);
            str[l - m - 1] = '\0';
        }
    }
public:
    // str : c-type string (char array) with tailing '\0'
    jdes(const char *str)
    {
        this->s = str;
        this->l = 0;
    }
    // current parsing position in string.
    int pos() { return l; }
    // return whether there is error in the last parsing.
    // if ever error occurred, all successive parsing will do nothing.
    bool has_err() { return err; }
    // current parsing line in string.
    int curr_line() { return ln; }
    template<typename T>
//    int scanf(const char *fmt, ...)
//    {
//        va_list va;
//        va_start(va, fmt);
//        int n = vsscanf(s + l, fmt, va);
//        va_end(va);
//        return n;
//    }
    // parse a named element by using "fmt" just as in scanf().
    void named_elem(const char *fmt, T &elem)
    {
        discard_next(':');
        if(1 != sscanf(s + l, fmt, &elem)) err = true;
    }
    // parse an anonymous element by using "fmt" just as in scanf().
    template<typename T>
    void anony_elem(const char *fmt, T &elem)
    {
        if(1 != sscanf(s + l, fmt, &elem)) err = true;
    }
    // parse a named string.
    void named_str(char *str)
    {
        discard_next(':');
        scanstr(str);
    }
    // parse an anonymous string.
    void anony_str(char *str)
    {
        scanstr(str);
    }
    // parse a 2D array by providing "parse" function which can parse each element in it.
    template<typename T>
    void array2d(T *array, size_t size1, size_t size0, const epfunc<T> &parse)
    {
        this->array<T>(array, size0, size1, [&](T &it){
            this->array<T>(&it, size0, [&](T &it){
                parse(it);
            });
        });
    }
    // parse an array by providing "parse" function which can parse each element in it.
    template<typename T>
    void array(T *array, size_t len, const epfunc<T> &parse)
    {
        this->array(array, 1, len, parse);
    }
    // parse an array by providing "parse" function which can parse each element in it.
    // step is intended for dealing mult-dimensional array.
    template<typename T>
    void array(T *array, size_t step, size_t len, const epfunc<T> &parse)
    {
        discard_next('[');
        size_t i;
        for(i = 0; i < len - 1; i++)
        {
            parse(array[i * step]);
            discard_next(',');
        }
        parse(array[i * step]);
        discard_next(']');
    }
    // parse an struct by providing "parse" function which can parse elements in it.
    template<typename T>
    void structure(T &item, const epfunc<T> &parse)
    {
        discard_next('{');
        parse(item);
        discard_next('}');
    }
};

}
