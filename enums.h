#include <sstream>
#include <iostream>
#include <vector>

#ifndef __Enums__
#define __Enums__
std::string formatted_string(int i){
    std::stringstream ss;
    if (i < 10)
        ss << "  " << i;
    else if (i < 100)
        ss << " " << i;
    else
        ss << i;
    return ss.str();
}
/// Parses relative filepath and returns just the filename at the end
std::string get_filename(std::string filepath){
    std::vector<std::string> components = std::vector<std::string>();
    std::string delimiter = "/";
    size_t pos = 0;
    std::string token;
    while ((pos = filepath.find(delimiter)) != std::string::npos) {
        token = filepath.substr(0, pos);
        components.push_back(token);
        filepath.erase(0, pos + delimiter.length());
    }
    return filepath;
}

/// Separates the filename into a list of components we can parse
std::vector<std::string> get_filename_components(std::string filename){
    std::vector<std::string> components = std::vector<std::string>();
    std::string delimiter = ".";
    size_t pos = 0;
    std::string token;
    while ((pos = filename.find(delimiter)) != std::string::npos) {
        token = filename.substr(0, pos);
        components.push_back(token);
        filename.erase(0, pos + delimiter.length());
    }
    return components;
}
    
#pragma region PixelType
/// What type of pixel
enum PixelType{
    RED = 0,
    GREEN = 1,
    BLUE = 2,
};

std::string decscription_of(PixelType layer){
    std::string channelDescription = "NONE_ERROR";
    if (layer == RED)
        channelDescription = "RED";
    else if (layer == GREEN)
        channelDescription = "GREEN";
    else if (layer == BLUE)
        channelDescription = "BLUE";
    
    return channelDescription;
}

#pragma endregion

#pragma region ImageSection

/// Section of the image
enum ImageSection{
    TOP_LEFT = 0,
    TOP_MID = 1,
    TOP_RIGHT = 2,
    BOTTOM_LEFT = 3,
    BOTTOM_MID = 4,
    BOTTOM_RIGHT = 5,
};


std::string decscription_of(ImageSection section){

    std::string section_x_desc = "NONE_X";
    std::string section_y_desc = "NONE_Y";
    if (section == TOP_LEFT || section == BOTTOM_LEFT){
        section_x_desc = "LEFT";
    }else if (section == TOP_MID || section == BOTTOM_MID){
        section_x_desc = "MID";
    }else if (section == TOP_RIGHT || section == BOTTOM_RIGHT){
        section_x_desc = "RIGHT";
    }

    if (section == TOP_LEFT || section == TOP_MID || section == TOP_RIGHT){
        section_y_desc = "TOP";
    }else if (section == BOTTOM_LEFT || section == BOTTOM_MID || section == BOTTOM_RIGHT){
        section_y_desc = "BOTTOM";
    }

    std::stringstream ss;
    ss << section_y_desc << "-" << section_x_desc;
    return ss.str();
}
#pragma endregion

#pragma region Weather

/// What the weather is (Category 2)
enum Weather{
    SUNNY = 0,
    CLOUDY = 1,
    NIGHT = 2,
    RAIN = 3,
    SNOW = 4,
    FOGGY = 5,
};

std::string abbreviation_for(Weather w){
    switch (w){
    case SUNNY: return "su";
    case CLOUDY: return "cd";
    case NIGHT: return "nt";
    case RAIN: return "rn";
    case SNOW: return "sn";
    case FOGGY: return "fg";
    default: return "unknown";
    }
}

std::string decscription_of(Weather w){
    switch (w){
    case SUNNY: return "sunny";
    case CLOUDY: return "cloudy";
    case NIGHT: return "night";
    case RAIN: return "rain";
    case SNOW: return "snow";
    case FOGGY: return "foggy";
    default: return "unknown";
    }
}

Weather weather_from_abbreviation(std::string abbreviation){
    if (abbreviation.compare(abbreviation_for(SUNNY)) == 0)
        return SUNNY;
    else if (abbreviation.compare(abbreviation_for(CLOUDY)) == 0)
        return CLOUDY;
    else if (abbreviation.compare(abbreviation_for(NIGHT)) == 0)
        return NIGHT;
    else if (abbreviation.compare(abbreviation_for(RAIN)) == 0)
        return RAIN;
    else if (abbreviation.compare(abbreviation_for(SNOW)) == 0)
        return SNOW;
    else if (abbreviation.compare(abbreviation_for(FOGGY)) == 0)
        return FOGGY;
    else{
        std::cout << "ERROR PARSING WEATHER! (abbreviation: `" << abbreviation << "`)\n";
        return SUNNY; // Just return sunny by default.. shouldnt ever get this
    }
}

/// Weather parsed from file name
Weather get_weather(std::string filepath){
    std::vector<std::string> filename_components = get_filename_components(filepath);
    std::string second_component = filename_components.at(1);
    return weather_from_abbreviation(second_component);
}

#pragma endregion

#pragma region  Illumination

/// Illumination conditions in image (Category 3)
enum Illumination{
    SHADOW = 0,
    BACK_TO_SUN = 1,
    FACING_SUN = 2,
    DIRECT_LIGHT = 3,
    DARK = 4,
    PARTLY_CLOUDY = 5,
    OVERCAST = 6,
    STREETLIGHT = 7,
    HEADLIGHT_ONLY = 8,
};
std::string abbreviation_for(Illumination i){
    switch (i){
    case SHADOW: return "SD";
    case BACK_TO_SUN: return "BS";
    case FACING_SUN: return "FS";
    case DIRECT_LIGHT: return "DL";
    case DARK: return "DK";
    case PARTLY_CLOUDY: return "PC";
    case OVERCAST: return "OC";
    case STREETLIGHT: return "SL";
    case HEADLIGHT_ONLY: return "HO";
    default: return "unknown";
    }
}

std::string decscription_of(Illumination i){
    switch (i){
    case SHADOW: return "shadow";
    case BACK_TO_SUN: return "back-to-sun";
    case FACING_SUN: return "facing-sun";
    case DIRECT_LIGHT: return "direct-light";
    case DARK: return "dark";
    case PARTLY_CLOUDY: return "partly-cloudy";
    case OVERCAST: return "overcast";
    case STREETLIGHT: return "streetlight";
    case HEADLIGHT_ONLY: return "headlight-only";
    default: return "unknown";
    }
}

Illumination illumination_from_abbreviation(std::string abbreviation){
    if (abbreviation.compare(abbreviation_for(SHADOW)) == 0)
        return SHADOW;
    else if (abbreviation.compare(abbreviation_for(BACK_TO_SUN)) == 0)
        return BACK_TO_SUN;
    else if (abbreviation.compare(abbreviation_for(FACING_SUN)) == 0)
        return FACING_SUN;
    else if (abbreviation.compare(abbreviation_for(DIRECT_LIGHT)) == 0)
        return DIRECT_LIGHT;
    else if (abbreviation.compare(abbreviation_for(DARK)) == 0)
        return DARK;
    else if (abbreviation.compare(abbreviation_for(PARTLY_CLOUDY)) == 0)
        return PARTLY_CLOUDY;
    else if (abbreviation.compare(abbreviation_for(OVERCAST)) == 0)
        return OVERCAST;
    else if (abbreviation.compare(abbreviation_for(STREETLIGHT)) == 0)
        return STREETLIGHT;
    else if (abbreviation.compare(abbreviation_for(HEADLIGHT_ONLY)) == 0)
        return HEADLIGHT_ONLY;
    else{
        std::cout << "ERROR PARSING ILLUMINATION! (abbreviation: `" << abbreviation << "`)\n";
        return SHADOW; // Return shadow by default
    }
}

/// Illumination parsed from file name
Illumination get_illumination(std::string filepath){
    std::vector<std::string> filename_components = get_filename_components(filepath);
    std::string third_component = filename_components.at(2);
    return illumination_from_abbreviation(third_component);
}

#pragma endregion

#pragma region Environment

/// Additional items in the environment like Road, lane and grass
enum Environment{
    HIGHWAY = 0,
    RESIDENTIAL = 1,
    URBAN = 2,
    SUBURBAN = 3,
    FARM_AND_FOREST = 4,
    PARKING_LOT = 5,
    WET = 6,
    SNOW_COVERED = 7,
    BARRIER = 8,
};

std::string abbreviation_for(Environment e){
    switch (e){
    case HIGHWAY: return "h";
    case RESIDENTIAL: return "r";
    case URBAN: return "u";
    case SUBURBAN: return "s";
    case FARM_AND_FOREST: return "f";
    case PARKING_LOT: return "p";
    case WET: return "w";
    case SNOW_COVERED: return "c";
    case BARRIER: return "b";
    default: return "unknown";
    }
}

std::string decscription_of(Environment e){
    switch (e){
    case HIGHWAY: return "highway";
    case RESIDENTIAL: return "residential";
    case URBAN: return "urban";
    case SUBURBAN: return "suburban";
    case FARM_AND_FOREST: return "farm-and-forest";
    case PARKING_LOT: return "parking-lot";
    case WET: return "wet";
    case SNOW_COVERED: return "snow-covered";
    case BARRIER: return "barrier";
    default: return "unknown";
    }
}

Environment environment_from_abbreviation(std::string abbreviation){
    if (abbreviation.compare(abbreviation_for(HIGHWAY)) == 0)
        return HIGHWAY;
    else if (abbreviation.compare(abbreviation_for(RESIDENTIAL)) == 0)
        return RESIDENTIAL;
    else if (abbreviation.compare(abbreviation_for(URBAN)) == 0)
        return URBAN;
    else if (abbreviation.compare(abbreviation_for(SUBURBAN)) == 0)
        return SUBURBAN;
    else if (abbreviation.compare(abbreviation_for(FARM_AND_FOREST)) == 0)
        return FARM_AND_FOREST;
    else if (abbreviation.compare(abbreviation_for(PARKING_LOT)) == 0)
        return PARKING_LOT;
    else if (abbreviation.compare(abbreviation_for(WET)) == 0)
        return WET;
    else if (abbreviation.compare(abbreviation_for(SNOW_COVERED)) == 0)
        return SNOW_COVERED;
    else if (abbreviation.compare(abbreviation_for(BARRIER)) == 0)
        return BARRIER;
    else{
        std::cout << "ERROR PARSING ENVIRONMENT! (abbreviation: `" << abbreviation << "`)\n";
        return HIGHWAY; // Return highway by default
    }
}


/// Environment parsed from file name
Environment get_environment(std::string filepath){
    std::vector<std::string> filename_components = get_filename_components(filepath);
    std::string fourth_component = filename_components.at(3);
    return environment_from_abbreviation(fourth_component);
}

#pragma endregion

#endif //  __Enums__