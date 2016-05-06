// Registry.h

class CRegistry
{ 

public:
	CRegistry();
	CRegistry( HKEY, LPCWSTR   );
	~CRegistry();

	BOOL Open( HKEY, LPCWSTR  );
	BOOL Close( void );
	BOOL IsOpen( void );

	int EnumSubKey(OUT CStringArray & strArraySubKey);

	BOOL ReadDWORD( LPCWSTR , DWORD *, DWORD *pdwLastError = NULL );
	BOOL ReadString( LPCWSTR , LPVOID, int, DWORD *pdwLastError = NULL );
	char **ReadMultipleStrings( LPCWSTR , DWORD *pdwLastError = NULL );
	static void DeleteList( char ** );

	BOOL WriteDWORD( LPCWSTR , DWORD, DWORD *pdwLastError = NULL );
	BOOL WriteString( LPCWSTR , LPVOID, DWORD *pdwLastError = NULL );
	BOOL Write( LPCWSTR , LPVOID, DWORD, int );

	static BOOL CreateKey( HKEY, LPCWSTR , LPCWSTR  );
	static BOOL DeleteKey( HKEY, LPCWSTR  );

protected:
	HKEY m_hKey;
	DWORD m_dwLastError;
	int m_nSize;

	BOOL Read( LPCWSTR , LPVOID, int );

};
