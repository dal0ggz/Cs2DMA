# CS2 DMA ESP — Build Instructions

## Prerequisiti sul secondo PC (quello con la DMA card)

### Hardware
- **Captain DMA 35T** collegata via USB al secondo PC
- La FPGA deve essere installata in uno slot PCIe del PC di gioco

### Software
- Visual Studio 2022/2024 con workload "Desktop development with C++"
- Windows SDK (incluso con VS)

### File necessari nella cartella del progetto (root)
Scarica da [MemProcFS releases](https://github.com/ufrisk/MemProcFS/releases):
- `vmm.dll`
- `leechcore.dll`
- `FTD3XX.dll`
- `dbghelp.dll` (per CR3 fix)
- `symsrv.dll` (per CR3 fix)
- `info.db` (per CR3 fix)

### ImGui
Scarica ImGui da https://github.com/ocornut/imgui e copia questi file
nella cartella `imgui/` del progetto:
- `imgui.h`, `imgui.cpp`
- `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`
- `imgui_internal.h`, `imconfig.h`, `imstb_rectpack.h`, `imstb_textedit.h`, `imstb_truetype.h`
- `backends/imgui_impl_dx11.h`, `backends/imgui_impl_dx11.cpp`
- `backends/imgui_impl_win32.h`, `backends/imgui_impl_win32.cpp`

### Offsets CS2
Esegui [cs2-dumper](https://github.com/a2x/cs2-dumper) sul PC di gioco,
poi copia i 3 file generati in `sdk/generated/`:
- `offsets.hpp`
- `client_dll.hpp`
- `buttons.hpp`

**IMPORTANTE:** Devi ri-dumpare dopo ogni aggiornamento di CS2!

---

## Setup progetto Visual Studio

### 1. Crea nuovo progetto
- File → New → Project → "Empty Project" (C++)
- Nome: `CS2_DMA_ESP`
- Location: questa cartella

### 2. Aggiungi file sorgente al progetto
- `main.cpp`
- `overlay.cpp`
- `features/esp.cpp`
- Tutti i file `.cpp` di ImGui dalla cartella `imgui/`
- Tutti i file `.cpp` della DMALibrary:
  - `DMALibrary-Master/DMALibrary/Memory/Memory.cpp`
  - `DMALibrary-Master/DMALibrary/Memory/InputManager.cpp`
  - `DMALibrary-Master/DMALibrary/Memory/Registry.cpp`
  - `DMALibrary-Master/DMALibrary/Memory/Shellcode.cpp`
  - `DMALibrary-Master/DMALibrary/pch.cpp`
  - `DMALibrary-Master/DMALibrary/DMALibrary.cpp`

### 3. Configurazione (Release x64)
Apri Project Properties:

**C/C++ → General → Additional Include Directories:**
```
$(SolutionDir)
$(SolutionDir)imgui
$(SolutionDir)imgui/backends
$(SolutionDir)..\DMALibrary-Master
```

**C/C++ → Precompiled Headers:**
- Imposta "Not Using Precompiled Headers"
  (oppure configura pch.h per la DMALibrary)

**C/C++ → Language:**
- C++ Language Standard: `ISO C++17 (/std:c++17)`

**Linker → Input → Additional Dependencies:**
```
d3d11.lib
dxgi.lib
vmm.lib
leechcore.lib
```

**Linker → General → Additional Library Directories:**
```
$(SolutionDir)..\DMALibrary-Master\DMALibrary\libs
```

**Linker → System → SubSystem:**
- Console (per vedere i log di inizializzazione)

### 4. Build
- Configuration: **Release**, Platform: **x64**
- Build → Build Solution (`Ctrl+B`)

### 5. Esecuzione
Copia nella cartella output (`x64/Release/`):
- `vmm.dll`, `leechcore.dll`, `FTD3XX.dll`
- `dbghelp.dll`, `symsrv.dll`, `info.db` (per CR3 fix)

Esegui `CS2_DMA_ESP.exe` dal secondo PC con CS2 in esecuzione sul PC di gioco.

---

## Controlli

| Tasto   | Azione            |
|---------|-------------------|
| INSERT  | Toggle menu       |
| END     | Chiudi programma  |

---

## Risoluzione schermo

Modifica `g::screenWidth` e `g::screenHeight` in `globals.h`
in modo che corrispondano alla risoluzione di CS2 sul PC di gioco.
La finestra overlay deve avere la stessa dimensione per far funzionare
correttamente WorldToScreen.
