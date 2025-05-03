#include "ClothScanning.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

ClothScanning::ClothScanning() : m_images(), m_textures() {
    std::cout << "[ClothScanner] Initialized." << std::endl;
}

ClothScanning::~ClothScanning() {
    m_images.clear();
    m_textures.clear();
    std::cout << "[ClothScanner] Destroyed." << std::endl;
}

bool ClothScanning::loadImage(const std::string &imagePath) {
    // 1. Resmi yuklemeye calis
    cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR); // Renkli (RGB) olarak oku

    // 2. Goruntu basariyla yuklendi mi kontrol et
    if (image.empty()) {
        std::cerr << "[ClothScanner::loadImage] Error: Failed to load image from path: " << imagePath << std::endl;
        return false;
    }

    // 3. Basariliysa, m_images vektorune ekle
    m_images.push_back(image);

    // 4. Basari mesaji ver
    std::cout << "[ClothScanner::loadImage] Successfully loaded image: " << imagePath << std::endl;

    // 5. Basariyi bildir
    return true;
}

bool ClothScanning::loadImages(const std::vector<std::string> &imagePaths) {
    // 1. Başarı durumunu tutacak bir değişken tanımla
    bool allLoaded = true;

    // 2. Görüntü yolları vektörünü sırayla gez
    for (const auto &path : imagePaths) {
        // Her bir dosya için loadImage çağır
        bool success = loadImage(path);

        // Eger yukleme basarisizsa
        if (!success) {
            std::cerr << "[ClothScanner::loadImages] Warning: Failed to load image: " << path << std::endl;
            allLoaded = false; // Genel sonucu false yap
        }
    }

    // 3. Bilgi mesajı ver: Kaç adet resim başarıyla yüklendi
    std::cout << "[ClothScanner::loadImages] Total images loaded: " << m_images.size() << std::endl;

    // 4. Sonucu döndür (hepsi başarılıysa true, değilse false)
    return allLoaded;
}

bool ClothScanning::captureFromCamera(int cameraID) {
    // 1. Kamera açılır
    cv::VideoCapture cap(cameraID);
    if (!cap.isOpened()) {
        std::cerr << "[ClothScanner::captureFromCamera] Error: Could not open camera with ID " << cameraID << std::endl;
        return false;
    }

    // 2. Kamera üzerinden bir kare alınır
    cv::Mat frame;
    cap >> frame;

    // 3. Kare geçerli mi kontrol edilir
    if (frame.empty()) {
        std::cerr << "[ClothScanner::captureFromCamera] Error: Captured empty frame from camera." << std::endl;
        cap.release(); // kamerayı kapat
        return false;
    }

    // 4. Kare başarıyla alındıysa m_images vektörüne eklenir
    m_images.push_back(frame);

    // 5. Çekilen kareyi ekranda göster
    cv::namedWindow("Captured Frame", cv::WINDOW_NORMAL); // Normal pencere (yeniden boyutlandırılabilir)
    cv::imshow("Captured Frame", frame);

    std::cout << "[ClothScanner::captureFromCamera] Press any key to close the window..." << std::endl;
    cv::waitKey(0); // Kullanıcı bir tuşa basana kadar bekle

    // 6. Pencereyi kapat
    cv::destroyWindow("Captured Frame");

    // 7. Kamera serbest bırakılır
    cap.release();

    return true;
}

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
