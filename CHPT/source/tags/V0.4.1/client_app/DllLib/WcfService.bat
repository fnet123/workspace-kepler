::��װwindwos����-����
%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\installutil.exe  HXCDataServiceWinService.exe

::��װwindwos����-�ļ�
%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\installutil.exe  HXCFileServiceWinService.exe

::��װwindwos����-���ļ�
%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\installutil.exe  HXCFileTransferServiceWinService.exe

::��װwindwos����-session
%SystemRoot%\Microsoft.NET\Framework\v4.0.30319\installutil.exe  HXCSessionServiceWinService.exe
pause

net start HXCDataService
net start HXCFileService
net start HXCFileTransferService
net start HXCSessionService
pause 