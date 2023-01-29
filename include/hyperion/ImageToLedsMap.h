#ifndef IMAGETOLEDSMAP_H
#define IMAGETOLEDSMAP_H

// STL includes
#include <cassert>
#include <sstream>
#include <cmath>

// hyperion-utils includes
#include <utils/Image.h>
#include <utils/Logger.h>
#include <utils/ColorRgbScalar.h>
#include <utils/ColorSys.h>

// hyperion includes
#include <hyperion/LedString.h>

namespace hyperion
{
	/// Number of clusters for k-means calculation
	const int CLUSTER_COUNT {5};

	///
	/// The ImageToLedsMap holds a mapping of indices into an image to LEDs. It can be used to
	/// calculate the average (aka mean) or dominant color per LED for a given region.
	///
	class ImageToLedsMap
	{
	public:

		///
		/// Constructs an mapping from the absolute indices in an image to each LED based on the border
		/// definition given in the list of LEDs. The map holds absolute indices to any given image,
		/// provided that it is row-oriented.
		/// The mapping is created purely on size (width and height). The given borders are excluded
		/// from indexing.
		///
		/// @param[in] width            The width of the indexed image
		/// @param[in] height           The width of the indexed image
		/// @param[in] horizontalBorder The size of the horizontal border (0=no border)
		/// @param[in] verticalBorder   The size of the vertical border (0=no border)
		/// @param[in] leds             The list with led specifications
		///
		ImageToLedsMap(
				const int width,
				const int height,
				const int horizontalBorder,
				const int verticalBorder,
				const std::vector<Led> & leds);

		///
		/// Returns the width of the indexed image
		///
		/// @return The width of the indexed image [pixels]
		///
		int width() const;

		///
		/// Returns the height of the indexed image
		///
		/// @return The height of the indexed image [pixels]
		///
		int height() const;

		int horizontalBorder() const { return _horizontalBorder; }
		int verticalBorder() const { return _verticalBorder; }

		///
		/// Determines the mean color for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the led colors
		///
		/// @return The vector containing the output
		///
		template <typename Pixel_T>
		std::vector<ColorRgb> getMeanLedColor(const Image<Pixel_T> & image) const
		{
			std::vector<ColorRgb> colors(_colorsMap.size(), ColorRgb{0,0,0});
			getMeanLedColor(image, colors);
			return colors;
		}

		///
		/// Determines the mean color for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the LED colors
		/// @param[out] ledColors  The vector containing the output
		///
		template <typename Pixel_T>
		void getMeanLedColor(const Image<Pixel_T> & image, std::vector<ColorRgb> & ledColors) const
		{
			if(_colorsMap.size() != ledColors.size())
			{
				Debug(Logger::getInstance("HYPERION"), "ImageToLedsMap: colorsMap.size != ledColors.size -> %d != %d", _colorsMap.size(), ledColors.size());
				return;
			}

			// Iterate each led and compute the mean
			auto led = ledColors.begin();
			for (auto colors = _colorsMap.begin(); colors != _colorsMap.end(); ++colors, ++led)
			{
				const ColorRgb color = calcMeanColor(image, *colors);
				*led = color;
			}
		}

		///
		/// Determines the mean color squared for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the led colors
		///
		/// @return The vector containing the output
		///
		template <typename Pixel_T>
		std::vector<ColorRgb> getMeanLedColorSqrt(const Image<Pixel_T> & image) const
		{
			std::vector<ColorRgb> colors(_colorsMap.size(), ColorRgb{0,0,0});
			getMeanLedColorSqrt(image, colors);
			return colors;
		}

		///
		/// Determines the mean color squared for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the LED colors
		/// @param[out] ledColors  The vector containing the output
		///
		template <typename Pixel_T>
		void getMeanLedColorSqrt(const Image<Pixel_T> & image, std::vector<ColorRgb> & ledColors) const
		{
			if(_colorsMap.size() != ledColors.size())
			{
				Debug(Logger::getInstance("HYPERION"), "ImageToLedsMap: colorsMap.size != ledColors.size -> %d != %d", _colorsMap.size(), ledColors.size());
				return;
			}

			// Iterate each led and compute the mean
			auto led = ledColors.begin();
			for (auto colors = _colorsMap.begin(); colors != _colorsMap.end(); ++colors, ++led)
			{
				const ColorRgb color = calcMeanColorSqrt(image, *colors);
				*led = color;
			}
		}

		///
		/// Determines the mean color of the image and assigns it to all LEDs
		///
		/// @param[in] image  The image from which to extract the led color
		///
		/// @return The vector containing the output
		///
		template <typename Pixel_T>
		std::vector<ColorRgb> getUniLedColor(const Image<Pixel_T> & image) const
		{
			std::vector<ColorRgb> colors(_colorsMap.size(), ColorRgb{0,0,0});
			getUniLedColor(image, colors);
			return colors;
		}

		///
		/// Determines the mean color of the image and assigns it to all LEDs
		///
		/// @param[in] image  The image from which to extract the LED colors
		/// @param[out] ledColors  The vector containing the output
		///
		template <typename Pixel_T>
		void getUniLedColor(const Image<Pixel_T> & image, std::vector<ColorRgb> & ledColors) const
		{
			if(_colorsMap.size() != ledColors.size())
			{
				Debug(Logger::getInstance("HYPERION"), "ImageToLedsMap: colorsMap.size != ledColors.size -> %d != %d", _colorsMap.size(), ledColors.size());
				return;
			}

			// calculate uni color
			const ColorRgb color = calcMeanColor(image);
			//Update all LEDs with same color
			std::fill(ledColors.begin(),ledColors.end(), color);
		}

		///
		/// Determines the dominant color for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the LED color
		///
		/// @return The vector containing the output
		///
		template <typename Pixel_T>
		std::vector<ColorRgb> getDominantLedColor(const Image<Pixel_T> & image) const
		{
			std::vector<ColorRgb> colors(_colorsMap.size(), ColorRgb{0,0,0});
			getDominantLedColor(image, colors);
			return colors;
		}

		///
		/// Determines the dominant color for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the LED colors
		/// @param[out] ledColors  The vector containing the output
		///
		template <typename Pixel_T>
		void getDominantLedColor(const Image<Pixel_T> & image, std::vector<ColorRgb> & ledColors) const
		{
			// Sanity check for the number of LEDs
			if(_colorsMap.size() != ledColors.size())
			{
				Debug(Logger::getInstance("HYPERION"), "ImageToLedsMap: colorsMap.size != ledColors.size -> %d != %d", _colorsMap.size(), ledColors.size());
				return;
			}

			// Iterate each led and compute the dominant color
			auto led = ledColors.begin();
			for (auto colors = _colorsMap.begin(); colors != _colorsMap.end(); ++colors, ++led)
			{
				const ColorRgb color = calculateDominantColor(image, *colors);
				*led = color;
			}
		}

		///
		/// Determines the dominant color  using a k-means algorithm for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the LED color
		///
		/// @return The vector containing the output
		///
		template <typename Pixel_T>
		std::vector<ColorRgb> getDominantLedColorAdv(const Image<Pixel_T> & image) const
		{
			std::vector<ColorRgb> colors(_colorsMap.size(), ColorRgb{0,0,0});
			getDominantLedColorAdv(image, colors);
			return colors;
		}

		///
		/// Determines the dominant color using a k-means algorithm for each LED using the LED area mapping given
		/// at construction.
		///
		/// @param[in] image  The image from which to extract the LED colors
		/// @param[out] ledColors  The vector containing the output
		///
		template <typename Pixel_T>
		void getDominantLedColorAdv(const Image<Pixel_T> & image, std::vector<ColorRgb> & ledColors) const
		{
			// Sanity check for the number of LEDs
			if(_colorsMap.size() != ledColors.size())
			{
				Debug(Logger::getInstance("HYPERION"), "ImageToLedsMap: colorsMap.size != ledColors.size -> %d != %d", _colorsMap.size(), ledColors.size());
				return;
			}

			// Iterate each led and compute the dominant color
			auto led = ledColors.begin();
			for (auto colors = _colorsMap.begin(); colors != _colorsMap.end(); ++colors, ++led)
			{
				const ColorRgb color = calculateDominantColorAdv(image, *colors);
				*led = color;
			}
		}

	private:
		/// The width of the indexed image
		const int _width;
		/// The height of the indexed image
		const int _height;

		const int _horizontalBorder;

		const int _verticalBorder;

		/// The absolute indices into the image for each led
		std::vector<std::vector<int>> _colorsMap;

		///
		/// Calculates the 'mean color' over the given image. This is the mean over each color-channel
		/// (red, green, blue)
		///
		/// @param[in] image The image a section from which an average color must be computed
		/// @param[in] pixels The list of pixel indices for the given image to be evaluated///
		///
		/// @return The mean of the given list of colors (or black when empty)
		///
		template <typename Pixel_T>
		ColorRgb calcMeanColor(const Image<Pixel_T> & image, const std::vector<int32_t> & pixels) const
		{
			const auto pixelNum = pixels.size();
			if (pixelNum == 0)
			{
				return ColorRgb::BLACK;
			}

			// Accumulate the sum of each separate color channel
			uint_fast32_t cummRed   = 0;
			uint_fast32_t cummGreen = 0;
			uint_fast32_t cummBlue  = 0;

			const auto& imgData = image.memptr();
			for (const int pixelOffset : pixels)
			{
				const auto& pixel = imgData[pixelOffset];
				cummRed   += pixel.red;
				cummGreen += pixel.green;
				cummBlue  += pixel.blue;
			}

			// Compute the average of each color channel
			const uint8_t avgRed   = uint8_t(cummRed/pixelNum);
			const uint8_t avgGreen = uint8_t(cummGreen/pixelNum);
			const uint8_t avgBlue  = uint8_t(cummBlue/pixelNum);

			// Return the computed color
			return {avgRed, avgGreen, avgBlue};
		}

		///
		/// Calculates the 'mean color' over the given image. This is the mean over each color-channel
		/// (red, green, blue)
		///
		/// @param[in] image The image a section from which an average color must be computed
		///
		/// @return The mean of the given list of colors (or black when empty)
		///
		template <typename Pixel_T>
		ColorRgb calcMeanColor(const Image<Pixel_T> & image) const
		{
			// Accumulate the sum of each separate color channel
			uint_fast32_t cummRed   = 0;
			uint_fast32_t cummGreen = 0;
			uint_fast32_t cummBlue  = 0;

			const unsigned pixelNum = image.width() * image.height();
			const auto& imgData = image.memptr();

			for (unsigned idx=0; idx<pixelNum; idx++)
			{
				const auto& pixel = imgData[idx];
				cummRed   += pixel.red;
				cummGreen += pixel.green;
				cummBlue  += pixel.blue;
			}

			// Compute the average of each color channel
			const uint8_t avgRed   = uint8_t(cummRed/pixelNum);
			const uint8_t avgGreen = uint8_t(cummGreen/pixelNum);
			const uint8_t avgBlue  = uint8_t(cummBlue/pixelNum);

			// Return the computed color
			return {avgRed, avgGreen, avgBlue};
		}

		///
		/// Calculates the 'mean color' squared over the given image. This is the mean over each color-channel
		/// (red, green, blue)
		///
		/// @param[in] image The image a section from which an average color must be computed
		/// @param[in] pixels The list of pixel indices for the given image to be evaluated
		///
		/// @return The mean of the given list of colors (or black when empty)
		///
		template <typename Pixel_T>
		ColorRgb calcMeanColorSqrt(const Image<Pixel_T> & image, const std::vector<int32_t> & pixels) const
		{
			const auto pixelNum = pixels.size();
			if (pixelNum == 0)
			{
				return ColorRgb::BLACK;
			}

			// Accumulate the squared sum of each separate color channel
			uint_fast32_t cummRed   = 0;
			uint_fast32_t cummGreen = 0;
			uint_fast32_t cummBlue  = 0;

			const auto& imgData = image.memptr();

			for (const int colorOffset : pixels)
			{
				const auto& pixel = imgData[colorOffset];

				cummRed   += pixel.red * pixel.red;
				cummGreen += pixel.green * pixel.green;
				cummBlue  += pixel.blue * pixel.blue;
			}

			// Compute the average of each color channel
			const uint8_t avgRed   = uint8_t(std::min(std::lround(sqrt(static_cast<double>(cummRed/pixelNum))), 255L));
			const uint8_t avgGreen = uint8_t(std::min(std::lround(sqrt(static_cast<double>(cummGreen/pixelNum))), 255L));
			const uint8_t avgBlue  = uint8_t(std::min(std::lround(sqrt(static_cast<double>(cummBlue/pixelNum))), 255L));

			// Return the computed color
			return {avgRed, avgGreen, avgBlue};
		}

		///
		/// Calculates the 'mean color' squared over the given image. This is the mean over each color-channel
		/// (red, green, blue)
		///
		/// @param[in] image The image a section from which an average color must be computed
		///
		/// @return The mean of the given list of colors (or black when empty)
		///
		template <typename Pixel_T>
		ColorRgb calcMeanColorSqrt(const Image<Pixel_T> & image) const
		{
			// Accumulate the squared sum of each separate color channel
			uint_fast32_t cummRed   = 0;
			uint_fast32_t cummGreen = 0;
			uint_fast32_t cummBlue  = 0;

			const unsigned pixelNum = image.width() * image.height();
			const auto& imgData = image.memptr();

			for (int idx=0; idx<pixelNum; ++idx)
			{
				const auto& pixel = imgData[idx];
				cummRed   += pixel.red * pixel.red;
				cummGreen += pixel.green * pixel.green;
				cummBlue  += pixel.blue * pixel.blue;
			}

			// Compute the average of each color channel
			const uint8_t avgRed   = uint8_t(std::lround(sqrt(static_cast<double>(cummRed/pixelNum))));
			const uint8_t avgGreen = uint8_t(std::lround(sqrt(static_cast<double>(cummGreen/pixelNum))));
			const uint8_t avgBlue  = uint8_t(std::lround(sqrt(static_cast<double>(cummBlue/pixelNum))));

			// Return the computed color
			return {avgRed, avgGreen, avgBlue};
		}

		///
		/// Calculates the 'dominant color' of an image area defined by a list of pixel indices
		///
		/// @param[in] image The image for which a dominant color is to be computed
		/// @param[in] pixels The list of pixel indices for the given image to be evaluated
		///
		/// @return The image area's dominant color or black, if no pixel indices provided
		///
		template <typename Pixel_T>
		ColorRgb calculateDominantColor(const Image<Pixel_T> & image, const std::vector<int> & pixels) const
		{
			ColorRgb dominantColor {ColorRgb::BLACK};

			const auto pixelNum = pixels.size();
			if (pixelNum > 0)
			{
				const auto& imgData = image.memptr();

				QMap<QRgb,int> colorDistributionMap;
				int count = 0;
				for (const int pixelOffset : pixels)
				{
					QRgb color = imgData[pixelOffset].rgb();
					if (colorDistributionMap.contains(color)) {
						colorDistributionMap[color] = colorDistributionMap[color] + 1;
					}
					else  {
						colorDistributionMap[color] = 1;
					}

					int colorsFound  =  colorDistributionMap[color];
					if (colorsFound > count)  {
						dominantColor.setRgb(color);
						count = colorsFound;
					}
				}
			}
			return dominantColor;
		}

		///
		/// Calculates the 'dominant color' of an image
		///
		/// @param[in] image The image for which a dominant color is to be computed
		///
		/// @return The image's dominant color
		///
		template <typename Pixel_T>
		ColorRgb calculateDominantColor(const Image<Pixel_T> & image) const
		{
			const unsigned pixelNum = image.width() * image.height();

			std::vector<int> pixels(pixelNum);
			std::iota(pixels.begin(), pixels.end(), 0);

			return calculateDominantColor(image, pixels);
		}

		template <typename Pixel_T>
		struct ColorCluster {

			ColorCluster():count(0) {}

			Pixel_T color;
			Pixel_T newColor;
			int count;
		};

		///
		/// Calculates the 'dominant color' of an image area defined by a list of pixel indices
		/// using a k-means algorithm (https://robocraft.ru/computervision/1063)
		///
		/// @param[in] image The image for which a dominant color is to be computed
		/// @param[in] pixels The list of pixel indices for the given image to be evaluated
		///
		/// @return The image area's dominant color or black, if no pixel indices provided
		///
		template <typename Pixel_T>
		ColorRgb calculateDominantColorAdv(const Image<Pixel_T> & image, const std::vector<int> & pixels) const
		{
			ColorRgb dominantColor {ColorRgb::BLACK};

			const auto pixelNum = pixels.size();
			if (pixelNum > 0)
			{
				ColorCluster<ColorRgbScalar> clusters[CLUSTER_COUNT];

				// initial cluster colors
				switch (CLUSTER_COUNT) {
				case 4:
					clusters[4].newColor = ColorRgbScalar(ColorRgb::YELLOW);
				case 3:
					clusters[3].newColor = ColorRgbScalar(ColorRgb::RED);
				case 2:
					clusters[2].newColor = ColorRgbScalar(ColorRgb::WHITE);
				case 1:
					clusters[1].newColor = ColorRgbScalar(ColorRgb::GREEN);
				case 0:
					clusters[0].newColor = ColorRgbScalar(ColorRgb::BLACK);
					break;
				default:
					for(int k = 0; k < CLUSTER_COUNT; ++k)
					{
						int randomRed = rand() % static_cast<int>(256);
						int randomGreen = rand() % static_cast<int>(256);
						int randomBlue = rand() % static_cast<int>(256);

						clusters[k].newColor = ColorRgbScalar(randomRed, randomGreen, randomBlue);
					}
					break;
				}

				// k-means
				double min_rgb_euclidean {0};
				double old_rgb_euclidean {0};

				while(1)
				{
					for(int k = 0; k < CLUSTER_COUNT; ++k)
					{
						clusters[k].count = 0;
						clusters[k].color = clusters[k].newColor;
						clusters[k].newColor.setRgb(ColorRgb::BLACK);
					}

					const auto& imgData = image.memptr();
					for (const int pixelOffset : pixels)
					{
						const auto& pixel = imgData[pixelOffset];

						min_rgb_euclidean = 255 * 255 * 255;
						int clusterIndex = -1;
						for(int k = 0; k < CLUSTER_COUNT; ++k)
						{
							double euclid = ColorSys::rgb_euclidean(ColorRgbScalar(pixel), clusters[k].color);

							if(  euclid < min_rgb_euclidean ) {
								min_rgb_euclidean = euclid;
								clusterIndex = k;
							}
						}

						clusters[clusterIndex].count++;
						clusters[clusterIndex].newColor += ColorRgbScalar(pixel);
					}

					min_rgb_euclidean = 0;
					for(int k = 0; k < CLUSTER_COUNT; ++k)
					{
						if (clusters[k].count > 0)
						{
							// new color
							clusters[k].newColor /= clusters[k].count;
							double ecli = ColorSys::rgb_euclidean(clusters[k].newColor, clusters[k].color);
							if(ecli > min_rgb_euclidean)
							{
								min_rgb_euclidean = ecli;
							}
						}
					}

					if( fabs(min_rgb_euclidean - old_rgb_euclidean) < 1)
					{
						break;
					}

					old_rgb_euclidean = min_rgb_euclidean;
				}

				int colorsFoundMax = 0;
				int dominantClusterIdx {0};

				for(int clusterIdx=0; clusterIdx < CLUSTER_COUNT; ++clusterIdx){
					int colorsFoundinCluster = clusters[clusterIdx].count;
					if (colorsFoundinCluster > colorsFoundMax)  {
						colorsFoundMax = colorsFoundinCluster;
						dominantClusterIdx = clusterIdx;
					}
				}

				dominantColor.red = static_cast<uint8_t>(clusters[dominantClusterIdx].newColor.red);
				dominantColor.green = static_cast<uint8_t>(clusters[dominantClusterIdx].newColor.green);
				dominantColor.blue = static_cast<uint8_t>(clusters[dominantClusterIdx].newColor.blue);
			}

			return dominantColor;
		}

		///
		/// Calculates the 'dominant color' of an image area defined by a list of pixel indices
		/// using a k-means algorithm (https://robocraft.ru/computervision/1063)
		///
		/// @param[in] image The image for which a dominant color is to be computed
		///
		/// @return The image's dominant color
		///
		template <typename Pixel_T>
		ColorRgb calculateDominantColorAdv(const Image<Pixel_T> & image) const
		{
			const unsigned pixelNum = image.width() * image.height();

			std::vector<int> pixels(pixelNum);
			std::iota(pixels.begin(), pixels.end(), 0);

			return calculateDominantColorAdv(image, pixels);
		}
	};

} // end namespace hyperion

#endif // IMAGETOLEDSMAP_H
