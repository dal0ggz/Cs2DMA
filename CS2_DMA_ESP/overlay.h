#pragma once

namespace overlay {
    // Crea la finestra + D3D11 + ImGui
    bool Create(int width, int height);

    // Loop principale (blocking — ritorna quando l'utente chiude)
    void Run();

    // Cleanup
    void Destroy();
}
