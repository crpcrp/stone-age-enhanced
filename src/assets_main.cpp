// stone_assets export SA_V1_00.EXE DIRECTORY   unpack the original's resources into editable files
// stone_assets pack DIRECTORY stoneage.dat     build the package the game loads
// stone_assets verify SA_V1_00.EXE PATH        compare a directory or package with the original
#include "assets.hpp"
#include "machine.hpp"
#include <iostream>
#include <stdexcept>
int main(int argc, char** argv) {
    try {
        const std::string command = argc == 4 ? argv[1] : "";
        if (command == "export")
            stone::export_assets(argv[2], argv[3]);
        else if (command == "pack")
            stone::pack_assets(argv[2], argv[3]);
        else if (command == "verify") {
            stone::verify_assets(argv[2], argv[3]);
            std::cout << "All 37 resources and the program's texts, colours and passwords are "
                         "identical to the original."
                      << std::endl;
        } else
            throw std::runtime_error("Usage: stone_assets export EXE DIRECTORY | pack DIRECTORY "
                                     "PACKAGE | verify EXE PATH");
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << "\n";
        return 1;
    }
}
