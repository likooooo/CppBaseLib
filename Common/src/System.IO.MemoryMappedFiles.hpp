#ifndef SYSTEM_IO_MEMORY_MAPPEDFILES_HPP
#define SYSTEM_IO_MEMORY_MAPPEDFILES_HPP

#include "complied_entry.h"
#ifdef WIN32
#define _CRT_SECURE_NO_DEPRECATE
#include <Windows.h>
#endif
#include <System.IO.FileStream.hpp>
#include <map>

/* https://docs.microsoft.com/zh-cn/dotnet/api/system.io.memorymappedfiles?view=net-5.0 */
namespace System::IO::MemoryMappedFiles
{
    typedef enum _MemoryMappedFileAccess 
    {
        ReadWrite = 0,  /* ���ļ��Ķ�д����Ȩ�ޡ� */
        Read,           /* ���ļ���ֻ������Ȩ�ޡ� */
        Write,          /* ���ļ��Ķ�д����Ȩ�ޣ������������������̲��ῴ���κ�д������� */
        ReadExecute,    /* �Կ��Դ洢�����п�ִ�д�����ļ��Ķ�ȡ����Ȩ�ޡ� */
        ReadWriteExecute/* �Կ��Դ洢�����п�ִ�д�����ļ��Ķ�д����Ȩ�ޡ� */
    }MemoryMappedFileAccess;
    std::map<MemoryMappedFileAccess, int> PageAccessMapping =
    {
        {ReadWrite,         PAGE_READWRITE},
        {Read,              PAGE_READONLY},
        {Write,             PAGE_WRITECOPY},
        {ReadExecute,       PAGE_EXECUTE_READ},
        {ReadWriteExecute,  PAGE_EXECUTE_READWRITE}
    };
    std::map<MemoryMappedFileAccess, int> DesiredAccessMapping =
    {
        {ReadWrite,         FILE_MAP_READ|FILE_MAP_WRITE},
        {Read,              FILE_MAP_READ},
        {Write,             FILE_MAP_WRITE|FILE_MAP_COPY},
        {ReadExecute,       FILE_MAP_READ|FILE_MAP_EXECUTE},
        {ReadWriteExecute,  FILE_MAP_READ|FILE_MAP_WRITE|FILE_MAP_EXECUTE}
    };
    typedef enum _MemoryMappedFileOptions
    {
        None = 0,
        DelayAllocatePages = 67108864 /* �ڴ������� */
    }MemoryMappedFileOptions;
    typedef struct _MemMappingInfo
    {
        HANDLE p;
        size_t size;
        MemoryMappedFileAccess access;
        MemoryMappedFileOptions opt;
    }MemMappingInfo;
    typedef std::map<const char*, MemMappingInfo> MemoryHandleMap;
    MemoryHandleMap memoryHandleMap;
    class MemoryMappedViewAccessor;
    class MemoryMappedFile;
}

class System::IO::MemoryMappedFiles::MemoryMappedViewAccessor: public System::IO::Stream
{
    friend class System::IO::MemoryMappedFiles::MemoryMappedFile;
private:
    void* Buffer;
    int   Offset;
    size_t   Length;
    MemoryMappedViewAccessor(void* buf, int offset, size_t len):Buffer(buf), Offset(offset), Length(len){}
public:
    ~MemoryMappedViewAccessor()
    {
        if (Buffer)
        {
            UnmapViewOfFile(Buffer);
            Buffer = NULL;
        }

    }
    void* GetBuffer(){return Buffer;}
    size_t GetSize(){return Length;}
    void Memset(int val)
    {
        memset(Buffer, val, Length);
    }
    void Memcpy(void* dest, size_t len)
    {
        memcpy(dest, Buffer, len);
    }
    void Fush(size_t size)
    {
        FlushViewOfFile(Buffer, size);
    }
};

class System::IO::MemoryMappedFiles::MemoryMappedFile/* https://docs.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-createfilemappinga */
{
private:
    const char* key;
    MemoryMappedFile(const char* _key) : key(_key){}
    static bool HandleExists(const char* key){return memoryHandleMap.find(key) != memoryHandleMap.end();}
public:
    ~MemoryMappedFile()
    {
        if(!HandleExists(key))
        {
            return;
        }
        if(NULL != memoryHandleMap[key].p)
        {
            CloseHandle(memoryHandleMap[key].p);
        }
        memoryHandleMap.erase(key);
    }
    static MemoryMappedFile* CreateFromFile (const char* path)/* �����ļ������ڴ棬�����ǰ���̻����������Ѿ�������������NULL */
    {
        size_t size = FileSize(path);
        MemoryMappedFileAccess access;
        if(0 == size)
        {
            return NULL;
        }
        if(HandleExists(path))
        {
            return NULL;
        }
        if(File::CanRead(path))
        {
            access = File::CanWrite(path) ? MemoryMappedFileAccess::ReadWrite : MemoryMappedFileAccess::Read;
        }
        else if(File::CanWrite(path))
        {
            access = MemoryMappedFileAccess::Write;
        }     
        HANDLE handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PageAccessMapping[access], 0, (DWORD)size, path);
        if(ERROR_ALREADY_EXISTS == GetLastError())
        {
			CloseHandle(handle);
            return NULL;
        }
        if(NULL == handle)
        {
            return NULL;
        }
        memoryHandleMap[path] = {handle, size, access, MemoryMappedFileOptions::None};
        return new MemoryMappedFile(path);
    }
    static MemoryMappedFile* CreateNew(const char* mapName, size_t size,/* ���������ڴ棬�����ǰ���̻����������Ѿ�������������NULL */
        MemoryMappedFileAccess access = MemoryMappedFileAccess::ReadWrite, 
        MemoryMappedFileOptions opt = MemoryMappedFileOptions::None)
    {
        if(0 == size)
        {
            return NULL;
        }
        if(HandleExists(mapName))
        {
            return NULL;
        }
        if(None != opt)
        {
            memoryHandleMap[mapName] = {NULL, size, access, opt};
            return new MemoryMappedFile(mapName);
        }
        HANDLE handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PageAccessMapping[access], 0, (DWORD)size, mapName);
        if(ERROR_ALREADY_EXISTS == GetLastError())
        {
			CloseHandle(handle);
            return NULL;
        }
        if(NULL == handle)
        {
            return NULL;
        }
        memoryHandleMap[mapName] = {handle, size, access, opt};
        return new MemoryMappedFile(mapName);
    }
    static MemoryMappedFile* CreateOrOpen(const char* mapName, size_t size, MemoryMappedFileAccess access = MemoryMappedFileAccess::ReadWrite)/* ���������ڴ棬�����ǰ���̻����������Ѿ������������Դ򿪹����ڴ� */
    {
        if(0 == size)
        {
            return NULL;
        }
        if(HandleExists(mapName))
        {
            return NULL != memoryHandleMap[mapName].p || None != memoryHandleMap[mapName].opt ? new MemoryMappedFile(mapName) : NULL;
        }
        else
        {
            HANDLE handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PageAccessMapping[access], 0, (DWORD)size, mapName);
            if(ERROR_ALREADY_EXISTS == GetLastError())
            {
			    CloseHandle(handle);
                handle = OpenFileMappingA(PageAccessMapping[access], false, mapName);  
            }
            if(NULL == handle)
            {
                return NULL;
            }
            memoryHandleMap[mapName] = 
            {
                handle, 
                size, 
                access, 
                None
            };
            return new MemoryMappedFile(mapName);
        }

    }
    static MemoryMappedFile* OpenExisting(const char* mapName, MemoryMappedFileAccess access = MemoryMappedFileAccess::ReadWrite)/* ���Ѿ��������Ĺ����ڴ棬��ʱMemMappingInfo::SizeĬ��Ϊ0 */
    {
        if(HandleExists(mapName))
        {
            return NULL != memoryHandleMap[mapName].p || None != memoryHandleMap[mapName].opt ? new MemoryMappedFile(mapName) : NULL;
        }
        else
        {
            HANDLE handle = OpenFileMapping(DesiredAccessMapping[access], false, mapName);  
            if(NULL == handle)
            {
                return NULL;
            }
            memoryHandleMap[mapName] = 
            {
                handle, 
                0, 
                ReadWrite, 
                None
            };  
            return new MemoryMappedFile(mapName);
        }

    }

    MemoryMappedViewAccessor* CreateViewAccessor()
    {   
        MemMappingInfo* info = &memoryHandleMap[key]; 
        if(None != info->opt)
        {
            // info->p = OpenFileMapping(DesiredAccessMapping[info->access], false, key);
            info->p = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PageAccessMapping[info->access], 0, (DWORD)info->size, key);
            if(ERROR_ALREADY_EXISTS == GetLastError())
            {
			    CloseHandle(info->p);
                return NULL;
            }
        }  
        if(NULL == info->p)
        {
            return NULL;
        }  
        char* scan0 = (char*)MapViewOfFile(info->p, DesiredAccessMapping[info->access], 0, 0, 0);
        printf("%d", GetLastError());
        return NULL == scan0 ? NULL : new MemoryMappedViewAccessor(scan0, 0, info->size);
    }
};

#endif