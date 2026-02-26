#pragma once

// ============================================================================
// Offset files generati da cs2-dumper (https://github.com/a2x/cs2-dumper)
// Devi copiarli nella cartella sdk/generated/ OPPURE cambiare i path qui sotto.
//
// Per generarli: esegui cs2-dumper sul PC di gioco con CS2 aperto,
// poi copia i 3 file .hpp sul secondo PC (quello con la DMA card).
// IMPORTANTE: ri-dumpa dopo ogni aggiornamento di CS2!
// ============================================================================

#include "generated/offsets.hpp"
#include "generated/client_dll.hpp"
#include "generated/buttons.hpp"

namespace offsets = cs2_dumper::offsets::client_dll;
namespace schemas = cs2_dumper::schemas::client_dll;
namespace buttons = cs2_dumper::buttons;
