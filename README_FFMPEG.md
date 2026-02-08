# FFmpeg Kurulum Rehberi

## FFmpeg Full Build Kurulumu

FFmpeg development dosyalarını (include ve lib) kullanmak için full build kurmanız gerekiyor.

### Otomatik Kurulum

```powershell
powershell -ExecutionPolicy Bypass -File fix_ffmpeg.ps1
```

### Manuel Kurulum

1. **FFmpeg Full Build İndirin:**
   - https://www.gyan.dev/ffmpeg/builds/ adresine gidin
   - **"ffmpeg-release-full.zip"** dosyasını indirin (yaklaşık 100MB)

2. **Kurulum:**
   - ZIP dosyasını `C:\ffmpeg` dizinine açın
   - Dizin yapısı şöyle olmalı:
     ```
     C:\ffmpeg\
       ├── bin\
       ├── include\
       │   └── libavcodec\
       │       └── avcodec.h
       └── lib\
           ├── avcodec.lib
           ├── avformat.lib
           └── ...
     ```

### CMake'de Manuel Path Belirtme

Eğer FFmpeg farklı bir konumda kuruluysa:

1. **CMakeSettings.json'da:**
   ```json
   {
       "name": "FFMPEG_ROOT",
       "value": "C:/path/to/ffmpeg",
       "type": "PATH"
   }
   ```

2. **Veya CMake komut satırında:**
   ```bash
   cmake .. -DFFMPEG_ROOT=C:/path/to/ffmpeg
   ```

### Doğrulama

FFmpeg'in doğru kurulduğunu kontrol etmek için:

```powershell
powershell -ExecutionPolicy Bypass -File check_ffmpeg_full.ps1
```

### Not

- FFmpeg **opsiyoneldir** - proje FFmpeg olmadan da çalışır
- FFmpeg olmadan video işleme özellikleri devre dışı kalır
- OpenSSL zaten bulundu ve kullanılabilir
