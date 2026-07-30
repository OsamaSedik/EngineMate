*EngineMate** is a feature-rich Unreal Engine editor plugin designed to streamline level design workflows, automate asset housekeeping, run instant performance diagnostic commands, and enforce project validation rules—all within intuitive editor utility widgets and context menus.
---
## 🌟 Key Features
### 📐 Level Assist Suite (`WBP_LevelAssist`)
- **Advanced Selection Tools**:
  - Select actors by fuzzy/exact name matching or actor class.
  - One-click selection of all level lights.
  - Filter and select actors based on texture resolution thresholds (256x256 to 8192x8192).
- **Batch Level Optimization**:
  - Batch disable cast shadows for lights or static actors.
  - Disable Mesh Distance Fields on selected static meshes.
  - Batch disable collisions and Tick functions on selected actors.
  - Instantly identify and remove null/empty actors.
- **Outliner & Lighting Management**:
  - Automatically organize the World Outliner folder structure by Actor type.
  - Batch set lighting channels (`Channel 1 / 2 / 3`) for selected actors and light sources.
- **Parametric Duplication & Alignment**:
  - Linear duplicate actors with custom count, offset spacing, and axis alignment (X, Y, Z).
- **Transformation Randomizer**:
  - Apply controlled random rotations (Yaw, Pitch, Roll) and position offsets for natural prop/foliage scattering.
---
### ⚡ Performance Monitor (`WBP_Performance_Monitor`)
- Toolbar button integration for one-click access in the Level Viewport toolbar.
- Quick button-based trigger for essential Unreal Engine stat debugging commands (FPS, Unit, RHI, GPU, Memory, Scene Rendering, InitViews, etc.) without typing console commands.
---
### 🧹 Content Browser Automation
- **Delete Unused Assets**: Recursively checks package dependencies, automatically resolves object redirectors, and safely deletes orphaned assets under selected folders.
- **Delete Empty Folders**: Recursively detects and removes empty subdirectories (with built-in safeguards for system folders like `Collections`, `Developers`, `__ExternalActors__`).
---
### 🔒 Viewport Actor Selection Locking
- Prevents accidental selection or movement of background level elements.
- Context menu options in the Level Viewport: **Lock Actor Selection** and **Unlock Actor Selection**.
---
### 📋 Project Settings & Asset Validation
Configurable directly via `Project Settings -> Developer Settings -> Engine Mate Settings`:
- **Naming Convention Enforcement**: Pre-configured prefix rules for 35+ asset types (`BP_`, `SM_`, `SK_`, `T_`, `M_`, `MI_`, `WBP_`, `NS_`, `IA_`, `IMC_`, etc.).
- **Texture Validation**: Warns when textures exceed configurable maximum resolution limits (2K, 4K, 8K).
- **Blueprint Optimization Checks**: Detects enabled Tick functions and empty mesh components.
- **Mesh Integrity Rules**: Validates minimum LOD counts and collision settings for static meshes.
---
## 🚀 Installation & Setup
1. **Clone or Download** this repository into your Unreal Engine project's `Plugins` directory:
   ```bash
   cd YourProject/Plugins/
   git clone https://github.com/YourUsername/EngineMate.git
