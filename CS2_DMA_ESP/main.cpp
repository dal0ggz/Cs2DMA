#include <iostream>
#include <Windows.h>
#include <DMALibrary/Memory/Memory.h>
#include "overlay.h"
#include "globals.h"
#include "config.h"
#include "features/makcu.h"

int main()
{
    SetConsoleTitleA("CS2 DMA ESP - Captain DMA 35T");

    std::cout << "[*] CS2 DMA ESP - Inizializzazione..." << std::endl;

    // ------------------------------------------------------------------
    // 0. Carica configurazione salvata
    // ------------------------------------------------------------------
    config::Load();
    std::cout << "[+] Config caricata" << std::endl;

    // ------------------------------------------------------------------
    // 1. Inizializza DMA (connessione FPGA via USB)
    // ------------------------------------------------------------------
    if (!mem.Init("cs2.exe", true, true)) {
        std::cout << "[-] Impossibile inizializzare DMA. Controlla:" << std::endl;
        std::cout << "    - La Captain DMA 35T e' collegata via USB" << std::endl;
        std::cout << "    - FTD3XX.dll, vmm.dll, leechcore.dll sono nella cartella" << std::endl;
        std::cout << "    - CS2 e' in esecuzione sul PC di gioco" << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[+] DMA inizializzato" << std::endl;

    // ------------------------------------------------------------------
    // 2. Fix CR3 per bypass EAC (Easy Anti-Cheat)
    // ------------------------------------------------------------------
    std::cout << "[*] Tentativo fix CR3..." << std::endl;
    if (mem.FixCr3())
        std::cout << "[+] CR3 fixato con successo" << std::endl;
    else
        std::cout << "[!] CR3 fix fallito (potrebbe funzionare comunque)" << std::endl;

    // ------------------------------------------------------------------
    // 3. Trova client.dll nel processo cs2.exe
    // ------------------------------------------------------------------
    g::clientBase = mem.GetBaseDaddy("client.dll");
    if (!g::clientBase) {
        std::cout << "[-] Impossibile trovare client.dll" << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[+] client.dll base: 0x" << std::hex << g::clientBase << std::dec << std::endl;

    // ------------------------------------------------------------------
    // 3.5. Inizializza MAKCU (auto-detect COM port)
    // ------------------------------------------------------------------
    std::cout << "[*] Ricerca MAKCU (scansione COM1-COM20)..." << std::endl;
    if (makcu::Init(nullptr, 115200))
        std::cout << "[+] MAKCU connesso su " << makcu::GetConnectedPort() << std::endl;
    else
        std::cout << "[!] MAKCU non trovato (aimbot disabilitato)" << std::endl;

    // ------------------------------------------------------------------
    // 4. Crea overlay e avvia loop principale
    // ------------------------------------------------------------------
    std::cout << "[*] Creazione overlay " << g::screenWidth << "x" << g::screenHeight << "..." << std::endl;

    if (!overlay::Create(g::screenWidth, g::screenHeight)) {
        std::cout << "[-] Impossibile creare overlay D3D11" << std::endl;
        system("pause");
        return 1;
    }

    std::cout << "[+] Overlay attivo. INSERT = menu, END = esci" << std::endl;

    // Blocking — ritorna quando l'utente preme END o chiude la finestra
    overlay::Run();

    // ------------------------------------------------------------------
    // 5. Cleanup
    // ------------------------------------------------------------------
    overlay::Destroy();
    makcu::Shutdown();
    std::cout << "[*] Chiuso." << std::endl;
    return 0;
}
