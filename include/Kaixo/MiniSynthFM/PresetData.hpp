#pragma once

// ------------------------------------------------

#include "Kaixo/Core/Definitions.hpp"
#include "Kaixo/Core/Serializable.hpp"

// ------------------------------------------------

namespace Kaixo {

	// ------------------------------------------------

	class PresetData : public Serializable {
	public:

		// ------------------------------------------------

		std::string name = "";
		std::string author = "";
		std::string type = "";
		std::string description = "";

		// ------------------------------------------------

		std::string loadedFromFolder = "";
		std::string loadedFromFilename = "";

		// ------------------------------------------------

		void init() override;
		basic_json serialize() override;
		void deserialize(basic_json& json) override;

		// ------------------------------------------------

	};

	// ------------------------------------------------

}

// ------------------------------------------------
