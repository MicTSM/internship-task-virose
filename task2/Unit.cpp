#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "Unit.hpp"

namespace RJ = rapidjson;

    namespace {
bool parseJsonFile(const std::string& path, RJ::Document& doc, std::string& err) {
    std::ifstream ifs(path);
    if (!ifs.is_open()) { err = "Cannot open: " + path; return false; }
    RJ::IStreamWrapper isw(ifs);
    doc.ParseStream(isw);
    if (doc.HasParseError()) { err = "JSON parse error in: " + path; return false; }
    return true;
}
}

bool Unit::load(const std::string& xlRoot, int unitId,
                std::vector<std::vector<int>>& frames, std::string& err) const
{
    frames.clear();
    const std::string path = xlRoot + "/motion_unit/" + std::to_string(unitId) + ".json";

    rapidjson::Document doc;
    if (!parseJsonFile(path, doc, err)) return false;

    if (!doc.HasMember("motion_frame") || !doc["motion_frame"].IsArray()) {
        err = "Missing/invalid 'motion_frame' in " + path;
        return false;
    }

    for (const auto& row : doc["motion_frame"].GetArray()) {
        if (!row.IsArray()) continue;
        std::vector<int> line;
        line.reserve(row.Size());
        for (const auto& v : row.GetArray()) {
            if (v.IsInt())         line.push_back(v.GetInt());
            else if (v.IsNumber()) line.push_back(static_cast<int>(v.GetDouble())); // permissive
            else                   line.push_back(0);
        }
        frames.push_back(std::move(line));
    }
    return true;
}