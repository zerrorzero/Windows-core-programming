#include <windows.h>
#include <stdio.h>
#include <time.h>

#define SLEEP_TIME 5000
#define INPUTFILE "C:\\input.txt"
#define OUTPUTFILE "C:\\output.txt"
#define CLEANFILE "del C:\\input.txt"


SERVICE_STATUS          ServiceStatus; 
SERVICE_STATUS_HANDLE   hStatus; 


void  ServiceMain(int argc, char** argv); 
void  ControlHandler(DWORD request); 
int InitService();

int WriteToLog(char* str)
{
	FILE* log;
	log = fopen(OUTPUTFILE, "a+");
	if (log == NULL){
		printf("Output file open failed.");
		return -1;
	}
	char cTimeString[30] = ("");
	time_t currentTime = time(NULL);
	strncat(cTimeString, ctime(&currentTime), 30);
	cTimeString[strlen(cTimeString) - 1] = '\0';
	fprintf(log, "[%s.] ", cTimeString);
	fprintf(log, "%s\n", str);
	fclose(log);
	return 0;
}



int txtcmd(char* cmd, char* result) {
	char buffer[4096];
	FILE* pipe = _popen(cmd, "r");  //open the pipe and run command
	if (!pipe)
	{
		WriteToLog("execute fail!!");
		return -1;
	}

	while (!feof(pipe)) {
		if (fgets(buffer, 4096, pipe))
		{
			strcat_s(result, 4096, buffer);
		}

	}
	_pclose(pipe);
	return 0;
}




// Service initialization
int InitService() 
{ 
	OutputDebugString("Monitoring started.");
	int result;
	result = WriteToLog("Monitoring started.");
	return(result); 
}

// Control Handler
void ControlHandler(DWORD request) 
{ 
   switch(request) 
   { 
      case SERVICE_CONTROL_STOP: 
		 OutputDebugString("Monitoring stopped.");
         WriteToLog("Monitoring stopped.");

         ServiceStatus.dwWin32ExitCode = 0; 
         ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
         SetServiceStatus (hStatus, &ServiceStatus);
         return; 
 
      case SERVICE_CONTROL_SHUTDOWN: 
		 OutputDebugString("Monitoring stopped.");
         WriteToLog("Monitoring stopped.");

         ServiceStatus.dwWin32ExitCode = 0; 
         ServiceStatus.dwCurrentState = SERVICE_STOPPED; 
         SetServiceStatus (hStatus, &ServiceStatus);
         return; 
        
      default:
         break;
    } 
 
    // Report current status
    SetServiceStatus (hStatus, &ServiceStatus);
 
    return; 
}

void ServiceMain(int argc, char** argv) 
{ 
   int error; 
 
   ServiceStatus.dwServiceType = 
      SERVICE_WIN32; 
   ServiceStatus.dwCurrentState = 
      SERVICE_START_PENDING; 
   ServiceStatus.dwControlsAccepted   =  
      SERVICE_ACCEPT_STOP | 
      SERVICE_ACCEPT_SHUTDOWN;
   ServiceStatus.dwWin32ExitCode = 0; 
   ServiceStatus.dwServiceSpecificExitCode = 0; 
   ServiceStatus.dwCheckPoint = 0; 
   ServiceStatus.dwWaitHint = 0; 
 
   hStatus = RegisterServiceCtrlHandler(
      "DoSomething", 
      (LPHANDLER_FUNCTION)ControlHandler); 
   if (hStatus == (SERVICE_STATUS_HANDLE)0) 
   { 
      // Registering Control Handler failed
	   WriteToLog("Registering Control Handler failed");
       return; 
   }  

   // Initialize Service 
   error = InitService(); 
   if (error) 
   {
      // Initialization failed
      ServiceStatus.dwCurrentState = 
         SERVICE_STOPPED; 
      ServiceStatus.dwWin32ExitCode = -1; 
      SetServiceStatus(hStatus, &ServiceStatus); 
	  WriteToLog("Initialization failed");
      return; 
   } 
   // We report the running status to SCM. 
   ServiceStatus.dwCurrentState = 
      SERVICE_RUNNING; 
   SetServiceStatus (hStatus, &ServiceStatus);
 
   // The worker loop of a service
   while (ServiceStatus.dwCurrentState == 
          SERVICE_RUNNING)
   {

	  WriteToLog("begin!!");
	  FILE *fp;
	  fp = fopen(INPUTFILE, "rb");

	  if (fp == NULL)
	  {
		  WriteToLog("there are no such file!!!");
		  ServiceStatus.dwCurrentState =
			  SERVICE_RUNNING;
		  SetServiceStatus(hStatus, &ServiceStatus);
	  }
	  else
	  {
		  int i = 0;
		  fseek(fp, 0, SEEK_END);
		  int len = ftell(fp);
		  fseek(fp, 0, SEEK_SET);
		  //printf("len:%d\n", len);
		  char *buf = (char *)malloc(len*sizeof(char));
		  char *islen = (char *)&len;
		  //WriteToLog(islen);
		  if (fread(buf, 1, len, fp) != len)
		  {
			  WriteToLog("READ ERROR!!!");
			  break;
		  }

		  char *line = (char *)malloc(1024 * sizeof(char));
		  memset(line, 0, sizeof(line));
		  char *result = (char *)malloc(4096 * sizeof(char));
		  memset(result, 0, sizeof(result));
		  fclose(fp);

		  while (i < len+1)
		  {
			  char *P = (char *)malloc(2 * sizeof(char));
			  P[1] = '\0';
			  strncpy(P, buf + i, 1);

			  if (*P != 0x0a && *P != 0x0d && i!=len)
			  {
				  strcat_s(line, 1024, P);
			  }
			  else
			  {
				  WriteToLog("line:");
				  WriteToLog(line);
				  OutputDebugString(line);
				  line[strlen(line)] = '\0';
				  txtcmd(line, result);
				  WriteToLog(result);
				  memset(line, 0, sizeof(line));
				  memset(result, 0, sizeof(result));
			  }
			  i++;

			  free(P);
		  }
		  WriteToLog("DONE");
		  free(line);
		  free(buf);

		  txtcmd(CLEANFILE, result);
		  WriteToLog("clean");
		  int check = WriteToLog(result);

		  if (check)
		  {
			  WriteToLog("stop");
			  ServiceStatus.dwCurrentState =
				  SERVICE_STOPPED;
			  ServiceStatus.dwWin32ExitCode = -1;
			  SetServiceStatus(hStatus,
				  &ServiceStatus);
			  WriteToLog("clean fail!!!");
			  return;
		  }

		  free(result);
	  }

	  Sleep(SLEEP_TIME);
   }
   return;
}

void main(int argc, char* argv[])
{ 
   SERVICE_TABLE_ENTRY ServiceTable[2];
   ServiceTable[0].lpServiceName = "DoSomething";
   ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;

   ServiceTable[1].lpServiceName = NULL;
   ServiceTable[1].lpServiceProc = NULL;
   // Start the control dispatcher thread for our service
   StartServiceCtrlDispatcher(ServiceTable);
}

