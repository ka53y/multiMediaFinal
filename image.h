
#ifndef __IMAGE__
#define __IMAGE__

#include <string>
#include <sstream>
#include <math.h>
#include "enums.h"

#ifndef  VERBOSE_LOGGING
#define VERBOSE_LOGGING 0
#endif // ! VERBOSE_LOGGING

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" 
/* Include JPEG reading utility file referenced in stack overflow article
Copied from Open Source github repository at https://github.com/nothings/stb/blob/master/stb_image.h
-Jasper Heist 12/6/21*/

/// Caluculates and returns the hue value of this pixel in the image
/// Sources on how to calcualte hue https://stackoverflow.com/questions/23090019/fastest-formula-to-get-hue-from-rgb
int get_hue_for_pixel(int r, int g, int b){
    double count, average;

    PixelType max_pixel;
    float min, max;
    float red = (float)r;
    float green = (float)g;
    float blue = (float)b;
    red /= 255;
    green /= 255;
    blue /= 255;

    if (red == blue && red == green){
        return 0;
    }else if (red >= blue && red >= green){
        max_pixel = RED;
        max = red;
        if (blue > green)
            min = green;
        else
            min = blue;
    }else if (blue >= green && blue >= red){
        max_pixel = BLUE;
        max = blue;
        if (red > green)
            min = green;
        else
            min = red;
    }else if (green >= blue && green >= red){
        max_pixel = GREEN;
        max = green;
        if (red > blue)
            min = blue;
        else
            min = red;
    }

    float max_min = max - min;
    if (max_min == 0)
        return 0;
    
    float hue;
    switch (max_pixel)
    {
    case RED:
        hue = (green - blue)/max_min;
        break;
    case GREEN:
        hue = 2 + (blue - red)/max_min;
        break;
    case BLUE:
        hue = 4 + (red - green)/max_min;
        break;
    default:
        return 0;
    }

    // find the sector of 60 degrees to which the color 
    // https://www.pathofexile.com/forum/view-thread/1246208/page/45 - hsl col
    hue = hue * 60;
    int hue_int = round(hue);

    // Hue is a value between 0 and 360
    hue_int = hue_int % 360;

    if (hue_int < 0) {
        //std::cout<<"hue int: "<<hue_int<<std::endl;
        //count++;
        //average += hue_int +360;
        return hue_int + 360;
    }
    else {
        //std::cout<<"hue int: "<<hue_int<<std::endl;
        //count++;
        //average += hue_int;
        return hue_int;
    }
}

/// Taking in the RGB values, gets the chroma of the pixel. Returns the max(r,g,b) - min(r,g,b) (as per assignment description advice)
int get_chroma_for_pixel(int red, int green, int blue){
        // Get max of rgb values
        int max = red;
        if (green > max)
            max = green;
        
        if (blue > max)
            max = blue;

        // Get min of rgb values
        int min = red;
        if (green < min)
            min = green;
        
        if (blue < min)
            min = blue;
        //std::cout<<"Chroma: "<<max - min<<std::endl;
        return max - min;
}

/// Gets the intensity of the pixel given the Red, Green and Blue values (Average of the three)
int get_intensity_for_pixel(int red, int green, int blue){
    float r = (float)red;
    float g = (float)green;
    float b = (float)blue;
    float intensity = red+green+blue;
    //std::cout<<"intensity: "<<(int)round(intensity/3)<<std::endl;
    return (int)round(intensity / 3.0);
}

/// Uses the values of the max and min pixel to compute the contrast (highest pixel intensity - lowest pixel intensity)
int get_contrast(int max_intensity_pixel, int min_intensity_pixel){
    return abs(max_intensity_pixel - min_intensity_pixel);
}

class RGBPixelInfo{
    public:

    /// Initializer from the values
    RGBPixelInfo(int red, int green, int blue, int x_loc, int y_loc):
    r(red), 
    g(green), 
    b(blue),
    x(x_loc),
    y(y_loc),
    chroma(get_chroma_for_pixel(red, green, blue)),
    hue(get_hue_for_pixel(red, green, blue)),
    intensity(get_intensity_for_pixel(red, green, blue))
    {
    }

    
    /// This pixel's red value
    int r;
    /// This pixel's green value
    int g;
    /// This pixel's blue value
    int b;
    /// This pixel's x location in the original image
    int x;
    /// This pixel's y location in the original image
    int y;
    /// This pixel's calculated chroma
    int chroma;
    /// This pixel's calculated hue
    int hue;
    /// This pixel's calculated intensity
    int intensity;
};

/// Object to wrap up the information about an image after it is analyzed (Does not include DataArray)
class ImageInfo{
    public:

    ImageInfo(std::string relative_path, Weather weather, Illumination illumination, Environment environment, RGBPixelInfo min_pixel, RGBPixelInfo max_pixel, int average_hue, int average_chroma, int average_intensity, int image_height, int image_width, int channel_count):
    filepath(relative_path),
    expected_weather(weather),
    expected_illumination(illumination),
    expected_environment(environment),
    min(min_pixel), 
    max(max_pixel), 
    hue(average_hue), 
    chroma(average_chroma), 
    intensity(average_intensity),
    contrast(abs(round(float(max_pixel.intensity - min_pixel.intensity)/float(min_pixel.intensity + max_pixel.intensity)))),
    height(image_height), 
    width(image_width), 
    channels(channel_count){}

    std::string long_description(){
        std::stringstream ss;
        ss << get_filename(this->filepath) << "," << decscription_of(this->expected_weather) << "," << decscription_of(this->expected_illumination) << "," << decscription_of(this->expected_environment) << ","  << this->chroma << "," << this->hue << "," << this->intensity << "," << this->contrast << "," << this->min.intensity << "," << this->max.intensity;
        return ss.str();
    }

    /// Extracts the filename from the full relative filepath
    std::string filename(){
        return get_filename(filepath);
    }

    /// Path to the file
    std::string filepath;
    /// Weather info parsed from the filename
    Weather expected_weather;
    /// Ilumination info parsed from the filename
    Illumination expected_illumination;
    /// Environment info parsed from the filename
    Environment expected_environment;
    /// Pixel value of image with the minimm entensity
    RGBPixelInfo min;
    /// Pixel with max intensity
    RGBPixelInfo max;
    /// CALCULATED Hue of image
    int hue;
    /// CALCULATED Chroma of image
    int chroma;
    /// CALCULATED Intensity of image
    int intensity;
    /// CALCULATED Contrast
    int contrast;
    /// Height of image
    int height;
    /// Width of image
    int width;
    /// Number of channels (3 (RGB) or 1 (Greyscale))
    int channels;
};

/// Class to hold a JPEG image and make it easier to separate the channels and layers for analyzing.
class Image{

    public:

    /// General initializer supplying pointer to image's pixel array, the width, and number of chennels in the array.
    Image(uint8_t* image, int width, int height, int channels):
    image(image),
    width(width),
    height(height),
    total_channels(channels),
    initialized_via_stbi(false)
    {}

    // Initializes JPEG image object from a path to the file. NOTE: This uses the stbi library that's in the `stbi_image.h` file.
    Image(std::string path){
        // Read in rgb image
        this->image = stbi_load(path.c_str(), &this->width, &this->height, &this->total_channels, 3);
        this->initialized_via_stbi = true;
        this->filepath = path;
        if(VERBOSE_LOGGING == 1)
            std::cout << "Image read in with stbi library (Width: " << this->width << " - Height: " << this->height << " - Channels: " << this->total_channels << ")\n";
    }

    /// Initiales a new image with the with the same height and width as the parent but just grabbing one of the channels. If there is only one channel, it will create a copy of the parentImage.
    Image(Image* parentImage, PixelType layer){
        this->width = parentImage->width;
        this->height = parentImage->height;
        this->total_channels = 1; // One channel since this is just a single color channel we are grabbing.
        this->initialized_via_stbi = false;
        this->filepath = parentImage->filepath;
    
        // Disregard specified channel and log
        if (parentImage->total_channels == 1){
            layer = RED;
            std::cout << "Parent image only has one channel, initializing a copy.\n";
        }
        uint8_t new_image[this->height*this->width];
        for(int h = 0; h < this->height; h++){
            for(int w = 0; w < this->width; w++){
                uint8_t pixel = parentImage->pixel(w, h, layer);
                new_image[h * this->width + w] = pixel;
            }
        }
        this->image = new_image;
        std::string channelDescription = "NONE___ERROR";
        if (parentImage->total_channels == 3){
            channelDescription = decscription_of(layer);
        }else{
            channelDescription = "ONLY";
        }
        if(VERBOSE_LOGGING == 1)
            std::cout << "Image created from parent's `"  << channelDescription << "` channel (Width: " << this->width << " - Height: " << this->height << " - Channels: " << this->total_channels << ")\n";
    }

    /// Initiales a new image with the same number of chanels but just one of the 8 images in the section.
    Image(Image* parentImage, ImageSection section){
        this->width = parentImage->width / 3;
        this->height = parentImage->height / 2;
        this->total_channels = parentImage->total_channels;
        this->initialized_via_stbi = false;
        this->filepath = parentImage->filepath;

        // Build image from section
        int start_x, end_x, start_y, end_y;
        
        // Determie start and end bounds in x direction
        if (section == TOP_LEFT || section == BOTTOM_LEFT){
            start_x = 0;
            end_x = parentImage->width / 3;
        }else if (section == TOP_MID || section == BOTTOM_MID){
            start_x = parentImage->width / 3 + 1;
            end_x = parentImage->width * 2 / 3;
        }else if (section == TOP_RIGHT || section == BOTTOM_RIGHT){
            start_x = parentImage->width * 2 / 3 + 1;
            end_x = parentImage->width;
        }

        // Determine start and end bounds in y direction
        if (section == TOP_LEFT || section == TOP_MID || section == TOP_RIGHT){
            start_y = 0;
            end_y = parentImage->height/2;
        }else if (section == BOTTOM_LEFT || section == BOTTOM_MID || section == BOTTOM_RIGHT){
            start_y = parentImage->height/2 + 1;
            end_y = parentImage->height;
        }
        if(VERBOSE_LOGGING == 1)
            std::cout << "Creating image from parent's `"  << decscription_of(section) << "` section (Width: " << this->width << " - Height: " << this->height << " - Channels: " << this->total_channels << ")\n\n";

        /// Image to represent this section of the image
        uint8_t image_for_section[this->width * this->height * this->total_channels];
        for(int x = start_x; x < end_x; x++){
            for(int y = start_y; y < end_y; y++){
                /// Row in new image for this pixel to go
                int new_row = y - start_y;
                /// Column in new image for this pixel to go
                int new_column = x - start_x;
                /// Pixel location in the child's image array
                int pixel_location = new_row * this->width * this->total_channels + new_column * this->total_channels;

                if(VERBOSE_LOGGING == 1)
                    std::cout << "x: " << x << " new_row: " << new_row << " - y:" << y << " new_column: " << new_column << "(Pixel Location: " << pixel_location << ")\n";

                // Get images based on channels
                if (parentImage->total_channels ==  3){
                    uint8_t red_pixel_value = parentImage->pixel(x, y, RED);
                    uint8_t green_pixel_value = parentImage->pixel(x, y, GREEN);
                    uint8_t blue_pixel_value = parentImage->pixel(x, y, BLUE);
                    image_for_section[pixel_location] = red_pixel_value;
                    image_for_section[pixel_location + 1] = green_pixel_value;
                    image_for_section[pixel_location + 2 ] = blue_pixel_value;
                }else{
                    uint8_t pixel_value = parentImage->pixel(x, y, RED);
                     image_for_section[pixel_location] = pixel_value;
                }
            }
            this->image = image_for_section;
        }
    }    

    /// Deconstructor for the image
    ~Image(){
        // Free image for memory management
        if (this->initialized_via_stbi)
            stbi_image_free(this->image);
    }

    /// Public Getter for image width
    int get_width(){
        return this->width;
    }
    /// Public Getter for image height
    int get_height(){
        return this->height;
    }
    /// Size of data in this JPEG images array
    int get_data_size(){
        return this->width * this->height * this->total_channels;
    }
    /// Public getter for channel count of this image
    int get_total_channels(){
        return this->total_channels;
    }
    
    /// Weather parsed from file name
    Weather known_weather(){
        return get_weather(filepath);
    }
    
    /// Illumination parsed from file name
    Illumination known_illumination(){
        return get_illumination(filepath);
    }

    /// Environment parsed from file name
    Environment known_environment(){
        return get_environment(filepath);
    }

    /// Writes the entire image to a raw format at specified path.
    void write_to_raw(std::string output){
        std::stringstream ss;
        ss << "output_test/" << output;

        FILE* output_file_id = fopen(ss.str().c_str(), "w");
        std::cout << "Opened '" << ss.str() << "' (id: " << output_file_id << ") for writing." << std::endl;

        fwrite(this->image, 1, this->get_data_size(), output_file_id);
        fclose(output_file_id);
        std::cout << std::endl;
    }
    
    /// Gets the pixel value for the specified location in the image and pixel type (R, G, or B)
    uint8_t pixel(int x, int y, PixelType type){
        int row = y;
        int column = x*this->total_channels;
        int pixels_start = y * this->width * this->total_channels + column;
        int pixel_for_color = this->image[pixels_start + type];
        return pixel_for_color;
    }

    /// Caluculates and returns the hue value of this pixel in the image
    /// Sources on how to calcualte hue https://stackoverflow.com/questions/23090019/fastest-formula-to-get-hue-from-rgb
    int get_hue(int x, int y){
        double count, average;
        // Disregard hue if greyscale image, they have no hue. One channel means greyscale
        if (this->total_channels == 1)
            return 0; 

        int red = (int)pixel(x, y, RED);
        int green = (int)pixel(x, y, GREEN);
        int blue = (int)pixel(x, y, BLUE);
        return get_hue_for_pixel(red, green, blue);
    }

    // TODO: Perhaps use smarter approach giving weight to each channel (luminance)
    /// Gets the intensity of the image using the simple approach of: Intensity = r + g + b
    int get_intensity(int x, int y){
        int red = pixel(x, y, RED);
        int green = pixel(x, y, GREEN);
        int blue = pixel(x, y, BLUE);

        return get_intensity_for_pixel(red, green, blue);
    }

    /// Get chroma of pixel 
    /// Uses logic from Assignment description: " Chroma, i.e., max(r,g,b)-min(r,g,b)," -> https://iu.instructure.com/courses/2012992/assignments/12838337
    int get_chroma(int x, int y){
        int red = pixel(x,y,RED);
        int green = pixel(x,y,GREEN);
        int blue = pixel(x,y,BLUE);
        return get_chroma_for_pixel(red, green, blue);
    }


    /// Computes if pixel at certain location is dark
    bool is_night(int x, int y){
        // int r = pixel(x,y,RED);
        // int g = pixel(x,y,GREEN);
        // int b = pixel(x,y,BLUE);

        // int compare;
        // if (r >= g && r >= b) {
        //     compare = r;
        // }
        // if (g > r && g >= b) {
        //     compare = g;
        // }
        // else {
        //     compare = b;
        // }
        int chroma = get_chroma(x,y);
        // int intensity = get_intensity(x,y);
        if(chroma < 15)
            return true;
        else
            return false;
    }
    
    bool is_sunny(int x, int y){
        // TODO: Should we calculate this based off of intensity?
        // uint8_t compare;
        /*
        if (r >= g && r >= b) {
            compare = r;
        }
        if (g > r && g >= b) {
            compare = g;
        }
        else {
            compare = b;
        }
        */
        int intensity = get_intensity(x,y);

        if(intensity > 117)
            return true;
        else
            return false;
    }
    
    bool is_cloudy(int x, int y){
        // TODO: Should we calculate this based off of intensity?
        // uint8_t compare;
        
        /*
        if (r >= g && r >= b) {
            compare = r;
        }
        if (g > r && g >= b) {
            compare = g;
        }
        else {
            compare = b;
        }
        */
        int intensity = get_intensity(x,y);
        if(intensity > 117 && intensity < 137)
            return true;
        else
            return false;
    }

    /// Relative filepath to image we read this image from
    std::string filepath;
    private:
    /// Width of image
    int width;
    /// Height of image
    int height;
    /// Total channels found in the image
    int total_channels;
    /// Pointer to data array
    uint8_t* image;
    /// Was this image read in from a filepath and initialized as a JPEG image via STBI's library?
    bool initialized_via_stbi;

};

/// Uses the top three sections to calculate all the values for the images
ImageInfo* get_image_info_based_on_top_three(Image* image){

    RGBPixelInfo* max_pixel = nullptr; //= new RGBPixelInfo(0,0,0,0,0);
    RGBPixelInfo* min_pixel = nullptr; //= new RGBPixelInfo(255,255,255,0,0);
    int average_chroma = 0;
    int average_hue = 0;
    int average_intensity = 0;

    // Assume all images are same size
    int image_height = image->get_height();
    int image_width = image->get_width();

    for(int y = 0; y < image_height; y++){
        for(int x = 0; x < image_width; x++){
            int r = image->pixel(x, y, RED);
            int g = image->pixel(x, y, GREEN);
            int b = image->pixel(x, y, BLUE);
            // std::cout << r+g+b << std::endl;
            RGBPixelInfo* pixel_info = new RGBPixelInfo(r,g,b,x,y);

            if (max_pixel == nullptr){
                delete pixel_info;
                min_pixel = new RGBPixelInfo(r,g,b,x,y);
                max_pixel = new RGBPixelInfo(r,g,b,x,y);
            }else{
                if (min_pixel->intensity > pixel_info->intensity){
                    delete min_pixel;
                    min_pixel = pixel_info;
                }else if (max_pixel->intensity < pixel_info->intensity){
                    delete max_pixel;
                    max_pixel = pixel_info;
                }else{
                    delete pixel_info;
                }
            }
            average_hue += pixel_info->hue;
            average_chroma += pixel_info->chroma;
            average_intensity += pixel_info->intensity;
        }
    }
    std::vector<ImageInfo*>* infos = new std::vector<ImageInfo*>();
    ImageInfo* info = new ImageInfo(
        image->filepath,
        image->known_weather(),
        image->known_illumination(),
        image->known_environment(),
        *max_pixel,
        *min_pixel, 
        average_hue/(image_height*image_width),
        average_chroma/(image_height*image_width),
        average_intensity/(image_height*image_width),
        image->get_height(),
        image->get_width(),
        image->get_total_channels()
    );
    return info;
}

/// Entire image to calculate all the values for the images
ImageInfo* get_image_info_based_on_entire_image(Image* image){

    RGBPixelInfo* max_pixel = nullptr; //= new RGBPixelInfo(0,0,0,0,0);
    RGBPixelInfo* min_pixel = nullptr; //= new RGBPixelInfo(255,255,255,0,0);
    int average_chroma = 0;
    int average_hue = 0;
    int average_intensity = 0;

    // Assume all images are same size
    int image_height = image->get_height();
    int image_width = image->get_width();

    for(int y = 0; y < image_height; y++){
        for(int x = 0; x < image_width; x++){
            int r = image->pixel(x, y, RED);
            int g = image->pixel(x, y, GREEN);
            int b = image->pixel(x, y, BLUE);
            // std::cout << r+g+b << std::endl;
            RGBPixelInfo* pixel_info = new RGBPixelInfo(r,g,b,x,y);

            if (max_pixel == nullptr){
                delete pixel_info;
                min_pixel = new RGBPixelInfo(r,g,b,x,y);
                max_pixel = new RGBPixelInfo(r,g,b,x,y);
            }else{
                if (min_pixel->intensity > pixel_info->intensity){
                    delete min_pixel;
                    min_pixel = pixel_info;
                }else if (max_pixel->intensity < pixel_info->intensity){
                    delete max_pixel;
                    max_pixel = pixel_info;
                }else{
                    delete pixel_info;
                }
            }
            average_hue += pixel_info->hue;
            average_chroma += pixel_info->chroma;
            average_intensity += pixel_info->intensity;
        }
    }
    std::vector<ImageInfo*>* infos = new std::vector<ImageInfo*>();
    ImageInfo* info = new ImageInfo(
        image->filepath,
        image->known_weather(),
        image->known_illumination(),
        image->known_environment(),
        *max_pixel,
        *min_pixel, 
        average_hue/(image_height*image_width),
        average_chroma/(image_height*image_width),
        average_intensity/(image_height*image_width),
        image->get_height(),
        image->get_width(),
        image->get_total_channels()
    );
    return info;
}



// Known width and height of images - We will get these values from print outs... but these are the known sizes.
// #define WIDTH 2562  (/3 = 845)
// #define HEIGHT 1944 (/2 = 972)

/// Runs through each pixel and generates values
void test_calculation(Image * image){
    int averageH, averageI, averageC;
    int count = image->get_height() * image->get_width();
        for(int y = 0; y < image->get_height(); y++){
            for(int x = 0; x < image->get_width(); x++){
                float hue = image->get_hue(x, y);
                int intensity = image->get_intensity(x, y);
                int chroma = image->get_chroma(x, y);
                averageH+=hue;
                averageI+=intensity;
                averageC+=chroma;
                // int contrast = image->get_constrast(x,y);
                if(VERBOSE_LOGGING){
                    //std::cout << "Point (" << x << "," << y << ")\n";
                    //std::cout << "   - Hue: "<< hue << std::endl;
                    //std::cout << "   - Intensity: "<< intensity << std::endl;
                    //std::cout << "   - Chroma: "<< chroma << std::endl;
                    // std::cout << "   - contrast: "<< hue << std::endl;
                }
            }
        }
        std::cout << "Average values for image: "<<std::endl;
        std::cout<<"This is the number of loops: "<<count<<std::endl;
        std::cout << "   - Hue: "<< averageH/count << std::endl;
        std::cout << "   - Intensity: "<< averageI/count << std::endl;
        std::cout << "   - Chroma: "<< averageC/count << std::endl;
        
}

#pragma region WeatherGuesses OLD FRAME ANALYSIS WAY OF GUESSING


// Evaluates if the image is dark or light.
bool is_entire_image_night(Image * image){
    int countLight, countNight;
    for (size_t x = 0; x < image->get_width(); x++)
    {
        for (size_t y = 0; y < image->get_height(); y++)
        {
            if (image->is_night(x,y))
                countNight += 1;
            else 
                countLight += 1;
        }
        
    }
    if (VERBOSE_LOGGING){
        std::cout<<"number of Night pixels: "<<countNight<<std::endl;
        std::cout<<"number of light pixels: "<<countLight<<std::endl;
    }
    
    return (countNight > countLight);
}

bool is_night(Image* image){
    Image* top_right = new Image(image, TOP_RIGHT);
    Image* top_mid = new Image(image, TOP_MID);
    Image* top_left = new Image(image, TOP_LEFT);

    if (is_entire_image_night(top_mid))
        return true;
    if (is_entire_image_night(top_right))
        return true;
    if (is_entire_image_night(top_left))
        return true;

    return false;
}

bool is_entireimage_sunny(Image * image){
    int countLight, countNight;
    for (size_t x = 0; x < image->get_width(); x++)
    {
        for (size_t y = 0; y < image->get_height(); y++)
        {
            if (image->is_sunny(x,y))
                countLight += 1;
            else 
                countNight += 1;
        }
        
    }
    if (VERBOSE_LOGGING){
        std::cout<<"number of Night pixels: "<<countNight<<std::endl;
        std::cout<<"number of light pixels: "<<countLight<<std::endl;
    }
    
    return (countLight > countNight);
}

bool is_sunny(Image* image){
    Image* top_right = new Image(image, TOP_RIGHT);
    Image* top_mid = new Image(image, TOP_MID);
    Image* top_left = new Image(image, TOP_LEFT);

    if (is_entireimage_sunny(top_mid))
        return true;
    if (is_entireimage_sunny(top_right))
        return true;
    if (is_entireimage_sunny(top_left))
        return true;

    return false;
}

bool is_entireimage_cloudy(Image * image){
    int countC, countNight;
    for (size_t x = 0; x < image->get_width(); x++)
    {
        for (size_t y = 0; y < image->get_height(); y++)
        {
            if (image->is_cloudy(x,y))
                countC += 1;
            else 
                countNight += 1;
        }
        
    }
    if (VERBOSE_LOGGING){
        std::cout<<"number of other pixels: "<<countNight<<std::endl;
        std::cout<<"number of cloudy pixels: "<<countC<<std::endl;
    }
    
    return (countC > countNight);
}

bool is_cloudy(Image* image){
    Image* top_right = new Image(image, TOP_RIGHT);
    Image* top_mid = new Image(image, TOP_MID);
    Image* top_left = new Image(image, TOP_LEFT);

    if (is_entireimage_cloudy(top_mid))
        return true;
    if (is_entireimage_cloudy(top_right))
        return true;
    if (is_entireimage_cloudy(top_left))
        return true;

    return false;
}

bool is_rainy(Image* image){
    return false;
}

bool is_snow(Image* image){
    return false;
}

bool is(Weather w, Image* image){
    switch (w){
        case SUNNY: return is_sunny(image);
        case CLOUDY: return is_cloudy(image);
        case NIGHT: return is_night(image);
        case RAIN: return is_rainy(image);
        case SNOW: return is_snow(image);
        default: return false;
    }
}

#pragma endregion

#endif // !__IMAGE__