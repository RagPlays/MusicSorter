#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>

namespace fs = std::filesystem;

void SortMusicFiles(const fs::path& path)
{
    for (const auto& entry : fs::directory_iterator(path))
    {
        try
        {
            std::string filename = entry.path().filename().string();
            std::size_t dashPos = filename.find('-');

            if (dashPos != std::string::npos)
            {
                std::string artistName = filename.substr(0, dashPos);

                // Trim spaces at both ends
                artistName.erase(
                    artistName.begin(),
                    std::find_if(artistName.begin(), artistName.end(),
                        [](unsigned int ch)
                        {
                            return !std::isspace(ch);
                        }
                    )
                );

                artistName.erase(
                    std::find_if(
                        artistName.rbegin(), artistName.rend(),
                        [](unsigned int ch)
                        {
                            return !std::isspace(ch);
                        }
                ).base(), artistName.end());

                // Replace remaining spaces with underscores
                std::replace(artistName.begin(), artistName.end(), ' ', '_');

                fs::path artistDir = path / artistName;
                if (!fs::exists(artistDir))
                {
                    fs::create_directory(artistDir);
                }

                fs::rename(entry.path(), artistDir / filename);
            }
            else
            {
                std::cout << "File not following 'Artist Name - Song Name' format: " << filename << "\n";
            }
        }
        catch (const std::system_error& exception)
        {
            std::cout << "Caught exception: " << exception.what() << "\t\t File will be skipped!\n";
        }
    }
}

int main()
{
    std::string musicDir;
    std::cout << "Enter the path to your music directory: \n";
    std::getline(std::cin, musicDir);

    SortMusicFiles(musicDir);

    return 0;
}
