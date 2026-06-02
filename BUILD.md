# Build

This project builds as a 32-bit Visual Studio C++ dynamic library and emits a
`.asi` plugin.

## Requirements

- Visual Studio 2022 or Visual Studio Build Tools 2022
- MSVC v143 build tools
- Windows SDK
- Local third-party dependencies under `ThirdParty/`

`ThirdParty/` is intentionally ignored by Git. Keep local SDKs and binary
libraries out of the repository unless their licenses explicitly allow them to
be redistributed.

## ThirdParty Layout

Prepare this directory at the repository root:

```text
ThirdParty/
  include/
    bass.h
    MinHook.h
    nlohmann/
      json.hpp
  lib/
    bass.lib
    libMinHook.x86.lib
  bin/                 # optional runtime DLLs for local testing/packaging
    bass.dll
    MinHook.x86.dll
  DX9SDK/
    Include/
      d3dx9.h
    Lib/
      x86/
        d3dx9.lib
```

Only the headers and import libraries are needed to compile and link. Runtime
DLLs are useful for local testing or packaging, but they are not required for a
successful link.

## Build Commands

From a Visual Studio Developer PowerShell or a shell where `MSBuild.exe` is on
`PATH`, build through the solution with:

```powershell
MSBuild.exe .\CarbonRhythm.slnx /m /t:Rebuild /p:Configuration=Release /p:Platform=x86
```

Or build the project file directly with:

```powershell
MSBuild.exe .\CarbonRhythm\CarbonRhythm.vcxproj /m /t:Rebuild /p:Configuration=Release /p:Platform=Win32
```

The solution uses `x86`; the project uses `Win32`. These refer to the same
32-bit target.

## Output

Solution builds write:

```text
Release/CarbonRhythm.asi
```

Direct project builds write:

```text
CarbonRhythm/Release/CarbonRhythm.asi
```

## Troubleshooting

If a header cannot be found, check `ThirdParty/include` and
`ThirdParty/DX9SDK/Include`.

Common missing-header errors:

- `bass.h`: missing BASS header
- `MinHook.h`: missing MinHook header
- `json.hpp`: missing nlohmann/json headers
- `d3dx9.h`: missing DirectX 9 SDK headers

If the linker cannot open a `.lib`, check `ThirdParty/lib` and
`ThirdParty/DX9SDK/Lib/x86`.

Common missing-library errors:

- `bass.lib`: missing BASS import library
- `libMinHook.x86.lib`: missing MinHook x86 library
- `d3dx9.lib`: missing DirectX 9 SDK x86 library

If `Release|Win32` is rejected when building `CarbonRhythm.slnx`, use
`Release|x86` for the solution. `Win32` is the platform name used by the
`.vcxproj`.

If MSBuild reports that `v145` build tools are missing, make sure you are
building `Release|x86` through the solution or `Release|Win32` through the
project. Other configurations may still be configured for older toolsets.
