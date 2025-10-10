#pragma once
#include <string>
#include <vector>
using namespace std;

struct UnitRef {
    int id = -1;    // buat motion_unit/<id>.json
    int repeat = 0; // berapa kali harus mengulang ngeprint frame
};

class Movie {
public:
    // ngeload XL/motion_movie/<movieID>.json dan ngefill unit refs (id, repeat *pengulangan)
    bool load(const string& xlRoot, int movieID,
              vector<UnitRef>& outUnits, string& err) const;
};