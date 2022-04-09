#include <iostream>
#include "jserdes.h"

struct Complex
{
    float real = 0.0f;
    float imag = 0.0f;
};

struct CplxCoef
{
    int order = 1;
    Complex coef;
    // example of intrusive ser
    void ser(jserdes::jser<> &ser) const
    {
        // ser.newline();
        ser.elem("order", "%2d", order);
        ser.no_newline();
        ser.structure<Complex>("coef", coef, [&](const Complex &it){
            ser.elem("real", "%9.6f", it.real);
            ser.elem("imag", "%9.6f", it.imag);
        });
    }
    // example of intrusive des
    void des(jserdes::jdes &des)
    {
        des.named_elem("%d", order);
        des.structure<Complex>(coef, [&](const Complex &it){
            des.named_elem("%f", it.real);
            des.named_elem("%f", it.imag);
        });
    }
};

struct TestStruct
{
    char testStrs[2][64] = {"This is the 1st string of a string array.", "This is the 2nd string of a string array."};
    char testStr[64]     = "This is a test string.";
    CplxCoef coefs[4] = {
            {1, { 1.23456789f, -2.34567890f}},
            {2, { 3.14159265f, -2.71828183f}},
            {3, {-1.41421356f,  1.73205081f}},
            {4, { 2.23606798f,  2.64575131f}}
    };
    struct Seqs
    {
        int idxs[8] = {2, 3, 5, 7, 11, 13, 17, 19};
        int length = 4;
    } seqs;
    float corrMat[3][4] = {
            { 0.123456f,  1.012345f, -0.012345f,  0.001234f},
            {-0.123456f,  0.013579f, -1.009876f,  0.013579f},
            { 0.098765f,  0.001234f, -0.012345f, -0.991234f}
    };
};

// example of non-intrusive ser for TestStruct
void serTestStruct(const TestStruct &it, jserdes::jser<> &ser)
{
    ser.array<char>("testStrs", &it.testStrs[0][0], 64, 2, [&](const char &c){
        ser.newline();
        ser.str("", &c);
    });
    ser.newline();
    ser.str("testStr", it.testStr);
    ser.array<CplxCoef>("coefs", it.coefs, 4, [&](const CplxCoef &it){
        ser.structure<CplxCoef>("", it, [&](const CplxCoef &it){
            it.ser(ser);
        });
    });
    ser.structure<TestStruct::Seqs>("seqs", it.seqs, [&](const TestStruct::Seqs &it){
        ser.array<int>("idxs", it.idxs, 8, [&](const int &it){
            ser.elem("", "%2d", it);
        });
        ser.newline();
        ser.elem("length", "%2d", it.length);
    });
    ser.array2d<float>("corrMat", &it.corrMat[0][0], 3, 4, [&](const float &it){
        ser.elem("", "%9.6f", it);
    });
}

// example of non-intrusive des for TestStruct
void desTestStruct(TestStruct &it, jserdes::jdes &des)
{
    des.array<char>(&it.testStrs[0][0], 64, 2, [&](char &c){
        des.anony_str(&c);
    });
    des.named_str(it.testStr);
    des.array<CplxCoef>(it.coefs, 4, [&](CplxCoef &it){
        it.des(des);
    });
    des.structure<TestStruct::Seqs>(it.seqs, [&](TestStruct::Seqs &it){
        des.array<int>(it.idxs, 8, [&](int &it){
            des.anony_elem("%d", it);
        });
        des.named_elem("%d", it.length);
    });
    des.array2d<float>(&it.corrMat[0][0], 3, 4, [&](float &it){
        des.anony_elem("%f", it);
    });
}

int main(int argc, const char *argv[])
{
    using namespace std;
    std::cout << "jserdes test." << std::endl;
    // the struct to be serialized
    TestStruct testStruct;
    // string buffer
    char *str = new char[1024];
    // jser with initial indent = 0, and "\n" as line seperator.
    jserdes::jser<> ser(str, 0, "\n");
    // serialize testStruct
    ser.structure<TestStruct>("", testStruct, [&](const TestStruct &it){
        serTestStruct(it, ser);
    });
    cout << "seralized testStruct:" << endl;
    cout << str << endl;
    TestStruct test2;
    jserdes::jdes des(str);
    des.structure<TestStruct>(test2, [&](TestStruct &it){
        desTestStruct(it, des);
    });
    delete[] str;
    return 0;
}

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
