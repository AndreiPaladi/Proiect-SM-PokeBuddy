# Vizualizare 3D și Web Server pe ESP32

**Autori:** Paladi Andrei · Ciobanu Andrei · Timofte Alin‑Gabriel  
**Data:** Mai 2025

## Cuprins
- [Introducere](#introducere)
- [Scopul Proiectului](#scopul-proiectului)
- [Componente Hardware](#componente-hardware)
- [Arhitectura Sistemului](#arhitectura-sistemului)
- [Funcționalitate Software](#funcționalitate-software)
  - [Exemple de cod](#exemple-de-cod)
- [Interfața Web](#interfața-web)
- [Testare și Dificultăți](#testare-și-dificultăți)
- [Ce Am Învățat](#ce-am-învățat)
- [Concluzii](#concluzii)
- [Bibliografie](#bibliografie)
- [Demo & Cod](#demo--cod)

---

## Introducere
Internet of Things (IoT) este un domeniu în plină expansiune care permite interconectarea dispozitivelor fizice prin rețele digitale. Acest proiect urmărește realizarea unei aplicații care combină senzori, procesare și interfață grafică, toate implementate pe un microcontroler **ESP32‑C3**.

## Scopul Proiectului
Scopul este de a implementa un sistem embedded care:
- Preia date de la un senzor de accelerație (**MPU6500**);
- Afișează în timp real un cub 3D rotativ pe un ecran **ST7789**;
- Oferă o interfață web unde este vizualizată mișcarea sub forma unei mingi animate.

## Componente Hardware

| Componentă             | Rol                | Detalii                                                      |
|------------------------|--------------------|--------------------------------------------------------------|
| **ESP32‑C3 Super Mini**| Controller principal | Procesor RISC‑V, Wi‑Fi, GPIO, consum redus                  |
| **MPU6500**            | Senzor IMU         | Accelerometru + giroscop pe 3 axe, I²C                       |
| **ST7789**             | Display SPI        | LCD color 240×240 px, randare 2D/3D                          |

## Arhitectura Sistemului
```text
MPU6500 ──> ESP32‑C3 ──> ST7789
                 │
                 └──> Web Browser
```
- Senzorul **MPU6500** măsoară accelerația.  
- **ESP32‑C3** citește datele, le filtrează, le afișează și le publică prin Wi‑Fi.  
- **ST7789** afișează cubul 3D.  
- Browser‑ul redă mingea animată pe baza datelor primite.

## Funcționalitate Software
Proiectul este scris în **C++** pe framework‑ul Arduino. Structura codului:
1. **Inițializare hardware** – senzor, ecran, Wi‑Fi;  
2. **Prelucrare date** – filtru *low‑pass*;  
3. **Redare grafică** – cub 3D pe ecran;  
4. **Interfață Web** – actualizare mingea virtuală.

### Exemple de cod

**Filtrarea senzorului MPU6500**
```cpp
Wire.begin(SDA_PIN, SCL_PIN);
mpu.init(calib);
mpu.update();
mpu.getAccel(&accel);

filteredX = alpha * accel.accelX + (1 - alpha) * filteredX;
filteredY = alpha * accel.accelY + (1 - alpha) * filteredY;
```

**Proiecția cubului 3D**
```cpp
float rx = vertex[i][2] * sin(y) + vertex[i][0] * cos(y);
float ry = rx * cos(x) - vertex[i][1] * sin(x);
float rz = rx * cos(z) - ry * sin(z);

float screenX = rx * viewDistance / (viewDistance + rz) + xOrigin;
float screenY = ry * viewDistance / (viewDistance + rz) + yOrigin;
```

## Interfața Web
Microcontrolerul rulează un web‑server care servește o pagină **HTML/CSS/JS**. Mișcarea mingii este actualizată periodic printr‑un *fetch*:
```js
fetch('/data')
  .then(r => r.json())
  .then(data => {
    velX += data.accelX * accelScale;
    posX += velX;
    document.getElementById('ball').style.left = posX + 'px';
  });
```

## Testare și Dificultăți

| Problemă                                       | Soluție                                        |
|-----------------------------------------------|------------------------------------------------|
| Documentație incompletă pentru MPU6500        | Biblioteca **FastIMU** pentru abstractizare    |
| Configurare inițială ST7789                   | Ajustarea pinii SPI + driver **TFT_eSPI**      |
| Zgomot al accelerometrului                    | Filtru *low‑pass* adaptiv                      |

## Ce Am Învățat
- Inițializarea și utilizarea senzorilor I²C în sisteme embedded;
- Cum funcționează un server web minimalist pe microcontroler;
- Bazele proiecției 3D pe 2D și transformărilor matematice aferente;
- Sincronizarea task‑urilor și desenarea în timp real.

## Concluzii
Am demonstrat că **ESP32‑C3** poate gestiona un sistem IoT cu vizualizare grafică locală și interfață web. Sistemul poate fi extins pentru detectarea mișcării, control prin gesturi sau integrare în aplicații AR/VR.

## Bibliografie
- <https://randomnerdtutorials.com>  
- <https://github.com/mjs513/FastIMU>  
- <https://github.com/Bodmer/TFT_eSPI>  
- <https://docs.espressif.com>  

## Demo & Cod
- 🎬 Video demo: <https://drive.google.com/file/d/1_Hu4CtCq_ipnwNAGPkaHM4mkgjyMcDfi/view>  
- 🎬 Video demo: <https://drive.google.com/file/d/1DvNk_oyfeJiqVnGHw953YaIxfmRQ6-sX/view>  
- 💻 Surse complete GitHub (PokeBuddy): <https://github.com/AndreiPaladi/Proiect-SM-PokeBuddy>
