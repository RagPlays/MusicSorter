#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <cassert>
#include <windows.h>

namespace fs = std::filesystem;

void EnableVirtualTerminalProcessing()
{
    // Get the handle to the standard output device.
    HANDLE hOut{ GetStdHandle(STD_OUTPUT_HANDLE) };

    // Retrieve the current console mode.
    DWORD dwMode{};
    GetConsoleMode(hOut, &dwMode);

    // Enable the virtual terminal processing mode.
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void ClearConsole();
void PrintGreenText(const std::string& text);
void PrintRedText(const std::string& text);
void PrintLightBlueText(const std::string& text);
void PrintOrderCompleted(const std::string& filename);
void PrintOrderFailed(const std::string& filename);
void PrintOrderFailedException(const std::string& execption);

const std::string GetDirectory();
bool GetIsTestRun();

void SortMusicFiles(const fs::path& path, bool isTestRun)
{
    int filesSorted{};
    int filesNotSorted{};

    for (const auto& entry : fs::directory_iterator(path))
    {
        // dont go into folders
        if (entry.is_directory()) continue;

        if (!fs::is_regular_file(entry.status()))
        {
            PrintRedText("...No regular entry...\n");
            PrintRedText("There is a file that is not a regular file.\n");
            continue;
        }

        try
        {
            const std::string filename{ entry.path().filename().string() };
            std::cout << "Ordering: " << filename << "\n";
            const std::size_t dashPos{ filename.find('-') };

            if (dashPos == std::string::npos || fs::exists(filename))
            {
                PrintOrderFailed(filename);
                ++filesNotSorted;
                continue;
            }

            std::string artistName{ filename.substr(0, dashPos) };

            // Trim spaces at both ends and replace remaining spaces with underscores
            auto left{ std::find_if_not(artistName.begin(), artistName.end(), ::isspace) };
            auto right{ std::find_if_not(artistName.rbegin(), artistName.rend(), ::isspace).base() };
            artistName = std::string{ left, right };
            std::replace(artistName.begin(), artistName.end(), ' ', '_');

            const fs::path artistDir{ path / artistName };

            if (!isTestRun)
            {
                if (!fs::exists(artistDir))
                {
                    fs::create_directory(artistDir);
                }
                fs::rename(entry.path(), artistDir / filename);
            }

            PrintOrderCompleted(filename);
            ++filesSorted;
        }
        catch (const std::exception& exception)
        {
            PrintOrderFailedException(std::string{ exception.what() });
            ++filesNotSorted;
        }
    }

    PrintLightBlueText("\n\n --------------- Done ---------------\n");

    PrintLightBlueText("Total ordering attemps: ");
    std::cout << std::to_string(filesSorted + filesNotSorted) << "\n";

    PrintLightBlueText("Orderings completed: ");
    PrintGreenText(std::to_string(filesSorted) + "\n");

    PrintLightBlueText("Orderings failed: ");
    PrintRedText(std::to_string(filesNotSorted) + "\n");
}

void ClearConsole()
{
    std::cout << "\033c";
}

void PrintGreenText(const std::string& text)
{
    // Set Color Green
    std::cout << "\033[1;32m";
    // Print Text
    std::cout << text;
    // Reset Color
    std::cout << "\033[0m";
}

void PrintRedText(const std::string& text)
{
    // Set Color Red
    std::cout << "\033[1;31m";
    // Print Text
    std::cout << text;
    // Reset Color
    std::cout << "\033[0m";
}

void PrintLightBlueText(const std::string& text)
{
    // Set Color LightBlue
    std::cout << "\033[1;34m";
    // Print Text
    std::cout << text;
    // Reset Color
    std::cout << "\033[0m";
}

void PrintOrderCompleted(const std::string& filename)
{
    PrintGreenText("Ordering completed with file: " + filename + "\n");
}

void PrintOrderFailed(const std::string& filename)
{
    PrintRedText("Ordering failed with file: " + filename + "\n");
}

void PrintOrderFailedException(const std::string& execptionWhat)
{
    PrintRedText("Caught exception: " + execptionWhat + "\t File will be skipped!\n");
}

const std::string GetDirectory()
{
    std::string musicDir;
    PrintLightBlueText("Enter the path to your music directory: \n");
    do
    {
        std::getline(std::cin, musicDir);
        if (fs::exists(musicDir) && musicDir.size() > 1)
        {
            break;
        }
        else
        {
            PrintRedText("Path incorrect, fill in existing pathname.\n");
        }
    } while (true);

    std::cout << "\n";
    return musicDir;
}

bool GetIsTestRun()
{
    PrintLightBlueText("Do you want to do a file test run? ");
    std::cout << "Y/N\n";
    std::string input;
    do
    {
        std::getline(std::cin, input);

        if (input.size() < 1) continue;

        if (input != "Y" && input != "y" && input != "N" && input != "n")
        {
            PrintRedText("Invalid input. Please try again.\n");
        }
    }
    while (input != "Y" && input != "y" && input != "N" && input != "n");

    std::cout << "\n";

    switch (input[0])
    {
    case 'Y':
    case 'y':
        return true;
        break;
    case 'N':
    case 'n':
        return false;
        break;
    default:
        assert(false);
        throw std::exception{};
        break;
    }
}

int main()
{
    EnableVirtualTerminalProcessing();

    const std::string dir{ GetDirectory() };
    bool isTestRun{ GetIsTestRun() };

    if (isTestRun)
    {
        PrintLightBlueText(" ---- TestRun is starting ----\n");
    }
    else
    {
        PrintLightBlueText(" ---- Sorting is starting ----\n");
    }

    SortMusicFiles(dir, isTestRun);

    std::cin.get();
    return 0;
}