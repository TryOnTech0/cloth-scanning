#ifndef CLOTHSCANNING_H
#define CLOTHSCANNING_H

#include <string>
#include <vector>
#include <opencv2/core/core.hpp>

class ClothScanning {
public:
    ClothScanning();
    ~ClothScanning();

    // Load image from file
    bool loadImage(const std::string &imagePath);

    // Load multiple images from file
    bool loadImages(const std::vector<std::string> &imagePaths);

    // Capture image from connected camera (live capture)
    bool captureFromCamera(int cameraID);

    // Preprocess all loaded images
    void preprocessImages();

    // Segment cloth region from background
    cv::Mat segmentCloth(const cv::Mat &image);

    // Extract 2D texture map from segmented cloth image
    cv::Mat extractTexture(const cv::Mat &segmentedImage);

    // Save scan data (texture, masks, etc.) locally
    bool saveScanData(const std::string &dataPath);

    // Save scan metadata and files to remote database
    bool saveToDatabase(const std::string &dbConnectionString, const std::string &clothID);

private:
    std::vector<cv::Mat> m_images;
    std::vector<cv::Mat> m_textures;
};

#endif // CLOTHSCANNING_H
