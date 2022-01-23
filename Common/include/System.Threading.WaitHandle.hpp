/*
 * @Description: Ŀǰ��֧�� windows ƽ̨
 * @Version: 1.0
 * @Autor: like
 * @Date: 2022-01-17 15:23:58
 * @LastEditors: like
 * @LastEditTime: 2022-01-21 14:44:18
 */
#ifndef SYSTEM_THREADING_WAITHANDLE_HPP
#define SYSTEM_THREADING_WAITHANDLE_HPP

#include <CompliedEntry.h>
#ifndef COMPLIER_MSVC
#pragma error(fatal, -1, "Windows Plantform Support Only : System.Threading.WaitHandle.hpp")
#endif
#include <System.IDisposable.h>
#include <Windows.h>
#include <stdio.h>

namespace System::Threading
{
    class WaitHandle;           /* https://docs.microsoft.com/zh-cn/dotnet/api/system.threading.waithandle.waittimeout?view=net-5.0 */
}

/**
 * @brief ��װ�ȴ��Թ�����Դ���ж�ռ���ʵĲ���ϵͳ�ض��Ķ���
 * # Refrence
 * - WaitOne            : WaitForSingleObject       - https://docs.microsoft.com/zh-cn/windows/win32/api/synchapi/nf-synchapi-waitforsingleobject
 * - WaitAll / WaitAny  : WaitForMultipleObjects    - https://docs.microsoft.com/zh-cn/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects
 *
 */
class System::Threading::WaitHandle : public IDisposable
{
protected:
    HANDLE  m_hWaitHandle;
    bool    m_bDisposed;
    WaitHandle() : m_bDisposed(false){}
    WaitHandle(HANDLE h) : m_hWaitHandle(h), m_bDisposed(NULL == m_hWaitHandle){}
    /**
     * @brief �ṩ������һ�������� WaitHandle �ͷŹ���
     * 
     * @param disposing ������������Ƿ���øýӿڵ��ͷ�ʵ��
     */
    virtual void Dispose(bool disposing)
    {
        if(disposing && !m_bDisposed)
        {
            if(!CloseHandle(m_hWaitHandle))
            {
                printf("System::Threading::WaitHandle Dispose Failed , Error Code : %d\n", GetLastError());
                return;
            }
            m_hWaitHandle = NULL;
            m_bDisposed = true;
        }
    }
public:
    const int WaitTimeout = 258;
    /**
     * @brief ��ȡ�����ñ�������ϵͳ���
     * 
     * @return HANDLE& 
     */
    inline HANDLE& Handle()
    {
        return m_hWaitHandle;
    }
    /**
     * @brief �ͷ� WaitHandle ��ĵ�ǰʵ����ʹ�õ�������Դ
     * 
     */
    virtual void Close()
    {
        Dispose(true);
    }
    /**
     * @brief �ͷ� WaitHandle ��ĵ�ǰʵ����ʹ�õ�������Դ
     * 
     */
    void Dispose() override
    {
        Dispose(true);
    } 
    /**
     * @brief �����ǰʵ���յ��źţ���Ϊ true�� �����ǰʵ�����������źţ��� WaitOne() ��������
     * 
     * @return true 
     * @return false 
     */
    virtual bool WaitOne()
    {
        return WaitOne(INFINITE);
    }
    /**
     * @brief �����ǰʵ���յ��źţ���Ϊ true������Ϊ false��
     * 
     * @param millisecond �ȴ��ĺ�����, -1 ��ʾ�����ڵȴ�
     * @return true 
     * @return false 
     */
    virtual bool WaitOne(DWORD millisecond)
    {
        int status;
        if(WAIT_OBJECT_0 == (status = WaitForSingleObject(m_hWaitHandle, millisecond)))
        {
            return true;
        }
        else if(WAIT_TIMEOUT == status)
        {
            printf("WaitAll WaitForSingleObject Timeout\n");
            return false;
        }
        printf("WaitOne WaitForSingleObject Failed , Error Code : %d\n", GetLastError());
        return false;
    }
    /**
     * @brief ��ֹ��ǰ�̣߳�ֱ����ǰ�� WaitHandle �յ��źŻ򵽴�ָ��ʱ����Ϊֹ����ָ���Ƿ��ڵȴ�֮ǰ�˳�ͬ����
     * 
     * @param millisecond �ȴ��ĺ�����, -1 ��ʾ�����ڵȴ� 
     * @param exitContext 
     * @return true 
     * @return false 
     */
    virtual bool WaitOne(DWORD millisecond, bool exitContext/* �˳������Ĳ�֪�����ʵ�� */)
    {  
        if(exitContext)
        {
            /* TODO : �˳�������ͬ���� */
        }
        WaitOne(millisecond);
        return true;
    }
    /**
     * @brief �ȴ�ָ�������е�����Ԫ�ض��յ��ź�
     * 
     * @param waitHandles 
     * @param count 
     * @return true 
     */
    static bool WaitAll(WaitHandle* waitHandles, DWORD count)
    {
        return WaitAll(waitHandles, count, INFINITE);
    }
    /**
     * @brief �ȴ�ָ�������е�����Ԫ�ؽ����źŻ򵽴�ָ��ʱ����Ϊֹ
     * 
     * @param waitHandles 
     * @param count 
     * @param millisecond 
     * @return true 
     * @return false 1. ��ʱ
     */
    static bool WaitAll(WaitHandle* waitHandles, DWORD count, DWORD millisecond)
    {
        if(count > WAIT_ABANDONED_0)
        {
            throw "WaitAll  Support Max Handle Count : 128";
        }
        HANDLE* handles = (HANDLE*)malloc(sizeof(HANDLE) * count);
        for(DWORD i = 0; i < count; i++)
        {
            handles[i] = waitHandles[i].m_hWaitHandle;
        }
        DWORD status =  WaitForMultipleObjects(count, handles, true, millisecond);
        free(handles);
        if(status < WAIT_ABANDONED_0)
        {
            return true;
        }
        else if(WAIT_TIMEOUT == status)
        {
            printf("WaitAll WaitForMultipleObjects Timeout\n");
            return false;
        }
        else if(WAIT_FAILED == status)
        {
            printf("WaitAll WaitForMultipleObjects Failed, Error Code : %d\n", GetLastError());
            return false;
        }
        else if (WAIT_ABANDONED_0 >= status && status <= (status - WAIT_ABANDONED_0))/* ��Ȼû��ͨ���ȴ�, �������߳��Ѿ����ͷ� */
        {
            waitHandles[status - WAIT_ABANDONED_0].Dispose(); 
            return true;
        }
    }
    /**
     * @brief �ȴ�ָ�������е�����Ԫ�ؽ����źŻ򵽴�ָ��ʱ����Ϊֹ����ָ���Ƿ��ڵȴ�֮ǰ�˳�ͬ����
     * 
     * @param waitHandles 
     * @param count 
     * @param millisecond 
     * @param exitContext 
     * @return true 
     * @return false 
     */
    static bool WaitAll(WaitHandle* waitHandles, DWORD count, DWORD millisecond, bool exitContext)
    {
        if(exitContext)
        {
            /* TODO : �˳�������ͬ���� */
        }
        return WaitAll(waitHandles, count, millisecond);
    }
    /**
     * @brief �ȴ�ָ�������е���һԪ���յ��ź�
     * 
     * @param waitHandles 
     * @param count 
     * @return int ��������
     */
    static int WaitAny(WaitHandle* waitHandles, DWORD count)
    {
        return WaitAny(waitHandles, count, INFINITE);
    }
    /**
     * @brief �ȴ�ָ�������е�����Ԫ�ؽ����źŻ򵽴�ָ��ʱ����
     * 
     * @param waitHandles 
     * @param count 
     * @param millisecond 
     * @return int 1. �ɹ��ȴ�������������; 2. ��ʱ���� millisecond; 3. �ȴ�ʧ�ܷ��� -1
     */
    static int WaitAny(WaitHandle* waitHandles, DWORD count, DWORD millisecond)
    {
        if(count > WAIT_ABANDONED_0)
        {
            throw "WaitAny Support Max Handle Count : 128";
        }
        HANDLE* handles = (HANDLE*)malloc(sizeof(HANDLE) * count);
        for(DWORD i = 0; i < count; i++)
        {
            handles[i] = waitHandles[i].m_hWaitHandle;
        }
        DWORD status = WaitForMultipleObjects(count, handles, false, millisecond);
        free(handles);
        if(status < WAIT_ABANDONED_0)
        {
            return status;
        }
        else if(WAIT_TIMEOUT == status)
        {
            printf("WaitAny WaitForMultipleObjects Timeout\n");
            return millisecond;
        }
        else if(WAIT_FAILED == status)
        {
            printf("WaitAny WaitForMultipleObjects Failed, Error Code : %d\n", GetLastError());
            return -1;
        }
        else if (WAIT_ABANDONED_0 >= status && status <= (status - WAIT_ABANDONED_0))/* ��Ȼû��ͨ���ȴ�, �������߳��Ѿ����ͷ� */
        {
            waitHandles[status - WAIT_ABANDONED_0].Dispose(true);
            return status - WAIT_ABANDONED_0;
        }
    }
    /**
     * @brief �ȴ�ָ�������е�����Ԫ�ؽ����źŻ򵽴�ָ��ʱ��������ָ���Ƿ��ڵȴ�֮ǰ�˳�ͬ����
     * 
     * @param waitHandles 
     * @param count 
     * @param millisecond 
     * @param exitContext 
     * @return int 
     */
    static int WaitAny(WaitHandle* waitHandles, DWORD count, DWORD millisecond, bool exitContext)
    {
        if(exitContext)
        {
            /* TODO : �˳�������ͬ���� */
        }
        return WaitAny(waitHandles, count, millisecond);
    }
    /**
     * @brief ����źź͵ȴ����ɹ���ɣ���Ϊ true������ȴ�û����ɣ���˷���һֱ����������
     * 
     * @return true 
     */
    static bool SignalAndWait(WaitHandle toSignal, WaitHandle toWaitOn)
    {
        if(!SetEvent(toSignal.m_hWaitHandle))
        {
            printf("SignalAndWait(WaitHandle toSignal, WaitHandle toWaitOn) SetEvent Failed , Error Code : %d\n", GetLastError());
            return false;
        }
        return toWaitOn.WaitOne();
    }
    /**
     * @brief ��һ�� WaitHandle �����źŲ��ȴ���һ����ָ����ʱ���Ϊ 32 λ�з�����������ָ���ڽ���ȴ�ǰ�Ƿ��˳������ĵ�ͬ����
     * 
     * @param toSignal 
     * @param toWaitOn 
     * @param millisecond 
     * @param exitContext 
     * @return true 
     * @return false 1. ��ʱ
     */
    static bool SignalAndWait(WaitHandle toSignal, WaitHandle toWaitOn, DWORD millisecond, bool exitContext)
    {
        if(!SetEvent(toSignal.m_hWaitHandle))
        {
            printf("SignalAndWait(WaitHandle toSignal, WaitHandle toWaitOn) SetEvent Failed , Error Code : %d\n", GetLastError());
            return false;
        }
        return toWaitOn.WaitOne(millisecond, exitContext);
    }
};

#endif