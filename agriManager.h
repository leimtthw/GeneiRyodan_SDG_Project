//FILE:     agriManager.cpp
//AUTHORS:  DELCASTILLO, MANALO, VIRAY, PATDO
//SUBJECT:  COMPUTER PROGRAMMING 2 
//DATE:     May 2026
//Purpose:  Declares AdvisoryModule abstract base class, its four derived modules,
//          as well as the core UI and navigation functions

#ifndef AGRI_MANAGER_H
#define AGRI_MANAGER_H

#include <iostream>
#include "dailyRecord.h"   // provides FarmSession struct and MAX_DAYS

// Abstract Base Class: AdvisoryModule
// defines the common interface for all advisory modules.
// each module receives the full session state and produces
// a console recommendation. the pure virtual run() method
// forces every derived class to implement its own logic,
// enabling polymorphic dispatch from the main menu loop.
class AdvisoryModule {
public:
    // pure virtual: every derived module must implement this.
    // accepts the session as const — advisory modules only
    // read data, they never modify the session state.
    virtual void run(const FarmSession& session) = 0;

    // virtual destructor: required whenever a base class is
    // used with pointer-based polymorphism to ensure the
    // correct derived destructor runs on delete.
    virtual ~AdvisoryModule() {}

protected:
    // shared guard reused by all four derived modules.
    // returns true if session data is available, false otherwise.
    // eliminates the duplicate daysRecorded == 0 check across subclasses.
    bool checkDataAvailable(int daysRecorded) const {
        if (daysRecorded == 0) {
            std::cout << "No data available. Please input farm data first." << std::endl;
            return false;
        }
        return true;
    }
};

// Derived Class 1: CropStageModule
// interprets total accumulated GDD to identify the current
// physiological growth stage of the rice crop.
class CropStageModule : public AdvisoryModule {
public:
    void run(const FarmSession& session) override;
};

// Derived Class 2: FertilizerModule
// recommends fertilizer application based on compound conditions:
// current GDD stage AND runoff risk from recent rainfall.
class FertilizerModule : public AdvisoryModule {
public:
    void run(const FarmSession& session) override;
};

// Derived Class 3: IrrigationModule
// counts consecutive dry days from the most recent entry backwards.
// recommends immediate irrigation if a prolonged dry spell or
// high evaporation temperature is detected.
class IrrigationModule : public AdvisoryModule {
public:
    void run(const FarmSession& session) override;
};

// Derived Class 4: HarvestModule
// predicts harvest readiness based on total GDD thresholds.
// issues a critical alert if the crop is mature but rain is
// detected, as this poses a grain shattering and rotting risk.
class HarvestModule : public AdvisoryModule {
public:
    void run(const FarmSession& session) override;
};

// Free Functions (non-advisory — not part of the hierarchy)

// UI / navigation
void displayMenu();
void clearInputBuffer();

// core farm data entry
bool inputFarmData(FarmSession& session);
void loadFromTextFile(FarmSession& session);

// reporting
void saveToTextFile(const FarmSession& session); // function to sync memory to database.txt
void displaySummaryReport(const FarmSession& session);

#endif // AGRI_MANAGER_H