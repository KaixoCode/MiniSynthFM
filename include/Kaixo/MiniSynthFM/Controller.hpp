#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------
	
	constexpr std::size_t Voices = 8;
	constexpr std::size_t Oscillators = 3;
	constexpr std::size_t Envelopes = 2;
	constexpr std::size_t Lfos = 2;

	// ------------------------------------------------

	class MiniSynthFMController : public Controller {
	public:

		// ------------------------------------------------

		MiniSynthFMController();

		// ------------------------------------------------

	};
    
	// ------------------------------------------------

}
    
// ------------------------------------------------
