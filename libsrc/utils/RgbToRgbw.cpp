#include <utils/ColorRgb.h>
#include <utils/ColorRgbw.h>
#include <utils/RgbToRgbw.h>
#include <utils/Logger.h>

#define ROUND_DIVIDE(number, denom) (((number) + (denom) / 2) / (denom))

namespace RGBW {

const WhiteCalibration WhiteCalibration::Default = {false, 255, 255, 255, 255};

WhiteAlgorithm stringToWhiteAlgorithm(const QString& str)
{
	if (str == "subtract_minimum")
	{
		return WhiteAlgorithm::SUBTRACT_MINIMUM;
	}
	if (str == "sub_min_warm_adjust")
	{
		return WhiteAlgorithm::SUB_MIN_WARM_ADJUST;
	}
	if (str == "sub_min_cool_adjust")
	{
		return WhiteAlgorithm::SUB_MIN_COOL_ADJUST;
	}
    if (str == "cold_white")
    {
        return WhiteAlgorithm::COLD_WHITE;
    }
    if (str == "neutral_white")
    {
        return WhiteAlgorithm::NEUTRAL_WHITE;
    }
    if (str == "auto")
    {
        return WhiteAlgorithm::AUTO;
    }
    if (str == "auto_max")
    {
        return WhiteAlgorithm::AUTO_MAX;
    }
    if (str == "auto_accurate")
    {
        return WhiteAlgorithm::AUTO_ACCURATE;
    }
    if (str == "custom")
    {
        return WhiteAlgorithm::CUSTOM;
    }
    if (str == "custom_accurate")
    {
        return WhiteAlgorithm::CUSTOM_ACCURATE;
    }
    if (str.isEmpty() || str == "white_off")
	{
		return WhiteAlgorithm::WHITE_OFF;
	}
	return WhiteAlgorithm::INVALID;
}

void Rgb_to_Rgbw(ColorRgb input, ColorRgbw * output, WhiteAlgorithm algorithm, const WhiteCalibration * calibration)
{
	//const uint8_t white = (uint8_t)qRound(qMin(calibration->white * input.white, 255.0f));
    //const uint8_t red = (uint8_t)qRound(qMin(input.red * calibration->red / 255.0, 255.0));
    //const uint8_t green = (uint8_t)qRound(qMin(input.green * calibration->green / 255.0, 255.0));
    //const uint8_t blue = (uint8_t)qRound(qMin(input.blue * calibration->blue / 255.0, 255.0));

	switch (algorithm)
	{
		case WhiteAlgorithm::SUBTRACT_MINIMUM:
		{
			output->white = static_cast<uint8_t>(qMin(qMin(input.red, input.green), input.blue));
			if (calibration->enabled) {
				output->white = (uint8_t)qRound(qMin(calibration->white * output->white / 255.0, 255.0));
			}
			output->red   = input.red   - output->white;
			output->green = input.green - output->white;
			output->blue  = input.blue  - output->white;
			break;
		}

		case WhiteAlgorithm::SUB_MIN_WARM_ADJUST:
		{
			// http://forum.garagecube.com/viewtopic.php?t=10178
			// warm white
			const double F1(0.274);
			const double F2(0.454);
			const double F3(2.333);

			output->white = static_cast<uint8_t>(qMin(input.red*F1,qMin(input.green*F2,input.blue*F3)));
			if (calibration->enabled) {
				output->white = (uint8_t)qRound(qMin(calibration->white * output->white / 255.0, 255.0));
			}
			output->red   = input.red   - static_cast<uint8_t>(output->white/F1);
			output->green = input.green - static_cast<uint8_t>(output->white/F2);
			output->blue  = input.blue  - static_cast<uint8_t>(output->white/F3);
			break;
		}

		case WhiteAlgorithm::SUB_MIN_COOL_ADJUST:
		{
			// http://forum.garagecube.com/viewtopic.php?t=10178
			// cold white
			const double F1(0.299);
			const double F2(0.587);
			const double F3(0.114);

			output->white = static_cast<uint8_t>(qMin(input.red*F1,qMin(input.green*F2,input.blue*F3)));
			if (calibration->enabled) {
				output->white = (uint8_t)qRound(qMin(calibration->white * output->white / 255.0, 255.0));
			}
			output->red   = input.red   - static_cast<uint8_t>(output->white/F1);
			output->green = input.green - static_cast<uint8_t>(output->white/F2);
			output->blue  = input.blue  - static_cast<uint8_t>(output->white/F3);
			break;
		}

		case WhiteAlgorithm::WHITE_OFF:
		{
			output->red = input.red;
			output->green = input.green;
			output->blue = input.blue;
			output->white = 0;
			break;
		}

        case WhiteAlgorithm::AUTO_MAX:
        {
            output->red = input.red;
            output->green = input.green;
            output->blue = input.blue;
            output->white = input.red > input.green ? (input.red > input.blue ? input.red : input.blue) : (input.green > input.blue ? input.green : input.blue);
			if (calibration->enabled) {
				output->white = (uint8_t)qRound(qMin(calibration->white * output->white / 255.0, 255.0));
			}
            break;
        }

        case WhiteAlgorithm::AUTO_ACCURATE:
        {
            output->white = input.red < input.green ? (input.red < input.blue ? input.red : input.blue) : (input.green < input.blue ? input.green : input.blue);
            output->red = input.red - output->white;
            output->green = input.green - output->white;
            output->blue = input.blue - output->white;
			if (calibration->enabled) {
				output->white = (uint8_t)qRound(qMin(calibration->white * output->white / 255.0, 255.0));
			}
            break;
        }

        case WhiteAlgorithm::AUTO:
        {

            output->red = input.red;
            output->green = input.green;
            output->blue = input.blue;
            output->white = input.red < input.green ? (input.red < input.blue ? input.red : input.blue) : (input.green < input.blue ? input.green : input.blue);
			if (calibration->enabled) {
				output->white = (uint8_t)qRound(qMin(calibration->white * output->white / 255.0, 255.0));
			}
            break;
        }
        case WhiteAlgorithm::NEUTRAL_WHITE:
        case WhiteAlgorithm::COLD_WHITE:
        case WhiteAlgorithm::CUSTOM:
        case WhiteAlgorithm::CUSTOM_ACCURATE:
        {
            //cold white config
            uint8_t gain = 0xFF;
            uint8_t red = 0xA0;
            uint8_t green = 0xA0;
            uint8_t blue = 0xA0;

            if (algorithm == WhiteAlgorithm::NEUTRAL_WHITE) {
                gain = 0xFF;
                red = 0xB0;
                green = 0xB0;
                blue = 0x70;
            }

            if (algorithm == WhiteAlgorithm::CUSTOM || algorithm == WhiteAlgorithm::CUSTOM_ACCURATE) {
                gain = calibration->white;
                red = calibration->red;
                green = calibration->green;
                blue = calibration->blue;
            }

            uint8_t _r = qMin((uint32_t)(ROUND_DIVIDE(red * input.red,  0xFF)), (uint32_t)0xFF);
            uint8_t _g = qMin((uint32_t)(ROUND_DIVIDE(green * input.green,  0xFF)), (uint32_t)0xFF);
            uint8_t _b = qMin((uint32_t)(ROUND_DIVIDE(blue * input.blue,  0xFF)), (uint32_t)0xFF);

            output->white = qMin(_r, qMin(_g, _b));

			if (algorithm != WhiteAlgorithm::CUSTOM) {
				output->red = input.red - _r;
				output->green = input.green - _g;
				output->blue = input.blue - _b;
			} else {
				output->red = input.red;
				output->green = input.green;
				output->blue = input.blue;
			}

            uint8_t _w = qMin((uint32_t)(ROUND_DIVIDE(gain * output->white,  0xFF)), (uint32_t)0xFF);
            output->white = _w;
            break;
        }
		default:
			break;
	}
}

};
