#pragma once

#include <string>
#include <stdexcept>
#include <fstream>

class PPMWriter 
{
public:

	PPMWriter(std::string filename, uint32_t imageWidth, uint32_t imageHeight, uint32_t maxColorComponent)
		: ppmFileStream(filename + ".ppm", std::ios::out | std::ios::binary)
	{
		if (!ppmFileStream.is_open())
			throw std::runtime_error("Failed to open file: " + filename + ".ppm");

		ppmFileStream << "P3\n";
		ppmFileStream << imageWidth << " " << imageHeight << "\n";
		ppmFileStream << maxColorComponent << "\n";
	}

	~PPMWriter() 
	{
		if (ppmFileStream.is_open())
			ppmFileStream.close();
	}

	friend PPMWriter& operator<<(PPMWriter& writer, const std::string& data) 
	{
		writer.ppmFileStream << data;
		return writer;
	}

protected:
	std::ofstream ppmFileStream;
};