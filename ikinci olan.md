# 🌌 AETHER STUDIO - PROJECT MASTER BLUEPRINT & BUILD GUIDE

## 1. VİZYON VE KİMLİK
Aether Studio; kurgu (Edit), animasyon (VFX), tasarım (Photo/Photoshop), RAW fotoğraf işleme (Camera RAW), profesyonel renk düzenleme (Color) ve ses işleme (Audio) süreçlerini tek bir çatı altında toplayan bağımsız (standalone) bir profesyonel yaratıcı pakettir.
- **Motto:** "Donanımının Sınırlarını Zorla."
- **Amiral Gemisi:** Gelecekteki "Aether OS" (Creative Linux Distro) projesinin temel yazılımıdır.
- **Geliştirme Platformu:** Windows (C++ 20 / ImGui / Vulkan).

---

## 2. TEKNİK ÇEKİRDEK VE 8GB RAM OPTİMİZASYONU
Düşük RAM'li (8GB) sistemlerde (i5-12500H + RTX 4050) yüksek performans kuralları:
- **Universal GPU:** NVIDIA (RTX/GTX), AMD (RX/XT/HD) ve Intel iGPU desteği (Vulkan/OpenCL).
- **Tiling Render:** Görüntüyü 512x512 piksellik parçalara bölerek sadece ekranda görünen alanı işleme.
- **Smart Resource Streaming:** Kullanılmayan workspace verilerini VRAM'den anında boşaltma.
- **RAM Sınırı:** Bellek kullanımı toplam RAM'in %75'ini asla geçmez.

---

## 3. RENDER STRATEJİSİ (SIFIR MALİYETLİ PRO ÇÖZÜMLER)
- **Aether Link (LAN Rendering):** Yerel ağdaki diğer bilgisayarları render işlemine dahil etme.
- **Smart Background Cache:** Kullanıcı kurgu yaparken boşta kalan GPU gücüyle efektleri SSD'ye önbelleğe alma.
- **Proxy & AI Upscale:** Düşük çözünürlüklü kurgu, RTX Tensor çekirdekleri ile yüksek çözünürlüklü (8K'ya kadar) AI Upscale çıktı.

---

## 4. HESAP SİSTEMİ VE WEB ENTEGRASYONU
- **Web Sitesi:** Next.js, Tailwind CSS ve Supabase kullanılarak oluşturulmuş Dashboard ve Satın Alma sayfası.
- **Aktivasyon (RSA-256):** Donanım Kimliği (HWID) tabanlı kilitli aktivasyon sistemi. Kodlar sadece eşleşen cihazda çalışır.

---

## 5. ÇALIŞMA ALANLARI (WORKSPACES) & ARAÇLAR
| Sekme | Esin Kaynağı | Temel Araçlar |
| :--- | :--- | :--- |
| **EDIT** | Premiere/Resolve | Selection (V), Blade (C), Ripple (B), Slip (Y), Text (T). |
| **ANIMATION** | AE/Fusion | Pen Tool (P), Transform, Puppet Pin, Node Graph. |
| **PHOTO** | PS/Lightroom | Healing Brush (H), Clone Stamp, Adjustment Brush, RAW Sliderlar. |
| **COLOR** | Resolve | Primary Wheels, Qualifier (L), Power Windows, Scopes. |
| **AUDIO** | Fairlight/Audition | Range Selection, Envelope Pen, Spectral Cleaner. |

---

## 6. DERLEME REHBERİ (BUILD INSTRUCTIONS)
Projeyi Windows üzerinde derlemek ve EXE haline getirmek için gerekli araçlar ve adımlar:

### A. Gerekli Araçlar (Prerequisites)
1. **Visual Studio 2022:** "Desktop Development with C++" paketi yüklü olmalı.
2. **CMake (3.20+):** Derleme yönetimi için.
3. **Vulkan SDK:** RTX 4050 GPU iletişimi için.
4. **vcpkg (C++ Paket Yöneticisi):** Kütüphaneleri kurmak için.

### B. Kütüphane Kurulum Komutları
```bash
vcpkg install imgui[vulkan-binding,win32-binding,docking-experimental]
vcpkg install ffmpeg[gpl,nvcodec,swscale]
vcpkg install libraw nlohmann-json opencv4[ffmpeg] miniaudio