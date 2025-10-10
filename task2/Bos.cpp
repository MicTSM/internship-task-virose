#include <iostream>
#include <string>
#include <vector>
#include "Movie.hpp"
#include "Unit.hpp"

int main() {
    int movieId;
    if (!(std::cin >> movieId)) {
        std::cerr << "Invalid movie id\n";
        return 1;
    }

    const std::string xlRoot = "../../XL";

    std::vector<UnitRef> refs;
    std::string err;
    Movie mv;
    if (!mv.load(xlRoot, movieId, refs, err)) {
        std::cerr << "Movie error: " << err << "\n";
        return 2;
    }

    Unit un;
    for (const auto& r : refs) {
        std::vector<std::vector<int>> frames;
        if (!un.load(xlRoot, r.id, frames, err)) {
            std::cerr << "Unit " << r.id << " error: " << err << "\n";
            continue;
        }

        for (int k = 0; k < r.repeat; ++k) {
            std::cout << r.id << "\n";  
            for (const auto& row : frames) {
                for (size_t j = 0; j < row.size(); ++j)
                    std::cout << row[j] << (j + 1 < row.size() ? ' ' : '\n');
            }
        }
    }
    return 0;
}
