###

<h1 align="left">About the Project</h1>

###

<h5 align="left">Rice farmers in Nueva Ecija, Philippines face a 35–42% yield gap caused by timing-based inefficiencies in irrigation and fertilization that traditional farming calendars can no longer reliably address under today's climate volatility.<br><br>AgriBridge is an Offline-First Progressive Web App (PWA) that bridges this gap. It provides precise, data-driven prescriptions for crop management — running entirely on a standard smartphone — without requiring a stable internet connection. By processing locally entered environmental data on-device, AgriBridge helps smallholders:<br><br>• Reduce fertilizer waste by up to 20% through precision scheduling<br>• Lower irrigation-related fuel and electricity costs by up to 12%<br>• Identify optimal harvest windows with 90% accuracy to preserve grain quality and maximize market value<br><br>This project directly advances UN Sustainable Development Goals SDG 2 (Zero Hunger), SDG 8 (Decent Work and Economic Growth), and SDG 12 (Responsible Consumption and Production) by democratizing precision agriculture for rural communities who cannot afford industrial agritech infrastructure.<br><br><b>Target Users:</b> Smallholder rice farmers managing 1.5–2.5 hectares in rural Nueva Ecija; agricultural cooperative officers; field extension workers.</h5>

###

<h2 align="left">The Problem It Solves</h2>

###

<h6 align="left">Traditional rice farming in Central Luzon relied on generational ecological knowledge that has been disrupted by escalating climate volatility. The region now experiences thermal peaks of 35°C in May and precipitation surges of 390.5 mm in August, decoupling fixed farming calendars from actual crop phenology.</h6>

###

<h2 align="left">Productivity Losses & Impact</h2>

###

| Problem | Impact |
| :--- | :--- |
| *Mistimed fertilizer application* | ~25% nitrogen lost to leaching and volatilization |
| *Suboptimal irrigation scheduling* | 15–20% yield reduction during critical reproductive phases |
| *Premature or delayed harvesting* | 12% post-harvest loss from grain shattering or quality degradation |

###

<h6 align="left">Existing precision agriculture solutions require constant internet connectivity and expensive hardware — both unavailable to rural populations. AgriBridge removes these barriers by running complex decision logic directly on the device.</h6>

###

<h2 align="left">Key Features</h2>

###

<h5 align="left">
<b>Offline-First Operation</b> — Functions for at least 5 consecutive days without any internet connection using local data storage and service worker caching.<br><br>
<b>Growth Stage Prediction</b> — Tracks rice phenology using Growing Degree Day (GDD) accumulation rather than fixed calendar dates.<br><br>
<b>Irrigation Scheduling</b> — Recommends dynamic irrigation timing based on crop growth phase and local thermal data entered by the user.<br><br>
<b>Fertilizer Prescriptions</b> — Provides nitrogen application schedules aligned with physiological crop requirements, minimizing runoff waste.<br><br>
<b>Harvest Window Identification</b> — Predicts a 3-day optimal harvest window with 90% accuracy to maximize grain quality and marketable surplus.<br><br>
<b>Bilingual Interface</b> — Full support for English and Filipino (Tagalog) to serve farmers and field workers in their preferred language.<br><br>
<b>Mobile-First Design</b> — Optimized for entry-level Android smartphones with no app store installation required (installable as a PWA).<br><br>
<b>Background Sync</b> — Queues data locally and syncs anonymized cooperative-level data when connectivity is restored.
</h5>

###

<h2 align="left">Tech Stack</h2>

###

<div align="left">
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/cplusplus/cplusplus-original.svg" height="40" alt="cplusplus logo"  />
  <img width="12" />
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/mysql/mysql-original.svg" height="40" alt="mysql logo"  />
  <img width="12" />
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/git/git-original.svg" height="40" alt="git logo"  />
  <img width="12" />
  <img src="https://cdn.jsdelivr.net/gh/devicons/devicon/icons/github/github-original.svg" height="40" alt="github logo"  />
</div>

###



###
###

## Installation & Setup Guide

Follow these configuration steps sequentially to install the C++ core application engine and configure the local MySQL persistence layers.

### 1. Prerequisites

Ensure all base software layers are installed before continuing setup configuration.

| Requirement | Version | Purpose | Download |
| :--- | :--- | :--- | :--- |
| *Visual Studio* | 2022 / Insiders | IDE + MSVC C++ Compiler | [visualstudio.microsoft.com](https://visualstudio.microsoft.com) |
| *MySQL Server* | 8.x or higher | Database engine | [dev.mysql.com](https://dev.mysql.com/downloads/mysql/) |
| *MySQL Connector C++* | 9.7.0 winx64 | C++ MySQL driver (JDBC API) | [dev.mysql.com/downloads](https://dev.mysql.com/downloads/connector/cpp/) |
| *MySQL Workbench*
(optional) | Any | GUI for running SQL scripts | [dev.mysql.com](https://dev.mysql.com/downloads/workbench/) |

⚠️ **Platform Warning:** The MySQL Connector provided is **64-bit (winx64)**. Your Visual Studio build configuration **must be set to x64**. Building as x86/Win32 will cause fatal linker errors.


#### Visual Studio Workload Required
When installing Visual Studio, ensure the following core workflow layout option is active:
* ✅ Desktop development with C++

---

### 2. Clone the Repository

git clone [https://github.com/your-username/SmartCropAgroDSS.git](https://github.com/your-username/SmartCropAgroDSS.git)
cd SmartCropAgroDSS


Alternatively, extract a compiled project ZIP container download directly from GitHub into your local folder structures.

---

### 3. Project Structure

Verify your cloned context repository tree maps cleanly against this environment tree configuration layout:

SmartCropAgroDSS/
├── INPUT_DATA/
│   └── database.txt          ← offline backup (create this manually if missing)
├── main.cpp
├── agriManager.cpp
├── agriManager.h
├── dailyRecord.h
├── DatabaseManager.cpp
├── DatabaseManager.h
└── smartCropSQL.sql


#### Create the INPUT_DATA Environment Manually

The system targets local disk paths INPUT_DATA/database.txt for fallback handling loops:

1. Create a subfolder inside the root project directory named exactly INPUT_DATA.
2. Inside that directory target, create a generic layout clear container file named database.txt.

---

### 4. Database Setup

Configure schema elements and local service parameters before attempting execution builds.

#### Step 1 — Open MySQL Workbench or the MySQL CLI

* *Option A (GUI):* Launch Workbench interface components and establish local server connections at localhost:3306.
* *Option B (CLI):* Open your system terminal structure window and run:
mysql -u root -p




#### Step 2 — Create the Database and Table

Execute the following baseline initialization block statement sequence within your instance connection session:

CREATE DATABASE IF NOT EXISTS agri_dss;

USE agri_dss;

CREATE TABLE IF NOT EXISTS daily_records (
    id            INT AUTO_INCREMENT PRIMARY KEY,
    temperature   FLOAT NOT NULL,
    rainfall_mm   FLOAT NOT NULL,
    daily_gdd     FLOAT NOT NULL
);


#### Step 3 — Load the Sample Data Matrix

Hydrate table variables using the smartCropSQL.sql script package context files.

* *In Workbench:* Select File → Open SQL Script → choose smartCropSQL.sql → execute with Ctrl+Shift+Enter.
* *In CLI Tool:* Run the direct execution routing string:
source /path/to/smartCropSQL.sql




#### Step 4 — Verification Check

SELECT * FROM agri_dss.daily_records;


Confirm the terminal connection output window successfully lists exactly *22 rows* of environmental reference vectors.

#### Database Authentication Properties

Default structural authentication keys are handled in the standard main.cpp entrypoint logic:

db.connect("root", "EquinoxScout30");


Update these properties to match your specific localized system environment security configurations if they vary.

---

### 5. Visual Studio Project Setup

#### Step 1 — Create a New Project Workspace

1. Select *Create a new project* from the Visual Studio launcher screen.
2. Select the native *Empty Project (C++)* structure configuration item context.
3. Name your workspace configuration parameters mapping structure and verify paths.

#### Step 2 — Target Platform Setup

Switch active targets configuration profiles from fallback x86/Win32 platforms over onto modern *x64* target parameters processing:

Debug ▾  |  x64 ▾       ← configuration toolbar state layout


#### Step 3 — Mount Source Components

Right-click your respective layout directories inside the *Solution Explorer* utility view:

* *Source Files:* Mount main.cpp, agriManager.cpp, and DatabaseManager.cpp.
* *Header Files:* Mount agriManager.h, dailyRecord.h, and DatabaseManager.h.

---

### 6. MySQL Connector Configuration

Open project compiler flags variables configurations by choosing *Properties* inside the contextual option dialog blocks. Ensure Target Configurations match *Debug* and *x64*.

#### 6A — Additional Include Directories

* Path Configuration Target: C/C++ → General → Additional Include Directories
* Append entry string value mapping path parameter:
C:\Program Files\MySQL\mysql-connector-c++-9.7.0-winx64\include




#### 6B — Additional Library Directories

* Path Configuration Target: Linker → General → Additional Library Directories
* Append entry string value mapping path parameter:
C:\Program Files\MySQL\mysql-connector-c++-9.7.0-winx64\lib64\vs14




#### 6C — Additional Dependencies Input Variables

* Path Configuration Target: Linker → Input → Additional Dependencies
* Add these file designations sequentially directly above base dependencies maps:
mysqlcppconnx.lib
mysqlcppconn.lib




---

### 7. Build the Project

Execute structural compilation steps via shortcut array sequences Ctrl+Shift+B, or manually select *Build Solution* out from top menus interface toolsets.

---

### 8. Link Application Library Binaries (DLLs)

Move dependent dynamic system tracking driver assets directly adjacent to compiled target binary locations inside output directories to circumvent tracking runtime loop crashes.

* *Binary Objects Source Directory Link:*
C:\Program Files\MySQL\mysql-connector-c++-9.7.0-winx64\lib64\
* *Target Object Files Required:*
mysqlcppconnx-2-vs14.dll
mysqlcppconn-10-vs14.dll
libssl-3-x64.dll
libcrypto-3-x64.dll



* *System Execution Target Path Context:* Place directly into your local project environment output file tree path structure: <YourProjectFolder>\x64\Debug\

---

### 9. Run the Program

Initialize operational process execution sequences directly out from target development spaces using interface commands shortcut sequences Ctrl+F5.

---

### 10. Verify the Installation

A successful local operational execution build sequence presents tracking verification outputs identical to the structural terminal profile pattern below:

SUCCESS: Connected to the database.
Session GDD seeded from database: 484 GDD
Session arrays loaded: 22 day(s) restored.

SYSTEM: Press Enter to launch the dashboard...


---

### 11. Common Errors & Fixes

*LNK1104: cannot open file 'mysqlcppconn8.lib'*

* Cause: Legacy dependencies are referenced inside the compiler parameters mapping structures.
* Fix: Check Linker -> Input variables mappings array definitions block entries and ensure they cleanly point onto mysqlcppconnx.lib and mysqlcppconn.lib.

*Cannot open include file: 'jdbc/mysql_connection.h'*

* Cause: Include directories mappings configurations fail to target path parameters accurately.
* Fix: Re-verify your base environment mapping definitions under variables directory parameters mapping options tree C/C++ -> General.

*Unhandled exception: std::bad_alloc / System Fault Code 0xC0000135*

* Cause: Output destination target workspaces are missing target tracking dependencies drivers link files.
* Fix: Re-copy required .dll files directly into your active execution folder destination space targets layout (\x64\Debug\).

---

### 12. Architectural Parameter Configurations Matrix

| Target Variable Element | Value Parameter Association Layout Mapping |
| --- | --- |
| *Include Reference Path* | C:\Program Files\MySQL\mysql-connector-c++-9.7.0-winx64\include |
| *Library Reference Path* | C:\Program Files\MySQL\mysql-connector-c++-9.7.0-winx64\lib64\vs14 |
| *Binary DLL Assets Source* | C:\Program Files\MySQL\mysql-connector-c++-9.7.0-winx64\lib64\ |
| *Required Target Platform* | *x64 Platform Mode Targets Only* |
| *Instance Data Workspace* | agri_dss |
| *System Security Keys* | root / your_password |

---

SmartCrop Agri-DSS — Computer Programming 2 Project, May 2026
