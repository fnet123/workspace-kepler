::��װwindwos����-����
sc create HXCDataServiceWinService binpath= "%cd%\HXCDataServiceWinService.exe" 
pause
::��װwindwos����-�ļ�
sc create HXCFileServiceWinService binpath= "%cd%\HXCFileServiceWinService.exe" 
pause
::��װwindwos����-���ļ�
sc create HXCFileTransferServiceWinService binpath= "%cd%\HXCFileTransferServiceWinService.exe" 
pause
::��װwindwos����-session
sc create HXCSessionServiceWinService binpath= "%cd%\HXCSessionServiceWinService.exe" 
pause

net start HXCDataServiceWinService
net start HXCFileServiceWinService
net start HXCFileTransferServiceWinService
net start HXCSessionServiceWinService
pause 