#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <limits>
#include <algorithm>

using namespace std;

// Base Class: Vehicle
class Vehicle {
protected:
    string vehicleNumber;
    string model;
    string category; // "Car" or "Bike"
    double rentPerDay;
    bool isAvailable;
    int rentedDays;

public:
    // Parameterized Constructor
    Vehicle(string vNum = "", string mod = "", string cat = "", double rent = 0.0, bool avail = true, int days = 0)
        : vehicleNumber(vNum), model(mod), category(cat), rentPerDay(rent), isAvailable(avail), rentedDays(days) {}

    // Virtual Destructor
    virtual ~Vehicle() {}

    // Getters (Encapsulation)
    string getVehicleNumber() const { return vehicleNumber; }
    string getModel() const { return model; }
    string getCategory() const { return category; }
    double getRentPerDay() const { return rentPerDay; }
    bool getAvailability() const { return isAvailable; }
    int getRentedDays() const { return rentedDays; }

    // Virtual function for calculating rent (Runtime Polymorphism)
    virtual double calculateRent(int days) const {
        return rentPerDay * days;
    }

    // Virtual function to display details
    virtual void displayDetails() const {
        cout << left << setw(14) << vehicleNumber
             << setw(18) << model
             << setw(10) << category
             << "Rs. " << setw(10) << fixed << setprecision(2) << rentPerDay
             << setw(14) << (isAvailable ? "Available" : "Rented") << endl;
    }

    // Rent the vehicle
    bool rentVehicle(int days) {
        if (!isAvailable || days <= 0) {
            return false;
        }
        isAvailable = false;
        rentedDays = days;
        return true;
    }

    // Return the vehicle and compute final charge
    double returnVehicle() {
        if (isAvailable) {
            return -1.0;
        }
        double totalAmount = calculateRent(rentedDays);
        isAvailable = true;
        rentedDays = 0;
        return totalAmount;
    }

    // Serialize object data to CSV line for file storage
    virtual string toFileString() const {
        return category + "," + vehicleNumber + "," + model + "," +
               to_string(rentPerDay) + "," + (isAvailable ? "1" : "0") + "," +
               to_string(rentedDays);
    }
};

// Derived Class 1: Car (is-a Vehicle)
class Car : public Vehicle {
private:
    int seatingCapacity;

public:
    Car(string vNum, string mod, double rent, int seats = 5, bool avail = true, int days = 0)
        : Vehicle(vNum, mod, "Car", rent, avail, days), seatingCapacity(seats) {}

    // Overridden rent calculation: Includes 10% insurance/maintenance fee for Cars
    double calculateRent(int days) const override {
        double base = rentPerDay * days;
        double insuranceFee = base * 0.10;
        return base + insuranceFee;
    }

    void displayDetails() const override {
        cout << left << setw(14) << vehicleNumber
             << setw(18) << model
             << setw(10) << ("Car(" + to_string(seatingCapacity) + "S)")
             << "Rs. " << setw(10) << fixed << setprecision(2) << rentPerDay
             << setw(14) << (isAvailable ? "Available" : "Rented") << endl;
    }

    string toFileString() const override {
        return Vehicle::toFileString() + "," + to_string(seatingCapacity);
    }
};

// Derived Class 2: Bike (is-a Vehicle)
class Bike : public Vehicle {
private:
    bool helmetIncluded;

public:
    Bike(string vNum, string mod, double rent, bool helmet = true, bool avail = true, int days = 0)
        : Vehicle(vNum, mod, "Bike", rent, avail, days), helmetIncluded(helmet) {}

    // Overridden rent calculation: Flat Rs. 50 helmet/gear sanitization fee for Bikes
    double calculateRent(int days) const override {
        return (rentPerDay * days) + (helmetIncluded ? 50.0 : 0.0);
    }

    void displayDetails() const override {
        cout << left << setw(14) << vehicleNumber
             << setw(18) << model
             << setw(10) << (helmetIncluded ? "Bike(+H)" : "Bike")
             << "Rs. " << setw(10) << fixed << setprecision(2) << rentPerDay
             << setw(14) << (isAvailable ? "Available" : "Rented") << endl;
    }

    string toFileString() const override {
        return Vehicle::toFileString() + "," + (helmetIncluded ? "1" : "0");
    }
};

// Manager Class: Handles fleet operations and file persistence
class RentalSystem {
private:
    vector<Vehicle*> fleet;
    const string filename = "vehicles_data.txt";

    // Helper: Convert string to uppercase for case-insensitive comparison
    string toUpper(string str) const {
        transform(str.begin(), str.end(), str.begin(), ::toupper);
        return str;
    }

    // Helper: Find vehicle index by number
    int findVehicleIndex(const string& vNum) const {
        string target = toUpper(vNum);
        for (size_t i = 0; i < fleet.size(); ++i) {
            if (toUpper(fleet[i]->getVehicleNumber()) == target) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

public:
    RentalSystem() {
        loadFromFile();
        // Seed initial data if file is empty or missing
        if (fleet.empty()) {
            fleet.push_back(new Car("DL01AB1234", "Maruti Swift", 1500.0, 5));
            fleet.push_back(new Car("MH12CD5678", "Hyundai Creta", 2500.0, 5));
            fleet.push_back(new Bike("UP32EF9012", "Royal Enfield", 800.0, true));
            fleet.push_back(new Bike("DL05GH3456", "Yamaha R15", 700.0, true));
            saveToFile();
        }
    }

    ~RentalSystem() {
        saveToFile();
        for (Vehicle* v : fleet) {
            delete v;
        }
        fleet.clear();
    }

    // Save all records to file
    void saveToFile() const {
        ofstream outFile(filename);
        if (!outFile) {
            cerr << "Error: Unable to open file for saving records.\n";
            return;
        }
        for (const Vehicle* v : fleet) {
            outFile << v->toFileString() << "\n";
        }
        outFile.close();
    }

    // Load records from file
    void loadFromFile() {
        ifstream inFile(filename);
        if (!inFile) {
            return; // File does not exist yet
        }
        string line;
        while (getline(inFile, line)) {
            if (line.empty()) continue;
            vector<string> tokens;
            size_t start = 0, end = 0;
            while ((end = line.find(',', start)) != string::npos) {
                tokens.push_back(line.substr(start, end - start));
                start = end + 1;
            }
            tokens.push_back(line.substr(start));

            if (tokens.size() >= 7) {
                string cat = tokens[0];
                string vNum = tokens[1];
                string mod = tokens[2];
                double rent = stod(tokens[3]);
                bool avail = (tokens[4] == "1");
                int days = stoi(tokens[5]);
                int extra = stoi(tokens[6]);

                if (cat == "Car") {
                    fleet.push_back(new Car(vNum, mod, rent, extra, avail, days));
                } else if (cat == "Bike") {
                    fleet.push_back(new Bike(vNum, mod, rent, extra == 1, avail, days));
                }
            }
        }
        inFile.close();
    }

    // 1. Add a new vehicle
    void addVehicle() {
        int typeChoice;
        cout << "\nSelect Vehicle Category (1. Car, 2. Bike): ";
        if (!(cin >> typeChoice) || (typeChoice != 1 && typeChoice != 2)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid category choice!\n";
            return;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string vNum, mod;
        double rent;
        cout << "Enter Vehicle Number (e.g., DL01XY9999): ";
        getline(cin, vNum);
        if (vNum.empty() || findVehicleIndex(vNum) != -1) {
            cout << "Error: Vehicle number is empty or already exists!\n";
            return;
        }

        cout << "Enter Vehicle Model: ";
        getline(cin, mod);

        cout << "Enter Rent Per Day (Rs.): ";
        if (!(cin >> rent) || rent <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: Rent per day must be a positive number!\n";
            return;
        }

        if (typeChoice == 1) {
            int seats;
            cout << "Enter Seating Capacity (e.g., 5 or 7): ";
            if (!(cin >> seats) || seats <= 0) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid seating capacity! Defaulting to 5.\n";
                seats = 5;
            }
            fleet.push_back(new Car(toUpper(vNum), mod, rent, seats));
        } else {
            int hChoice;
            cout << "Helmet Included? (1 for Yes, 0 for No): ";
            cin >> hChoice;
            fleet.push_back(new Bike(toUpper(vNum), mod, rent, hChoice == 1));
        }

        saveToFile();
        cout << "Vehicle added successfully!\n";
    }

    // 2. Display available vehicles (or all vehicles)
    void displayVehicles(bool onlyAvailable) const {
        cout << "\n----------------------------------------------------------------------\n";
        cout << left << setw(14) << "Vehicle No."
             << setw(18) << "Model"
             << setw(10) << "Category"
             << setw(14) << "Rent/Day"
             << setw(14) << "Status" << endl;
        cout << "----------------------------------------------------------------------\n";
        bool found = false;
        for (const Vehicle* v : fleet) {
            if (!onlyAvailable || v->getAvailability()) {
                v->displayDetails();
                found = true;
            }
        }
        if (!found) {
            cout << (onlyAvailable ? "No vehicles currently available for rent.\n" : "No vehicles in fleet.\n");
        }
        cout << "----------------------------------------------------------------------\n";
    }

    // 3. Search by vehicle number
    void searchVehicle() const {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string vNum;
        cout << "\nEnter Vehicle Number to search: ";
        getline(cin, vNum);

        int idx = findVehicleIndex(vNum);
        if (idx == -1) {
            cout << "Vehicle with number '" << vNum << "' not found.\n";
            return;
        }
        cout << "\nVehicle Found:\n";
        cout << "----------------------------------------------------------------------\n";
        fleet[idx]->displayDetails();
        if (!fleet[idx]->getAvailability()) {
            cout << "Currently rented for " << fleet[idx]->getRentedDays() << " day(s).\n";
        }
        cout << "----------------------------------------------------------------------\n";
    }

    // 4. Rent a vehicle
    void rentVehicleMenu() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string vNum;
        cout << "\nEnter Vehicle Number to rent: ";
        getline(cin, vNum);

        int idx = findVehicleIndex(vNum);
        if (idx == -1) {
            cout << "Error: Vehicle not found.\n";
            return;
        }
        if (!fleet[idx]->getAvailability()) {
            cout << "Error: Vehicle '" << fleet[idx]->getVehicleNumber() << "' is already rented!\n";
            return;
        }

        int days;
        cout << "Enter number of rental days: ";
        if (!(cin >> days) || days <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Error: Invalid number of days! Must be at least 1.\n";
            return;
        }

        fleet[idx]->rentVehicle(days);
        double estAmount = fleet[idx]->calculateRent(days);
        saveToFile();

        cout << "\n--- RENTAL CONFIRMED ---\n";
        cout << "Vehicle   : " << fleet[idx]->getModel() << " (" << fleet[idx]->getVehicleNumber() << ")\n";
        cout << "Duration  : " << days << " day(s)\n";
        cout << "Est. Total: Rs. " << fixed << setprecision(2) << estAmount
             << (fleet[idx]->getCategory() == "Car" ? " (incl. 10% insurance)" : " (incl. helmet fee if applicable)") << "\n";
    }

    // 5. Return a vehicle and calculate final bill
    void returnVehicleMenu() {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        string vNum;
        cout << "\nEnter Vehicle Number being returned: ";
        getline(cin, vNum);

        int idx = findVehicleIndex(vNum);
        if (idx == -1) {
            cout << "Error: Vehicle not found.\n";
            return;
        }
        if (fleet[idx]->getAvailability()) {
            cout << "Error: Vehicle '" << fleet[idx]->getVehicleNumber() << "' is not currently rented.\n";
            return;
        }

        int days = fleet[idx]->getRentedDays();
        double totalBill = fleet[idx]->returnVehicle();
        saveToFile();

        cout << "\n--- RENTAL RECEIPT ---\n";
        cout << "Vehicle Number : " << fleet[idx]->getVehicleNumber() << "\n";
        cout << "Model          : " << fleet[idx]->getModel() << " [" << fleet[idx]->getCategory() << "]\n";
        cout << "Days Rented    : " << days << "\n";
        cout << "Rate Per Day   : Rs. " << fixed << setprecision(2) << fleet[idx]->getRentPerDay() << "\n";
        cout << "Total Payable  : Rs. " << fixed << setprecision(2) << totalBill << "\n";
        cout << "Status Updated : Available\n";
        cout << "----------------------\n";
    }
};

int main() {
    RentalSystem system;
    int choice;

    do {
        cout << "\n============================================\n";
        cout << "     VEHICLE RENTAL MANAGEMENT SYSTEM       \n";
        cout << "============================================\n";
        cout << "1. Add New Vehicle\n";
        cout << "2. Display Available Vehicles\n";
        cout << "3. Display All Vehicles\n";
        cout << "4. Search Vehicle by Number\n";
        cout << "5. Rent a Vehicle\n";
        cout << "6. Return a Vehicle & Calculate Bill\n";
        cout << "7. Exit\n";
        cout << "Enter your choice (1-7): ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input! Please enter a number between 1 and 7.\n";
            continue;
        }

        switch (choice) {
            case 1: system.addVehicle(); break;
            case 2: system.displayVehicles(true); break;
            case 3: system.displayVehicles(false); break;
            case 4: system.searchVehicle(); break;
            case 5: system.rentVehicleMenu(); break;
            case 6: system.returnVehicleMenu(); break;
            case 7: cout << "Saving records and exiting... Thank you!\n"; break;
            default: cout << "Invalid choice! Please select 1-7.\n";
        }
    } while (choice != 7);

    return 0;
}
