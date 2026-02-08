# 🌌 AETHER STUDIO - MASTER BLUEPRINT (Professional Creative Suite)

## 1. VİZYON VE KİMLİK
Aether Studio; kurgu (Edit), animasyon (VFX), tasarım (Photo/Photoshop), RAW fotoğraf işleme (Camera RAW/Lightroom), profesyonel renk düzenleme (Color) ve ses işleme (Audio) süreçlerini tek bir çatı altında toplayan bağımsız (standalone) bir profesyonel yaratıcı pakettir.
- **Amiral Gemisi:** Gelecekteki "Aether OS" (Creative Linux Distro) projesinin temel yazılımıdır.
- **Geliştirme Platformu:** Windows (C++ 20 / ImGui / Vulkan).

---

## 2. TEKNİK ÇEKİRDEK VE PERFORMANS
- **Evrensel GPU Desteği:** - NVIDIA: RTX (CUDA/NVENC), GTX ve eski seriler (Vulkan).
    - AMD: RX, XT ve eski HD serileri (Vulkan/OpenCL).
    - Intel/Integrated: iGPU (QuickSync) hızlandırması ile işlemci yükünü azaltma.
- **8GB RAM Optimizasyonu:** "Tiling" ve "Sparse Textures" kullanarak sadece ekranda görünen pikselleri işleme.
- **Renk Bilimi:** 32-bit Floating Point YRGB & ACES Workflow. 8-bit, 10-bit (4:2:2) ve HDR desteği.
- **Video Motoru:** FFmpeg tabanlı, donanım hızlandırmalı (Hardware Accelerated) okuma ve yazma.

---

## 3. ÇALIŞMA ALANLARI (WORKSPACES) & ARAÇLAR

### 🎞️ EDIT (NLE - Kurgu)
- **Araçlar:** Selection (V), Universal Blade (C), Ripple Edit (B), Slip (Y), Text (T).
- **Özellikler:** Profesyonel Timeline, Multi-track yönetimi, Proxy iş akışı.

### ✨ ANIMATION (VFX & Motion Graphics)
- **Mantık:** Katman (Layer) bazlı başlar, istendiğinde Düğüm (Node) bazlı derinleşir.
- **Araçlar:** Transform (V), Advanced Pen (P), Puppet Pin, Magic Tracker (AI), Graph Editor (Spline).
- **Özellikler:** Sıfırdan kompozisyon yaratma, vektörel şekil ve yazı animasyon motoru.

### 📸 PHOTO & RAW (Tasarım - Photoshop/Lightroom Hibriti)
- **Mantık:** Photoshop (Katmanlı tasarım) ve Camera RAW (RAW banyo) entegrasyonu.
- **Araçlar:** Healing Brush (H), Clone Stamp (S), Adjustment Brush (B), Gradient (G), Sıfırdan Fırça (Brush) Motoru.
- **Özellikler:** 32-bit RAW işleme, HSL Mixer, AI Maskeleme (Özne/Gökyüzü), Sıfırdan Tuval (New Canvas).

### 🎨 COLOR (Grading - Resolve Seviyesi)
- **Araçlar:** Primary Wheels, Qualifier (L), Power Windows (W), RGB Curves, Scopes.
- **Özellikler:** Profesyonel HDR kontrolü, LUT desteği, Düğüm bazlı renk yönetimi.

### 🔊 AUDIO (Sound - Fairlight/Audition Hibriti)
- **Araçlar:** Range Selection (A), Envelope Pen (P), Spectral Cleaner.
- **Özellikler:** Çok kanallı mikser, VST3 desteği, spektral gürültü silme.

### 📦 DELIVER (Export / Render)
- **Özellikler:** Render Queue (Kuyruk), Multi-format export, NVIDIA/AMD/Intel donanım hızlandırmalı çıktı.

---

## 4. HESAP SİSTEMİ VE LİSANSLAMA
- **Aether Community (Ücretsiz):** 4K çıktı sınırlı, 8-bit işleme, temel araç seti.
- **Aether Studio (Pro):** - **Hesap:** Web sitesi üzerinden "Aether Account" entegrasyonu.
    - **Aktivasyon:** Satın alma sonrası hesaba tanımlanan RSA-256 şifreli Activation Code.
    - **Pro Özellikler:** 8K+ çıktı, 10-bit 4:2:2/HDR desteği, Gelişmiş AI araçları, Cloud Rendering.

---

## 5. CURSOR AI GELİŞTİRME YOL HARİTASI (ROADMAP)

### FAZ 1: TEMEL MİMARİ
1. **Project Startup Dialog:** "New/Open Project" penceresi (İsim, Çözünürlük, FPS - OK/Cancel).
2. **Global Project Context:** Tüm sekmelerin eriştiği ana veri yapısı (Singleton).
3. **Account & License:** Login ekranı ve aktivasyon kodu doğrulama sistemi.

### FAZ 2: DONANIM VE MEDYA HATTI
1. **Hardware Orchestrator:** Donanımı (GPU/iGPU) algılayıp render modlarını seçen motor.
2. **Keymap Manager:** JSON tabanlı, özelleştirilebilir "Neo" kısayol şeması.
3. **Universal Video Loader:** 8/10-bit videoları GPU üzerinden okuyan FFmpeg entegrasyonu.

### FAZ 3: ARAYÜZ VE ARAÇLAR
1. **Workspace Manager:** Sekmeler arası (Edit <-> Animation vb.) anlık geçiş sistemi.
2. **Toolbar System:** Her sekmenin kendi özel araçlarını (V, C, B, P vb.) yüklemesi.
3. **Core Render View:** GPU Shader (GLSL) tabanlı görüntüleme ve efekt motoru.

---

## 6. GELİŞTİRME KURALLARI
- **Bellek:** 8GB RAM sınırı nedeniyle akıllı pointer (`std::unique_ptr`) ve bellek sızıntısı kontrolü zorunludur.
- **Performans:** Ağır matematiksel işlemler her zaman GPU Shader'da (GLSL) yapılacak. UI thread asla dondurulmayacak.
- **Stabilite:** GPU hata verirse sistem otomatik olarak CPU (Software Fallback) moduna geçecek.