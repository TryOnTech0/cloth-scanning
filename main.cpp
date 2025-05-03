#include "ClothScanning.h"
#include <iostream>

int main() {
    // 1. ClothScanner nesnesi oluştur
    ClothScanning scanner;

    // 2. Görsel Yükleme Testi
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Testing image loading from disk..." << std::endl;

    std::string imagePath = "../sample_images/shirt1.jpg"; // Diskten yüklenecek görselin yolu
    if (scanner.loadImage(imagePath)) {
        std::cout << "[Main] Image loaded successfully.\n";
    } else {
        std::cerr << "[Main] Failed to load image.\n";
    }

    // 3. Kamera ID'yi belirle
    std::cout << "----------------------------------------" << std::endl;
    int cameraID = 0; // Varsayılan olarak laptop/webcam
    // Eğer harici bir kamera bağlarsan (örneğin USB kamera), bunu 1 veya 2 yapabilirsin.

    std::cout << "Testing camera capture (Camera ID: " << cameraID << ")..." << std::endl;

    // 4. Kameradan Görüntü Almayı Test Et
    if (scanner.captureFromCamera(cameraID)) {
        std::cout << "[Main] Camera capture successful.\n";
    } else {
        std::cerr << "[Main] Camera capture failed.\n";
    }

    std::cout << "----------------------------------------" << std::endl;
    std::cout << "All tests completed.\n";

    return 0;
}
