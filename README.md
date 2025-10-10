#  OpenGL Render Engine - Aim Trainer

An **aim trainer** built in **C++** and **OpenGL**, featuring a full **Physically Based Rendering (PBR)** pipeline with **Image-Based Lighting (IBL)**.  
This project goes beyond basic rendering to deliver realistic materials, lighting, and real time interactivity.

---

## 🖼️ Screenshots

<div style="display: flex; align-items: flex-start; gap: 10px;">
  <img src="AimEngine/Media/Screenshot1.png" width="500" />
  <img src="https://raw.githubusercontent.com/finn7199/aim-engine/refs/heads/cloth-simulation/AimEngine/Media/clothsim_gif1.gif" width="500" />
</div>

---

## ✨ Core Features

- **Real-time 3D Rendering** using modern OpenGL.  
- **Physically-Based Rendering (PBR)** for realistic material definition.  
- **Image-Based Lighting (IBL)** using an HDR skybox for realistic ambient light and reflections.  
- **Dynamic Lighting** including:
  - Directional sunlight  
  - Toggleable spotlight (flashlight)  
- **First-Person Camera Controller** with WASD movement and mouse look.  
- **Raycasting** from the camera for accurate hit detection on targets.  
- **Procedural Sphere Generation** for creating target geometry.
- Experimental cloth simulation (see the `cloth-physics` branch for details).
 
---

## 🧵 Extra Features: OGC Cloth Physics (Cloth-Physics Branch)

This project implements a **mass-spring system for cloth simulation**, enhanced with a **self-collision model** inspired by the SIGGRAPH 2025 paper *"Offset Geometric Contact"* (Chen et al., 2025). It leverages a **spatial hash grid** as a high-performance acceleration structure to make advanced collision checks feasible in real-time.

### Key Concepts Implemented

#### 1. Trust-Region Based Penetration Prevention
Instead of reacting to collisions after they happen, the simulation is **proactive**:

- **Conservative Bounds:** At the start of each simulation step, a "safety bubble" is computed for each particle.  
- **Displacement Truncation:** After the physics step, any particle that moves outside its safety bubble has its displacement truncated, ensuring a penetration-free state.

#### 2. Simplified OGC Contact Model with Acceleration Structure
- **Spatial Hash Grid:** Uses a spatial hash to quickly get potentially colliding triangles, avoiding costly brute-force checks.  
- **Fast Distance Calculation:** Conservative bounds are computed using a fast point-to-centroid distance approximation to find nearest surfaces efficiently.

#### 3. Highly Parallel, Local Approach
- **Local Bounds:** Each particle's safety bubble is calculated independently, maintaining the local nature of the algorithm.  
- **Parallelism:** CPU parallelization (`#pragma omp parallel for`) is used to speed up bound calculations, reflecting the algorithm’s massively parallel potential.

> ⚠️ **Note:** This project is currently maintained in the [`cloth-physics`](https://github.com/finn7199/aim-engine/tree/cloth-simulation) branch and is **not merged into `main`**. You can explore it by checking out that branch.

---

## 🎮 Controls

| Key / Action | Description |
|---------------|-------------|
| **W / A / S / D** | Move the camera forward, left, backward, right |
| **Mouse** | Look around |
| **Left Mouse Click** | Shoot (fire a raycast) |
| **E** | Toggle the flashlight (spotlight) |
| **ESC** | Close the application |

---

## 🚀 Future Work

- Implement a scoring system and timed game modes  
- Load textured PBR materials (albedo, normal, roughness maps)  
- Add dynamic shadows from the directional light  
- Expand the scene with more complex geometry  
- **Model Loading** for importing complex 3D assets  
- **Sound Effects** for shooting, hits, and ambient environment  

---

## 🙏 Acknowledgments

- **HDR Skybox** by [Poly Haven](https://polyhaven.com)

---
