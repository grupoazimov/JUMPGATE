#pragma once

#include <comdef.h>
#include <oleauto.h>
#include <string>

template <typename T>
class CComPtr
{
public:
    CComPtr() = default;
    CComPtr(const CComPtr&) = delete;
    CComPtr& operator=(const CComPtr&) = delete;

    CComPtr(CComPtr&& other) noexcept
        : ptr(other.ptr)
    {
        other.ptr = nullptr;
    }

    CComPtr& operator=(CComPtr&& other) noexcept
    {
        if (this != &other)
        {
            Release();
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }

    ~CComPtr()
    {
        Release();
    }

    T* operator->() const
    {
        return ptr;
    }

    T** operator&()
    {
        Release();
        return &ptr;
    }

    operator T* () const
    {
        return ptr;
    }

    HRESULT CoCreateInstance(REFCLSID clsid, LPUNKNOWN outer = nullptr, DWORD context = CLSCTX_INPROC_SERVER)
    {
        Release();
        return ::CoCreateInstance(clsid, outer, context, __uuidof(T), reinterpret_cast<void**>(&ptr));
    }

    template <typename U>
    HRESULT QueryInterface(U** out) const
    {
        if (!ptr)
        {
            return E_POINTER;
        }
        return ptr->QueryInterface(__uuidof(U), reinterpret_cast<void**>(out));
    }

    void Release()
    {
        if (ptr)
        {
            ptr->Release();
            ptr = nullptr;
        }
    }

private:
    T* ptr = nullptr;
};

class CComBSTR
{
public:
    CComBSTR() = default;

    explicit CComBSTR(const char* value)
    {
        if (!value)
        {
            return;
        }

        const int needed = MultiByteToWideChar(CP_UTF8, 0, value, -1, nullptr, 0);
        if (needed <= 0)
        {
            return;
        }

        std::wstring wide(static_cast<size_t>(needed), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, value, -1, wide.data(), needed);
        bstr = SysAllocString(wide.c_str());
    }

    explicit CComBSTR(const wchar_t* value)
    {
        if (value)
        {
            bstr = SysAllocString(value);
        }
    }

    ~CComBSTR()
    {
        SysFreeString(bstr);
    }

    BSTR* operator&()
    {
        SysFreeString(bstr);
        bstr = nullptr;
        return &bstr;
    }

    operator BSTR() const
    {
        return bstr;
    }

    bool operator!() const
    {
        return bstr == nullptr;
    }

    bool operator==(const CComBSTR& other) const
    {
        if (!bstr || !other.bstr)
        {
            return bstr == other.bstr;
        }
        return wcscmp(bstr, other.bstr) == 0;
    }

    unsigned int ByteLength() const
    {
        return SysStringByteLen(bstr);
    }

private:
    BSTR bstr = nullptr;
};

class CComVariant : public VARIANT
{
public:
    CComVariant()
    {
        VariantInit(this);
    }

    ~CComVariant()
    {
        Clear();
    }

    HRESULT ChangeType(VARTYPE vt)
    {
        return VariantChangeType(this, this, 0, vt);
    }

    void Clear()
    {
        VariantClear(this);
    }
};
