//FILE:     DatabaseManager.h
//AUTHORS:  DELCASTILLO, MANALO, VIRAY, PATDO
//SUBJECT:  COMPUTER PROGRAMMING 2 
//DATE:     May 2026
//Purpose:  Database bridge class declarations responsible for 
//          securely managing MySQL connection, as well as the 
//          CRUD operations

#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <jdbc/mysql_connection.h> 

class DatabaseManager {
private:
    // this connection pointer is hidden from the rest of the program
    sql::Connection* con;
    //now, this runs a DESCRIBE query on startup to confirm the table and all
    //required columns exist before any data operations are attempted 
    bool validateSchema();

public:
    // setup and breakdown
    DatabaseManager();
    ~DatabaseManager();

    // the core actions
    bool connect(std::string username, std::string password); 
    bool isConnected() const; 
    bool insertDailyRecord(float temp, float rainfall, float gdd);
    float getTotalGDD();

    void loadRecentRecords(int ids[], float temps[], float rainfall[], int& daysRecorded);
    void resetDatabase();

    void getRecordById(int id);
    bool updateDailyRecord(int id, float temp, float rainfall, float gdd);

    bool deleteRecord(int id); 

};

#endif