# MCEE Auth Bypass
<a href="https://github.com/acessors/MCEEAuthBypass/blob/main/logo.png" width=100%/>
#### Minecraft education edition auth bypass for Windows 7/8/10 by changing a memory address value.

# Usage

- Put a pointer address from PtrList.txt for your mcee version to the mee.ptr file
- Start the program
- Start Minecraft Education Edition

# Compiling

- Download mingw llvm-mingw from https://github.com/mstorsjo/llvm-mingw/releases
- Use x86_64-w64-mingw32-g++.exe for 64 bits and i686-w64-mingw32-g++.exe for 32 bits.
- g++ MCEEAuthBypass.cpp -static

# Todo
- Adding nickname changing functionality
