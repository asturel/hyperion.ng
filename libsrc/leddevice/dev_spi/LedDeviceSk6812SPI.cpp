#include "LedDeviceSk6812SPI.h"

/*
3200000 MAX

Reset time:
Reset time is 80uS = 256 bits = 32 bytes

*/


// Constants
namespace {

// Configuration settings
const char CONFIG_WHITE_CHANNEL_CALLIBRATION[] = "white_channel_calibration";
const char CONFIG_RESET_TIME[] = "resetTime";
const char CONFIG_WHITE_CHANNEL_LIMIT[] = "white_channel_limit";
const char CONFIG_WHITE_CHANNEL_RED[] = "white_channel_red";
const char CONFIG_WHITE_CHANNEL_GREEN[] = "white_channel_green";
const char CONFIG_WHITE_CHANNEL_BLUE[] = "white_channel_blue";

} //End of constants

LedDeviceSk6812SPI::LedDeviceSk6812SPI(const QJsonObject &deviceConfig)
	: ProviderSpi(deviceConfig)
	  , _whiteAlgorithm(RGBW::WhiteAlgorithm::INVALID)
	  , SPI_BYTES_PER_COLOUR(4)
	  , _spi_frame_end_latch_bytes(32)
	  , bitpair_to_byte {
		  0b10001000,
		  0b10001100,
		  0b11001000,
		  0b11001100,
		  }
{
}

LedDevice* LedDeviceSk6812SPI::construct(const QJsonObject &deviceConfig)
{
	return new LedDeviceSk6812SPI(deviceConfig);
}

bool LedDeviceSk6812SPI::init(const QJsonObject &deviceConfig)
{
	_baudRate_Hz = 3000000;

	bool isInitOK = false;

	// Initialise sub-class
	if ( ProviderSpi::init(deviceConfig) )
	{
		_white_channel_calibration  = deviceConfig[CONFIG_WHITE_CHANNEL_CALLIBRATION].toBool(false);
		double _white_channel_limit_percent = deviceConfig[CONFIG_WHITE_CHANNEL_LIMIT].toDouble(1);
		_white_channel_limit  = static_cast<uint8_t>(qMin(qRound(_white_channel_limit_percent * 255.0 / 100.0), 255));
		_white_channel_red  = static_cast<uint8_t>(qMin(deviceConfig[CONFIG_WHITE_CHANNEL_RED].toInt(255), 255));
		_white_channel_green = static_cast<uint8_t>(qMin(deviceConfig[CONFIG_WHITE_CHANNEL_GREEN].toInt(255), 255));
		_white_channel_blue = static_cast<uint8_t>(qMin(deviceConfig[CONFIG_WHITE_CHANNEL_BLUE].toInt(255), 255));
		int spi_frame_end_latch_bytes = deviceConfig[CONFIG_RESET_TIME].toInt();
		_spi_frame_end_latch_bytes = qRound(spi_frame_end_latch_bytes * _baudRate_Hz / 1000000.0 / 8.0);

		Debug(_log, "SPI frame end latch time [%d] us, [%d] bytes", spi_frame_end_latch_bytes, _spi_frame_end_latch_bytes);

		DebugIf(_white_channel_calibration, _log, "White channel limit: %i (%.2f%), red: %i, green: %i, blue: %i", _white_channel_limit, _white_channel_limit_percent, _white_channel_red, _white_channel_green, _white_channel_blue);

		QString whiteAlgorithm = deviceConfig["whiteAlgorithm"].toString("white_off");

		_whiteAlgorithm	= RGBW::stringToWhiteAlgorithm(whiteAlgorithm);
		if (_whiteAlgorithm == RGBW::WhiteAlgorithm::INVALID)
		{
			QString errortext = QString ("unknown whiteAlgorithm: %1").arg(whiteAlgorithm);
			this->setInError(errortext);
			isInitOK = false;
		}
		else
		{
			Debug( _log, "whiteAlgorithm : %s", QSTRING_CSTR(whiteAlgorithm));

			WarningIf(( _baudRate_Hz < 2050000 || _baudRate_Hz > 4000000 ), _log, "SPI rate %d outside recommended range (2050000 -> 4000000)", _baudRate_Hz);

			_ledBuffer.resize(_ledRGBWCount * SPI_BYTES_PER_COLOUR + _spi_frame_end_latch_bytes, 0x00);

			isInitOK = true;
		}
	}
	return isInitOK;
}

int LedDeviceSk6812SPI::write(const std::vector<ColorRgb> &ledValues)
{
	unsigned spi_ptr = 0;
	const int SPI_BYTES_PER_LED = sizeof(ColorRgbw) * SPI_BYTES_PER_COLOUR;


	for (const ColorRgb& color : ledValues)
	{
		RGBW::Rgb_to_Rgbw(color, &_temp_rgbw, _whiteAlgorithm);
		// FIXME: lift up
		if (_white_channel_calibration) {
			//_temp_rgbw.red   = static_cast<uint8_t>(qMin(qRound(_temp_rgbw.red  / 255.0 * _white_channel_red), 255));
			//_temp_rgbw.green = static_cast<uint8_t>(qMin(qRound(_temp_rgbw.green / 255.0 * _white_channel_green), 255));
			//_temp_rgbw.blue  = static_cast<uint8_t>(qMin(qRound(_temp_rgbw.blue / 255.0 * _white_channel_blue), 255));
			_temp_rgbw.white = static_cast<uint8_t>(qMin(qRound(_white_channel_limit / 255.0 * _temp_rgbw.white), 255));
		}
		uint32_t colorBits =
			((uint32_t)_temp_rgbw.red << 24) +
			((uint32_t)_temp_rgbw.green << 16) +
			((uint32_t)_temp_rgbw.blue << 8) +
			_temp_rgbw.white;

		for (int j=SPI_BYTES_PER_LED - 1; j>=0; j--)
		{
			_ledBuffer[spi_ptr+j] = bitpair_to_byte[ colorBits & 0x3 ];
			colorBits >>= 2;
		}
		spi_ptr += SPI_BYTES_PER_LED;
	}

	for (int j=0; j < _spi_frame_end_latch_bytes; j++)
	{
		_ledBuffer[spi_ptr++] = 0;
	}

	return writeBytes(_ledBuffer.size(), _ledBuffer.data());
}
