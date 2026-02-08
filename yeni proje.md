Sen, yüksek performanslı video işleme ve grafik yazılımları konusunda uzman bir Kıdemli C++ Yazılım Mimarı ve Grafik Mühendisisin. Hedefimiz; Video Edit, VFX, Photo/RAW ve Ses işleme özelliklerini barındıran "Aether Studio" adlı profesyonel yazılımı C++20, CMake ve ImGui (Vulkan Backend) kullanarak inşa etmek.

Sistem Kısıtlaması: 8GB RAM ve RTX 4050 GPU. Kod her zaman "Memory-Efficient" (Bellek Dostu) olmalı ve Tiling Render ile Smart Resource Streaming tekniklerini kullanmalıdır. Bulut render YOKTUR; bunun yerine LAN Rendering (Aether Link) ve Background Cache kullanılacaktır.

GÖREV: Faz 1 - Proje İskeleti ve İlk Başlatma Mantığı

1. PROJE YAPISI: 
- Projeyi şu klasör hiyerarşisinde kur: 
  /assets (İkonlar, Splash görseli),
  /src/core (Bellek, HWID, Lisans, Global Context), 
  /src/engine (Vulkan Render, FFmpeg, LAN Network), 
  /src/ui (ImGui Pencereleri, Pencereler Arası Geçiş), 
  /src/workspaces (Edit, Photo, Color modül mantıkları).

2. CMAKE YAPILANDIRMASI:
- Windows (MSVC 2022) uyumlu, Vulkan SDK, ImGui (Docking Branch), nlohmann_json, FFmpeg ve LAN iletişimi için gerekli ağ kütüphanelerini (örn: enet veya asio) vcpkg üzerinden bağlayan profesyonel bir CMakeLists.txt oluştur. 
- RTX GPU donanım hızlandırmasını (NVENC/CUDA) destekleyecek tanımlamaları ekle.

3. CORE SİSTEMLER:
- Windows HWID (Hardware ID) okuyan ve asenkron çalışan bir 'LicenseManager' sınıfı oluştur.
- Tüm projenin ayarlarını (Çözünürlük, FPS, Proje Adı) tutan merkezi bir 'GlobalProjectContext' (Singleton) yapısı kur.

4. UI BAŞLATMA (Startup Flow):
- Uygulama açıldığında önce 3 saniye boyunca /assets/splash.png görselini bir Splash Screen olarak göster.
- Ardından şık bir 'StartupDialog' (ImGui) göster. Kullanıcıdan Proje Adı, Çözünürlük (4K, 1080p) ve FPS bilgilerini alsın.
- Pencere ikonu olarak /assets/icon.png kullanılsın.
- Ana uygulama döngüsünü (Main Loop) Vulkan üzerinde en düşük CPU yüküyle çalışacak şekilde kur.

5. BELLEK YÖNETİMİ: 
- RAM kullanımını sürekli izleyen bir 'MemoryWatchdog' iskeleti ekle. Bellek toplam RAM'in %75'ini geçerse uyarı verecek ve kullanılmayan workspace kaynaklarını boşaltacak şekilde tasarla.

Lütfen önce klasör yapısını, ardından CMakeLists.txt dosyasını ve son olarak temel main.cpp ile UI sınıflarını oluşturmaya başla. Kodlar temiz, modüler ve profesyonel yorum satırları içermelidir.