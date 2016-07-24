#pragma once

#include "Object.h"
#include <string>
#include <assert.h>
#ifdef _DEBUG
#include <assert.h>
#endif
template <class T> class CSmartPointer
{
    T* mPointer;
#ifdef _DEBUG
#ifdef _MSC_VER
#define __FUNCNAME__ __FUNCSIG__
#elif defined(__GNUC__)
#define __FUNCNAME__ __PRETY_FUNCTION__
#else
#define __FUNCNAME__ ""
#endif
#endif
    friend class CObject;
#ifdef _DEBUG
    std::string m_Name;
#endif

public:
#ifdef _DEBUG
    void Name(std::string pValue){m_Name = pValue;};
    std::string Name(){return m_Name;};
#else
    void Name(std::string){};
    std::string Name(){return "";};
#endif
#ifdef _DEBUG
    CSmartPointer(T* pPointer = (T*)0, std::string pName = "") : mPointer(pPointer), m_Name(pName)
    {
        if (mPointer)
            mPointer->mRefrences.push_back((CSmartPointer<CObject>*)this);
    }
    CSmartPointer(const CSmartPointer& ptr) : mPointer(ptr.mPointer), m_Name(ptr.m_Name)
    {
        if (mPointer)
            mPointer->mRefrences.push_back((CSmartPointer<CObject>*)this);
    }
#else
    CSmartPointer(T* pPointer = (T*)0) : mPointer (pPointer)
    {
        if(mPointer)
            mPointer->mRefrences ++;
    }
    CSmartPointer(const CSmartPointer& ptr) : mPointer(ptr.mPointer)
    {
        if(mPointer)
            mPointer->mRefrences ++;
    }
#endif
#ifdef _DEBUG
    CSmartPointer& operator = (T* pPointer)
                              {
        if (mPointer == pPointer)
            return *this;
        if (mPointer != NULL)
        {
            assert(mPointer->mRefrences.size() >= 1);
            std::list<CSmartPointer<CObject>*>::iterator i;
            for (i = mPointer->mRefrences.begin(); i != mPointer->mRefrences.end();i++)
                if ((*i) == (CSmartPointer<CObject>*)this)
                    break;
            assert (i != mPointer->mRefrences.end());
            mPointer->mRefrences.erase(i);
            if (mPointer->mRefrences.size() == 0)
                delete mPointer;
        }
        mPointer = pPointer;
        if (mPointer != NULL)
            mPointer->mRefrences.push_back((CSmartPointer<CObject>*)this);
        return *this;
    }

    CSmartPointer& operator = (const CSmartPointer& pPointer)
                              {
        if (mPointer == pPointer.mPointer)
            return *this;
        if (mPointer != NULL)
        {
            assert(mPointer->mRefrences.size() >= 1);
            std::list<CSmartPointer<CObject>*>::iterator i;
            for (i = mPointer->mRefrences.begin(); i != mPointer->mRefrences.end();i++)
                if ((*i) == (CSmartPointer<CObject>*)this)
                    break;
            assert(i != mPointer->mRefrences.end());
            mPointer->mRefrences.erase(i);
            if (mPointer->mRefrences.size() == 0)
                delete mPointer;
        }
        mPointer = pPointer.mPointer;
        if (mPointer != NULL)
            mPointer->mRefrences.push_back((CSmartPointer<CObject>*)this);
        return *this;
    }
#else
    CSmartPointer& operator = (T* pPointer)
                              {
        if (mPointer == pPointer)
            return *this;
        if (mPointer != NULL)
        {
            mPointer->mRefrences --;
            if (mPointer->mRefrences == 0)
                delete mPointer;
        }
        mPointer = pPointer;
        if (mPointer != NULL)
            mPointer->mRefrences ++;
        return *this;
    }

    CSmartPointer& operator = (const CSmartPointer& pPointer)
                              {
        if (mPointer == pPointer.mPointer)
            return *this;
        if (mPointer != NULL)
        {
            mPointer->mRefrences --;
            if (mPointer->mRefrences == 0)
                delete mPointer;
        }
        mPointer = pPointer.mPointer;
        if (mPointer != NULL)
            mPointer->mRefrences ++;
        return *this;
    }
#endif
    inline T&   operator * ()const{return *mPointer;};
    inline      operator T*()const{return mPointer;};
    inline T*   operator ->()const{return mPointer;};
    inline T&   operator [](int i)const{return *(mPointer + i);};
    inline T*   operator + (int i)const{return mPointer + i;};
    inline bool operator !=(const CSmartPointer& Ptr)const{return mPointer != Ptr.mPointer;};
    inline bool operator ==(const CSmartPointer& Ptr)const{return mPointer == Ptr.mPointer;};
    inline bool operator <=(const CSmartPointer& Ptr)const{return mPointer <= Ptr.mPointer;};
    inline bool operator >=(const CSmartPointer& Ptr)const{return mPointer >= Ptr.mPointer;};
    inline bool operator < (const CSmartPointer& Ptr)const{return mPointer <  Ptr.mPointer;};
    inline bool operator > (const CSmartPointer& Ptr)const{return mPointer >  Ptr.mPointer;};
    inline bool operator !=(const T* Ptr)const{return mPointer != Ptr;};
    inline bool operator ==(const T* Ptr)const{return mPointer == Ptr;};
    inline bool operator <=(const T* Ptr)const{return mPointer <= Ptr;};
    inline bool operator >=(const T* Ptr)const{return mPointer >= Ptr;};
    inline bool operator < (const T* Ptr)const{return mPointer <  Ptr;};
    inline bool operator > (const T* Ptr)const{return mPointer >  Ptr;};

    inline void swap(CSmartPointer& Ptr)
    {
        if (mPointer && Ptr.mPointer)
        {
            mPointer ^= Ptr.mPointer;
            Ptr.mPointer ^= mPointer;
            mPointer ^= Ptr.mPointer;
        }
    }

    ~CSmartPointer()
    {
        this->operator =(NULL);
    }
};

#define CreateSmartPointer(X) typedef CSmartPointer<X> X##Ptr
