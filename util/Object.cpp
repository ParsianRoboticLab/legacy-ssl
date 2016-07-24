
#include "Object.h"
#include "SmartPointer.h"
#include <iostream>
#ifdef _DEBUG
#include <assert.h>
#include <list>
#include <string>
#include <cmath>
#include <stdio.h>
#endif

CObject::~CObject(void)
{
#ifdef _DEBUG
    if (mRefrences.size() != 0)
    {
        std::wstring s(L"Object has instances at :\n");
        std::string temp;
        for (std::list<CSmartPointer<CObject>* >::iterator i=mRefrences.begin(); i != mRefrences.end();i++)
        {
            if ((*i)->m_Name != "")
            {
                temp.resize((*i)->m_Name.size() + 1);
                sprintf((char*)temp.c_str(),"%s\n",(*i)->m_Name.c_str());
            }
            else
            {
                temp.resize((int) (log10f((int)*i) + 1));
                sprintf((char*)temp.c_str(),"%p\n",*i);
            }
            s.resize(s.size() + temp.size());
            std::copy(temp.begin(),temp.end(),s.begin() + s.size() - temp.size());
        }
        //_ASSERT_EXPR(false,s.c_str());
        assert(s.c_str());
    }
#pragma warning (pop)
#endif
}

