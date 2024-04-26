#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Controller.hpp"

// ------------------------------------------------

#include "Kaixo/MiniSynthFM/PresetData.hpp"
#include "Kaixo/MiniSynthFM/PresetDatabase.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	struct simd_path {
		using enum instruction_set;
		constexpr static instruction_sets Path0{}; // No SIMD
		constexpr static instruction_sets Path1 = Path0 | SSE | SSE2;
		constexpr static instruction_sets Path2 = Path1 | SSE3 | SSE4_1 | FMA;
		constexpr static instruction_sets Path3 = Path2 | AVX | AVX2;

		enum path { P0, P1, P2, P3 };

		KAIXO_INLINE static path choosePath() {
			instruction_sets c = find_supported_instruction_sets();
			if (c & Path3) return P3;
			if (c & Path2) return P2;
			if (c & Path1) return P1;
			return P0;
		}

		KAIXO_INLINE static auto execute(auto lambda) {
			switch (path) {
			case P0: return lambda.operator()<float>();
			case P1: return lambda.operator()<basic_simd<float, 128, Path1>>();
			case P2: return lambda.operator()<basic_simd<float, 128, Path2>>();
			case P3: return lambda.operator()<basic_simd<float, 256, Path3>>();
			default: return lambda.operator()<float>();
			}
		}

		static inline path path = choosePath();
	};

	// ------------------------------------------------
	
	constexpr std::string_view ThemePath = "themepath";
	constexpr std::string_view PresetPath = "presetpath";
	constexpr std::string_view ShowPiano = "showpiano";
	constexpr std::string_view WindowScale = "windowscale";
	constexpr std::string_view CablePhysics = "cablephysics";

	// ------------------------------------------------
	
	constexpr std::size_t Voices = 8;
	constexpr std::size_t Oscillators = 3;
	constexpr std::size_t ADSREnvelopes = 2;
	constexpr std::size_t ADEnvelopes = 1;
	constexpr std::size_t Envelopes = 3;
	constexpr std::size_t Lfos = 1;
	constexpr std::size_t MaxOversample = 16;

	// ------------------------------------------------
	
	enum class FilterAlgorithm {
		DirtyAnalog, CleanAnalog, Digital, Amount
	};

	// ------------------------------------------------
	
	enum class Quality {
		Low, Normal, High, Ultra, Extreme, Amount
	};

	// ------------------------------------------------

	constexpr std::size_t oversampleForQuality(Quality quality) {
		switch (quality) {
		case Quality::Low: return 1;
		case Quality::Normal: return 2;
		case Quality::High: return 4;
		case Quality::Ultra: return 8;
		case Quality::Extreme: return 16;
		default: return 1;
		}
	}

	// ------------------------------------------------
	
	enum class PhaseMode {
		Contiguous, Reset, Random, Amount
	};

	// ------------------------------------------------
	
	enum class ModSource { 
		Velocity, Random, ModWheel,
		Op1, Op2, Op3, 
		Envelope1, Envelope2, Envelope3,
		LFO, 
		Amount, None 
	};

	enum class ModDestination {
		FilterFreq, LfoDepth,
		Op1FM, Op2FM, Op3FM,
		Op1Amount, Op2Amount, Op3Amount,
		Op1Sync, Op2Sync, Op3Sync,
		Amount, None
	};

	constexpr std::string_view toString(ModSource source) {
		switch (source) {
		case ModSource::Velocity: return "velocity";
		case ModSource::Random: return "random";
		case ModSource::ModWheel: return "mod-wheel";
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
		case ModDestination::LfoDepth: return "lfo-depth";
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
		if (source == "mod-wheel") return ModSource::ModWheel;
		if (source == "random") return ModSource::Random;
		if (source == "velocity") return ModSource::Velocity;
		if (source == "op1") return ModSource::Op1;
		if (source == "op1") return ModSource::Op1;
		if (source == "op2") return ModSource::Op2;
		if (source == "op3") return ModSource::Op3;
		if (source == "env1") return ModSource::Envelope1;
		if (source == "env2") return ModSource::Envelope2;
		if (source == "env3") return ModSource::Envelope3;
		if (source == "lfo") return ModSource::LFO;
	}
	
	constexpr ModDestination destFromString(std::string_view dest) {
		if (dest == "lfo-depth") return ModDestination::LfoDepth;
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
		
		PresetDatabase presetDatabase{ *this };

		// ------------------------------------------------

	};
    
	// ------------------------------------------------

}
    
// ------------------------------------------------
