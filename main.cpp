#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include "math.h"
#include <vector>
#include <dirent.h>
#include <sys/stat.h>
#include "enums.h" // Custom Enum defintions and other helper functions
#include "image.h" // Custom images definistions
#include <map>


#ifndef  VERBOSE_LOGGING
#define VERBOSE_LOGGING 0
#endif // ! VERBOSE_LOGGING

#define GREEN_OUTPUT "\033[1;32m"
#define YELLOW_OUTPUT "\033[1;33m"
#define PURPLE_OUTPUT "\033[1;35m"
#define CYAN_OUTPUT "\033[1;36m"
#define RED_OUTPUT "\033[1;31m"
#define RESET_OUTPUT "\033[0m"

/// Set this to 1 when you run to print extra log statements (for debugging.. program runs much slower if we are printing out a bunch of stuff.)
#define VERBOSE_LOGGING 0

// /// Tracks the averate chroma for the dataset by weather
// int averageChromaByWeather[6];
// /// Tracks the averate chroma for the dataset weather illumination
// int averageChromaByIllumination[9];
// /// Tracks the averate Intensity for the dataset by weather
// int averageIntensityByWeather[6];
// /// Tracks the averate Intensity for the dataset weather illumination
// int averageIntensityByIllumination[9];
// /// Tracks the averate Hue for the dataset by weather
// int averageHueByWeather[6];
// /// Tracks the averate Hue for the dataset weather illumination
// int averageHueByIllumination[9];


// Prints the image width and height, tests initialization
void test_print_width_and_height(std::string jpeg_path);

/// Reads in all 1169 images and totals them by thier categories
void test_read_in_all_images(std::vector<std::string>* all_image_paths);

/// Returns a list of files in a directory (except the ones that begin with a dot)
/// NOTE: Got this from https://stackoverflow.com/questions/306533/how-do-i-get-a-list-of-files-in-a-directory-in-c to read in directories - Jasper
void GetFilesInDirectory(std::vector<std::string> &out, const std::string &directory);

/// Main test function right now for testing each type of weather.
void test_images_are_correctly(std::vector<Weather>* weathers_to_check, std::vector<std::string>* image_paths);

void read_in_files_and_write_results(std::vector<std::string>* image_paths);

/// Runs through all the images and fils the info_to_fill with the info about each image.
void gather_all_image_info(std::vector<ImageInfo*>* info_to_fill, std::vector<std::string>* filepaths);

/// Writes all info to a csv file
void write_image_info_to_csv(std::string output_filepath, std::vector<ImageInfo*>* info_to_write);

/// Reads in already generated data from csv
void gather_info_from_csv(std::string csv_path, std::vector<ImageInfo*>* info_to_fill);

/// FInds the k closest neighbors
void nearest_neighbors(int k, ImageInfo* imageInfo, std::vector<ImageInfo*>* allImages, std::vector<ImageInfo*>* nearest_neighbors){
    std::vector<float>* distanceLookup = new std::vector<float>();
    std::vector<ImageInfo*>* neighborsLookup = new std::vector<ImageInfo*>();
    // Calculate distance from all neighbors
    for(int i = 0; i < allImages->size(); i ++){
        ImageInfo* comparedImage = allImages->at(i);

        if (comparedImage->filepath.compare(imageInfo->filepath) == 0){
            continue;
        }
        int hueDiff = imageInfo->hue - comparedImage->hue;
        int chromaDiff = imageInfo->chroma - comparedImage->chroma;
        int contrastDiff = imageInfo->contrast - comparedImage->contrast;
        int intensityDiff = imageInfo->intensity - comparedImage->intensity;
        float distance = sqrt(float(hueDiff^2) + float(chromaDiff^2) + float(contrastDiff^2) + float(intensityDiff^2));
        if (distanceLookup->size() < k){
            distanceLookup->push_back(distance);
            neighborsLookup->push_back(comparedImage);
        }else{
            float max_distance = distanceLookup->at(0);
            int max_index = 0;
            for (int m = 1; m < k; m++){
                if (distanceLookup->at(m) > max_distance){
                    max_index = m;
                    max_distance = distanceLookup->at(0);
                }
            }
            if (max_distance > distance){
                distanceLookup->assign(max_index, distance);
                neighborsLookup->assign(max_index, comparedImage);
            }
        }
    }
    nearest_neighbors->clear();
    for (int i = 0; i < neighborsLookup->size(); i++){
        ImageInfo* neighbor = neighborsLookup->at(i);
        nearest_neighbors->push_back(neighbor);
    }
}

Weather guess_weather_with_knn(std::vector<ImageInfo*>* nearest_neighbors){
    Weather guessedWeather = SUNNY;
    int weatherResults[6];

    for(int i = 0; i < 6; i++){
        weatherResults[i] = 0;
    }
    
    for (std::vector<ImageInfo*>::iterator n = nearest_neighbors->begin(); n != nearest_neighbors->end(); ++n){
        ImageInfo* neighbor = *n.base();
        weatherResults[neighbor->expected_weather] ++;
    }

    for(int i = 0; i < 6; i++){
        if (weatherResults[guessedWeather] < weatherResults[i]){
            guessedWeather = (Weather)i;
        }
    }
    return guessedWeather;
}

Illumination guess_illumination_with_knn(std::vector<ImageInfo*>* nearest_neighbors){
    Illumination guessedIllumination = SHADOW;
    int illuminationResults[9];
    for(int i = 0; i < 9; i++){
        illuminationResults[i] = 0;
    }

    for (std::vector<ImageInfo*>::iterator n = nearest_neighbors->begin(); n != nearest_neighbors->end(); ++n){
        ImageInfo* neighbor = *n.base();
        illuminationResults[neighbor->expected_illumination] ++;
    }

    for(int i = 0; i < 9; i++){
        if (illuminationResults[guessedIllumination] < illuminationResults[i]){
            guessedIllumination = (Illumination)i;
        }
    }
    return guessedIllumination;
}

Environment guess_environment_with_knn(std::vector<ImageInfo*>* nearest_neighbors){
    Environment guessedEnvironment = (Environment)0;
    int environmentResults[6];

    for(int i = 0; i < 6; i++){
        environmentResults[i] = 0;
    }
    
    for (std::vector<ImageInfo*>::iterator n = nearest_neighbors->begin(); n != nearest_neighbors->end(); ++n){
        ImageInfo* neighbor = *n.base();
        environmentResults[neighbor->expected_environment] ++;
    }

    for(int i = 0; i < 6; i++){
        if (environmentResults[guessedEnvironment] < environmentResults[i]){
            guessedEnvironment = (Environment)i;
        }
    }
    return guessedEnvironment;
}


/// Runs test on just the 12 images (2 of each weather type)
void test_with_specific_images(){
    std::string test_image_locationCloudy = "imageSet/images/P2.cd.DK.c.001.JPG";
    std::string test_image_locationSun = "imageSet/images/P2.su.SD.s.001.JPG";
    std::string test_image_locationCloudy1 = "imageSet/images/P2.cd.DK.c.002.JPG";
    std::string test_image_locationSun1 = "imageSet/images/P2.su.SD.s.002.JPG";
    std::string test_image_locationFog1 = "imageSet/images/P2.fg.DK.f.001.JPG";
    std::string test_image_locationFog = "imageSet/images/P2.fg.DK.f.002.JPG";
    std::string test_image_locationNT1 = "imageSet/images/P2.nt.HO.c.001.JPG";
    std::string test_image_locationNT = "imageSet/images/P2.nt.HO.c.002.JPG";
    std::string test_image_locationRN1 = "imageSet/images/P2.rn.DK.s.001.JPG";
    std::string test_image_locationRN = "imageSet/images/P2.rn.DK.s.001.JPG";
    std::string test_image_locationSN1 = "imageSet/images/P2.sn.OC.b.001.JPG";
    std::string test_image_locationSN = "imageSet/images/P2.sn.OC.b.002.JPG";
    Image* testImageCL = new Image(test_image_locationCloudy);
    Image* testImageSU = new Image(test_image_locationSun);
    Image* testImageCL1 = new Image(test_image_locationCloudy1);
    Image* testImageSU1 = new Image(test_image_locationSun1);
    Image* testImageFG = new Image(test_image_locationFog);
    Image* testImageFG1 = new Image(test_image_locationFog1);
    Image* testImageNT = new Image(test_image_locationNT);
    Image* testImageNT1 = new Image(test_image_locationNT1);
    Image* testImageRN = new Image(test_image_locationRN);
    Image* testImageRN1 = new Image(test_image_locationRN1);
    Image* testImageSN = new Image(test_image_locationSN);
    Image* testImageSN1 = new Image(test_image_locationSN1);

    std::vector<std::string>* image_paths = new std::vector<std::string>();
    image_paths->push_back(test_image_locationCloudy);
    image_paths->push_back(test_image_locationSun);
    image_paths->push_back(test_image_locationCloudy1);
    image_paths->push_back(test_image_locationSun1);
    image_paths->push_back(test_image_locationFog);
    image_paths->push_back(test_image_locationFog1);
    image_paths->push_back(test_image_locationNT);
    image_paths->push_back(test_image_locationNT1);
    image_paths->push_back(test_image_locationRN);
    image_paths->push_back(test_image_locationRN1);
    image_paths->push_back(test_image_locationSN);
    image_paths->push_back(test_image_locationSN1);
    std::vector<ImageInfo*>* images_info = new std::vector<ImageInfo*>();

    gather_all_image_info(images_info, image_paths);
    write_image_info_to_csv("NewInfo-Small.csv", images_info);
    // int averageSnowX, averageSnowY;
    //averageSnowX = testImageSN->getHue + testImageSN1->getHue /2
    //averageSnowY = testImageSN->getIntensity + testImageSN1 -> getIntensity /2 
    //averageRainX = testImageSN->getHue + testImageSN1->getHue /2
    //averageSnowY = testImageSN->getIntensity + testImageSN1 -> getIntensity /2 
    //averageSunX = testImageSN->getHue + testImageSN1->getHue /2
    //averageSnowY = testImageSN->getIntensity + testImageSN1 -> getIntensity /2 
    //off each iteration euclidean distance = sqrt((averageSnowX - otherImageX)^2 + (averageSnowY - otherImageY)^2)
    //doing this for each weather type whichever euclidean distance is the lowest then that is the weather

}

/// Reads in all images and runs test on that
void test_with_all_images(){

    /// Relative path to the directory containing all of our images to test
    std::string images_directory = "imageSet/images";

    /// List of all files in the directory
    std::vector<std::string>* all_image_paths = new std::vector<std::string>();
    std::vector<ImageInfo*>* images_info = new std::vector<ImageInfo*>();
    
    gather_info_from_csv("NewInfo.csv", images_info);
    int highestKForWeather = 0;
    int highestKForIllumination = 0;
    // int highestKForEnvironment = 0;
    int highestWeatherCount = 0;
    int highestIlluminationCount = 0;
    // int highestEnvironmentCount = 0;
    float correctWeatherCount = 0;
    float correctIlluminationCount = 0;
    // float correctEnvironmentCount = 0;
    float totalCount = images_info->size();
    ImageInfo* info = nullptr;
    std::vector<ImageInfo*>* nearNeighbors = new std::vector<ImageInfo*>();
    int max_k = 100;
    for (int k = 1; k < max_k; k ++){
        correctWeatherCount = 0;
        correctIlluminationCount = 0;
        for (std::vector<ImageInfo*>::iterator i = images_info->begin(); i != images_info->end(); ++i){
            info = *i.base();
            // std::cout << "Nearest Neighbors for: "  << info->long_description() << std::endl;
            nearest_neighbors(k, info, images_info, nearNeighbors);
            for (std::vector<ImageInfo*>::iterator n = nearNeighbors->begin(); n != nearNeighbors->end(); ++n){
            
                ImageInfo* nearNeighbor = *n.base();
                // std::cout << "      - " << nearNeighbor->long_description() << std::endl;
            }

            // Guess weather
            Weather guessedW = guess_weather_with_knn(nearNeighbors);
            if (guessedW == info->expected_weather){
                std::cout << GREEN_OUTPUT;
                correctWeatherCount += 1;
            }else{
                std::cout << RED_OUTPUT;
            }

            // std::cout << "    Weather Guess: " << decscription_of((Weather)guessedW) << " - Expected: " << decscription_of(info->expected_weather) << RESET_OUTPUT << std::endl;
            
            // Guess Illumination
            Illumination guessedI = guess_illumination_with_knn(nearNeighbors);
            if (guessedI == info->expected_illumination){
                // std::cout << GREEN_OUTPUT;
                correctIlluminationCount += 1;
            }else{
                // std::cout << RED_OUTPUT;
            }
            // std::cout << "    Illumination Guess: " << decscription_of((Illumination)guessedI) << " - Expected: " << decscription_of(info->expected_illumination) << RESET_OUTPUT << std::endl;

            // // Guess Environment
            // Environment guessedE = guess_environment_with_knn(nearNeighbors);
            // if (guessedE == info->expected_environment){
            //     // std::cout << GREEN_OUTPUT;
            //     correctEnvironmentCount += 1;
            // }else{
            //     // std::cout << RED_OUTPUT;
            // }


        }
        if (correctWeatherCount > highestWeatherCount){
            highestKForWeather = k;
            highestWeatherCount = correctWeatherCount;
        }
        std::cout << "Weather Score (k = " << k << "): " << correctWeatherCount << "/" << totalCount << std::endl;
        if (correctIlluminationCount > highestIlluminationCount){
            highestIlluminationCount = correctIlluminationCount;
            highestKForIllumination = k;
        }
        std::cout << "Illumination Score (k = " << k << "): " << correctIlluminationCount << "/" << totalCount << std::endl;
        
        // if (correctEnvironmentCount > highestEnvironmentCount){
        //     highestKForEnvironment = k;
        //     highestEnvironmentCount = correctEnvironmentCount;
        // }
        // std::cout << "Environment Score (k = " << k << "): " << correctEnvironmentCount << "/" << totalCount << std::endl << std::endl;
    }

    std::cout << "Ran knn algorith for k = 1 -> k = " << max_k << std::endl;
    std::cout << "    - Highest k for weather: `" << highestKForWeather << "` (" << highestWeatherCount << "/" << totalCount << ")\n" ;
    std::cout << "    - Highest k for illumination: `" << highestKForIllumination << "` (" << highestIlluminationCount << "/" << totalCount << ")\n";
    // std::cout << "    - Highest k for environment: `" << highestKForEnvironment << "` (" << highestEnvironmentCount << "/" << totalCount << ")\n";
    // write_image_info_to_csv("ReadInValues.csv", images_info);

    // GetFilesInDirectory(*all_image_paths, images_directory);

    // // read_in_files_and_write_results(all_image_paths);

    // gather_all_image_info(images_info, all_image_paths);
    // write_image_info_to_csv("NewInfo.csv", images_info);
}

int main(int argc, char** argv)
{   
    std::cout << "Main Entered...|" << std::endl << std::endl;
    
    /// Set to true if we want to use the smaller defined subset of images
    bool useSpecificImages = false;
    if (useSpecificImages){
        test_with_specific_images();
    }else{
        test_with_all_images();
    }

    std::cout << "\n\n|...Main Exited" << std::endl;
}

void gather_info_from_csv(std::string csv_path, std::vector<ImageInfo*>* info_to_fill){
    std::ifstream csv;
    csv.open(csv_path, std::ios::in);  //open a file to perform read operation using file object
    if (csv.is_open()){   //checking whether the file is open
        std::string line_string;
        getline(csv, line_string, '\n');
        while(getline(csv, line_string, '\n')){  //read data from file object and put it into string.
            std::vector<std::string> elements = std::vector<std::string>();
            std::string delimiter = ",";
            size_t pos = 0;
            std::string token;
            while ((pos = line_string.find(delimiter)) != std::string::npos) {
                token = line_string.substr(0, pos);
                elements.push_back(token);
                line_string.erase(0, pos + delimiter.length());
            }
            // std::cout << line_string << std::endl;
            elements.push_back(line_string);
            std::string filename = elements.at(0);
            std::string weatherS = elements.at(1);
            Weather weather = SUNNY;
            for (int i = 0; i < 6; i++){
                if (weatherS.compare(decscription_of((Weather)i))==0){
                    weather = (Weather)i;
                    break;
                }
            }
            std::string illuminationS = elements.at(2);
            Illumination illumination = SHADOW;
            for (int i = 0; i < 9; i++){
                if (illuminationS.compare(decscription_of((Illumination)i))==0){
                    illumination = (Illumination)i;
                    break;
                }
            }

            std::string environmentS = elements.at(3);
            Environment environment = WET;
            for (int i = 0; i < 6; i++){
                if (environmentS.compare(decscription_of((Environment)i))==0){
                    environment = (Environment)i;
                    break;
                }
            }

            std::string chromaS = elements.at(4);
            std::string hueS = elements.at(5);
            std::string intensityS = elements.at(6);
            std::string contrastS = elements.at(7);
            std::string minS = elements.at(8);
            int min = stoi(minS);
            std::string maxS = elements.at(9);
            int max = stoi(maxS);

            ImageInfo* info = new ImageInfo(
                filename,
                weather,
                illumination,
                environment,
                RGBPixelInfo(min,min,min,0,0), // Just fill with min 3 times since it will yeild same intensity
                RGBPixelInfo(max,max,max,0,0),
                stoi(hueS),
                stoi(chromaS),
                stoi(intensityS),
                2562,
                1944,
                3
            );
            info_to_fill->push_back(info);
        }
        csv.close();   //close the file object.
   }
}

void gather_all_image_info(std::vector<ImageInfo*>* info_to_fill, std::vector<std::string>* filepaths){
    int total_files = filepaths->size();
    int files_left = total_files;
    std::vector<Image*>* images_to_read = new std::vector<Image*>();

    int progress_completed = 0;
    int total_progress = total_files;

    int index = 0;
    while (files_left > 0){
        std::string filepath = filepaths->at(index);

        // Ignore files that we dont expect to be an image file (Pesky PDF in folder or others)
        if (filepath.back() != 'G'){
            std::cout << "Ignoring file at `" << filepath << "`\n";
            total_files --;
            files_left --;
            index ++;
            continue;
        }

        Image* image = new Image(filepath);
        files_left -= 1;

        ImageInfo* info = get_image_info_based_on_entire_image(image);
        info_to_fill->push_back(info);
        index += 1;
        delete image;
        std::cout << "Progress: " << index << "/" << total_files << std::endl;
    }
}

void write_image_info_to_csv(std::string output_filepath, std::vector<ImageInfo*>* info_to_write){
    std::fstream outfile;
    outfile.open(output_filepath, std::ios::out);
    outfile << "filename"<< "," << "weather" << "," << "illumination" << "," << "environment" << ","  << "chroma" << "," << "hue" << "," << "intensity" << "," << "contrast" << ",min_pixel_intensity,max_pixel_intensity" << std::endl;

    for (std::vector<ImageInfo*>::iterator i = info_to_write->begin(); i != info_to_write->end(); ++i){
        ImageInfo* info = *i.base();

        outfile << info->long_description()<< std::endl;
    }
    outfile.close();
}

void read_in_files_and_write_results(std::vector<std::string>* image_paths){

    std::fstream outfile;
    outfile.open("allImages_Top3Sections.csv", std::ios::out);
    outfile << "filename"<< "," << "weather" << "," << "illumination" << "," << "environment" << ","  << "chroma" << "," << "hue" << "," << "intensity" << std::endl;
    int total_read = 0;
    int total_to_read = image_paths->size();
    for (std::vector<std::string>::iterator i = image_paths->begin(); i != image_paths->end(); ++i){
        std::string filepath = *i.base();
        // std::stringstream ss;
        // ss << "imageSet/images/" << filepath;
        // std::cout << filepath << std::endl;
        // Ignore all non-jpeg images
        if (filepath.back() != 'G' /*|| get_weather(filepath) != NIGHT*/){
            // std::cout << "Ignoring file at `" << filepath << "`\n";
            total_to_read --;
            continue;
        }
        Image* image = new Image(filepath);
        Image* top_right = new Image(image, TOP_RIGHT);
        Image* top_mid = new Image(image, TOP_MID);
        Image* top_left = new Image(image, TOP_LEFT);

        Weather w = image->known_weather();
        Environment e = image->known_environment();
        Illumination illum = image->known_illumination();
        int average_chroma = 0;
        int average_hue = 0;
        int average_intensity = 0;
        int count = 0;
        count += top_right->get_width() * top_right->get_height();
        count = count * 3;
        for(int y = 0; y < top_right->get_height(); y++){
            for(int x = 0; x < top_right->get_width(); x++){
                average_hue += top_right->get_hue(x, y);
                average_intensity += top_right->get_intensity(x, y);
                average_chroma += top_right->get_chroma(x, y);
                average_hue += top_left->get_hue(x, y);
                average_intensity += top_left->get_intensity(x, y);
                average_chroma += top_left->get_chroma(x, y);
                average_hue += top_mid->get_hue(x, y);
                average_intensity += top_mid->get_intensity(x, y);
                average_chroma += top_mid->get_chroma(x, y);
                
            }
        }
        average_chroma = average_chroma / count;
        average_hue = average_hue / count;
        average_intensity = average_intensity / count;
        //intensity 60<intensity<90 = rain
        //intensity intensity < 60 = night
        //intensity 90<intensity<120 = snow
        //chroma 35<chroma = sunny
        //
        
        outfile << get_filename(filepath) << "," << decscription_of(w) << "," << decscription_of(illum) << "," << decscription_of(e) << ","  << average_chroma << "," << average_hue << "," << average_intensity << std::endl;
        // std::cout << "Expected: " << decscription_of(w) << std::endl;

        delete image;
        delete top_right;
        delete top_left;
        delete top_mid;
        total_read ++;
        std::cout << "Progress: " << total_read << "/" << total_to_read << std::endl;
    }
    outfile.close();
}


void test_images_are_correctly(std::vector<Weather>* weathers_to_check, std::vector<std::string>* image_paths){

    std::vector<std::string>* true_positives[6];
    std::vector<std::string>* false_positives[6];
    std::vector<std::string>* true_negatives[6];
    std::vector<std::string>* false_negatives[6];
    int total_weather[6];
    int total_guessed[6];
    for (int i = 0; i < 6; i++){
        true_positives[i] = new std::vector<std::string>();
        false_positives[i] = new std::vector<std::string>();
        true_negatives[i] = new std::vector<std::string>();
        false_negatives[i] = new std::vector<std::string>();
        total_weather[i] = 0;
        total_guessed[i] = 0;
    }

    float total_analyzed = 0;
    float total_images = image_paths->size();

    for (std::vector<std::string>::iterator i = image_paths->begin(); i != image_paths->end(); ++i){
        std::string filepath = *i.base();

        // Ignore all non-jpeg images
        if (filepath.back() != 'G' /*|| get_weather(filepath) != NIGHT*/){
            total_images --;
            // std::cout << "Ignoring file at `" << filepath << "`\n";
            continue;
        }
        Image* image = new Image(filepath);

        Weather w = image->known_weather();
        total_weather[w] ++;
        total_analyzed ++;

        std::cout << "\n| " << GREEN_OUTPUT << "+" << RESET_OUTPUT << " | " << RED_OUTPUT << "+" << RESET_OUTPUT << " | " << GREEN_OUTPUT << "-" << RESET_OUTPUT << " | " << RED_OUTPUT << "-" << RESET_OUTPUT << " | E | - `" << get_filename(filepath) << "` - (TotalAnalyzed: " << total_analyzed << ")\n" ;

        // std::cout << "Expected: " << decscription_of(w) << std::endl;

        for (int w_index = 0; w_index < weathers_to_check->size(); w_index ++){
            Weather weather_to_check = weathers_to_check->at(w_index);
            Weather guessed_weather;
            if (is(weather_to_check, image)){
                guessed_weather = weather_to_check;
                // total_guessed[weather_to_check] ++;
                
                if (w == guessed_weather){
                    true_positives[weather_to_check]->push_back(filepath); // True Positive
                }else{
                    false_positives[weather_to_check]->push_back(filepath); // False Positive
                }

            }else{
                if (w == weather_to_check){
                    false_negatives[weather_to_check]->push_back(filepath); // False Negative
                }else{
                    true_negatives[weather_to_check]->push_back(filepath); // True Negative
                }
            }

            std::string true_positives_count = formatted_string(true_positives[weather_to_check]->size());
            std::string false_positives_count = formatted_string(false_positives[weather_to_check]->size());
            std::string true_negatives_count = formatted_string(true_negatives[weather_to_check]->size());
            std::string false_negative_count = formatted_string(false_negatives[weather_to_check]->size());

            // std::string guess_count = formatted_string(total_guessed[weather_to_check]);
            std::string expected_count = formatted_string(total_weather[weather_to_check]);
            std::string total_analyzed_count = formatted_string(total_analyzed);
            std::cout << "|" << true_positives_count <<"|" << false_positives_count << "|" << true_negatives_count << "|"<< false_negative_count << "|" << expected_count << "|" << " -> " << decscription_of(weather_to_check) << "\n";
        }
        delete image;
    }

    int total_night_images = true_positives[NIGHT]->size() + false_negatives[NIGHT]->size();
    std::cout << "Correctly matched " << true_positives[NIGHT]->size() << " / " << total_night_images << " " << decscription_of(NIGHT) << "images." << std::endl << std::endl;

    for (int i = 0; i < 6; i++){
        int correct = true_positives[i]->size();
        int wrong = false_negatives[i]->size();
        int total = correct + wrong;
        std::cout << decscription_of((Weather)i) << " total: " << total << "\n      - correct: " << correct << " - incorrect: " << wrong <<std::endl;
    }
    // std::cout << "Correctly matched " << false_negatives->size() << " / " << total_day_images << " " << "non-night" << "images." <<std::endl;
}

void test_print_width_and_height(std::string jpeg_path){

    Image* image = new Image(jpeg_path);
    image->write_to_raw("test.raw");

    // Test extracting layers from the image
    Image* red_image = new Image(image, RED);
    red_image->write_to_raw("red_child.raw");
    delete red_image;

    Image* green_image = new Image(image, GREEN);
    green_image->write_to_raw("green_child.raw");
    delete green_image;

    Image* blue_image = new Image(image, BLUE);
    blue_image->write_to_raw("blue_child.raw");
    delete blue_image;


    // Test extracting sections from the image
    Image* top_left_image = new Image(image, TOP_LEFT);
    top_left_image->write_to_raw("top_left_child.raw");
    delete top_left_image;

    Image* top_mid_image = new Image(image, TOP_MID);
    top_mid_image->write_to_raw("top_mid_child.raw");
    delete top_mid_image;

    Image* top_right_image = new Image(image, TOP_RIGHT);
    top_right_image->write_to_raw("top_right_child.raw");
    delete top_right_image;

    Image* bottom_left_image = new Image(image, BOTTOM_LEFT);
    bottom_left_image->write_to_raw("bottom_left_child.raw");
    delete bottom_left_image;

    Image* bottom_mid_image = new Image(image, BOTTOM_MID);
    bottom_mid_image->write_to_raw("bottom_mid_child.raw");
    delete bottom_mid_image;

    Image* bottom_right_image = new Image(image, BOTTOM_RIGHT);
    bottom_right_image->write_to_raw("bottom_right_child.raw");
    delete bottom_right_image;
    
    // Deallocate original image
    delete image;
}

void test_read_in_all_images(std::vector<std::string>* all_image_paths){
    int totalCount = all_image_paths->size();
    int readCount = 0;
    int weather_lookup[6] = {0,0,0,0,0,0};
    int illumination_lookup[9] = {0,0,0,0,0,0,0,0,0};
    int environment_lookup[9] = {0,0,0,0,0,0,0,0,0};

    for (std::vector<std::string>::iterator i = all_image_paths->begin(); i != all_image_paths->end(); ++i){
        std::string filepath = *i.base();

        if (filepath.back() != 'G'){
            std::cout << "Ignoring file at `" << filepath << "`\n";
            totalCount -= 1;
            continue;
        }
        Image* image = new Image(filepath);
        Weather w = image->known_weather();
        weather_lookup[w] += 1;
        Illumination illum = image->known_illumination();
        illumination_lookup[illum] += 1;
        Environment e = image->known_environment();
        environment_lookup[e] += 1;
        // std::cout << "Read in image at path `" << filepath << "` (Weather: " << decscription_of(w) << ", Illumination: " << decscription_of(illum) << ", Environment: " << decscription_of(e) << ")" << std::endl;
        delete image;
        readCount += 1;
        std::cout << readCount  << "/" << totalCount << std::endl;
    }

    std::cout << "--Weather--\n";
    for(int i = 0; i < 6; i++)
        std::cout << "   - " << decscription_of((Weather)i) << ": " << weather_lookup[i] << std::endl;

    std::cout << "--Illumination--\n";
    for(int i = 0; i < 9; i++)
        std::cout << "   - " << decscription_of((Illumination)i) << ": " << illumination_lookup[i] << std::endl;

    std::cout << "--Environment--\n";
    for(int i = 0; i < 9; i++)
        std::cout << "   - " << decscription_of((Environment)i) << ": " << environment_lookup[i] << std::endl;

}

void GetFilesInDirectory(std::vector<std::string> &out, const std::string &directory){
#ifdef WINDOWS
    HANDLE dir;
    WIN32_FIND_DATA file_data;

    if ((dir = FindFirstFile((directory + "/*").c_str(), &file_data)) == INVALID_HANDLE_VALUE)
        return; /* No files found */

    do {
        const string file_name = file_data.cFileName;
        const string full_file_name = directory + "/" + file_name;
        const bool is_directory = (file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        if (file_name[0] == '.')
            continue;

        if (is_directory)
            continue;

        out.push_back(full_file_name);
    } while (FindNextFile(dir, &file_data));

    FindClose(dir);
#else
    DIR *dir;
    class dirent *ent;
    class stat st;

    dir = opendir(directory.c_str());
    while ((ent = readdir(dir)) != NULL) {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = directory + "/" + file_name;

        if (file_name[0] == '.')
            continue;

        if (stat(full_file_name.c_str(), &st) == -1)
            continue;

        const bool is_directory = (st.st_mode & S_IFDIR) != 0;

        if (is_directory)
            continue;

        out.push_back(full_file_name);
    }
    closedir(dir);
#endif
} // GetFilesInDirectory
