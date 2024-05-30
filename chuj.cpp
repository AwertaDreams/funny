#include <SDL.h>
#include <SDL_image.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <windows.h>
#include <filesystem>
#include <vector>
#include <string>
//pok - png
//meow - jpg

namespace fs = std::filesystem;

void renameFileExtensions(const std::string& directory) {
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            fs::path filePath = entry.path();
            std::string extension = filePath.extension().string();
            if (extension == ".meow") {
                fs::path newFilePath = filePath;
                newFilePath.replace_extension(".jpg");
                fs::rename(filePath, newFilePath);
            }
            else if (extension == ".pok") {
                fs::path newFilePath = filePath;
                newFilePath.replace_extension(".png");
                fs::rename(filePath, newFilePath);
            }
        }
    }
}

std::vector<std::string> findImageFiles(const std::string& directory) {
    std::vector<std::string> files;
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (entry.is_regular_file()) {
            std::string extension = entry.path().extension().string();
            if (extension == ".png" || extension == ".jpg" || extension == ".jpeg") {
                files.push_back(entry.path().string());
            }
        }
    }
    return files;
}

void copyFiles(const std::wstring& sourceDir, const std::wstring& destinationDir) {
    try {
        if (!fs::exists(destinationDir)) {
            fs::create_directories(destinationDir);
        }
        for (const auto& entry : fs::directory_iterator(sourceDir)) {
            fs::path destPath = destinationDir / entry.path().filename();
            fs::copy(entry.path(), destPath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);
        }
    }
    catch (const fs::filesystem_error& e) {
        std::cerr << "Error copying files: " << e.what() << std::endl;
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    std::wstring sourceDir = exePath;
    sourceDir = sourceDir.substr(0, sourceDir.find_last_of(L"\\/"));

    std::wstring destinationDir = L"C:\\ProgramData\\Samsung";

    // Rename file extensions in debdir

    // Copy files after renaming
    copyFiles(sourceDir, destinationDir);

    std::string debdir = fs::path(destinationDir).string() + "\\debdir";
    renameFileExtensions(debdir);

    std::vector<std::string> imageFiles = findImageFiles(debdir);

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        MessageBox(NULL, L"SDL could not initialize!", L"Error", MB_ICONERROR | MB_OK);
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        MessageBox(NULL, L"SDL_image could not initialize!", L"Error", MB_ICONERROR | MB_OK);
        SDL_Quit();
        return 1;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    int screenWidth = displayMode.w;
    int screenHeight = displayMode.h;

    std::srand(std::time(nullptr));

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Texture* texture = nullptr;

    while (true) {
        int waitTime = 120 + (std::rand() % 5);
        std::this_thread::sleep_for(std::chrono::seconds(waitTime));

        if (imageFiles.empty()) {
            MessageBox(NULL, L"No image files found in debdir!", L"Error", MB_ICONERROR | MB_OK);
            break;
        }

        std::string randomImage = imageFiles[std::rand() % imageFiles.size()];
        SDL_Surface* image = IMG_Load(randomImage.c_str());
        if (!image) {
            MessageBox(NULL, L"Unable to load image!", L"Error", MB_ICONERROR | MB_OK);
            continue;
        }

        window = SDL_CreateWindow("Fullscreen Image", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_FULLSCREEN);
        if (!window) {
            MessageBox(NULL, L"Window could not be created!", L"Error", MB_ICONERROR | MB_OK);
            SDL_FreeSurface(image);
            continue;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            MessageBox(NULL, L"Renderer could not be created!", L"Error", MB_ICONERROR | MB_OK);
            SDL_DestroyWindow(window);
            SDL_FreeSurface(image);
            continue;
        }

        texture = SDL_CreateTextureFromSurface(renderer, image);
        SDL_FreeSurface(image);
        if (!texture) {
            MessageBox(NULL, L"Unable to create texture!", L"Error", MB_ICONERROR | MB_OK);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            continue;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
    }

    IMG_Quit();
    SDL_Quit();

    return 0;
}
