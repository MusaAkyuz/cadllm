# Teknik Şartname (Tech Spec): LLM Destekli Cross-Platform CAD Uygulaması

> **Durum:** Taslak v0.1
> **Tarih:** 2026-07-31
> **Sahip:** Musakka
> **Lisans:** Açık kaynak (LGPL uyumlu)
> **Tip:** Kişisel / açık kaynak ürün projesi

---

## 1. Özet ve Vizyon

Bu belge, OpenCASCADE (OCCT) geometri çekirdeği üzerine kurulu, hem 2B teknik çizim hem 3B katı modelleme yapabilen, **cross-platform** (Windows, macOS, Linux) masaüstü bir CAD uygulamasının teknik şartnamesidir.

Projenin **ayırt edici özelliği**, kullanıcının kendi LLM API anahtarını bağlayarak (Anthropic Claude, OpenAI GPT veya yerel Ollama modelleri) doğal dil ile tasarım yapabilmesidir. Kritik yenilik: kullanıcı ekrandaki 3B/2B nesnenin belirli bir **yüzeyini (face), kenarını (edge), noktasını (vertex)** seçip etiketleyerek LLM'e "nerede değişiklik yapması gerektiğini" bağlam olarak verir. Bu, LLM'in geometriyi çok daha isabetli algılamasını sağlar.

### Yol Haritası Felsefesi
1. **Faz 1 (MVP):** Sağlam bir çekirdek + 3B/2B görüntüleyici + temel modelleme/çizim. LLM **yok**.
2. **Faz 2:** Seçim/etiketleme altyapısı ve LLM entegrasyonu (bu belgede tasarım kararları için deney planı ile).
3. **Faz 3:** Multimodal bağlam, gelişmiş parametrik geçmiş, eklenti sistemi.

---

## 2. Hedefler ve Hedef Dışı Kapsam

### 2.1 Hedefler (Goals)
- OCCT ile B-Rep tabanlı 3B katı modelleme (kutu, silindir, extrude, revolve, boolean, fillet/chamfer).
- 2B kroki (sketch) çizimi ve krokiden 3B üretim (extrude/revolve).
- STEP ve IGES import/export; native belge formatı.
- Etkileşimli 3B görüntüleyici: döndürme, yakınlaştırma, yüzey/kenar/nokta seçimi.
- Undo/redo ve belge yönetimi.
- Üç masaüstü platformunda da çalışan tek kod tabanı.
- (Faz 2) Sağlayıcı-bağımsız LLM entegrasyonu ve seçim-tabanlı bağlam sağlama.

### 2.2 Hedef Dışı (Non-Goals) — en azından başlangıçta
- Bulut/web sürümü, çok kullanıcılı eş zamanlı düzenleme.
- Mesh/heykel (sculpting) modelleme, CAM/toolpath üretimi.
- Sonlu elemanlar (FEA) simülasyonu.
- Mobil platformlar.

---

## 3. Teknoloji Yığını (Tech Stack)

| Katman | Teknoloji | Gerekçe |
|---|---|---|
| Geometri çekirdeği | **OpenCASCADE (OCCT) 7.8+** | B-Rep modelleme, STEP/IGES, tessellation için endüstri standardı açık kaynak çekirdek. |
| Dil | **C++17** (veya C++20) | OCCT'nin native dili; en yüksek performans ve API erişimi. |
| GUI | **Qt 6** (Widgets veya Qt Quick/QML) | Olgun, cross-platform, OCCT viewer entegrasyonu iyi belgelenmiş. |
| 3B render | **OCCT AIS / V3d (OpenGL/GLES)** | Kendi renderer yazmaya gerek yok; `QOpenGLWidget` içine gömülür. |
| Belge/Undo | **OCCT OCAF** | Transaction tabanlı undo/redo + native belge formatı hazır gelir. |
| Build sistemi | **CMake** | OCCT ve Qt ile standart; cross-platform. |
| Bağımlılık yönetimi | **vcpkg** veya **Conan** | OCCT + TBB + FreeType zincirini derlemeyi kolaylaştırır. |
| LLM entegrasyonu (Faz 2) | HTTP istemcisi (Qt Network / libcurl) + sağlayıcı soyutlaması | Claude/OpenAI/Ollama'yı tek arayüz ardında toplar. |
| Test | **GoogleTest** (birim) + Qt Test (UI) | Geometri çekirdeği regresyon testleri kritik. |
| Paketleme | Windows: MSI/NSIS · macOS: `.app` + notarization · Linux: AppImage/Flatpak | Her platform için ayrı dağıtım hattı. |

### 3.1 Neden C++ + Qt (Python değil)?
Kullanıcı native performans ve tam kontrol istediği için C++ + Qt seçildi. Bu, masaüstü CAD için standart yoldur (FreeCAD çekirdeği de C++). Öğrenme ve build eğrisi Python'a göre dik, ama uzun vadeli performans, paketleme kontrolü ve OCCT API'sine tam erişim bunu telafi eder.

> **Not:** Prototipleme hızlandırmak istenirse, üst düzey iş mantığı ileride Python (pythonocc) ile betiklenebilir; ancak MVP tamamen C++ hedeflenmektedir.

---

## 4. Mimari

### 4.1 Katmanlı Mimari

```
┌─────────────────────────────────────────────────────────┐
│  Sunum Katmanı (Qt UI)                                   │
│  - Ana pencere, menüler, araç çubukları, ağaç görünümü   │
│  - 3B Viewport (QOpenGLWidget + V3d_View)                │
│  - Komut satırı / LLM sohbet paneli (Faz 2)              │
├─────────────────────────────────────────────────────────┤
│  Uygulama Katmanı (Application Logic)                    │
│  - Komut sistemi (Command pattern) + Undo/Redo           │
│  - Seçim yöneticisi (face/edge/vertex etiketleme)        │
│  - LLM Orkestratörü (Faz 2)                              │
├─────────────────────────────────────────────────────────┤
│  Belge / Veri Modeli (OCAF)                              │
│  - Parametrik geçmiş, adlandırılmış öğeler, kayıt/yükle  │
├─────────────────────────────────────────────────────────┤
│  Geometri Çekirdeği (OCCT)                               │
│  - Modelleme (BRepPrimAPI, BRepAlgoAPI, BRepFilletAPI)   │
│  - Import/Export (STEP, IGES)                            │
│  - Tessellation (BRepMesh)                               │
└─────────────────────────────────────────────────────────┘
```

### 4.2 Modül Ayrımı
Kod tabanını bağımsız kütüphanelere böl (test edilebilirlik ve LLM entegrasyonunu izole etmek için kritik):

- `core_geometry` — OCCT sarmalayıcıları; UI'dan bağımsız, saf mantık.
- `core_document` — OCAF tabanlı belge, undo/redo, kayıt/yükle.
- `core_command` — Command pattern; her modelleme işlemi bir komut nesnesi.
- `ui_viewport` — Qt + OCCT viewer köprüsü, seçim olayları.
- `ui_app` — Ana pencere, menüler, panolar.
- `llm_bridge` (Faz 2) — Sağlayıcı soyutlaması, bağlam serileştirme, tool-calling.

> **Neden bu ayrım önemli:** LLM ayağı, komut sistemini (`core_command`) çağırdığı sürece — UI'yi taklit etmek yerine — hem manuel hem AI kaynaklı işlemler aynı undo/redo ve doğrulama yolundan geçer. Bu, LLM entegrasyonunun en temiz mimari temelidir.

### 4.3 Komut Sistemi (LLM'in de kullanacağı omurga)
Her modelleme işlemi (create_box, extrude, fillet_edge, move_face...) tekil bir **Command** nesnesi olmalı:
- Parametreleri yapılandırılmış (serileştirilebilir) tutar.
- `execute()` / `undo()` içerir.
- OCAF transaction'ı içinde çalışır.

Bu tasarım, Faz 2'de LLM'in **tool-calling** ile aynı komutları çağırmasını sağlar — ayrı bir "AI yolu" yazmana gerek kalmaz.

---

## 5. LLM Entegrasyonu (Faz 2) — Deney Tabanlı Tasarım

Kullanıcı, LLM'in çıktıyı nasıl üreteceğine (komut/DSL mi, kod mu, tool-calling mi) deneme-yanılma ile karar verecek. Aşağıda üç yaklaşım ve **önerilen sıra** verilmiştir.

### 5.1 LLM Çıktı Yöntemi — Seçenekler ve Öneri

| Yaklaşım | Nasıl çalışır | Artı | Eksi |
|---|---|---|---|
| **Tool-calling (ÖNERİLEN başlangıç)** | LLM, tanımlı araç setini (`create_box`, `fillet_edge`...) yapılandırılmış argümanlarla çağırır | Denetlenebilir, güvenli, undo/redo ile uyumlu, mevcut komut sistemine doğrudan oturur | Araç setini önceden tanımlaman gerekir; çok yaratıcı/serbest işlemlerde sınırlı |
| **Parametrik komut/DSL üretimi** | LLM, uygulamanın kendi metin DSL'ini üretir; parser çalıştırır | İnsan-okunur, sürümlenebilir, tekrar oynatılabilir | DSL tasarımı ve parser bakımı gerekir |
| **Doğrudan kod (pythonocc) üretimi** | LLM Python/OCCT kodu yazar, sandbox'ta koşar | En esnek | **Güvenlik riski yüksek**, kararsız, sandbox zorunlu — açık kaynak üründe risk |

**Öneri:** Tool-calling ile başla (komut sistemin zaten bunu destekleyecek şekilde tasarlandı). DSL'i ikinci deney olarak dene. Doğrudan kod üretimini yalnızca güçlü sandbox varsa ve son çare olarak düşün.

### 5.2 Deney Planı (kullanıcının "deneme-yanılma" ihtiyacı için)
Faz 2'yi ölçülebilir bir deneye dönüştür:
1. 8–10 temsili tasarım görevi tanımla (ör. "bu yüzeye 5mm derinlik cep aç").
2. Her yöntemi (tool-calling / DSL) aynı görevlerde çalıştır.
3. Metrikler: başarı oranı, gereken düzeltme adımı sayısı, hata tipi, token maliyeti.
4. Kazanan yöntemi varsayılan yap, diğerini deneysel bayrak ardında tut.

### 5.3 Seçim / Etiketleme Mekanizması (Bağlam Sağlama)
Bu, projenin farkını yaratan çekirdek. MVP'de temel seçim altyapısını kurup Faz 2'de LLM bağlamına bağla. Tasarım seçenekleri (kullanıcı çekirdek geliştirirken netleştirecek):

- **Kalıcı kimlikler (kritik):** OCCT topolojisi işlem sonrası yeniden numaralandığından, seçilen face/edge/vertex'e **kalıcı, kararlı bir kimlik** atamak gerekir (OCCT'nin "topological naming problem"i). OCAF'ın `TNaming` mekanizması bunun için tasarlanmıştır — bu problemi baştan ciddiye al.
- **Bağlam serileştirme:** Seçilen öğe(ler) LLM'e nasıl gider? Öneri: yapılandırılmış JSON — öğe tipi, kalıcı kimlik, geometrik özellikler (alan, normal, koordinat, komşuluklar), kullanıcı etiketi.
- **Multimodal (ileri):** Viewport render'ı (görsel) + seçili öğe kimliği birlikte gönderilerek LLM'in görsel algısı güçlendirilebilir (Claude/GPT vision).

### 5.4 Sağlayıcı Soyutlaması
Tek bir `LlmProvider` arayüzü ardında:
- `AnthropicProvider` (Claude — tool-calling güçlü)
- `OpenAiProvider` (GPT)
- `OllamaProvider` (yerel, gizlilik, internet gerektirmez)

Kullanıcı ayarlardan sağlayıcı seçer ve API anahtarını girer. Anahtarlar **OS keychain** (Windows Credential Manager / macOS Keychain / libsecret) ile güvenli saklanır — düz metin dosyada asla.

---

## 6. Veri Modeli ve Dosya Formatları

- **Native format:** OCAF tabanlı `.xbf` (binary) veya `.xml`. Parametrik geçmiş ve adlandırılmış öğeleri saklar.
- **Import/Export:** STEP (AP203/AP214/AP242), IGES. `STEPControl_Reader/Writer`, `IGESControl_*`.
- **Mesh export (ileri):** STL/OBJ (3B baskı için).
- **Sağlamlık:** Gerçek dünyada bozuk/uyumsuz STEP dosyaları yaygındır; import katmanına doğrulama ve onarım (`ShapeFix`) ekle.

---

## 7. Kullanıcı Arayüzü (Üst Düzey)

- Ana pencere: menü, araç çubuğu, model ağacı (belge yapısı), 3B viewport, özellik paneli.
- **3B Viewport:** yörünge kamerası, seçim vurgulama, ölçü gösterimi.
- **2B Sketch modu:** düzlem seç, çizgi/yay/daire/kısıt, extrude/revolve ile 3B'ye geçiş.
- **LLM paneli (Faz 2):** sohbet girişi, seçili öğe "çipleri", önerilen işlemin önizleme + onay akışı ("LLM şunu yapacak: ... [Uygula] [İptal]").

> **Güvenlik ilkesi:** LLM önerdiği her geometrik değişiklik, uygulanmadan önce kullanıcı onayından geçmeli (özellikle açık kaynak/deneysel evrede). Undo her zaman mümkün olmalı.

---

## 8. Başlıca Riskler ve Zorluklar

| Risk / Zorluk | Etki | Azaltma |
|---|---|---|
| **OCCT öğrenme eğrisi** | Yüksek — en büyük zaman kaybı burada | Küçük prototiplerle başla; `Handle_`, `TopExp_Explorer`, geometri/topoloji ayrımını erken öğren |
| **Topological naming problem** | Yüksek — seçim/etiketleme ve LLM bağlamının temeli buna dayanır | OCAF `TNaming` kullan; kalıcı kimlik stratejisini MVP'de tasarla |
| **Build & bağımlılık cehennemi** (özellikle Windows) | Orta-Yüksek | vcpkg/Conan; CI'da üç platformu erken kur |
| **Viewer'ı Qt'ye gömme** (OpenGL context, pick olayları) | Orta | Bilinen `QOpenGLWidget + V3d_View` desenini takip et; hazır örnekleri incele |
| **Cross-platform paketleme & macOS notarization** | Orta | Baştan CI/CD kur; imzalama sertifikalarını erken hazırla |
| **OCCT thread güvenliği değil** | Orta | Ağır işlemleri (boolean, mesh) worker thread'de izole et; UI'yı dondurma |
| **Büyük model performansı** (tessellation, seçim) | Orta | Deflection ayarı, LOD, seçim optimizasyonu |
| **LLM çıktısının güvenilmezliği** | Yüksek (Faz 2) | Tool-calling + zorunlu onay + undo; asla otomatik uygulama |
| **LLM güvenliği** (kod üretimi yolu seçilirse) | Yüksek | Kod yolundan kaçın veya güçlü sandbox; tercih tool-calling |
| **API anahtarı güvenliği** | Orta | OS keychain; düz metin saklama yok |
| **LGPL lisans uyumu** | Düşük-Orta | Dinamik linkleme; lisans yükümlülüklerini README'de belgele |

---

## 9. Kilometre Taşları (Milestones)

**M0 — Temel altyapı (1–2 hafta)**
CMake + Qt + OCCT derleyen iskelet; boş pencere + boş viewport; üç platformda CI derlemesi.

**M1 — Görüntüleyici + STEP import**
`QOpenGLWidget` + V3d_View; STEP/IGES aç ve görüntüle; yörünge kamerası; temel seçim (face/edge/vertex vurgulama).

**M2 — Temel 3B modelleme**
Primitifler (kutu, silindir), extrude/revolve, boolean, fillet/chamfer; Command pattern + OCAF undo/redo.

**M3 — 2B Sketch**
Düzlem üzerine kroki, temel kısıtlar, krokiden extrude/revolve.

**M4 — Belge & Export**
Native kayıt/yükle (OCAF), STEP/IGES/STL export; kalıcı öğe kimlikleri (TNaming).

**M5 — Paketleme & 1.0 çekirdek sürümü**
Windows/macOS/Linux dağıtım paketleri; kullanıcı belgeleri.

**M6+ — LLM Faz 2**
Sağlayıcı soyutlaması → seçim-tabanlı bağlam → tool-calling deneyleri → onay akışlı LLM paneli.

---

## 10. Test Stratejisi

- **Birim testleri (GoogleTest):** geometri komutları — her komut için "girdi şekli → beklenen çıktı" regresyonu.
- **Golden-file testleri:** STEP import/export round-trip; kayıp/bozulma kontrolü.
- **UI/entegrasyon (Qt Test):** temel etkileşim akışları.
- **LLM değerlendirme paketi (Faz 2):** §5.2'deki görev seti üzerinde otomatik başarı/regresyon ölçümü.
- **CI:** üç platformda derleme + test her PR'da.

---

## 11. Açık Kaynak & Topluluk

- **Lisans:** OCCT LGPL 2.1 (+ istisna) ile uyumlu bir lisans seç (LGPL veya daha izin verici bir üst katman). Dinamik linkleme ile LGPL yükümlülüklerini karşıla.
- **Depo:** README (kurulum/build), CONTRIBUTING, mimari dokümanı, ADR klasörü (teknoloji kararları için).
- **CI/CD:** GitHub Actions ile üç platform build + release artefaktları.

---

## 12. Açık Kararlar (Sonraki Adımlarda Netleşecek)

- LLM çıktı yöntemi: tool-calling vs DSL — §5.2 deney sonucuna göre.
- Seçim/etiketleme UX'i: çekirdek geliştirilirken netleşecek.
- Native dosya formatı: binary (`.xbf`) vs XML.
- Qt Widgets vs Qt Quick (QML) tercih.
- Parametrik geçmiş derinliği (tam parametrik ağaç mı, doğrudan modelleme mi).

---

*Bu taslak, kararlar netleştikçe güncellenmelidir. Her önemli teknoloji kararı için ayrı bir ADR (Architecture Decision Record) tutulması önerilir.*