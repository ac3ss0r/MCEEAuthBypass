#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <io.h>
#include <psapi.h>
#include <wchar.h>
#include <string.h>
#include <TlHelp32.h>
#pragma comment(lib, "psapi.lib")

int * pointer_path;
int num_ptr;

// get the base address of the process main module
uintptr_t GetProcessBaseAddress(DWORD processId) {

  uintptr_t baseAddress = 0;
  HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);
  HMODULE * moduleArray;
  LPBYTE moduleArrayBytes;
  DWORD bytesRequired;

  if (processHandle) {
    if (EnumProcessModules(processHandle, NULL, 0, & bytesRequired)) {
      if (bytesRequired) {
        moduleArrayBytes = (LPBYTE) LocalAlloc(LPTR, bytesRequired);

        if (moduleArrayBytes) {
          int moduleCount;

          moduleCount = bytesRequired / sizeof(HMODULE);
          moduleArray = (HMODULE * ) moduleArrayBytes;

          if (EnumProcessModules(processHandle, moduleArray, bytesRequired, & bytesRequired)) {
            baseAddress = (uintptr_t)(moduleArray[0]);
          }

          LocalFree(moduleArrayBytes);
        }
      }
    }

    CloseHandle(processHandle);
  }

  return baseAddress;
}

// convert char arr to wchar
const wchar_t *ToWchar(const char *c) {
    const size_t cSize = strlen(c)+1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs (wc, c, cSize);
    return wc;
}

// get process ID from executable name
DWORD GetProcId(WCHAR * name) {
  PROCESSENTRY32 entry;
  entry.dwSize = sizeof(PROCESSENTRY32);

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (Process32First(snapshot, & entry) == TRUE) {
    while (Process32Next(snapshot, & entry) == TRUE) {
      if (wcscmp(ToWchar(entry.szExeFile), name) == 0) {
        return entry.th32ProcessID;
      }
    }
  }

  CloseHandle(snapshot);
  return 0;
}

// main (entry point)
int main(int argc, char * argv[]) {
  FILE * ptr_file;
  char MEE_POINTER_FILE[0x2048];
  int LOGIN_STEP_VALUE = -1;

  char * tmp;
  #ifdef _WIN64
  printf_s("x64 Version can ONLY be used for the 64 Bit Versions of the game.\n");
  #else
  printf_s("x86 Version can ONLY be used for the 32 Bit Versions of the game.\n");
  #endif

  strncpy_s(MEE_POINTER_FILE, 0x2048, "mee.ptr", 0x2048);
  if (argc > 1) {
    for (int i = 0; i < argc; i++) {

      if (strcmp(argv[i], "--help") == 0) {
        printf_s("--ptr <mee.ptr file>\n");
        printf_s("--lstep <custom login step value>\n");
        return 0;
      }

      if (strcmp(argv[i], "--ptr") == 0)
        strncpy_s(MEE_POINTER_FILE, 0x2048, argv[i + 1], 0x2048);

      if (strcmp(argv[i], "--lstep") == 0)
        LOGIN_STEP_VALUE = strtol(argv[i + 1], & tmp, 10);

    }

    printf_s("MEE.PTR FILE : %s\nLOGIN STEP VALUE: %i\n", MEE_POINTER_FILE, LOGIN_STEP_VALUE);
  }

  printf_s("Loading %s\n", MEE_POINTER_FILE);
  if ((_access(MEE_POINTER_FILE, 0)) != -1) // If the mee.ptr file exists AND the process has permission to read it..
  {
    // Open the pointer file
    fopen_s( & ptr_file, MEE_POINTER_FILE, "r");

    /*
    This next part just gets the size
    of mee.ptr file
    */

    // Seek to the end of the file
    fseek(ptr_file, 0, SEEK_END);
    // Get the current position
    int sz = ftell(ptr_file) + 1;
    // Seek to the start of the file
    fseek(ptr_file, 0, SEEK_SET);

    /*
    Allocate a buffer the size of mee.ptr
    then read the contents of the mee.ptr file
    into that buffer
    */

    // Allocate (sz) bytes of memory for the buffer.
    char * file_contents = (char * ) malloc(sz);
    // Set the newly allocated buffer to all 0x00
    memset(file_contents, 0x00, sz);
    // Read the contents of mee.ptr into the buffer.
    fread(file_contents, sz, 1, ptr_file);

    /*
    Create a copy of the pointer path file buffer
    This is so we can scan through and mess with the pointers to it
    Without messing up the our original copy
    */

    // Allocate (sz) bytes of memory for the buffer
    char * work_buf = (char * ) malloc(sz);
    // Copy contents of previously read file into the new buffer
    memcpy_s(work_buf, sz, file_contents, sz);

    /*
     *	Count the total number of elements
     *	From mee.ptr file.
     */

    // Set the total number of elements in the pointer path to 0
    num_ptr = 0;

    // Pointer to the next element, NULL for now.
    char * next_token1 = NULL;

    // Get a pointer to the first occurance of " > "
    char * token = strtok_s(work_buf, " > ", & next_token1);

    // Repeat this until there is no more " > " left
    while (token != NULL) {
      // Get the pointer to the next " > ".
      token = strtok_s(NULL, " > ", & next_token1);
      // Add 1 to the total number of elements
      num_ptr++;
    }

    // Free up memory used for counting the list of elements
    free(work_buf);

    /*
     *	Create a new 2nd copy of the pointer path file buffer
     */

    // Allocate buffer the size of the mee.ptr file
    work_buf = (char * ) malloc(sz);
    // Copy contents of mee.ptr file into new buffer
    memcpy_s(work_buf, sz, file_contents, sz);

    /*
     *	Allocate the memory required for the interger array
     *	Then convert each ASCII Hex Code to an int
     *	And store it in the pointer paths array.
     */

    // Allocate memory buffer the size of the required int array
    pointer_path = (int * ) malloc(num_ptr * sizeof(int));

    // Pointer to the next element, NULL for now.
    char * next_token2 = NULL;
    // Get the pointer to the first element (seperated by " > ")
    char * ptrs = strtok_s(work_buf, " > ", & next_token2);
    // Convert the ascii string to a int value, and put it into the int array
    pointer_path[0] = (int) strtol(ptrs, & tmp, 16);

    // Repeat until read all pointers
    for (int i = 1; i < num_ptr; i++) {
      // Read the next element (seperated by " > ")
      ptrs = strtok_s(NULL, " > ", & next_token2);
      // Convert the ascii string to a int value, and put it into the int array
      pointer_path[i] = (int) strtol(ptrs, & tmp, 16);
    }

    // Finally close the mee.ptr file
    fclose(ptr_file);

    // Free up memory used for reading the list of elements
    free(work_buf);
    // Free up memory used for the file contents
    free(file_contents);

    printf_s("Loaded %s!\n", MEE_POINTER_FILE);
  } else // If no mee.ptr file is found ... OR the process dont have permission to read it
  {
    // Set number of elements to a default number
    num_ptr = 8;
    // Allocate enough space for an int array of that size
    pointer_path = (int * ) malloc(num_ptr * sizeof(int));
    #ifdef _WIN64 // This code is only included if the project is built as win64 version
    printf_s("Failed, using default pointer path (MCEE 1.17.30 UWP x64)\n");
    // Use a default pointer path.
    pointer_path[0] = 0x3607D48;
    pointer_path[1] = 0x0;
    pointer_path[2] = 0x50;
    pointer_path[3] = 0xA8;
    pointer_path[4] = 0xE8;
    pointer_path[5] = 0x0;
    pointer_path[6] = 0x630;
    pointer_path[7] = 0x10;
    #else // if its NOT the win64 version..
    printf_s("Failed, using default pointer path (MCEE 1.17.30 Win32 x86)\n");
    // Use a default pointer path.
    pointer_path[0] = 0x2CBF0FC;
    pointer_path[1] = 0x0;
    pointer_path[2] = 0x7C;
    pointer_path[3] = 0x0;
    pointer_path[4] = 0x68;
    pointer_path[5] = 0xC;
    pointer_path[6] = 0x2E4;
    pointer_path[7] = 0x8;
    #endif
  }

  /*
   *	Print the currently in use
   *	pointer path to the output of the terminal
   */

  // Print "Pointer Path: "
  printf_s("\nPointer Path: ");
  for (int i = 0; i < num_ptr; i++) // Repeat for all elements in the path
  {
    // Print the hex encoding of the current element
    printf_s("%x", pointer_path[i]);
    if (i != num_ptr - 1) // if its not the last element
    {
      // Print " > "
      printf_s(" > ");
    }
  }
  // Print a newline
  printf_s("\n");

  /*
   *	This part is the part that actually patches
   *	Education edition's memory space
   */

  // Process ID of minecraft educattion ediion application, (currently 0)
  DWORD proc_id = 0;
  printf_s("\nPlease open Minecraft Education Edition\n");

  /*
   *	Repeatidly check if "Minecraft.Windows.exe" or "Minecraft.Win10.DX11.exe" is open
   */
  while (proc_id == 0) {
    proc_id = GetProcId(L"Minecraft.Windows.exe"); // Try to get the process ID for "Minecraft.Windows"
    if (proc_id == 0) // If the process ID is NULL (process not found)
      proc_id = GetProcId(L"Minecraft.Win10.DX11.exe"); // Try to get the process ID for "Minecrat.Win10.DX11.exe"
  }
  // Print the process ID
  printf_s("MCEE Process ID: %x\n", proc_id);
  // Try to open the process.
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, proc_id);
  // Print the process handle
  printf_s("MCEE Process Handle: 0x%p\n", hProcess);

  if (!hProcess) // If failed to open the process (eg no permission)
  {
    // Display a message box saying to try with admin rights
    MessageBox(NULL, "Cannot open process!\r\nTry \"Run as administrator\"", "Error!", MB_OK + MB_ICONERROR);
  } else {
    // Base Address of the minecraft education edition process (NULL for now)
    uintptr_t baseAddress = 0;
    while (baseAddress == 0) // Repeat until the base address is not NULL.
      // Try to get the base address of the Minecraft Education Edition Process
      baseAddress = GetProcessBaseAddress(proc_id);

    printf_s("MCEE Base Addr: 0x%p\n", (void * ) baseAddress);
    printf_s("Waiting for game to initalize....\n");

    read_ptr_path:

      // recalculate base address idk why but this seems to be required.
      baseAddress = GetProcessBaseAddress(proc_id);

    // Read first element
    uintptr_t first_ptr = pointer_path[0];
    uintptr_t cur_ptr = baseAddress + first_ptr;
    uintptr_t ptr = 0;
    uintptr_t new_ptr = 0;

    // Read first element from the games memory
    ReadProcessMemory(hProcess, (LPCVOID) cur_ptr, & ptr, sizeof(uintptr_t), 0);
    if (ptr == 0)
      goto read_ptr_path;

    /*
     *	Follow all elements
     *	in the path, until you reach
     *	the pointer to the login step value.
     */
    for (int i = 1; i < num_ptr - 1; i++) {
      cur_ptr = ptr + pointer_path[i];
      ReadProcessMemory(hProcess, (LPCVOID) cur_ptr, & new_ptr, sizeof(uintptr_t), 0);
      if (new_ptr == 0) {
        i -= 1;
        goto read_ptr_path;
      } else {
        ptr = new_ptr;

      }
    }

    // final addition
    ptr += pointer_path[num_ptr - 1];

    // Wait for Welcome Screen.
    int login_step_value = 0;
    ReadProcessMemory(hProcess, (LPCVOID) ptr, & login_step_value, sizeof(int), 0);

    if (login_step_value != 0x0) {

      printf_s("Final Ptr: 0x%p\n", (void * ) ptr);

      printf_s("Current Login Step: %i\n", login_step_value);
      if (LOGIN_STEP_VALUE != -1) {
        printf_s("Trying login stage %i", LOGIN_STEP_VALUE);
        WriteProcessMemory(hProcess, (LPVOID) ptr, & LOGIN_STEP_VALUE, sizeof(int), 0);
        goto finish;
      }

      printf_s("Trying login stage 5...\n"); // Backwards Comp (0.xx)
      int login_step_value = 5;
      WriteProcessMemory(hProcess, (LPVOID) ptr, & login_step_value, sizeof(int), 0);

      Sleep(1 * 200);

      printf_s("Trying login stage 6...\n"); // Backwards Comp (1.9 and lower)
      login_step_value = 6;
      WriteProcessMemory(hProcess, (LPVOID) ptr, & login_step_value, sizeof(int), 0);

      Sleep(1 * 200);

      printf_s("Trying login stage 8...\n");
      login_step_value = 8;
      WriteProcessMemory(hProcess, (LPVOID) ptr, & login_step_value, sizeof(int), 0);

    } else
      goto read_ptr_path;

    finish:

      CloseHandle(hProcess);

    printf_s("\nSuccess!\n");
    return 0;
  }
}