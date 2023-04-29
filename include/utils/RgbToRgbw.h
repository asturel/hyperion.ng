#pragma once
#include <QString>

#include <utils/ColorRgb.h>
#include <utils/ColorRgbw.h>

namespace RGBW {

	enum class WhiteAlgorithm {
		INVALID,
		SUBTRACT_MINIMUM,
		SUB_MIN_WARM_ADJUST,
		SUB_MIN_COOL_ADJUST,
		WHITE_OFF,
        COLD_WHITE,
        NEUTRAL_WHITE,
		CUSTOM,
		CUSTOM_ACCURATE,
        AUTO,
        AUTO_MAX,
        AUTO_ACCURATE
	};

	struct WhiteCalibration
	{
		bool enabled;
		/// The red color aspect
		uint8_t red;
		/// The green color aspect
		uint8_t green;
		/// The blue color aspect
		uint8_t blue;
		/// The white color channel limit
		uint8_t white;

		static const WhiteCalibration Default;
	};

	WhiteAlgorithm stringToWhiteAlgorithm(const QString& str);
	void Rgb_to_Rgbw(ColorRgb input, ColorRgbw * output, WhiteAlgorithm algorithm, const WhiteCalibration * calibration = &WhiteCalibration::Default);
}
