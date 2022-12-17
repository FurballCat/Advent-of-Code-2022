#include <stdio.h>
#include <stdlib.h>
#include <vector>

/* The idea for the solution is to only capture sensors that might overlap with the row of interest.
* So it all comes down to finding intersections between row and the area covered by sensors.
* Once we have sections found, we need to merge the ones that overlap. To do that, first we sort them by X, then do 'sweep and prune'.
* Then we have to subtract number of beacons that sit on the row.
* And that's it, this is the answer.
*/

// used for parsing numbers
bool is_digit_or_minus(char chr)
{
	return '0' <= chr && chr <= '9' || chr == '-';
}

// returns amount of characters read, the actual number ends up in outNumber
int parse_number(const char* line, int& outNumber)
{
	char number_string[32];	// 32 is enough, as int has 32 bits, so in base 10 it's not gonna take 32 characters for sure
	int number_string_length = 0;

	// read the number into string
	while (is_digit_or_minus(*line))
	{
		number_string[number_string_length] = *line;
		number_string_length++;
		line++;
	}

	// add \0 to make it a c-string
	number_string[number_string_length] = '\0';

	// convert c-string to integer
	outNumber = atoi(number_string);

	return number_string_length;
}

struct SensorEntry
{
	int sensor_x;
	int sensor_y;
	int beacon_x;
	int beacon_y;
};

// returns number of characters read, the number is saved into outNumber
bool read_input(FILE* file, SensorEntry& outEntry)
{
	char line[128];	// 128 is enough (we know the input line format)

	// read single line until new line character
	if (!fgets(line, 128, file))
		return false;

	const char* ptr = line + 12;	// because "Sensor at x=" has 12 characters
	ptr += parse_number(ptr, outEntry.sensor_x);

	ptr += 4;		// because ", y=" has 4 characters
	ptr += parse_number(ptr, outEntry.sensor_y);

	ptr += 25;	// because ": closest beacon is at x=" has 12 characters
	ptr += parse_number(ptr, outEntry.beacon_x);

	ptr += 4;		// because ", y=" has 4 characters
	ptr += parse_number(ptr, outEntry.beacon_y);

	return true;
}

struct Section
{
	int x;
	int length;
};

int compare_sections_func(const void* a, const void* b)
{
	const int value_a = ((const Section*)a)->x;
	const int value_b = ((const Section*)b)->x;

	if (value_a < value_b)
		return -1;
	if (value_a > value_b)
		return 1;

	return 0;
}

int main()
{
	FILE* file;
	fopen_s(&file, "aoc15.txt", "r");
	const int row_y = 2000000;

	std::vector<Section> sections;
	sections.reserve(32);	// reserve a bit, to avoid realloc

	std::vector<int> beacons_on_row;
	beacons_on_row.reserve(32);	// reserve a bit, to avoid realloc

	// read input lines one by one and extract sections that are overlapping with row of interest
	SensorEntry entry;
	while (read_input(file, entry))
	{
		// get "radius" of the sensed area
		const int radius = abs(entry.sensor_x - entry.beacon_x) + abs(entry.sensor_y - entry.beacon_y);

		// calculate distance to row of interest
		const int distance_to_row = abs(entry.sensor_y - row_y);

		// where would section begin on X relative to sensor_x
		const int section_begin_offset = radius - distance_to_row;

		// calculate what would be the section length (smaller than or equal to zero means no overlapping is happening
		const int section_length = section_begin_offset * 2 + 1;

		// if not overlapping with the row of interest, then skip
		if (section_length <= 0)
			continue;

		// save section
		sections.push_back({ entry.sensor_x - (radius - distance_to_row), section_length });

		// see if there's beacon on the row
		if (entry.beacon_y == row_y)
		{
			// save unique beacons on row
			bool beacon_already_found = false;

			for (int i = 0; i < beacons_on_row.size(); ++i)
			{
				if (beacons_on_row[i] == entry.beacon_x)
				{
					beacon_already_found = true;
					break;
				}
			}

			if(!beacon_already_found)
				beacons_on_row.push_back(entry.beacon_x);
		}
	}

	// just a safety check
	if (sections.empty())
		return 0;

	// sort sections by X starting point
	std::qsort(sections.data(), sections.size(), sizeof(Section), compare_sections_func);

	int final_counter = 0;

	// merge sections
	Section lastMergedSection = sections[0];
	for (int i = 1; i < sections.size(); ++i)
	{
		const int overlap_x = sections[i].x - lastMergedSection.x;
		if (overlap_x < lastMergedSection.length)	// in case the sections overlap
		{
			const int to_end_length = lastMergedSection.length - overlap_x;
			const int diff_in_lengths = sections[i].length - to_end_length;

			// see if we need to extend merged section
			if (diff_in_lengths > 0)
			{
				lastMergedSection.length += diff_in_lengths;
			}
		}
		else // in case the sections do not overlap
		{
			// add last merged section length into the final counter
			final_counter += lastMergedSection.length;

			// start merging next batch of sections
			lastMergedSection = sections[i];
		}
	}

	final_counter += lastMergedSection.length;	// need to add the last merged section, because the last one is never finished inside the for loop
	final_counter -= (int)beacons_on_row.size();

	printf("Answer: %i (beacons on row: %i)\n", final_counter, (int)beacons_on_row.size());

	return 0;
}
