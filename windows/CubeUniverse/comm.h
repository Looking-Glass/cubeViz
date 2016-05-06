#if !defined(AFX_COMM_H__7742400D_6F8D_11D5_8D6F_00105A83C2FF__INCLUDED_)
#define AFX_COMM_H__7742400D_6F8D_11D5_8D6F_00105A83C2FF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif 


#define MAXRECEBLOCK	512		//4096
#define MAXSENDBLOCK	512		//4096

class CComm
{
//protected:
private:
	COMSTAT		ComState;   //通信设备状态
	BOOL		m_bOpen;
//	CWinThread *m_pThread;		//接收处理的线程
	DCB			dcb;         //通信设备的DCB结构
public:
	HANDLE		m_hCom;       //通信设备句柄
	OVERLAPPED	m_osWrite;
	OVERLAPPED	m_osRead;
	volatile HANDLE m_hWaitEvent;	//用于等待,防止堵塞
//	volatile HANDLE m_hWriteEvent;	//串行通信中用于线程间同步的事件句柄
	int			m_nComPort;			//串口号(>=1)
	BOOL		m_bEnDtr;
public:
	CComm();
	virtual ~CComm();

private:
	BOOL DoInitialize();	//初始化(m_osWrite,m_osRead,m_hWriteEvent...)

public:


	HANDLE GetHandle();//取得通信设备句柄
	//是否已打开
	BOOL IsOpen();
	//创建及初始化串行口
	BOOL OpenComm(int m_PortNo=1,DWORD m_BaudRate=9600,BYTE m_ByteSize=8,
		BYTE m_Parity=ODDPARITY,BYTE m_StopBits=ONESTOPBIT);
	BOOL OpenComm(LPCSTR lpszCom,LPCSTR lpszDCB="COM1:9600,o,8,1");
	void CloseCommPort();  //关闭串行口
	//设置串行通信设备的DCB结构
	BOOL SetCommDCB(DWORD m_BaudRate,BYTE m_ByteSize,BYTE m_Parity,BYTE m_StopBits);
	BOOL FlushComm(BOOL bRece,BOOL bSend);	//清除接收发送缓存
//	BOOL CreateCommThread();				//通讯线程
	//设置校验位,bn:0-4=no,odd,even,mark,space,3和4用于多机通讯
	//返回以前的值,0xff则表示失败
	BYTE SetParityBit(BYTE bn);	
	//返回当前校验位值,0-4=no,odd,even,mark,space
	//0xff则表示失败
	BYTE GetParityBit();

	//发送WriteBuffer中的dwWriteLength个字节数据
	BOOL Write(LPBYTE WriteBuffer,DWORD dwWriteLength,DWORD dwWriteTimeLim=100);
	//接收dwMaxNumber个字节数据放入ReadBuffer中
	DWORD Read(LPBYTE ReadBuffer,DWORD dwMaxNumber,long dms=0);

	int GetReadLen();
	//
	void SetDtr(BOOL bSet=TRUE);
};

#endif
