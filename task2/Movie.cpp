#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include "Movie.hpp"

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


bool Movie::load(const std::string& xlRoot, int movieId,
                 std::vector<UnitRef>& outUnits, std::string& err) const
{
    outUnits.clear();
    const std::string path = xlRoot + "/motion_movie/" + std::to_string(movieId) + ".json";

    rapidjson::Document doc;
    if (!parseJsonFile(path, doc, err)) return false;

    if (!doc.HasMember("motion_unit") || !doc["motion_unit"].IsArray()) {
        err = "Missing/invalid 'motion_unit' in " + path;
        return false;
    }

    for (const auto& entry : doc["motion_unit"].GetArray()) {
        if (!entry.IsObject()) continue;
        UnitRef r;
        if (entry.HasMember("id") && entry["id"].IsInt())     r.id = entry["id"].GetInt();
        if (entry.HasMember("loop") && entry["loop"].IsInt()) r.repeat = entry["loop"].GetInt();
        if (r.id >= 0 && r.repeat >= 0) outUnits.push_back(r);
    }

    if (outUnits.empty()) {
        err = "No valid unit references found in movie file.";
        return false;
    }
    return true;
}