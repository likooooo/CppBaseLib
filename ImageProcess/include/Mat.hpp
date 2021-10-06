/*
 * @Description: ����
 * @Version: 1.0
 * @Autor: like
 * @Date: 2021-08-14 21:10:54
 * @LastEditors: like
 * @LastEditTime: 2021-10-06 23:53:33
 */
#ifndef MAT_HPP
#define MAT_HPP

#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string>

using namespace std;

template<typename TKernel>
/**
 * @description: ??????????????to_string(TKernel), ????????? 
 * @param {??????} t
 * @return {*}
 * @author: like
 */
inline std::string ToString(const TKernel* t)
{
    return to_string(*t);
}

template<typename TKernel>
class Mat   /* ?????????? */
{
public:
    TKernel * p;
    int w;
    int h;
    size_t length;
    Mat() : p(NULL){}
    Mat(int width, int height) : w(width), h(height),length(width * height), p((TKernel*)calloc(length, sizeof(TKernel))){}
    Mat(const void* source, int width, int height){}
    Mat(int width, int height, const TKernel* array):w(width), h(height), length(width * height)
    {
        p = (TKernel*)calloc(length, sizeof(TKernel))
        memcpy(p, array, sizeof(TKernel) * length);
    }
    ~Mat()
    {
        if(p)
        {
            free(p);
        }
    }
    void Init(const TKernel& val)
    {
        for(int i = 0; i < w * h; i++)
        {
            p[i] = val;
        }
    }
    TKernel* RowAt(int index)
    {
        return p + index * w; 
    }
    TKernel* operator[](int i)
    {
        return p + i;
    }
    string ToMatString(string strSplit = " ")
    {
        TKernel* scan0 = p;
        string str = ToString<TKernel>(scan0++);
        for(size_t i = 1; i < length; i++, scan0++)
        {     
            if(i % w)
            {
                str += strSplit;
            }  
            else
            {
                str += "\n";
            }    
            str += ToString<TKernel>(scan0);
        }
        str += "\n";
        return str;
    }
};

#endif