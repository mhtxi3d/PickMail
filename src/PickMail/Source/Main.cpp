/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include <algorithm>
#include <iterator>
#include <random>

// Change this macro to 1 will enable tests written with Catch2
#define ENABLE_TESTS			0

#if ENABLE_TESTS
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#endif // ENABLE_TESTS

using namespace std;

File GetTestFile()
{
	return File::getSpecialLocation(File::hostApplicationPath)
		.getParentDirectory()
		.getChildFile("small.csv");
}

File GetOutputFile()
{
	return File::getSpecialLocation(File::hostApplicationPath)
		.getParentDirectory()
		.getChildFile("output.txt");
}

StringArray GetEmailAddresses(const File& file)
{
	auto raw = StringArray::fromLines(file.loadFileAsString());
	raw.remove(0);
	return raw;
}

auto GetEmailToPick(const File& csv_file) -> std::vector<std::string>
{
	const auto raw_list = GetEmailAddresses(csv_file);

	std::vector<std::string> emails;
	std::transform(begin(raw_list), end(raw_list),
				   std::back_inserter(emails),
				   [](const auto& line)
	{
		return line.upToFirstOccurrenceOf(",", false, false).toStdString();
	});
	
	return emails;
}

auto GetShuffledEmailToPick(const File& csv_file)
{
	std::vector<std::string> emails = GetEmailToPick(csv_file);

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(begin(emails), end(emails), g);

	return emails;
}

auto GetShuffledEmailToPick(const File& csv_file, int count) -> std::vector<std::string>
{
	const auto emails = GetShuffledEmailToPick(csv_file);
	
	if (emails.size() < count)
		return emails;

	return {begin(emails), begin(emails) + count};
}

void SaveEmailToFile(const std::vector<std::string>& emails, const File& file_path)
{
	std::unique_ptr<FileOutputStream> fos(file_path.createOutputStream());
	for (const auto& e : emails) {
		*fos << e.c_str() << "\n";
	}
}

#if ENABLE_TESTS
TEST_CASE("Read email address from file")
{
	const auto csv_file = GetTestFile();
	CHECK(csv_file.existsAsFile());

	REQUIRE(GetEmailAddresses(csv_file).size() > 0);
}

TEST_CASE("Transform raw data to email only container")
{
	const auto raw_list = GetEmailAddresses(GetTestFile());
	CHECK(!raw_list.isEmpty());

	const std::vector<std::string> emails = GetEmailToPick(GetTestFile());

	REQUIRE(!emails.empty());
	REQUIRE(emails.size() == raw_list.size());

	for (const auto& e : emails)
		printf("%s\n", e.c_str());
}

TEST_CASE("Shuffle vecotr of string")
{
	std::vector<std::string> emails = GetShuffledEmailToPick(GetTestFile());

	printf("SHUFFLED Emails:\n");
	for (const auto& e : emails)
		printf("\t%s\n", e.c_str());
}

TEST_CASE("Get specific number of emails from shuffled email list")
{
	SECTION("The list has more emails then asked") {
		const auto emails = GetShuffledEmailToPick(GetTestFile(), 3);
		REQUIRE(emails.size() == 3);
	}

	SECTION("The list has less emails then asked") {
		const auto emails = GetShuffledEmailToPick(GetTestFile(), 6);
		REQUIRE(emails.size() == 4);
	}
}

TEST_CASE("Save email list to file")
{
	auto output_file = GetOutputFile();
	output_file.deleteFile();

	const auto emails = GetShuffledEmailToPick(GetTestFile(), 3);
	CHECK(!emails.empty());

	SaveEmailToFile(emails, output_file);
	REQUIRE(output_file.existsAsFile());

	auto r = StringArray::fromLines(output_file.loadFileAsString());
	REQUIRE(std::includes(begin(emails), end(emails), begin(r), end(r)));
}

#else // ENABLE_TESTS

int main(int argc, char* argv[])
{
	if (argc < 2) {
		printf("Usage: pickmail.exe {csv file} [email count]\n");
		return -1;
	}

	const auto csv_file = File(argv[1]);
	if (!csv_file.existsAsFile()) {
		printf("Error: the csv file not eixsts.\n");
		return -1;
	}

	int email_count = 3;
	if (argc > 2)
		email_count = atoi(argv[2]);

	auto emails = GetShuffledEmailToPick(csv_file, email_count);

	printf("Here are the %d lucky subscribers:\n", email_count);
	for (const auto& e : emails) {
		printf("\t%s\n", e.c_str());
	}

	auto result_file = File::getSpecialLocation(File::hostApplicationPath)
		.getParentDirectory()
		.getChildFile("picked_emails.txt");
	result_file.deleteFile();

	SaveEmailToFile(emails, result_file);

	return 0;
}
#endif // ENABLE_TESTS
