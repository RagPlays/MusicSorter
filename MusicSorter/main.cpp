#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <cassert>
#include <windows.h>
#include <regex>

enum class ErrorType
{
    NON_REGULAR_FILE,
    EXEPTIONTHROW,
    WRONG_EXTENTION,
    MULTIPLE_SPACES,
    WRONG_FORMAT,
    UNSUPPORTED_CHARACTERS
};

struct Error
{
    explicit Error(ErrorType errorType, const std::string& filename)
        : errorType{ errorType }
        , filename{ filename }
    {
    }
    explicit Error(ErrorType errorType)
        : errorType{ errorType }
        , filename{ std::string{} }
    {
        switch (errorType)
        {
        case ErrorType::NON_REGULAR_FILE:
        case ErrorType::EXEPTIONTHROW:
        default:
            // Can not be right need to use Error(ErrorType errorType, const std::string& filename) instead
            assert(false);
            break;
        }
    }
    ErrorType errorType;
    std::string filename;
};

struct FolderData
{
    size_t readableFileCount{};
    size_t folderCount{};
    size_t unreadableFileCount{};
    size_t hiddenFileCount{};
};

std::vector<Error> g_Errors{};
const std::regex g_MultiSpace{ "  +" };

namespace fs = std::filesystem;

void SortMusicFiles(const fs::path& path, bool isTestRun);
void PrintFolderInfo(const FolderData& folderData);
void PrintErrorLog();
void EnableVirtualTerminalProcessing();
void ClearConsole();
void PrintRegularText(const std::string& text);
void PrintGreenText(const std::string& text);
void PrintRedText(const std::string& text);
void PrintLightBlueText(const std::string& text);
void PrintOrderCompleted(const std::string& filename);
void PrintOrderFailed(const std::string& filename);
void PrintWrongExtention(const std::string& filename);
void PrintOrderFailedException(const std::string& execption);
const std::string GetDirectory();
bool GetYesNo(const std::string& question);
bool IsError(fs::directory_entry entry, const std::string& filename, size_t dashPos);
bool IsHiddenFile(const fs::path& p);
FolderData GetFileData(const fs::path& path);

void SortMusicFiles(const fs::path& path, bool isTestRun)
{
    if (isTestRun)
    {
        PrintLightBlueText(" ---- TestRun is starting ----\n");
    }
    else
    {
        PrintLightBlueText(" ---- Sorting is starting ----\n");
    }

    size_t nrFilesSorted{};
    size_t nrFilesNotSorted{};

    for (const auto& entry : fs::directory_iterator(path))
    {
        // dont go into folders
        if (entry.is_directory()) continue;

        // skip hidden files
        if (IsHiddenFile(entry.path())) continue;

        if (!fs::is_regular_file(entry.status()))
        {
            PrintRedText("...No regular entry...\n");
            PrintRedText("There is a file that is not a regular file.\n");
            g_Errors.emplace_back(Error{ ErrorType::NON_REGULAR_FILE });
            ++nrFilesNotSorted;
            continue;
        }

        try
        {
            const std::string filename{ entry.path().filename().string() };
            std::cout << "Sorting: " << filename << "\n";
            const std::size_t dashPos{ filename.find('-') };

            // Checks
            if (IsError(entry, filename, dashPos))
            {
                PrintOrderFailed(filename);
                ++nrFilesNotSorted;
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
            ++nrFilesSorted;
        }
        catch (const std::exception& exception)
        {
            PrintOrderFailedException(std::string{ exception.what() });
            g_Errors.emplace_back(Error{ ErrorType::EXEPTIONTHROW });
            ++nrFilesNotSorted;
        }
    }

    PrintLightBlueText("\n\n --------------- Done ---------------\n");

    PrintLightBlueText(" Total sorting attemps: ");
    std::cout << std::to_string(nrFilesSorted + nrFilesNotSorted) << "\n";

    PrintLightBlueText(" Sorting completed: ");
    PrintGreenText(std::to_string(nrFilesSorted) + "\n");

    PrintLightBlueText(" Sorting failed: ");
    PrintRedText(std::to_string(nrFilesNotSorted) + "\n\n");
}

void PrintFolderInfo(const FolderData& folderData)
{
    PrintLightBlueText(" ------------ FolderInfo ------------\n");
    PrintLightBlueText(" Folders found: ");
    PrintRegularText(std::to_string(folderData.folderCount) + "\n");
    PrintLightBlueText(" Readable files found: ");
    PrintRegularText(std::to_string(folderData.readableFileCount) + "\n");
    PrintLightBlueText(" Unreadable filed found: ");
    PrintRegularText(std::to_string(folderData.unreadableFileCount) + "\n");
    PrintLightBlueText(" Hidden files found: ");
    PrintRegularText(std::to_string(folderData.hiddenFileCount) + " (these files are ignored)\n\n");
}

void PrintErrorLog()
{
    if (g_Errors.empty()) return;

    if (!GetYesNo("Do you want a error log?")) return;

    PrintLightBlueText("\n ------------- ErrorLog -------------\n");

    size_t nrNonRegularFiles{};
    size_t nrExecptionsThrown{};

    for (const auto& error : g_Errors)
    {
        switch (error.errorType)
        {
        case ErrorType::NON_REGULAR_FILE:
            ++nrNonRegularFiles;
            break;
        case ErrorType::EXEPTIONTHROW:
            ++nrExecptionsThrown;
            break;
        case ErrorType::WRONG_EXTENTION:
            PrintRedText("WRONG FILE EXTENTION::" + error.filename + "\n");
            break;
        case ErrorType::MULTIPLE_SPACES:
            PrintRedText("MULTIPLE SPACES::" + error.filename + "\n");
            break;
        case ErrorType::WRONG_FORMAT:
            PrintRedText("WRONG FORMAT EXTENTION::" + error.filename + "\n");
            break;
        case ErrorType::UNSUPPORTED_CHARACTERS:
            PrintRedText("UNSUPPORTED CHARACTERS::" + error.filename + "\n");
            break;
        default:
            PrintRegularText("There was a unknown error.\n");
            break;
        }
    }

    if (nrNonRegularFiles)
    {
        PrintRedText("There where ");
        PrintRegularText(std::to_string(nrNonRegularFiles));
        PrintRedText(" non-regular files found.\n");
    }
    if (nrExecptionsThrown)
    {
        PrintRedText("There where ");
        PrintRegularText(std::to_string(nrExecptionsThrown));
        PrintRedText(" exceptions thrown. (undefined error)\n");
    }

    g_Errors.clear();
}

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

void ClearConsole()
{
    std::cout << "\033c";
}

void PrintRegularText(const std::string& text)
{
    std::cout << text;
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

void PrintWrongExtention(const std::string& filename)
{
    PrintRedText("Extention not supported: " + filename + "\n");
}

void PrintOrderFailedException(const std::string& execptionWhat)
{
    PrintRedText("Caught exception: " + execptionWhat + "\t File will be skipped!\n");
}

const std::string GetDirectory()
{
    std::string musicDir{};
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

    PrintRegularText("\n");
    return musicDir;
}

bool GetYesNo(const std::string& question)
{
    PrintLightBlueText(question);
    PrintRegularText(" Y/N\n");
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

    PrintRegularText("\n");

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

bool IsError(fs::directory_entry entry, const std::string& filename, size_t dashPos)
{
    for (char c : filename)
    {
        if (!std::isprint(static_cast<unsigned char>(c)))
        {
            g_Errors.emplace_back(Error{ ErrorType::UNSUPPORTED_CHARACTERS, filename });
            return true;
        }
    }

    bool isThereError{ false };
    if (entry.path().extension() != ".mp3")
    {
        isThereError = true;
        g_Errors.emplace_back(Error{ ErrorType::WRONG_EXTENTION, filename });
    }
    else if (dashPos == std::string::npos)
    {
        isThereError = true;
        g_Errors.emplace_back(Error{ ErrorType::WRONG_FORMAT, filename });
    }
    else if (std::regex_search(filename, g_MultiSpace))
    {
        isThereError = true;
        g_Errors.emplace_back(Error{ ErrorType::MULTIPLE_SPACES, filename });
    }
    return isThereError;
}

bool IsHiddenFile(const fs::path& p)
{
    DWORD attributes{ GetFileAttributesW(p.c_str()) };

    return (attributes != INVALID_FILE_ATTRIBUTES &&
        (attributes & FILE_ATTRIBUTE_HIDDEN || attributes & FILE_ATTRIBUTE_SYSTEM));
}

FolderData GetFileData(const fs::path& path)
{
    FolderData folderData{};
    for (const auto& entry : fs::directory_iterator(path))
    {
        if (entry.is_directory())
        {
            ++folderData.folderCount;
            continue;
        }

        if (!fs::is_regular_file(entry.status()))
        {
            ++folderData.unreadableFileCount;
            continue;
        }

        if (IsHiddenFile(entry.path()))
        {
            ++folderData.hiddenFileCount;
            continue;
        }

        ++folderData.readableFileCount;
    }

    return folderData;
}

int main()
{
    EnableVirtualTerminalProcessing();

    const fs::path dir{ GetDirectory() };
    const bool isTestRun{ GetYesNo("Do you want to do a file test run?")};

    FolderData folderData{ GetFileData(dir) };
    g_Errors.reserve(folderData.readableFileCount / 3);

    SortMusicFiles(dir, isTestRun);

    PrintFolderInfo(folderData);

    PrintErrorLog();

    std::cin.get();
    return 0;
}