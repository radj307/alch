#pragma once
/**
 * @file	SerializerDefs.h
 * @author	radj307
 * @brief	Adds definitions for the nlohmann json serializer to support custom object types.
 */
#include <nlohmann/json.hpp>

 // Include objects that need serialization support
#include "INamedObject.hpp"
#include "EKeywordDisposition.h"
#include "Keyword.hpp"
#include "Effect.hpp"
#include "Ingredient.hpp"
#include "Potion.hpp"

// Create serializer definitions
namespace alchlib2 {
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(INamedObject, name);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Keyword, name, formID, disposition);
	/*NLOHMANN_JSON_SERIALIZE_ENUM(EKeywordDisposition, {
								 { Unknown, "Unknown" },
								 { Neutral, "Neutral" },
								 { Positive, "Positive" },
								 { Cure, "Cure" },
								 { FortifyStat, "FortifyStat" },
								 { Negative, "Negative" },
								 { InfluenceOther, "InfluenceOther" }
								 });*/
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Effect, name, magnitude, duration, keywords);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Ingredient, name, effects);
	NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Potion, name, effects);
}
