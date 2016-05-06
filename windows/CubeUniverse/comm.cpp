#include "stdafx.h"
#include "time.h"
#include "comm.h"
//#include "tpd.h"

//extern int Counts;
void BeginWaitCursor();
void EndWaitCursor();
void RestoreWaitCursor();

void BeginWaitCursor()
	{ AfxGetApp()->DoWaitCursor(1); }
void EndWaitCursor()
	{ AfxGetApp()->DoWaitCursor(-1); }
void RestoreWaitCursor()
	{ AfxGetApp()->DoWaitCursor(0); }


CComm::CComm()
{	m_hCom=0;
	m_bOpen=FALSE;
	m_bEnDtr=FALSE;
}


CComm::~CComm()
{
	CloseCommPort();

/*
	CloseHandle(m_hPauseEvent);
	CloseHandle(m_hAppendResponseQueueEvent);
	CloseHandle(m_hAnalyzeResponse);
	CloseHandle(m_ThreadWait);
	CloseHandle(m_hWaitEvent);
	CloseHandle(m_hHangUpEvent);
	if(m_BoxTestThread != NULL){
		delete m_BoxTestThread;
		m_BoxTestThread	= NULL;
	}
*/

}
HANDLE CComm::GetHandle()
{
	return m_hCom;
}
BOOL CComm::IsOpen()
{
	if(m_hCom==NULL) return FALSE;
	return m_bOpen;
}

BOOL CComm::DoInitialize()
{
//	m_stTestCtrlInfo.ToDoInitialize(m_nComIndex);
	memset(&m_osWrite,0,sizeof(OVERLAPPED));
	if ((m_osWrite.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
		return FALSE;
	memset(&m_osRead,0,sizeof(OVERLAPPED));
	if ((m_osRead.hEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
		return FALSE;
//	if ((m_hWriteEvent = CreateEvent(NULL,TRUE,TRUE,NULL)) == NULL)
//		return FALSE;

//	if ((m_hWaitEvent	= CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)	
//		return FALSE;
/*
	if ((m_hPauseEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
		return FALSE;
	if ((m_hHangUpEvent = CreateEvent(NULL,TRUE,TRUE,NULL)) == NULL)
		return FALSE;
	if ((m_hWaitEvent = CreateEvent(NULL,TRUE,FALSE,NULL)) == NULL)
		return FALSE;
	if ((m_hAppendResponseQueueEvent = CreateEvent(NULL,TRUE,TRUE,NULL)) == NULL)
		return FALSE;
	if ((m_hAnalyzeResponse = CreateEvent(NULL,TRUE,TRUE,NULL)) == NULL)
		return FALSE;
*/
	return (TRUE);
}

/*
#define NOPARITY            0
#define ODDPARITY           1
#define EVENPARITY          2
#define MARKPARITY          3
#define SPACEPARITY         4
*/
BOOL CComm::OpenComm(int PortNo,DWORD m_BaudRate,BYTE m_ByteSize,BYTE m_Parity,BYTE m_StopBits)
{
	char szParity[]="noems";
	char coms[20],str[80];
	int i;
	i=m_Parity%5;
	wsprintf(str,"%d,%c,8,%d",m_BaudRate,szParity[i],m_StopBits);
	wsprintf(coms,"\\\\.\\COM%d",PortNo);
	return OpenComm(coms,str);
}
/*
//
// DTR Control Flow Values.
//
#define DTR_CONTROL_DISABLE    0x00
#define DTR_CONTROL_ENABLE     0x01
#define DTR_CONTROL_HANDSHAKE  0x02

//
// RTS Control Flow Values
//
#define RTS_CONTROL_DISABLE    0x00
#define RTS_CONTROL_ENABLE     0x01
#define RTS_CONTROL_HANDSHAKE  0x02
*/

BOOL CComm::OpenComm(LPCSTR lpszCom,LPCSTR lpszDCB)
{
	char coms[100];
	int comno=1;
	int len=strlen(lpszCom);
	char str[41];
	if(len>40)
	{	len=40; 
		str[len]='\0';
	}
	strcpy(str,lpszCom); strupr(str);
	char *p=strstr(str,"COM");
	if(p)
	{
		comno=atoi(p+3);
	}
	wsprintf(coms,"\\\\.\\COM%d",comno);

	if(IsOpen())
		CloseCommPort();
	DoInitialize();
	m_hCom=CreateFile(coms,GENERIC_READ|GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,NULL);//打开串口,并取得通信设备句柄
	if(m_hCom == INVALID_HANDLE_VALUE)
	{//	MessageBox(NULL,"Open Comm Error",NULL,MB_OK); 
//		m_bOpen=FALSE;
		m_hCom=NULL;
		CloseCommPort();		
		return FALSE;
	}
//	PrintMsgInfo("%s",lpszCom);
	SetupComm(m_hCom,MAXRECEBLOCK,MAXSENDBLOCK);
	PurgeComm(m_hCom,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);//清空通信缓冲区

	COMMTIMEOUTS  TimeOuts ;
	TimeOuts.ReadIntervalTimeout=MAXDWORD; 
	TimeOuts.ReadTotalTimeoutMultiplier=0; 
	TimeOuts.ReadTotalTimeoutConstant=0; 
	/* 设置写超时以指定WriteComm成员函数中的
	GetOverlappedResult函数的等待时间*/
	TimeOuts.WriteTotalTimeoutMultiplier=0; 
	TimeOuts.WriteTotalTimeoutConstant=0; //100;
	SetCommTimeouts(m_hCom, &TimeOuts);

	if(!BuildCommDCB(lpszDCB,&dcb))
	{	//MessageBox(NULL,"Build DCB Error",NULL,MB_OK); 
		CloseCommPort();
//		m_bOpen=FALSE;
//		CloseHandle(m_hCom);	m_hCom=NULL;
		return FALSE;
	}
	dcb.fParity=0;
	dcb.fBinary=1;
	dcb.fDtrControl=(m_bEnDtr?DTR_CONTROL_ENABLE:DTR_CONTROL_DISABLE);
	dcb.fRtsControl=RTS_CONTROL_DISABLE;
	if(!SetCommState(m_hCom, &dcb))
	{	//MessageBox(NULL,lpszDCB,NULL,MB_OK); 
		CloseCommPort();
		return FALSE;
	}
	m_nComPort=comno; 
	m_bOpen=TRUE;
	return TRUE;
}
BOOL CComm::FlushComm(BOOL bRece,BOOL bSend)
{
	DWORD flag=0;
	if(!m_hCom) return FALSE;
	if(bRece) flag=PURGE_RXABORT|PURGE_RXCLEAR;
	if(bSend) flag|=(PURGE_TXABORT|PURGE_TXCLEAR);
	return PurgeComm(m_hCom,flag);//清空通信缓冲区
}
void CComm::CloseCommPort()
{
	if(m_hCom) CloseHandle(m_hCom);// 关闭串口
	m_hCom=NULL;
	m_bOpen=FALSE;

	if(m_osWrite.hEvent) CloseHandle(m_osWrite.hEvent);
	if(m_osRead.hEvent) CloseHandle(m_osRead.hEvent);
//	if(m_hWriteEvent) CloseHandle(m_hWriteEvent);
	m_osWrite.hEvent=NULL;
	m_osRead.hEvent=NULL;
//	m_hWriteEvent=NULL;
//	if(m_hWaitEvent) CloseHandle(m_hWaitEvent);
//	m_hWaitEvent=NULL;
}

/*
typedef struct _DCB { 
  DWORD DCBlength;           // sizeof(DCB) 
  DWORD BaudRate;            // current baud rate 
  DWORD fBinary: 1;          // binary mode, no EOF check 
  DWORD fParity: 1;          // enable parity checking 
  DWORD fOutxCtsFlow:1;      // CTS output flow control 
  DWORD fOutxDsrFlow:1;      // DSR output flow control 
  DWORD fDtrControl:2;       // DTR flow control type 
  DWORD fDsrSensitivity:1;   // DSR sensitivity 
  DWORD fTXContinueOnXoff:1; // XOFF continues Tx 
  DWORD fOutX: 1;          // XON/XOFF out flow control 
  DWORD fInX: 1;           // XON/XOFF in flow control 
  DWORD fErrorChar: 1;     // enable error replacement 
  DWORD fNull: 1;          // enable null stripping 
  DWORD fRtsControl:2;     // RTS flow control 
  DWORD fAbortOnError:1;   // abort on error 
  DWORD fDummy2:17;        // reserved 
  WORD wReserved;          // not currently used 
  WORD XonLim;             // transmit XON threshold 
  WORD XoffLim;            // transmit XOFF threshold 
  BYTE ByteSize;           // number of bits/byte, 4-8 
  BYTE Parity;             // 0-4=no,odd,even,mark,space 
  BYTE StopBits;           // 0,1,2 = 1, 1.5, 2 
  char XonChar;            // Tx and Rx XON character 
  char XoffChar;           // Tx and Rx XOFF character 
  char ErrorChar;          // error replacement character 
  char EofChar;            // end of input character 
  char EvtChar;            // received event character 
  WORD wReserved1;         // reserved; do not use 
} DCB; 
*/
BYTE CComm::SetParityBit(BYTE bn)
{
	BYTE obn;
	if(!m_hCom) return 0xff;
	if(!GetCommState(m_hCom,&dcb)) return 0xff;
	obn=dcb.Parity;
	dcb.Parity=bn;
	SetCommState(m_hCom, &dcb);
	return obn;
}
BYTE CComm::GetParityBit()
{
	if(!m_hCom) return 0xff;
	if(!GetCommState(m_hCom,&dcb)) return 0xff;
	return dcb.Parity;
}

BOOL CComm::SetCommDCB(DWORD m_BaudRate,BYTE m_ByteSize,BYTE m_Parity,
					   BYTE m_StopBits)
{
	if(!m_hCom) return FALSE;
	if(!GetCommState(m_hCom,&dcb)) return FALSE;
/*
	CString s;
	s.Format("%d,%d,%d,%d,%d\n%d,%d,%d,%d,%d\n%d,%d,%d,%d\n %d,%d,%d,%d,%d\n %d,%d,%d,%d,%d\n",
		dcb.BaudRate,
		dcb.fBinary,//: 1;          // binary mode, no EOF check 
		dcb.fParity,//: 1;          // enable parity checking 
		dcb.fOutxCtsFlow,//:1;      // CTS output flow control 
		dcb.fOutxDsrFlow,//:1;      // DSR output flow control 
  dcb.fDtrControl,//:2;       // DTR flow control type 
  dcb.fDsrSensitivity,//:1;   // DSR sensitivity 
  dcb.fTXContinueOnXoff,//:1; // XOFF continues Tx 
  dcb.fOutX,//: 1;          // XON/XOFF out flow control 
  dcb.fInX,//: 1;           // XON/XOFF in flow control 
  dcb.fErrorChar,//: 1;     // enable error replacement 
  dcb.fNull,//: 1;          // enable null stripping 
  dcb.fRtsControl,//:2;     // RTS flow control 
  dcb.fAbortOnError,//:1;   // abort on error 
//  dcb.fDummy2:17;        // reserved 

  //dcb.wReserved;          // not currently used 
  dcb.XonLim,             // transmit XON threshold 
  dcb.XoffLim,            // transmit XOFF threshold 
  dcb.ByteSize,          // number of bits/byte, 4-8 
  dcb.Parity,             // 0-4=no,odd,even,mark,space 
  dcb.StopBits,           // 0,1,2 = 1, 1.5, 2 

  dcb.XonChar,            // Tx and Rx XON character 
  dcb.XoffChar,          // Tx and Rx XOFF character 
  dcb.ErrorChar,          // error replacement character 
  dcb.EofChar,            // end of input character 
  dcb.EvtChar            // received event character 
	);
	MessageBox(NULL,s,NULL,MB_OK);
*/
	dcb.BaudRate=m_BaudRate;
	dcb.ByteSize=m_ByteSize;
	dcb.Parity=m_Parity;
	dcb.StopBits=m_StopBits;

	dcb.fBinary=1; //TRUE;
	dcb.fParity=0; 
	

	// 硬件流控制设置
/*	int m_nFlowCtrl=0;
	dcb.fOutxCtsFlow=m_nFlowCtrl==1;
	dcb.fRtsControl=m_nFlowCtrl==1?RTS_CONTROL_HANDSHAKE:RTS_CONTROL_ENABLE;
	// XON/XOFF流控制设置
	dcb.fInX=dcb.fOutX=m_nFlowCtrl==2;
	dcb.XonChar=XON;
	dcb.XoffChar=XOFF;
	dcb.XonLim=50;
	dcb.XoffLim=50;
*/
	dcb.fDtrControl=0; //DTR_CONTROL_ENABLE; //1;
	dcb.fRtsControl=0; //RTS_CONTROL_ENABLE; //1;
	return SetCommState(m_hCom, &dcb);
}

int CComm::GetReadLen()
{
	DWORD dwErrorFlag;
	ClearCommError(m_hCom,&dwErrorFlag,&ComState);//获取通信设备当前状态
	return ComState.cbInQue;
}

DWORD CComm::Read(LPBYTE ReadBuffer,DWORD dwMaxNumber,long dms)
{
	if(!m_hCom) return 0;
	DWORD dwBytesRead,dwErrorFlag;

	dwBytesRead=dwMaxNumber;
	if(dwBytesRead<=0) return 0;
	long sl,ll;
	if(dms>0)
	{
		sl=clock();
		for(;;)
		{
			ClearCommError(m_hCom,&dwErrorFlag,&ComState);//获取通信设备当前状态
			if(ComState.cbInQue>=dwMaxNumber) break;
//			WaitForSingleObject(m_hWaitEvent,50);	//30
			ll=clock();
			if(ll-sl>=dms) break;
		}
	}

	ClearCommError(m_hCom,&dwErrorFlag,&ComState);//获取通信设备当前状态
	if(!ReadFile(m_hCom,ReadBuffer,dwBytesRead,&dwBytesRead,&m_osRead))
	{
		if(ERROR_IO_PENDING==GetLastError()){
			if(WaitForSingleObject(m_osRead.hEvent,1000))
				return 0;
		}
		
	}
	return dwBytesRead;

}


/*
DWORD CComm::Read(char *ReadBuffer,DWORD dwMaxNumber)//dwWriteLength)
{
	if(!m_hCom) return 0;
	DWORD dwBytesRead,dwErrorFlag;

	int OutTime=1;
	long s1,s2;
	 s1=clock();
	 for(;;)
	 {
		ClearCommError(m_hCom,&dwErrorFlag,&ComState);//获取通信设备当前状态
		if(ComState.cbInQue>=dwMaxNumber)
		{	dwBytesRead=dwMaxNumber; break; }
		s2=clock()-s1;
		if(s2>1000*(long)OutTime) //CLK_TCK
		{	dwBytesRead=ComState.cbInQue; break;} //return FALSE;}
	 }

//	ClearCommError(m_hCom,&dwErrorFlag,&ComState);//获取通信设备当前状态
	if(ComState.cbInQue>dwMaxNumber)
		dwBytesRead=dwMaxNumber;
//	else
//		dwBytesRead=ComState.cbInQue;
	if(dwBytesRead>0)
	{
		if(!ReadFile(m_hCom,ReadBuffer,dwBytesRead,&dwBytesRead,NULL))
			return 0;//读dwBytesRead 个字节到ReadBuffer中
	}
	return dwBytesRead;

}

*/
/*
DWORD CTerminalDoc::ReadComm(char *buf,DWORD dwLength)
{
	DWORD length=0;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	ClearCommError(m_hCom,&dwErrorFlags,&ComStat);
	length=min(dwLength, ComStat.cbInQue);
	ReadFile(m_hCom,buf,length,&length,&m_osRead);
	return length;
}
*/



BOOL CComm::Write(LPBYTE buf, DWORD dwLength, DWORD dwWriteTimeLim)
{
	if(!m_hCom) return FALSE;
	BOOL fState;
	DWORD length = dwLength;
	COMSTAT ComStat;
	DWORD dwErrorFlags;
	ClearCommError(m_hCom,&dwErrorFlags,&ComStat);
	fState = WriteFile(m_hCom,buf,length,&length,&(m_osWrite));
	if (!fState)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			WaitForSingleObject(m_osWrite.hEvent,dwWriteTimeLim);
			if (!GetOverlappedResult(m_hCom,&m_osWrite,&length,FALSE))
				length = 0;
		}
		else
			length = 0;
	}
	if (length == 0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}
void CComm::SetDtr(BOOL bSet)
{
	if(IsOpen())
		::EscapeCommFunction(m_hCom,bSet?CLRDTR:SETDTR);
//	::EscapeCommFunction(m_hCom,bSet?SETDTR:CLRDTR);
}



