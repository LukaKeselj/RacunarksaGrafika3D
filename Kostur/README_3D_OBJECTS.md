# 3D Objects Documentation - Aim Trainer

Kompletan vodi? kroz sve 3D objekte koji se generišu u **3D Aim Trainer** projektu.

---

## ?? Pregled 3D Objekata

Projekat sadrži **5 glavnih tipova 3D objekata**:

1. **Target Cylinders** (3D Targeti)
2. **Room** (Soba)
3. **Light Source** (Izvor svjetla)
4. **Wall Weapons** (Oružja na zidu - OBJ modeli)
5. **2D UI Elements** (HUD/Overlay)

---

## ?? 1. TARGET CYLINDERS (3D Targeti)

### Opis
Cilindri koji se pojavljuju kao targeti u igri. Uvijek okrenuti prema kameri (billboard effect).

### Tehni?ki Detalji

**Fajl:** `AimTrainer.cpp` ? `initCylinder()`

**Geometrija:**
- **Tip**: Proceduralno generisan 3D cilindar
- **Segments**: 32 (broj segmenata oko kružnice)
- **Radius**: 1.0 (default, skalira se pri renderovanju)
- **Depth**: 0.05 (jako tanak cilindar)

**Struktura Vertices:**
```cpp
Vertex Format: [Position(3), Normal(3), TexCoords(2)] = 8 floats
- Front center: (0, 0, +depth)
- Back center: (0, 0, -depth)
- Front circle: 33 vertices (0° do 360°)
- Back circle: 33 vertices
- Side vertices: 66 vertices (2 per segment)
```

**Komponente:**
1. **Front Face**: Triangle fan od centra
2. **Back Face**: Triangle fan od centra (reversed winding)
3. **Side**: Quad strips izme?u front i back circles

**Broj vertices:**
- Centers: 2
- Front circle: 33
- Back circle: 33
- Side: 66
- **Total: 134 vertices**

**Broj triangles:**
- Front: 32
- Back: 32
- Side: 64
- **Total: 128 triangles = 384 indices**

### Rendering

**Shader:** `sphere3d.vert` + `sphere3d.frag`

**Features:**
- Billboard rotation (uvijek okrenut prema kameri)
- Phong lighting (ambient + diffuse + specular)
- Pulsating glow effect (sinusoidal)
- Texture mapping (terrorist.png / counter.png)
- Transparency support

**Uniforms:**
- `uModel` - Model matrix (rotation + scale)
- `uView` - View matrix (kamera)
- `uProjection` - Projection matrix
- `uLightPos` - Pozicija svjetla (0, 4, 0)
- `uViewPos` - Pozicija kamere
- `uTime` - Vrijeme (za pulsiranje)
- `uTexture` - Tekstura targeta

### Spawn Sistem

**Funkcija:** `spawnTarget()`

**Spawn Pravila:**
- Spawn-uje se na zidovima sobe (4 mogu?a zida)
- Mora biti u FOV kamere (dot product > 0.707 = 45°)
- Mora biti centriran (offset < 0.5 horizontalno, < 0.4 vertikalno)
- Margin od ivica: 3.5 * radius
- Max pokušaja: 20

**Pozicije:**
```cpp
Front wall:  z = -10 + 0.5 = -9.5
Back wall:   z = +10 - 0.5 = +9.5
Left wall:   x = -10 + 0.5 = -9.5
Right wall:  x = +10 - 0.5 = +9.5
```

### Životni Ciklus

```
1. spawn() ? Active = true
2. update(dt) ? LifeTime -= dt
3. LifeTime <= 0 ? Active = false, Lives--
4. raySphereIntersection() ? Hit detected
5. Active = false ? Removed from vector
```

**Difficulty Scaling:**
- `targetLifeTimeMultiplier` = 1.0 - (elapsedTime / 5.0) * 0.12
- Min: 0.4 (60% brže nakon 30s)

---

## ?? 2. ROOM (Soba)

### Opis
Zatvorena prostorija sa teksturiranim zidovima, podom i plafonom.

### Tehni?ki Detalji

**Fajl:** `AimTrainer.cpp` ? `initRoom()`

**Dimenzije:**
- **Width**: 20.0 (X osa)
- **Height**: 10.0 (Y osa)
- **Depth**: 20.0 (Z osa)
- **Half dimensions**: (10, 5, 10)

**Struktura:**
```
Total: 6 faces (4 walls + floor + ceiling)
Vertices per face: 4
Total vertices: 24
Total triangles: 12 (2 per face)
```

**Vertex Format:**
```cpp
[Position(3), Normal(3), TexCoords(2)] = 8 floats
```

**Faces i Normals:**
```cpp
Front wall  (z=-10): Normal = (0, 0, +1)  // Obrnut jer smo UNUTRA
Back wall   (z=+10): Normal = (0, 0, -1)
Left wall   (x=-10): Normal = (+1, 0, 0)
Right wall  (x=+10): Normal = (-1, 0, 0)
Floor       (y=-5):  Normal = (0, +1, 0)
Ceiling     (y=+5):  Normal = (0, -1, 0)
```

**Texture Scaling:**
```cpp
Walls:   4x tiling horizontalno, 2x vertikalno
Floor:   4x4 tiling
Ceiling: 4x4 tiling
```

**Winding Order:**
- Walls: CCW (counter-clockwise) kada gledaš spolja
- Floor: Reversed (jer gledamo odozdo)
- Ceiling: Normal CCW

### Rendering

**Shader:** `room.vert` + `room.frag`

**Features:**
- Phong lighting model
- Texture mapping sa tiling-om
- Ambient light: 0.3
- Diffuse + Specular komponente
- Light attenuation po distanci

**Teksture:**
- Walls: `smooth-white-brick-wall.jpg`
- Floor: `floor.jpg`
- Ceiling: `ceiling.png`

**Uniforms:**
- `uModel` - Identity matrix (fiksirana pozicija)
- `uView` - View matrix
- `uProjection` - Projection matrix
- `uWallColor` - (0.8, 0.8, 0.8)
- `uUseTexture` - 1 (enabled)
- `uLightPos` - (0, 4, 0)
- `uViewPos` - Camera position
- `uWallTexture` - Sampler2D

### Draw Calls

Renderuje se u **6 odvojenih draw call-ova:**
```cpp
1. Front wall  (indices 0-5)
2. Back wall   (indices 6-11)
3. Left wall   (indices 12-17)
4. Right wall  (indices 18-23)
5. Floor       (indices 24-29) + floorTexture
6. Ceiling     (indices 30-35) + ceilingTexture
```

---

## ?? 3. LIGHT SOURCE (Izvor Svjetla)

### Opis
Svijetle?a kutija koja emituje svjetlo. Vizuelni reprezentacija point light-a.

### Tehni?ki Detalji

**Fajl:** `AimTrainer.cpp` ? `initLight()`

**Dimenzije:**
- **Width**: 2.0
- **Height**: 0.3 (jako pljosnata)
- **Depth**: 2.0

**Pozicija:**
```cpp
Position: (0.0, 4.5, 0.0)  // Visoko iznad centra sobe
Light calculation position: (0.0, 4.0, 0.0)
```

**Geometrija:**
- **Tip**: Box (kocka)
- **Faces**: 6
- **Vertices**: 24 (4 per face)
- **Triangles**: 12

**Vertex Format:**
```cpp
[Position(3), Normal(3)] = 6 floats
```

### Rendering

**Shader:** `light.vert` + `light.frag`

**Features:**
- **Emissive material** - emituje svjetlo
- **Glow effect** - edge glow na ivicama
- **No texture** - solid color emission

**Lighting Equation:**
```glsl
emission = uLightColor * uIntensity
edgeGlow = pow(1.0 - abs(dot(Normal, ViewDir)), 2.0) * 0.3
finalColor = emission + (edgeGlow * uLightColor)
```

**Uniforms:**
- `uModel` - Translation matrix
- `uView` - View matrix
- `uProjection` - Projection matrix
- `uLightColor` - (1.0, 0.95, 0.8) - toplo bijela
- `uIntensity` - 2.0 (jaka emisija)

**Attenuation Formula** (koristi se u ostalim shader-ima):
```cpp
distance = length(lightPos - fragPos)
attenuation = 1.0 / (1.0 + 0.045 * distance + 0.0075 * distance²)
```

---

## ?? 4. WALL WEAPONS (Oružja na Zidu)

### Opis
3D modeli oružja (AK-47 i USP-S) u?itani iz OBJ fajlova i postavljeni na zidove.

### Tehni?ki Detalji

**Fajl:** `AimTrainer.cpp` ? `initWallWeapons()`

**Loader:** `OBJLoader.cpp` ? `loadOBJ()`

### AK-47

**Model File:** `obj/ak47.obj`
**Texture:** `obj/weapon_rif_ak47.png`

**Transform:**
```cpp
Position: (7.0, -3.5, -9.5)     // Bottom-right corner, front wall
Rotation: (0°, 180°, 90°)       // Rotiran za display
Scale:    (150, 150, 150)       // Jako velik model
```

**Hitbox:**
```cpp
Original: position ± scale
Reduced:  position ± (scale * 0.004)  // 0.4% originalne veli?ine!
Purpose: Ta?no pucanje za pickup
```

### USP-S

**Model File:** `obj2/usp.obj`
**Texture:** `obj2/weapon_pist_usp_silencer.png`

**Transform:**
```cpp
Position: (-8.5, -3.5, -9.5)    // Bottom-left corner, front wall
Rotation: (0°, 180°, 90°)
Scale:    (150, 150, 150)
```

### OBJ Loader Details

**Podržani Formati:**
```
v   x y z           - Vertex position
vt  u v             - Texture coordinate
vn  x y z           - Vertex normal
f   v/vt/vn ...     - Face (triangles only!)
```

**Vertex Format:**
```cpp
[Position(3), Normal(3), TexCoords(2)] = 8 floats
```

**Funkcionalnost:**
1. Parse OBJ file
2. Triangulate faces (mora biti 3 vertices!)
3. Generate vertex buffer sa interleaved data
4. Generate element buffer (indices)
5. Setup VAO + VBO + EBO
6. Load texture

### Rendering

**Shader:** `sphere3d.vert` + `sphere3d.frag` (isti kao targeti!)

**Features:**
- Phong lighting
- Texture mapping
- Face culling **DISABLED** (da se vide obe strane)
- Alpha blending enabled

**Uniforms:**
```cpp
uModel, uView, uProjection  - Transform matrices
uLightPos, uViewPos         - Lighting
uTime                       - Animation (unused)
uTexture                    - Weapon texture
```

### Pickup System

**Raycasting:** `rayAABBIntersection()`

**AABB (Axis-Aligned Bounding Box):**
```cpp
min = weapon.position - weapon.scale * 0.004f
max = weapon.position + weapon.scale * 0.004f

Example:
Position: (7.0, -3.5, -9.5)
Scale: (150, 150, 150)
Hitbox size: ±0.6 units (JAKO MALI!)
```

**Pickup Logic:**
```cpp
1. Ray-AABB intersection check
2. If hit && weapon.isAK && currentMode != AK47:
   ? Switch to AK-47
3. If hit && !weapon.isAK && currentMode != USP:
   ? Switch to USP-S
4. Return (ne count kao klik)
```

---

## ?? 5. 2D UI ELEMENTS (HUD/Overlay)

Iako nisu "prava" 3D geometrija, renderuju se kao 2D quad-ovi preko 3D scene.

### Komponente

#### A) **Rectangles** (Boxovi)

**Funkcija:** `drawRect()`

**Geometrija:**
- **Type**: 2D Quad
- **Vertices**: 6 (2 triangles)
- **Koordinate**: Screen space (pixels)

**Vertex Format:**
```cpp
[Position(2)] = 2 floats
```

**Korištenje:**
- HUD background (black + gray boxes)
- Game Over screen background
- Buttons (Restart/Exit)
- Crosshair lines

**Shader:** `rect.vert` + `rect.frag`

**Projection:**
```cpp
Orthographic 2D:
x: [0, windowWidth]  ? [-1, +1]
y: [0, windowHeight] ? [+1, -1]  (invertovano!)
```

#### B) **Textures** (2D Teksture)

**Funkcija:** `drawTexture()`

**Geometrija:**
- **Type**: 2D Textured Quad
- **Vertices**: 6
- **Koordinate**: Screen space + UV

**Vertex Format:**
```cpp
[Position(2), TexCoords(2)] = 4 floats
```

**Korištenje:**
- Heart icons (lives)
- Student info panel (indeks.png)
- Weapon icon (ak.png / usp.png)

**Shader:** `texture.vert` + `texture.frag`

**Alpha Blending:**
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

#### C) **Text** (FreeType Font Rendering)

**Renderer:** `TextRenderer` class

**Tehnologija:** FreeType library

**Korištenje:**
- Timer (MM:SS:CC format)
- Score (Hits: X/Y)
- Speed (Avg hit time)
- Weapon mode (USP / AK-47)
- Game Over stats

**Shader:** `freetype.vert` + `freetype.frag`

---

## ?? Crosshair (Nišan)

### Opis
Dinami?ki crosshair koji se širi pri pucanju (recoil effect).

### Geometrija

**Type:** 4 pravougaonika (linije)

**Struktura:**
```
     [Top line]
         |
[Left] --+-- [Right]
         |
    [Bottom line]
```

**Dimenzije:**
```cpp
Base size: 18 pixels
Thickness: 4 pixels
Gap: 6 pixels (od centra)

Recoil expansion:
Size *= (1.0 + recoilAmount)
Gap  *= (1.0 + recoilAmount)
```

**Recoil Recovery:**
```cpp
recoilRecoverySpeed = 8.0 units/second
recoilAmount -= recoverySpeed * deltaTime
```

**Color:**
```cpp
Green: (0.0, 0.8 + recoil*0.2, 0.0)
Alpha: 0.95
```

---

## ?? Performance Stats

### Vertex Counts (per frame)

```
Target (1x):        134 vertices
Room:                24 vertices
Light:               24 vertices
AK-47:           ~5000 vertices (loaded from OBJ)
USP-S:           ~3000 vertices (loaded from OBJ)
UI Rectangles:     ~100 vertices (variable)
Text:              ~200 vertices (variable)

Total: ~8500-9000 vertices per frame (približno)
```

### Draw Calls (per frame)

```
Room:              6 calls (walls + floor + ceiling)
Light:             1 call
Targets:         1-5 calls (depends on active targets)
Wall weapons:      2 calls (AK + USP)
UI Rectangles:  10-15 calls
UI Textures:     5-8 calls
Text:            5-10 calls

Total: ~30-50 draw calls per frame
```

### Shader Programs

```
1. rectShaderProgram      - UI boxes
2. textureShaderProgram   - UI textures
3. freetypeShaderProgram  - Text rendering
4. cylinderShaderProgram  - Targets
5. roomShaderProgram      - Room walls/floor/ceiling
6. lightShaderProgram     - Light source
7. weaponShaderProgram    - Wall weapons (reuses sphere3d)

Total: 6 unique shader programs
```

---

## ?? Rendering Pipeline Order

```
1. Enable Depth Test + Face Culling
2. Clear color + depth buffers

3. Draw 3D Scene:
   a. drawRoom()         ? Room geometry
   b. drawLight()        ? Light source
   c. drawWallWeapons()  ? AK-47 + USP-S
   d. drawCylinder3D()   ? Active targets (loop)

4. Disable Depth Test + Face Culling
5. Enable Blending

6. Draw 2D Overlay:
   a. HUD background boxes
   b. Hearts (lives)
   c. Text (timer, stats, mode)
   d. Student info panel
   e. Weapon icon
   f. Crosshair (4 lines)

7. If Game Over:
   a. Dark overlay
   b. Game Over box
   c. Stats text
   d. Buttons (Restart/Exit)
```

---

## ?? Interaction Systems

### Raycasting

**Used for:**
- Target hit detection ? `raySphereIntersection()`
- Weapon pickup ? `rayAABBIntersection()`

**Ray Generation:**
```cpp
rayOrigin = camera->getPosition()
rayDir = camera->getFront()
```

**Sphere Intersection:**
```cpp
oc = rayOrigin - sphereCenter
a = dot(rayDir, rayDir)
b = 2.0 * dot(oc, rayDir)
c = dot(oc, oc) - radius²
discriminant = b² - 4ac
hit = discriminant >= 0
```

**AABB Intersection:**
```cpp
invDir = 1.0 / rayDir
t0 = (boxMin - rayOrigin) * invDir
t1 = (boxMax - rayOrigin) * invDir
tmin = min(t0, t1)
tmax = max(t0, t1)
tNear = max(tmin.x, tmin.y, tmin.z)
tFar = min(tmax.x, tmax.y, tmax.z)
hit = tNear <= tFar && tFar >= 0
```

---

## ?? Dependencies

### External Libraries

```
GLEW      - OpenGL Extension Wrangler
GLFW      - Window + Input handling
GLM       - Math library (vectors, matrices)
FreeType  - Font rendering
stb_image - Image loading (PNG, JPG)
```

### Custom Classes

```
Camera       - FPS camera implementation
TextRenderer - FreeType text wrapper
OBJLoader    - OBJ file parser
Util         - Shader compilation + texture loading
```

---

## ?? Texture Files

```
Resources/indeks.png                  - Student info card
Resources/terrorist.png               - Target texture 1
Resources/counter.png                 - Target texture 2
Resources/heart.png                   - Full life icon
Resources/empty-heart.png             - Empty life icon
Resources/ak.png                      - AK-47 icon (2D)
Resources/usp.png                     - USP-S icon (2D)
Resources/smooth-white-brick-wall.jpg - Wall texture
Resources/floor.jpg                   - Floor texture
Resources/ceiling.png                 - Ceiling texture
obj/weapon_rif_ak47.png              - AK-47 model texture
obj2/weapon_pist_usp_silencer.png    - USP-S model texture
```

---

## ?? Debugging Tools

### Toggle Keys

```cpp
D - Toggle Depth Test
    ON:  Pravilna dubina (default)
    OFF: Rendering bugovi (targets overlap)

F - Toggle Face Culling
    ON:  Samo front faces (default)
    OFF: Obe strane vidljive (slower)
```

**Console Output:**
```
[DEPTH TEST] ? ENABLED - Pravilna dubina objekata
[FACE CULLING] ? ENABLED - Samo prednje strane su vidljive
```

---

## ?? Koordinatni Sistem

### OpenGL Conventions

```
+X ? Right
+Y ? Up
+Z ? Out of screen (towards camera)

Camera looks down -Z axis by default
```

### Room Coordinate Space

```
Center: (0, 0, 0)

Bounds:
X: [-10, +10] (width 20)
Y: [-5,  +5]  (height 10)
Z: [-10, +10] (depth 20)

Walls:
Front:  z = -10 (camera looks at this)
Back:   z = +10
Left:   x = -10
Right:  x = +10
Floor:  y = -5
Ceiling: y = +5
```

---

## ?? Optimization Notes

### Current Optimizations

1. **Static geometry caching:**
   - Room, Light, Cylinder VAO/VBO created once

2. **Instanced texture binding:**
   - Texture units reused per shader

3. **Face culling:**
   - Backfaces culled (except weapons)

4. **Early exit:**
   - Inactive targets removed from vector

### Potential Improvements

1. **Uniform location caching:**
   - Currently calls `glGetUniformLocation()` every frame
   - Should cache in constructor

2. **Batch rendering:**
   - All targets could be instanced
   - UI rectangles could be batched

3. **Texture atlas:**
   - Combine small textures into one atlas

4. **Frustum culling:**
   - Don't render off-screen targets

---

## ?? Autor & Projekat

**Student:** Luka Kešelj  
**Projekat:** 3D Aim Trainer  
**Kurs:** Ra?unarska Grafika 3D  
**Framework:** OpenGL 3.3 Core  
**Jezik:** C++14  

**Repository:** https://github.com/LukaKeselj/RacunarksaGrafika3D

---

## ?? Reference

### Lighting Model (Phong)

```glsl
ambient = 0.5 * lightColor
diffuse = max(dot(normal, lightDir), 0.0) * lightColor
specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * 0.5 * lightColor

result = (ambient + diffuse + specular) * objectColor
```

### Billboard Rotation

```cpp
direction = normalize(cameraPos - objectPos)
up = (0, 1, 0)
right = normalize(cross(up, direction))
newUp = cross(direction, right)

rotation = [right, newUp, direction]
```

---

## ? Checklist - 3D Objects

- [x] Targets (Cylinders) - Proceduralno generisani
- [x] Room (Box) - 6 faces sa teksturama
- [x] Light (Emissive box) - Glow effect
- [x] Wall Weapons (OBJ models) - AK-47 + USP-S
- [x] UI Overlays (2D quads) - HUD + Game Over
- [x] Crosshair (Dynamic lines) - Recoil effect
- [x] Raycasting (Hit detection) - Sphere + AABB
- [x] Lighting (Phong model) - Point light sa attenuation

---

## ?? Kraj Dokumentacije

Svaki 3D objekat je detaljno dokumentovan sa:
- ? Geometrijskom strukturom
- ? Vertex formatima
- ? Shader uniforms
- ? Rendering pipeline
- ? Interaction systems
- ? Performance metrics

**Za dodatne informacije, pogledaj izvorni kod u:**
- `Source/AimTrainer.cpp`
- `Header/AimTrainer.h`
- `Shaders/` folder
