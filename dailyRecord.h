//FILE:     dailyRecord.h
//AUTHORS:  DELCASTILLO, MANALO, VIRAY, PATDO
//SUBJECT:  COMPUTER PROGRAMMING 2 
//DATE:     May 2026
//Purpose:  Secure the encapsulation of the FarmSession class, protecting
//          daily temperature arrays, rainfall data, and GDD accumulations

#ifndef DAILY_RECORD_H
#define DAILY_RECORD_H

const int MAX_DAYS = 30;

// converted from a plain struct to a fully encapsulated class
// to strictly satisfy NFR2: Structural Modularity.
// all data payload variables are now private and secured.
class FarmSession {
private:
    int recordIds[MAX_DAYS];
    float rainfall[MAX_DAYS];
    float temps[MAX_DAYS];
    int daysRecorded;
    float totalGDD;

public:
    // constructor: initializes all memory arrays and counters to zero.
    // this replaces the manual zeroing loop previously found in main.cpp,
    // ensuring the object is always born in a safe, clean state.
    FarmSession() {
        for (int i = 0; i < MAX_DAYS; i++) {
            recordIds[i] = 0;
            rainfall[i] = 0.0f;
            temps[i] = 0.0f;
        }
        daysRecorded = 0;
        totalGDD = 0.0f;
    }

    ~FarmSession() {}

    // getters (read-only access for advisory modules and reporting)
    int getRecordId(int index) const { return recordIds[index]; }
    int getDaysRecorded() const { return daysRecorded; }
    float getTotalGDD() const { return totalGDD; }
    float getRainfall(int index) const { return rainfall[index]; }
    float getTemp(int index) const { return temps[index]; }

    // setters (controlled modification for input logic)
    void setRecordId(int index, int id) { recordIds[index] = id; }
    void setDaysRecorded(int days) { daysRecorded = days; }
    void setTotalGDD(float gdd) { totalGDD = gdd; }
    void setRainfall(int index, float rain) { rainfall[index] = rain; }
    void setTemp(int index, float temp) { temps[index] = temp; }
};

#endif