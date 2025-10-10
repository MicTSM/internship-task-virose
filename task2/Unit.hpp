#pragma once
#include <string>
#include <vector>
using namespace std;

class Unit {
public:
    // ngeload XL/motion_unit/<unitID>.json dan return output row motion_frame
    bool load(const string& xlRoot, int unitID,
              vector<vector<int>>& frames, string& err) const;
};