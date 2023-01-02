#include "common_pty.h"
#include <Windows.h>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <process.h>
#include <span>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <winerror.h>

// Forward declarations
HRESULT InitializeStartupInfoAttachedToPseudoConsole(STARTUPINFOEXA *, HPCON);

namespace common_pty {

struct CommonPtyImpl {
  HPCON hpc_ = INVALID_HANDLE_VALUE;
  HANDLE hPipeIn_ = INVALID_HANDLE_VALUE;
  HANDLE hPipeOut_ = INVALID_HANDLE_VALUE;

  STARTUPINFOEXA startupInfo_{};
  PROCESS_INFORMATION piClient_{};

  std::vector<char> buffer_;
  std::mutex mtx_;
  std::vector<char> tmp_;

  void Shutdown() {
    // Now safe to clean-up client app's process-info & thread
    CloseHandle(piClient_.hThread);
    CloseHandle(piClient_.hProcess);

    // Cleanup attribute list
    DeleteProcThreadAttributeList(startupInfo_.lpAttributeList);
    free(startupInfo_.lpAttributeList);

    // Close ConPTY - this will terminate client process if running
    ClosePseudoConsole(hpc_);

    // Clean-up the pipes
    if (INVALID_HANDLE_VALUE != hPipeOut_)
      CloseHandle(hPipeOut_);
    if (INVALID_HANDLE_VALUE != hPipeIn_)
      CloseHandle(hPipeIn_);
  }

  HRESULT CreatePseudoConsoleAndPipes(int rows, int cols) {
    HRESULT hr{E_UNEXPECTED};
    HANDLE hPipePTYIn{INVALID_HANDLE_VALUE};
    HANDLE hPipePTYOut{INVALID_HANDLE_VALUE};

    // Create the pipes to which the ConPTY will connect
    if (CreatePipe(&hPipePTYIn, &hPipeOut_, NULL, 0) &&
        CreatePipe(&hPipeIn_, &hPipePTYOut, NULL, 0)) {
      // Determine required size of Pseudo Console
      COORD consoleSize{};
      // CONSOLE_SCREEN_BUFFER_INFO csbi{};
      // HANDLE hConsole{GetStdHandle(STD_OUTPUT_HANDLE)};
      // if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
      consoleSize.X = cols;
      consoleSize.Y = rows;
      // }

      // Create the Pseudo Console of the required size, attached to the PTY-end
      // of the pipes
      hr = CreatePseudoConsole(consoleSize, hPipePTYIn, hPipePTYOut, 0, &hpc_);

      // Note: We can close the handles to the PTY-end of the pipes here
      // because the handles are dup'ed into the ConHost and will be released
      // when the ConPTY is destroyed.
      if (INVALID_HANDLE_VALUE != hPipePTYOut)
        CloseHandle(hPipePTYOut);
      if (INVALID_HANDLE_VALUE != hPipePTYIn)
        CloseHandle(hPipePTYIn);
    }

    return hr;
  }

  // Initializes the specified startup info struct with the required properties
  // and updates its thread attribute list with the specified ConPTY handle
  HRESULT
  InitializeStartupInfoAttachedToPseudoConsole() {

    auto pStartupInfo = &startupInfo_;
    pStartupInfo->StartupInfo.cb = sizeof(STARTUPINFOEX);

    // Get the size of the thread attribute list.
    size_t attrListSize{};
    InitializeProcThreadAttributeList(NULL, 1, 0, &attrListSize);

    // Allocate a thread attribute list of the correct size
    pStartupInfo->lpAttributeList =
        reinterpret_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(malloc(attrListSize));

    // Initialize thread attribute list
    HRESULT hr{E_UNEXPECTED};
    if (pStartupInfo->lpAttributeList &&
        InitializeProcThreadAttributeList(pStartupInfo->lpAttributeList, 1, 0,
                                          &attrListSize)) {
      // Set Pseudo Console attribute
      hr = UpdateProcThreadAttribute(pStartupInfo->lpAttributeList, 0,
                                     PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE, hpc_,
                                     sizeof(HPCON), NULL, NULL)
               ? S_OK
               : HRESULT_FROM_WIN32(GetLastError());
    } else {
      hr = HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;
  }

  bool Launch(const char *exec) {
    // Initialize the necessary startup info struct
    if (FAILED(InitializeStartupInfoAttachedToPseudoConsole())) {
      return false;
    }

    // Launch ping to emit some text back via the pipe
    auto hr =
        CreateProcessA(NULL,         // No module name - use Command Line
                       (char *)exec, // Command Line
                       NULL,         // Process handle not inheritable
                       NULL,         // Thread handle not inheritable
                       FALSE,        // Inherit handles
                       EXTENDED_STARTUPINFO_PRESENT, // Creation flags
                       NULL, // Use parent's environment block
                       NULL, // Use parent's starting directory
                       &startupInfo_.StartupInfo, // Pointer to STARTUPINFO
                       &piClient_) // Pointer to PROCESS_INFORMATION
            ? S_OK
            : GetLastError();
    return SUCCEEDED(hr);
  }

  void Enqueue(const char *buf, DWORD len) {
    if (len == 0) {
      return;
    }

    std::lock_guard<std::mutex> lock(mtx_);
    auto size = buffer_.size();
    buffer_.resize(size + len);
    memcpy(buffer_.data() + size, buf, len);

    // std::cout << "Enqueue>>" << std::string_view(buf, len) << std::endl;
  }

  auto Dequeue(std::vector<char> &buffer) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::swap(buffer, buffer_);
    buffer_.clear();
  }

  bool IsClosed() {
    auto result = WaitForSingleObject(piClient_.hThread, 0);
    return result == WAIT_OBJECT_0;
  }

  void Kill() { TerminateProcess(piClient_.hProcess, 9); }

  void Write(const char *buf, size_t size) {
    DWORD write_size;
    WriteFile(hPipeOut_, buf, size, &write_size, NULL);
  }

  void NotifyTermSize(unsigned short rows, unsigned short cols) {
    // Retrieve width and height dimensions of display in
    // characters using theoretical height/width functions
    // that can retrieve the properties from the display
    // attached to the event.
    COORD size;
    size.X = cols;
    size.Y = rows;

    // Call pseudoconsole API to inform buffer dimension update
    ResizePseudoConsole(hpc_, size);
  }

  std::span<char> Read() {
    Dequeue(tmp_);
    return {tmp_.begin(), tmp_.end()};
  }
};

static void __cdecl PipeListener(LPVOID p) {
  auto impl = (CommonPtyImpl *)p;
  HANDLE hPipe{impl->hPipeIn_};
  HANDLE hConsole{GetStdHandle(STD_OUTPUT_HANDLE)};

  const DWORD BUFF_SIZE{512};
  char szBuffer[BUFF_SIZE]{};

  DWORD dwBytesWritten{};
  DWORD dwBytesRead{};
  BOOL fRead{FALSE};
  do {
    // Read from the pipe
    fRead = ReadFile(hPipe, szBuffer, BUFF_SIZE, &dwBytesRead, NULL);

    // Write received text to the Console
    // Note: Write to the Console using WriteFile(hConsole...), not
    // printf()/puts() to prevent partially-read VT sequences from corrupting
    // output
    // WriteFile(hConsole, szBuffer, dwBytesRead, &dwBytesWritten, NULL);
    impl->Enqueue(szBuffer, dwBytesRead);

  } while (fRead && dwBytesRead >= 0);

  std::cout << "PipeListener finished." << std::endl;
}

Pty::Pty() : impl_(new CommonPtyImpl) {}
Pty::~Pty() {
  if (impl_) {
    delete impl_;
    impl_ = nullptr;
  }
}

void Pty::Launch(int rows, int cols, const char *prog, const char *TERM) {

  //  Create the Pseudo Console and pipes to it
  auto hr = impl_->CreatePseudoConsoleAndPipes(rows, cols);
  if (FAILED(hr)) {
    return;
  }

  // Create & start thread to listen to the incoming pipe
  // Note: Using CRT-safe _beginthread() rather than CreateThread()
  HANDLE hPipeListenerThread{
      reinterpret_cast<HANDLE>(_beginthread(PipeListener, 0, impl_))};

  if (!impl_->Launch(prog)) {
    return;
  }
}

bool Pty::IsClosed() { return impl_->IsClosed(); }
void Pty::Kill() { impl_->Kill(); }
void Pty::Write(const char *buf, size_t size) { impl_->Write(buf, size); }
void Pty::NotifyTermSize(unsigned short rows, unsigned short cols) {
  impl_->NotifyTermSize(rows, cols);
}
std::span<char> Pty::Read() { return impl_->Read(); }

} // namespace common_pty