//FILE:     DatabaseManager.cpp
//AUTHORS:  DELCASTILLO, MANALO, VIRAY, PATDO
//SUBJECT:  COMPUTER PROGRAMMING 2 
//DATE:     May 2026
//Purpose:  MySQL query implementation for persistent storage
//          targeted deletion, and memory synchronization

#include <iostream>
#include "DatabaseManager.h"
#include "dailyRecord.h"
#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>

// the constructor: initializes the connection pointer to null.
// the program begins in a safely disconnected state until
// connect() is explicitly called.
DatabaseManager::DatabaseManager() {
    con = nullptr;
}

// destructor: releases the connection object when the program closes.
// this prevents memory leaks by cleaning up the heap-allocated connection.
DatabaseManager::~DatabaseManager() {
    if (con != nullptr) {
        delete con;
        con = nullptr;
    }
}

// establishes the connection to the agri_dss MySQL database.
// returns true on success; returns false and prints a diagnostic
// message on failure, allowing the caller to enter Offline Mode.
bool DatabaseManager::connect(std::string username, std::string password) {
    // safety check block
    if (con != nullptr) {
        delete con;
        con = nullptr;
    }
    try {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", username, password);
        con->setSchema("agri_dss");
        if (!validateSchema()) {
            std::cout << "CRITICAL: Validation failed. Switching to offline mode." << std::endl; 
            delete con; 
            con = nullptr; 
            return false; 
        }
        return true;
    }
    catch (sql::SQLException& e) {
        std::cout << "CRITICAL: Database connection failed. " << e.what() << std::endl;
        con = nullptr;
        return false;
    }
}

// returns true if the connection pointer is non-null (i.e., a
// successful connect() call was made and no fatal error has since
// invalidated the connection).
bool DatabaseManager::isConnected() const {
    return con != nullptr;
}

// inserts one day's weather record into the daily_records table.
// the gdd parameter must be the DAILY increment only, not the
// cumulative season total, so that getTotalGDD()'s SUM() query
// remains accurate across all sessions.
// resource safety: pstmt is deleted in both the normal path and
// the exception path to prevent a heap leak if executeUpdate() throws.
bool DatabaseManager::insertDailyRecord(float temp, float rainfall, float gdd) {
    if (con == nullptr) return false;

    sql::PreparedStatement* pstmt = nullptr;
    try {
        pstmt = con->prepareStatement(
            "INSERT INTO daily_records (temperature, rainfall_mm, daily_gdd) VALUES (?, ?, ?)"
        );
        pstmt->setDouble(1, temp);
        pstmt->setDouble(2, rainfall);
        pstmt->setDouble(3, gdd);
        pstmt->executeUpdate();
        delete pstmt;
        pstmt = nullptr;
        return true; 
    }
    catch (sql::SQLException& e) {
        delete pstmt;          // guaranteed cleanup on the error path
        pstmt = nullptr;
        std::cout << "Database Save Error: " << e.what() << std::endl;
        std::cout << "Switching to Offline Mode for the remainder of this session." << std::endl;
        delete con;            // invalidate a likely broken connection
        con = nullptr;
        return false; 
    }
}

// queries the SUM of all daily_gdd values stored in daily_records,
// reconstructing the full accumulated GDD for the current season.
// returns 0.0 if the connection is unavailable or if the table is empty.
// resource safety: both res and stmt are deleted in the normal path
// and in the exception path to prevent heap leaks.
float DatabaseManager::getTotalGDD() {
    if (con == nullptr) return 0.0f;

    float total = 0.0f;
    sql::Statement* stmt = nullptr;
    sql::ResultSet* res = nullptr;

    try {
        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT SUM(daily_gdd) AS gdd_sum FROM daily_records");

        if (res->next()) {
            // isNull() guard: SUM() returns NULL on an empty table.
            // Without this check, getDouble() on a NULL column is undefined.
            if (!res->isNull("gdd_sum")) {
                total = static_cast<float>(res->getDouble("gdd_sum"));
            }
        }

        delete res;
        res = nullptr;
        delete stmt;
        stmt = nullptr;
    }
    catch (sql::SQLException& e) {
        delete res;            // guaranteed cleanup on the error path
        res = nullptr;
        delete stmt;
        stmt = nullptr;
        std::cout << "Database Read Error: " << e.what() << std::endl;
        std::cout << "Switching to Offline Mode for the remainder of this session." << std::endl;
        delete con;            // invalidate a likely broken connection
        con = nullptr;
    }

    return total;
}

// queries the most recent rows from daily_records (up to MAX_DAYS) and
// populates the session arrays in chronological order so that advisory
// modules have immediate historical context on startup.
// Resource safety: res and stmt are deleted in both the normal
// and exception paths to prevent heap leaks.
void DatabaseManager::loadRecentRecords(int ids[], float temps[], float rainfall[], int& daysRecorded) {
    if (con == nullptr) return;
    sql::Statement* stmt = nullptr;
    sql::ResultSet* res = nullptr;

    try {
        stmt = con->createStatement();
        // subquery fetches the most recent MAX_DAYS rows by id descending,
        // then the outer ORDER BY re-sorts them oldest-first so the arrays
        // are filled in the correct chronological order.
        res = stmt->executeQuery(
            "SELECT id, temperature, rainfall_mm FROM "
            "(SELECT id, temperature, rainfall_mm FROM daily_records "
            " ORDER BY id DESC LIMIT 30) AS recent "
            "ORDER BY id ASC"
        );

        daysRecorded = 0;
        while (res->next() && daysRecorded < MAX_DAYS) {
            ids[daysRecorded] = res->getInt("id");
            temps[daysRecorded] = static_cast<float>(res->getDouble("temperature"));
            rainfall[daysRecorded] = static_cast<float>(res->getDouble("rainfall_mm"));
            daysRecorded++;
        }

        delete res;  res = nullptr;
        delete stmt; stmt = nullptr;
    }
    catch (sql::SQLException& e) {
        delete res;  res = nullptr;
        delete stmt; stmt = nullptr;
        std::cout << "Database Read Error: " << e.what() << std::endl;
        std::cout << "Switching to Offline Mode for the remainder of this session." << std::endl;
        delete con;
        con = nullptr;
    }
}

void DatabaseManager::resetDatabase() {
    if (con == nullptr) {
        std::cout << "Cannot reset. Database is currently offline." << std::endl;
        return;
    }

    sql::Statement* stmt = nullptr;
    try {
        stmt = con->createStatement();
        stmt->execute("TRUNCATE TABLE daily_records");
        delete stmt;
        std::cout << "Database records successfully wiped for the new season." << std::endl;
    }
    catch (sql::SQLException& e) {
        delete stmt;
        std::cout << "Database Reset Error: " << e.what() << std::endl;
    }
}

bool DatabaseManager::validateSchema() {
    sql::Statement* stmt = nullptr; 
    sql::ResultSet* res = nullptr; 
    
    int foundID = 0, foundTemp = 0, foundRainfall = 0, foundGDD = 0; 
    
    try {
        stmt = con->createStatement(); 
        res = stmt->executeQuery("DESCRIBE daily_records"); 

        while (res->next()) {
            std::string col = res->getString("Field"); 
            if (col == "id") foundID = 1;
            if (col == "temperature") foundTemp = 1;
            if (col == "rainfall_mm") foundRainfall = 1;
            if (col == "daily_gdd") foundGDD = 1;
        }
        delete res; res = nullptr; 
        delete stmt; stmt = nullptr; 

        if (!foundID) {
            std::cout << "ERROR: Required column 'id' is missing from daily_records." << std::endl; 
            return false;
        }
        if (!foundTemp) {
            std::cout << "ERROR: Required column 'temperature' is missing from daily_records." << std::endl;
            return false;
        }
        if (!foundRainfall) {
            std::cout << "ERROR: Required column 'rainfall_mm' is missing from daily_records." << std::endl;
            return false;
        }
        if (!foundGDD) {
            std::cout << "ERROR: Required column 'daily_gdd' is missing from daily_records." << std::endl;
            return false;
        }
        return true;
    }
    catch (sql::SQLException& e) {
        delete res; res = nullptr; 
        delete stmt; stmt = nullptr; 
        std::cout << "ERROR: Could not read daily_records." << e.what() << std::endl; 
        return false; 
    }

}
void DatabaseManager::getRecordById(int id) {
    if (con == nullptr) {
        std::cout << "Database is offline. Cannot search records." << std::endl;
        return;
    }

    sql::PreparedStatement* pstmt = nullptr;
    sql::ResultSet* res = nullptr;

    try {
        pstmt = con->prepareStatement("SELECT * FROM daily_records WHERE id = ?");
        pstmt->setInt(1, id);
        res = pstmt->executeQuery();

        std::cout << "\n--- Search Results ---" << std::endl;
        if (res->next()) {
            std::cout << "Record ID:   " << res->getInt("id") << std::endl;
            std::cout << "Temperature: " << res->getDouble("temperature") << " C" << std::endl;
            std::cout << "Rainfall:    " << res->getDouble("rainfall_mm") << " mm" << std::endl;
            std::cout << "Daily GDD:   " << res->getDouble("daily_gdd") << std::endl;
        }
        else {
            std::cout << "No record found with ID " << id << "." << std::endl;
        }

        delete res; res = nullptr; 
        delete pstmt; pstmt = nullptr; 
    }
    catch (sql::SQLException& e) {
        delete res;
        delete pstmt;
        std::cout << "Database Search Error: " << e.what() << std::endl;
    }
}
bool DatabaseManager::updateDailyRecord(int id, float temp, float rainfall, float gdd) {
    if (con == nullptr) return false;

    sql::PreparedStatement* pstmt = nullptr;
    try {
        pstmt = con->prepareStatement(
            "UPDATE daily_records SET temperature = ?, rainfall_mm = ?, daily_gdd = ? WHERE id = ?"
        );
        pstmt->setDouble(1, temp);
        pstmt->setDouble(2, rainfall);
        pstmt->setDouble(3, gdd);
        pstmt->setInt(4, id);

        // executeUpdate returns the number of rows affected
        int rowsUpdated = pstmt->executeUpdate();

        delete pstmt;
        pstmt = nullptr; 

        if (rowsUpdated > 0) {
            return true;
        }
        else {
            std::cout << "Update failed. Record ID " << id << " does not exist." << std::endl;
            return false;
        }
    }
    catch (sql::SQLException& e) {
        if (pstmt != nullptr) {
            delete pstmt; 
            pstmt = nullptr; 
        }
        std::cout << "Database Update Error: " << e.what() << std::endl;
        return false;
    }
}

bool DatabaseManager::deleteRecord(int id) {
     if (con == nullptr) return false;
    try {
        sql::PreparedStatement* pstmt = con->prepareStatement("DELETE FROM daily_records WHERE id = ?");
        pstmt->setInt(1, id);
        pstmt->executeUpdate();

        delete pstmt;
        pstmt = nullptr; // Resource safety: set to nullptr after delete
        return true;
    }
    catch (sql::SQLException& e) {
        std::cout << "Database Delete Error: " << e.what() << std::endl;
        return false;
    }
}
