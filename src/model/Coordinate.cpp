/**
 * @file Coordinate.cpp
 * @author Baudouin Feildel <baudouin.feildel@st.com>
 * @copyright 2016, STMicroelectronics, All rights reserved.
 */

#include "Coordinate.h"

#define LOG_TAG "teseo_hal_model_Coordinate"
#include <cutils/log.h>
#include <cmath>
#include <cstdlib>
#include <tuple>

namespace stm {

/**
 * @brief      Convert decimal degree coordinate to degree minute
 *
 * @param[in]  value  The decimal degree coordinate
 * @param[in]  lat    true if coordinate is a latitude, false otherwise
 *
 * @return     Tuple containing the degree as integer, the minute as double and the direction
 */
static std::tuple<int, double, CoordinateDirection>
	DecimalDegreeToDegreeMinute(double value, bool lat = true)
{
	CoordinateDirection direction;

	if(value > 0)
	{
		direction = lat ? CoordinateDirection::NORTH : CoordinateDirection::EAST;
	}
	else
	{
		direction = lat ? CoordinateDirection::SOUTH : CoordinateDirection::WEST;
		value = -value;
	}

	double degree = std::floor(value);
	
	double tmpMinutes = std::floor(60. * (value - degree));
	double tmpSeconds = 3600. * (value - degree) - 60. * tmpMinutes;

	return std::make_tuple(static_cast<int>(degree), tmpMinutes + (tmpSeconds / 60.), direction);
}

/**
 * @brief      Convert degree minute coordinate to decimal degree
 *
 * @param[in]  degree  The degree value
 * @param[in]  minute  The decimal minute value
 * @param[in]  dir     The direction
 *
 * @return     The decimal degree coordinate
 */
static double DegreeMinuteToDecimalDegree(int degree, double minute, CoordinateDirection dir)
{
	double val = (static_cast<double>(degree) + minute / 60.) *
		((dir == CoordinateDirection::NORTH || dir == CoordinateDirection::EAST) ? 1. : -1.);

	ALOGI("Decode GGA: convert deg min to decimal deg: %d°%lf = %lf", degree, minute, val);

	return val;
}

static double DegreeMinuteToDecimalDegree(const DegreeMinuteCoordinate & coord)
{
	return DegreeMinuteToDecimalDegree(coord.getDegree(), coord.getMinute(), coord.getDirection());
}

CoordinateDirection CoordinateDirectionParse(uint8_t byte)
{
	switch(byte)
	{
		case 'N': return CoordinateDirection::NORTH;
		case 'S': return CoordinateDirection::SOUTH;
		case 'E': return CoordinateDirection::EAST;
		case 'W': return CoordinateDirection::WEST;
		default:  return CoordinateDirection::UNKNOWN;
	}
}

DegreeMinuteCoordinate::DegreeMinuteCoordinate() :
	ICoordinate(),
	degree(0), minute(0), direction(CoordinateDirection::UNKNOWN)
{ }

DegreeMinuteCoordinate::DegreeMinuteCoordinate(const DegreeMinuteCoordinate & other) :
	ICoordinate(),
	degree(other.degree), minute(other.minute), direction(other.direction)
{ }

DegreeMinuteCoordinate::DegreeMinuteCoordinate(const DecimalDegreeCoordinate & other, bool lat) :
	ICoordinate()
{
	std::tie(degree, minute, direction) = DecimalDegreeToDegreeMinute(other.value(), lat);
}

DegreeMinuteCoordinate::DegreeMinuteCoordinate(int deg, double min, CoordinateDirection dir) :
	ICoordinate(),
	degree(deg), minute(min), direction(dir)
{ }

DegreeMinuteCoordinate::DegreeMinuteCoordinate(const ByteVector & coordinate, uint8_t dir) :
	ICoordinate(),
	degree(0), minute(0), direction(CoordinateDirectionParse(dir))
{
	std::size_t dotPos = 0;

	
	for(dotPos = 0; dotPos < coordinate.size() && coordinate[dotPos] != '.'; dotPos++);

	if(dotPos > 4)
	{
		degree = utils::byteVectorParse<int>(coordinate.begin(), coordinate.begin() + 3);
		minute = utils::byteVectorParse<double>(coordinate.begin() + 3, coordinate.end());
	}
	else
	{
		degree = utils::byteVectorParse<int>(coordinate.begin(), coordinate.begin() + 2);
		minute = utils::byteVectorParse<double>(coordinate.begin() + 2, coordinate.end());
	}
	
	ALOGI("Decode GGA: dotpos=%d coordinate: %d°%lf', %c", (int)dotPos, degree, minute, dir);
}

DegreeMinuteCoordinate DegreeMinuteCoordinate::asDegreeMinute() const
{
	return DegreeMinuteCoordinate(*this);
}

DecimalDegreeCoordinate DegreeMinuteCoordinate::asDecimalDegree() const
{
	return DecimalDegreeCoordinate(*this);
}

DecimalDegreeCoordinate::DecimalDegreeCoordinate() :
	ICoordinate(),
	coordinate(0)
{ }

DecimalDegreeCoordinate::DecimalDegreeCoordinate(const DecimalDegreeCoordinate & other) :
	ICoordinate(),
	coordinate(other.coordinate)
{ }

DecimalDegreeCoordinate::DecimalDegreeCoordinate(const DegreeMinuteCoordinate & other) :
	ICoordinate(),
	coordinate(DegreeMinuteToDecimalDegree(other))
{ }

DecimalDegreeCoordinate::DecimalDegreeCoordinate(double coordinate) :
	ICoordinate(),
	coordinate(coordinate)
{ }

DecimalDegreeCoordinate::DecimalDegreeCoordinate(const ByteVector & coordinate) :
	ICoordinate(),
	coordinate(utils::byteVectorParse<double>(coordinate))
{ }

DegreeMinuteCoordinate DecimalDegreeCoordinate::asDegreeMinute() const
{
	return DegreeMinuteCoordinate(*this);
}

DecimalDegreeCoordinate DecimalDegreeCoordinate::asDecimalDegree() const
{
	return DecimalDegreeCoordinate(*this);
}

} // namespace stm