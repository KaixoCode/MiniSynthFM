#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------
	
	constexpr std::string_view ThemePath = "themepath";
	constexpr std::string_view PresetPath = "presetpath";

	// ------------------------------------------------
	
	constexpr std::size_t Voices = 8;
	constexpr std::size_t Oscillators = 3;
	constexpr std::size_t ADSREnvelopes = 2;
	constexpr std::size_t ADEnvelopes = 1;
	constexpr std::size_t Envelopes = 3;
	constexpr std::size_t Lfos = 1;

	// ------------------------------------------------
	
	enum class Quality {
		Low, Normal, High, Ultra, Extreme, Amount
	};

	// ------------------------------------------------
	
	enum class ModSource { 
		Note, Velocity, Random, PitchBend, ModWheel,
		Op1, Op2, Op3, 
		Envelope1, Envelope2, Envelope3,
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

	constexpr std::string_view toString(ModSource source) {
		switch (source) {
		case ModSource::Op1: return "op1";
		case ModSource::Op2: return "op2";
		case ModSource::Op3: return "op3";
		case ModSource::Envelope1: return "env1";
		case ModSource::Envelope2: return "env2";
		case ModSource::Envelope3: return "env3";
		case ModSource::LFO: return "lfo";
		}
	}
	
	constexpr std::string_view toString(ModDestination source) {
		switch (source) {
		case ModDestination::FilterFreq: return "filter-freq";
		case ModDestination::Op1FM: return "op1-fm";
		case ModDestination::Op2FM: return "op2-fm";
		case ModDestination::Op3FM: return "op3-fm";
		case ModDestination::Op1Amount: return "op1-amount";
		case ModDestination::Op2Amount: return "op2-amount";
		case ModDestination::Op3Amount: return "op3-amount";
		case ModDestination::Op1Sync: return "op1-sync";
		case ModDestination::Op2Sync: return "op2-sync";
		case ModDestination::Op3Sync: return "op3-sync";
		}
	}

	constexpr ModSource sourceFromString(std::string_view source) {
		if (source == "op1") return ModSource::Op1;
		if (source == "op2") return ModSource::Op2;
		if (source == "op3") return ModSource::Op3;
		if (source == "env1") return ModSource::Envelope1;
		if (source == "env2") return ModSource::Envelope2;
		if (source == "env3") return ModSource::Envelope3;
		if (source == "lfo") return ModSource::LFO;
	}
	
	constexpr ModDestination destFromString(std::string_view dest) {
		if (dest == "filter-freq") return ModDestination::FilterFreq;
		if (dest == "op1-fm") return ModDestination::Op1FM;
		if (dest == "op2-fm") return ModDestination::Op2FM;
		if (dest == "op3-fm") return ModDestination::Op3FM;
		if (dest == "op1-amount") return ModDestination::Op1Amount;
		if (dest == "op2-amount") return ModDestination::Op2Amount;
		if (dest == "op3-amount") return ModDestination::Op3Amount;
		if (dest == "op1-sync") return ModDestination::Op1Sync;
		if (dest == "op2-sync") return ModDestination::Op2Sync;
		if (dest == "op3-sync") return ModDestination::Op3Sync;
	}

	// ------------------------------------------------
	
	class PresetData : public Serializable {
	public:
	
		// ------------------------------------------------

		std::string name = "";
		std::string author = "";
		std::string type = "";
		std::string description = "";

		// ------------------------------------------------

		void init() override;
		basic_json serialize() override;
		void deserialize(basic_json& json) override;

		// ------------------------------------------------

	};

	// ------------------------------------------------
	
	class ControllerData : public Serializable {
	public:

		// ------------------------------------------------

		struct Connection {

			// ------------------------------------------------

			ModSource source;
			ModDestination destination;

			// ------------------------------------------------

			int color = 0;

			// ------------------------------------------------

		};

		// ------------------------------------------------
		
		std::vector<Connection> connections{};

		// ------------------------------------------------
		
		ControllerData() { init(); }

		// ------------------------------------------------
		
		void init() override;
		basic_json serialize() override;
		void deserialize(basic_json& data) override;

		// ------------------------------------------------

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
