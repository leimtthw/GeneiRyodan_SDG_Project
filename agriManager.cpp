//FILE:     agriManager.cpp
//AUTHORS:  DELCASTILLO, MANALO, VIRAY, PATDO
//SUBJECT:  COMPUTER PROGRAMMING 2 
//DATE:     May 2026
//Purpose:  Implements the offline .txt file initialization, menu UI
//          and the specific agricultural computation logic for each module 

#include <fstream> //allows the program to have an access to hard drive.
#include <sstream>
#include <ctime>
#include <iomanip>
#include <iostream>
#include "agriManager.h"   // own header (AdvisoryModule hierarchy + free functions)

// UI / navigation

// prints the main navigation menu to the console
void displayMenu() {

    system("cls"); 

    //initiate time block
    time_t now = time(0);
    struct tm timeInfo;

    //localtime_s is used better in visual studio in order to function properly
    //without warnings
    localtime_s(&timeInfo, &now);

    char timeBuffer[80];
    //%I = 12-hour format, %M = minutes, %p adds AM or PM
    strftime(timeBuffer, sizeof(timeBuffer), "%I:%M %p", &timeInfo);

    std::cout << "=== Nueva Ecija Offline Agri-DSS ===" << std::endl;
    std::cout << "Time: " << timeBuffer << std::endl;
    std::cout << "1. Input Farm Data (Rain/Temp)" << std::endl;
    std::cout << "2. Analyze Crop Stage" << std::endl;
    std::cout << "3. Fertilizer Recommendation" << std::endl;
    std::cout << "4. Irrigation Schedule" << std::endl;
    std::cout << "5. Harvest Prediction" << std::endl;
    std::cout << "6. Search Specific Record" << std::endl;
    std::cout << "7. Edit Past Record" << std::endl;
    std::cout << "8. Reset Season Data" << std::endl;
    std::cout << "9. Delete Specific Record" << std::endl;
    std::cout << "10. Exit" << std::endl;
    std::cout << "Select an option: ";
}
void loadFromTextFile(FarmSession& session) {
    std::ifstream file("INPUT_DATA/database.txt");
    if (!file.is_open()) {
        std::cout << "SYSTEM: No database.txt found. Starting with empty session." << std::endl;
        return;
    }

    std::string line;
    int count = 0;
    while (std::getline(file, line) && count < MAX_DAYS) {
        std::stringstream ss(line);
        std::string id_str, t, r, g;

        if (std::getline(ss, id_str, ',') && std::getline(ss, t, ',') &&
            std::getline(ss, r, ',') && std::getline(ss, g, ',')) {
            session.setRecordId(count, std::stoi(id_str));
            session.setTemp(count, std::stof(t));
            session.setRainfall(count, std::stof(r));
            // Accumulate GDD based on parsed daily increments
            session.setTotalGDD(session.getTotalGDD() + std::stof(g));
            count++;
        }
    }
    session.setDaysRecorded(count);
    file.close();

    if (count >= 20) {
        std::cout << "SUCCESS: " << count << " records parsed from database.txt." << std::endl;
    }
    else {
        std::cout << "WARNING: Only " << count << " records found. Rubric requires 20." << std::endl;
    }
}
// recovers std::cin from a failed read and discards leftover characters
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(1000, '\n');
}

// Core Farm Data Entry (free function — not an advisory module)

// logs today's temperature and rainfall into the rolling session arrays.
// when the array is full (MAX_DAYS), shifts existing data left by one
// position to maintain a MAX_DAYS-day sliding window.
// accumulates GDD using a base temperature of 10 degrees C.
bool inputFarmData(FarmSession& session) {

    // temperature input with range validation
    // without initialization, if cin.fail() triggers on the very first read,
    // tempInput holds garbage and the range check reads uninitialized memory.
    float tempInput = 0.0;
    do {
        std::cout << "Enter today's average temperature (15.0C to 55.0C): ";
        std::cin >> tempInput;

        if (std::cin.fail()) {
            if (std::cin.eof()) {
                std::cout << "\nInput stream closed. Aborting data entry." << std::endl;
                return false; // safely back out to the main menu shutdown sequence
            }
            clearInputBuffer();
            std::cout << "CRITICAL ERROR: Invalid input type. Please enter a numeric value." << std::endl;
            tempInput = 0.0;  // force loop to retry
            continue;
        }
        if (tempInput < 15.0 || tempInput > 55.0) {
            std::cout << "INVALID TEMPERATURE: Out of valid biological range for Nueva Ecija rice variants." << std::endl;
        }
    } while (tempInput < 15.0 || tempInput > 55.0);

    // rainfall category input with validation
    // initialize rainChoice to -1 to prevent undefined behavior.
    // without initialization, if cin.fail() triggers on the very first read,
    // rainChoice holds garbage and the range check reads uninitialized memory.
    int rainChoice = -1;
    do {
        std::cout << "Select today's rainfall level:" << std::endl;
        std::cout << "0 = None (Dry Day)" << std::endl;
        std::cout << "1 = Light" << std::endl;
        std::cout << "2 = Moderate" << std::endl;
        std::cout << "3 = Heavy" << std::endl;
        std::cout << "Choice: ";
        std::cin >> rainChoice;

        if (std::cin.fail()) {
            if (std::cin.eof()) {
                std::cout << "\nInput stream closed. Aborting data entry." << std::endl;
                return false;
            }
            clearInputBuffer();
            rainChoice = -1;  // force loop to retry incase of invalid input
        }
        if (rainChoice < 0 || rainChoice > 3) {
            std::cout << "INVALID SELECTION: Please choose 0, 1, 2, or 3." << std::endl;
        }
    } while (rainChoice < 0 || rainChoice > 3);

    // map rainfall categories to mm equivalents for downstream calculations
    float rainMapped = 0.0;
    if (rainChoice == 0) rainMapped = 0.0;   // dry/sunny
    else if (rainChoice == 1) rainMapped = 5.0;   // light — low impact
    else if (rainChoice == 2) rainMapped = 15.0;  // moderate — medium impact
    else if (rainChoice == 3) rainMapped = 30.0;  // heavy — triggers alerts

    //rolling window shift when storage is full that only executes after
    //both inputs are successfully validated. prevents data loss.
    if (session.getDaysRecorded() >= MAX_DAYS) {
        std::cout << "Storage limit reached! Shifting local array data to maintain a rolling "
            << MAX_DAYS << "-day window..." << std::endl;
        for (int i = 0; i < MAX_DAYS - 1; i++) {
            session.setRainfall(i, session.getRainfall(i + 1));
            session.setTemp(i, session.getTemp(i + 1));
        }
        session.setDaysRecorded(MAX_DAYS - 1);
    }

    int currentDay = session.getDaysRecorded();
    session.setTemp(currentDay, tempInput);
    session.setRainfall(currentDay, rainMapped);

    float baseTemp = 10.0;
    if (session.getTemp(currentDay) > baseTemp) {
        float currentGDD = session.getTotalGDD();
        float dailyGain = session.getTemp(currentDay) - baseTemp;
        session.setTotalGDD(currentGDD + dailyGain);
    }

    session.setDaysRecorded(currentDay + 1);
    std::cout << "Farm data successfully logged into local memory." << std::endl;

    clearInputBuffer();
    return true;
}

// Derived Class Implementations

// CropStageModule::run()
// interprets total accumulated GDD to identify the current
// physiological growth stage of the rice crop.
void CropStageModule::run(const FarmSession& session) {
    std::cout << std::endl << "--- Crop Stage Analysis ---" << std::endl;
    if (!checkDataAvailable(session.getDaysRecorded())) return;

    std::cout << "Accumulated GDD: " << session.getTotalGDD() << std::endl;

    if (session.getTotalGDD() >= 0 && session.getTotalGDD() < 400) {
        std::cout << "Stage: Vegetative. Focus on canopy growth." << std::endl;
    }
    else if (session.getTotalGDD() >= 400 && session.getTotalGDD() < 1000) {
        std::cout << "Stage: Reproductive. Critical water needs." << std::endl;
    }
    else if (session.getTotalGDD() >= 1000) {
        std::cout << "Stage: Ripening. Prepare for harvest." << std::endl;
    }
}

// FertilizerModule::run()
// recommends fertilizer application based on compound conditions:
// current GDD stage AND runoff risk from recent rainfall.
void FertilizerModule::run(const FarmSession& session) {
    std::cout << std::endl << "--- Fertilizer Recommendation ---" << std::endl;
    if (!checkDataAvailable(session.getDaysRecorded())) return;

    if (session.getTotalGDD() >= 1000) {
        std::cout << "ACTION HOLD: Crop is in the ripening stage. Do not apply fertilizer to ensure grain quality." << std::endl;
        return;
    }

    int lastIndex = session.getDaysRecorded() - 1;
    float recentRain = session.getRainfall(lastIndex);

    if (session.getTotalGDD() < 400 && recentRain > 20.0) {
        std::cout << "ACTION HOLD: Heavy rainfall mapped. Applying fertilizer now will result in runoff waste." << std::endl;
    }
    else if (session.getTotalGDD() < 400 && recentRain <= 20.0) {
        std::cout << "ACTION APPLY: Ideal conditions for Topdress Nitrogen (40kg/ha)." << std::endl;
    }
    else {
        std::cout << "ACTION HOLD: Crop is past the optimal vegetative stage for nitrogen." << std::endl;
    }
}

// IrrigationModule::run()
// counts consecutive dry days (rainfall < 2mm) from the most recent
// entry backwards. recommends immediate irrigation if a prolonged
// dry spell or high evaporation temperature is detected.
void IrrigationModule::run(const FarmSession& session) {
    std::cout << std::endl << "--- Irrigation Schedule ---" << std::endl;
    if (!checkDataAvailable(session.getDaysRecorded())) return;

    if (session.getTotalGDD() >= 1000) {
        std::cout << "STATUS OVERRIDE: Crop is in the ripening stage. Do not irrigate to allow field drying." << std::endl;
        return;
    }

    int dryDays = 0;
    for (int i = session.getDaysRecorded() - 1; i >= 0; i--) {
        if (session.getRainfall(i) < 2.0) dryDays++;
        else break;
    }

    if (dryDays >= 3 || session.getTemp(session.getDaysRecorded() - 1) > 35.0) {
        std::cout << "ALERT: Prolonged dry spell or high evaporation ("
            << session.getTemp(session.getDaysRecorded() - 1)
            << "C) detected. Apply 30mm irrigation immediately." << std::endl;
    }
    else if (session.getDaysRecorded() < 3) {
        std::cout << "STATUS UNKNOWN: Only " << session.getDaysRecorded() << " day(s) logged. Cannot safely confirm soil moisture yet." << std::endl;
    }
    else {
        std::cout << "STATUS NORMAL: Soil moisture sufficient. Do not irrigate to save water." << std::endl;
    }
}

// HarvestModule::run()
// predicts harvest readiness based on total GDD thresholds.
// issues a critical alert if the crop is mature but rain is detected,
// as this poses a grain shattering and rotting risk.
void HarvestModule::run(const FarmSession& session) {
    std::cout << std::endl << "--- Harvest Prediction ---" << std::endl;
    if (!checkDataAvailable(session.getDaysRecorded())) return;

    if (session.getTotalGDD() >= 1000 && session.getTotalGDD() < 1200) {
        std::cout << "Nearing maturity. Drain field 10 days before harvest." << std::endl;
    }
    else if (session.getTotalGDD() >= 1200) {
        if (session.getRainfall(session.getDaysRecorded() - 1) > 10.0) {
            std::cout << "CRITICAL: Crop mature, but rain detected. Harvest IMMEDIATELY to prevent grain shattering and rotting." << std::endl;
        }
        else {
            std::cout << "OPTIMAL: Weather clear. Proceed with standard harvesting protocols." << std::endl;
        }
    }
    else {
        std::cout << "Too early for harvest prediction. Continue monitoring. (Threshold: 1000 GDD)" << std::endl;
    }
}

// Reporting (free function — not an advisory module)

// prints a formatted tabular summary of all logged days,
// showing temperature and rainfall values alongside totals.
void displaySummaryReport(const FarmSession& session) {
    std::cout << std::endl << "=== FINAL FARM SUMMARY REPORT ===" << std::endl;

    if (session.getDaysRecorded() == 0) {
        std::cout << "No data logged for this session." << std::endl;
        return;
    }

    std::cout << "Total Days Currently in Memory: " << session.getDaysRecorded() << std::endl;
    std::cout << "Accumulated GDD:   " << session.getTotalGDD() << std::endl;
    std::cout << "---------------------------------------" << std::endl;
    std::cout << "Rolling Window\tTemp (C)\tRainfall (mm)" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    std::cout << std::left << std::setw(15) << "Record ID"
        << std::setw(15) << "Temp (C)"
        << std::setw(15) << "Rainfall (mm)" << std::endl;
    std::cout << "-------------------------------------------" << std::endl;

    for (int i = 0; i < session.getDaysRecorded(); i++) {
        // Output just the ID and the data, perfectly aligned
        std::cout << std::left << std::setw(15) << session.getRecordId(i)
            << std::setw(15) << session.getTemp(i)
            << std::setw(15) << session.getRainfall(i) << std::endl;
    }
    std::cout << "-------------------------------------------" << std::endl;
}
void saveToTextFile(const FarmSession& session) {
    // open in truncate mode to refresh the file with the newest memory state
    std::ofstream file("INPUT_DATA/database.txt", std::ios::trunc);

    if (!file.is_open()) {
        std::cout << "CRITICAL: Could not sync to database.txt backup." << std::endl;
        return;
    }

    for (int i = 0; i < session.getDaysRecorded(); i++) {
        // formats the line to match your 4-column requirement [ID, Temp, Rain, GDD]
        file << session.getRecordId(i) << ","
            << session.getTemp(i) << ","
            << session.getRainfall(i) << ","
            << (session.getTemp(i) - 10.0) << std::endl; // calculates daily GDD gain
    }

    file.close();
}