//FILE:     main.cpp
//AUTHORS:  DELCASTILLO, MANALO, VIRAY, PATDO
//SUBJECT:  COMPUTER PROGRAMMING 2 
//DATE:     May 2026
//Purpose:  System entry point; this program handles startup initialization
//          database connection, and the polymorphic requirement implemented
//          in the main menu loop

#include <iostream>
#include "agriManager.h"     // AdvisoryModule hierarchy + free function prototypes
#include "dailyRecord.h"     // FarmSession struct + MAX_DAYS
#include "DatabaseManager.h" // persistent storage bridge

// DatabaseManager owns and manages the connection privately.

int main() {

    // start the database bridge and attempt connection.
    // if connect() returns false, the system runs in Offline Mode:
    // all session logic remains fully functional using FarmSession.
    DatabaseManager db;

    if (!db.connect("root", "ComputerProgramming2")) {
        std::cout << "WARNING: Database unavailable. Starting in Offline Mode." << std::endl;
        std::cout << "All features remain active. Session data will not be persisted." << std::endl;
    }
    else {
        std::cout << "SUCCESS: Connected to the database." << std::endl;
    }

    // initialize all session arrays to zero before use.
    // this prevents reading uninitialized memory on the first
    // advisory call if the user skips Option 1.
    FarmSession session;

    loadFromTextFile(session); 

    // seed session GDD from the database at startup.
    // this synchronizes the in-memory state with any records
    // saved in previous sessions, ensuring all advisory modules
    // operate on the correct accumulated season total from the start.
    if (db.isConnected()) {
        session.setTotalGDD(db.getTotalGDD());
        std::cout << "Session GDD seeded from database: " << session.getTotalGDD() << " GDD" << std::endl;

        // Use temporary arrays to extract data from the database, 
        // then inject them into the private session object.
        int tempIds[MAX_DAYS];
        float tempTemps[MAX_DAYS];
        float tempRain[MAX_DAYS];
        int tempDays = 0;

        db.loadRecentRecords(tempIds, tempTemps, tempRain, tempDays);

        for (int i = 0; i < tempDays; i++) {
            session.setRecordId(i, tempIds[i]);
            session.setTemp(i, tempTemps[i]);
            session.setRainfall(i, tempRain[i]);
        }
        session.setDaysRecorded(tempDays);
        saveToTextFile(session);

        std::cout << "Session arrays loaded: " << session.getDaysRecorded() << " day(s) restored." << std::endl;
    }

    // Polymorphic Advisory Module Registry
    // instantiate all four advisory modules and store them behind
    // AdvisoryModule base pointers. menu choices 2-5 index directly
    // into this array, so adding a new module only requires adding
    // a new class and one pointer here — main.cpp never changes again.

    // index 0 = menu option 2 (Crop Stage)
    // index 1 = menu option 3 (Fertilizer)
    // index 2 = menu option 4 (Irrigation)
    // index 3 = menu option 5 (Harvest)
    AdvisoryModule* modules[4] = {
        new CropStageModule(),
        new FertilizerModule(),
        new IrrigationModule(),
        new HarvestModule()
    };

    std::cout << "\nSYSTEM: Press Enter to launch the dashboard...";
    std::cin.get();

    // set 'choice' to 0 to prevent undefined behavior.
    // in a do-while loop, the condition is checked AFTER the body executes,
    // but if cin.fail() fires on the very first read, 'choice' is reset to 0
    // and the while(choice != 7) check uses a well-defined value.
    // without initialization, the compiler may warn or behave unpredictably
    // if 'choice' is ever read before a successful cin >> assignment.
    int choice = 0;

    // main program loop: runs until user selects Exit (10)
    do {
        displayMenu();
        std::cin >> choice;

        // guard against non-numeric input
        if (std::cin.fail()) {
            if (std::cin.eof()) {
                std::cout << "\nInput stream forcefully closed. Emergency shutdown." << std::endl;
                break;
            }
            clearInputBuffer();
            std::cout << "CRITICAL ERROR: Invalid input type. Please enter a numeric value." << std::endl;
            choice = 0;  // reset to prevent accidental exit
            continue;
        }
        clearInputBuffer();

        // route selection to the appropriate module
        switch (choice) {

        case 1: {
            // get GDD before inputFarmData() runs so the daily
            // increment can be isolated as (totalGDD after - totalGDD before).
            // this ensures only the single-day heat unit gain is written
            // to daily_records, keeping getTotalGDD()'s SUM() accurate.
            float gddBefore = session.getTotalGDD();

            if (inputFarmData(session)) {
                if (db.isConnected() && session.getDaysRecorded() > 0) {
                    int lastIndex = session.getDaysRecorded() - 1;
                    float dailyGDD = session.getTotalGDD() - gddBefore;

                    if (db.insertDailyRecord(session.getTemp(lastIndex), session.getRainfall(lastIndex), dailyGDD)) {
                        std::cout << "Record saved to database." << std::endl;
                        int tempIds[MAX_DAYS];
                        float tempTemps[MAX_DAYS];
                        float tempRain[MAX_DAYS];
                        int tempDays = 0;
                        db.loadRecentRecords(tempIds, tempTemps, tempRain, tempDays);
                        for (int i = 0; i < tempDays; i++) {
                            session.setRecordId(i, tempIds[i]);
                            session.setTemp(i, tempTemps[i]);
                            session.setRainfall(i, tempRain[i]);
                        }
                        session.setDaysRecorded(tempDays);
                        saveToTextFile(session);
                    }
                }
            }
            break;
        }

        case 2:
        case 3:
        case 4:
        case 5:
            // polymorphic dispatch: menu choices 2-5 map directly to
            // modules[0]-modules[3] via the index offset (choice - 2).
            // the base class pointer calls the correct derived run()
            // at runtime through the virtual function table (vtable).
            modules[choice - 2]->run(session);
            break;

        case 6: {
            std::cout << "Enter the Record ID to search: ";
            int searchId = 0;
            std::cin >> searchId;

            if (std::cin.fail()) {
                clearInputBuffer();
                std::cout << "Invalid input. Please enter a number." << std::endl;
                break;
            }
            clearInputBuffer();

            if (db.isConnected()) {
                db.getRecordById(searchId);
            }
            else {
                std::cout << "Search is disabled in Offline Mode." << std::endl;
            }
            break;
        }

        case 7: {
            std::cout << "Enter the Record ID to update: ";
            int updateId = 0;
            std::cin >> updateId;

            if (std::cin.fail()) {
                clearInputBuffer();
                std::cout << "Invalid input." << std::endl;
                break;
            }

            std::cout << "Enter corrected temperature (15.0 to 55.0): ";
            float newTemp = 0.0f;
            std::cin >> newTemp;

            std::cout << "Enter corrected rainfall (in mm): ";
            float newRain = 0.0f;
            std::cin >> newRain;

            clearInputBuffer();

            // recalculate the daily GDD for the update using the 10.0 base temperature
            float baseTemp = 10.0f;
            float newDailyGDD = 0.0f;
            if (newTemp > baseTemp) {
                newDailyGDD = newTemp - baseTemp;
            }

            if (db.isConnected()) {
                if (db.updateDailyRecord(updateId, newTemp, newRain, newDailyGDD)) {
                    std::cout << "Record " << updateId << " successfully updated." << std::endl;

                    // Trigger a reload of the session arrays so the memory matches the new database state
                    session.setTotalGDD(db.getTotalGDD());
                    int tempIds[MAX_DAYS];
                    float tempTemps[MAX_DAYS];
                    float tempRain[MAX_DAYS];
                    int tempDays = 0;

                    db.loadRecentRecords(tempIds, tempTemps, tempRain, tempDays);

                    for (int i = 0; i < tempDays; i++) {
                        session.setRecordId(i, tempIds[i]);
                        session.setTemp(i, tempTemps[i]);
                        session.setRainfall(i, tempRain[i]);
                    }
                    session.setDaysRecorded(tempDays);
                    saveToTextFile(session);
                    std::cout << "Session memory refreshed with updated data." << std::endl;
                }
            }
            else {
                std::cout << "Update is disabled in Offline Mode." << std::endl;
            }
            break;
        }

        case 8: {
            std::cout << "WARNING: This will permanently delete all farm data to start a new season." << std::endl;
            std::cout << "Are you sure? Type 1 to confirm, 0 to cancel: ";
            int confirm = 0;
            std::cin >> confirm;

            if (std::cin.fail()) {
                clearInputBuffer();
                std::cout << "Invalid input. Reset canceled." << std::endl;
                break;
            }

            clearInputBuffer();

            if (confirm == 1) {
                // wipe the local memory arrays
                session = FarmSession();

                // wipe the database
                if (db.isConnected()) {
                    db.resetDatabase();
                }
                std::cout << "System is now completely reset and ready for a new crop cycle." << std::endl;
            }
            else {
                std::cout << "Reset canceled. Your data is retained." << std::endl;
            }
            break;
        }

        case 9: { // targeted delete case
            std::cout << "--- Enter Record to Delete ---" << std::endl;
            std::cout << "Enter the Record ID: ";
            int delId = 0;
            std::cin >> delId;

            clearInputBuffer();

            if (db.isConnected()) {
                if (db.deleteRecord(delId)) {
                    std::cout << "Record " << delId << " removed." << std::endl;

                    // reload to sync memory with the deletion
                    int tempIds[MAX_DAYS];
                    float tempTemps[MAX_DAYS];
                    float tempRain[MAX_DAYS];
                    int tempDays = 0;

                    db.loadRecentRecords(tempIds, tempTemps, tempRain, tempDays);

                    for (int i = 0; i < tempDays; i++) {
                        session.setRecordId(i, tempIds[i]);
                        session.setTemp(i, tempTemps[i]);
                        session.setRainfall(i, tempRain[i]);
                    }
                    session.setDaysRecorded(tempDays);
                    saveToTextFile(session);
                    std::cout << "Session memory refreshed after deletion." << std::endl;
                }
                else {
                    // added for professional error handling
                    std::cout << "Deletion failed. Record " << delId << " may not exist." << std::endl;
                }
            }
            else {
                // added for silent failure preventions
                std::cout << "Action denied. Database is offline." << std::endl;
            }
            break;
        }
        case 10: { // Final Exit
            displaySummaryReport(session);
            if (db.isConnected()) {
                std::cout << "System Exit: Data is safely preserved in the database." << std::endl;
            }
            else {
                std::cout << "System Exit: Offline Mode. In-memory data was not persisted." << std::endl;
            }
            break;
            }
        }
        if (choice != 10) {
            std::cout << "\nSYSTEM: Press Enter to return to main menu.";
            std::cin.get();
        }

    } while (choice != 10);
     
    // the virtual destructor in AdvisoryModule ensures each derived
    // class destructor runs correctly through the base pointer.
    for (int i = 0; i < 4; i++) {
        delete modules[i];
        modules[i] = nullptr;
    }

    return 0;
}
