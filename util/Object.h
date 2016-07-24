#pragma once
#ifdef _DEBUG
#include <list>
#endif

template <class T> class CSmartPointer;

class CObject
{
#ifdef _DEBUG
    std::list<CSmartPointer<CObject>*> mRefrences;
#else
    int mRefrences;
#endif
    template <class T> friend class CSmartPointer;
public:
    CObject(void)
    {
#ifndef _DEBUG
        mRefrences = 0;
#endif
    };
    virtual ~CObject();
};
