#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------
	
	constexpr std::size_t Voices = 8;
	constexpr std::size_t Oscillators = 3;
	constexpr std::size_t Envelopes = 2;
	constexpr std::size_t Lfos = 1;

	// ------------------------------------------------
	
	enum class ModSource { 
		Op1, Op2, Op3, 
		Envelope1, Envelope2, 
		LFO, 
		Amount, None 
	};

	enum class ModDestination {
		FilterFreq,
		Op1FM, Op2FM, Op3FM,
		Op1Amount, Op2Amount, Op3Amount,
		Op1Sync, Op2Sync, Op3Sync,
		Amount, None
	};

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
