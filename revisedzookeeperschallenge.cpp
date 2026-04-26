#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <map>
#include <memory>  // to use unique ptr
#include <deque>   // for "front deletion" - simply moves a pointer or discards the first small block, reduces runtime significantly (to O(1) time)

using namespace std;

class Animal {
protected:
    string name, id, birthDate, color, sex, weight, origin, arrivalDate, species;

public:
    virtual ~Animal() = default;

    // now has setters using const string& to avoid unnecessary copy
    void setName(const string& n) { name = n; }
    void setID(const string& i) { id = i; }
    void setBirthDate(const string& b) { birthDate = b; }
    void setColor(const string& c) { color = c; }
    void setSex(const string& s) { sex = s; }
    void setWeight(const string& w) { weight = w; }
    void setOrigin(const string& o) { origin = o; }
    void setArrivalDate(const string& a) { arrivalDate = a; }

    string getReportLine() const {
        return id + "; " + name + "; birth date " + birthDate + "; " + color +
               "; " + sex + "; " + weight + "; " + origin + "; arrived " + arrivalDate;
    }
};

class Hyena : public Animal { public: Hyena() { species = "Hyena"; } };
class Lion : public Animal { public: Lion() { species = "Lion"; } };
class Tiger : public Animal { public: Tiger() { species = "Tiger"; } };
class Bear : public Animal { public: Bear() { species = "Bear"; } };

string genBirthDay(int age, const string& season) {
    time_t t = time(0);
    tm* now = localtime(&t);
    int birthYear = (1900 + now->tm_year) - age;

    string monthDay = "01-01";
    if (season.find("spring") != string::npos) monthDay = "03-21";
    else if (season.find("summer") != string::npos) monthDay = "06-21";
    else if (season.find("fall") != string::npos) monthDay = "09-21";
    else if (season.find("winter") != string::npos) monthDay = "12-21";

    return to_string(birthYear) + "-" + monthDay;
}

string getCurrentDate() {
    time_t t = time(0);
    tm* now = localtime(&t);
    stringstream ss;
    ss << (1900 + now->tm_year) << "-"
       << setfill('0') << setw(2) << (now->tm_mon + 1) << "-"
       << setw(2) << now->tm_mday;
    return ss.str();
}

int main() {
    map<string, deque<string>> nameRegistry;
    map<string, int> speciesCount = {{"hyena", 0}, {"lion", 0}, {"tiger", 0}, {"bear", 0}};
    
    // map of unique_ptr vectors to automate cleanup
    map<string, vector<unique_ptr<Animal>>> habitats;

    try {
        ifstream nameFile("animalNames.txt");
        if (!nameFile) throw runtime_error("Could not open animalNames.txt");

        string line, currentSpecies;
        while (getline(nameFile, line)) {
            if (line.find("Names:") != string::npos) {
                currentSpecies = line.substr(0, line.find(" "));
                for (auto &c : currentSpecies) c = tolower(c);
            } else if (!line.empty()) {
                stringstream ss(line);
                string segment;
                while (getline(ss, segment, ',')) {
                    segment.erase(0, segment.find_first_not_of(" "));
                    nameRegistry[currentSpecies].push_back(segment);
                }
            }
        }
        nameFile.close();

        ifstream arrivalFile("arrivingAnimals.txt");
        if (!arrivalFile) throw runtime_error("Could not open arrivingAnimals.txt");

        while (getline(arrivalFile, line)) {
            if (line.empty()) continue;

            unique_ptr<Animal> a = nullptr;
            string prefix, speciesPart;

            // determine species
            if (line.find("hyena") != string::npos) { a = make_unique<Hyena>(); prefix = "Hy"; speciesPart = "hyena"; }
            else if (line.find("lion") != string::npos) { a = make_unique<Lion>(); prefix = "Li"; speciesPart = "lion"; }
            else if (line.find("tiger") != string::npos) { a = make_unique<Tiger>(); prefix = "Ti"; speciesPart = "tiger"; }
            else if (line.find("bear") != string::npos) { a = make_unique<Bear>(); prefix = "Be"; speciesPart = "bear"; }

            if (!a) continue;

            // revised parsing logic
            stringstream ss(line);
            string segment;
            vector<string> parts;
            while (getline(ss, segment, ',')) {
                size_t first = segment.find_first_not_of(" ");
                if (first != string::npos) segment = segment.substr(first);
                parts.push_back(segment);
            }

            stringstream ssFirst(parts[0]);
            int age; string sex;
            ssFirst >> age >> segment >> segment >> sex;

            // assignment
            int count = ++speciesCount[speciesPart];
            a->setID(prefix + (count < 10 ? "0" : "") + to_string(count));
            a->setSex(sex);
            a->setBirthDate(genBirthDay(age, parts[1]));
            a->setColor(parts[2]);
            a->setWeight(parts[3]);
            a->setOrigin(parts[4]);
            a->setArrivalDate(getCurrentDate());

            if (!nameRegistry[speciesPart].empty()) {
                a->setName(nameRegistry[speciesPart].front());
                nameRegistry[speciesPart].pop_front(); // O(1) efficiency
            }

            // automatically sort into correct habitat bucket
            habitats[speciesPart].push_back(move(a));
        }

        // now: iterates through the map rather than hardcoding each species
        ofstream outFile("zooPopulation.txt");
        for (auto const& [species, list] : habitats) {
            string title = species;
            title[0] = toupper(title[0]); // Capitalize title
            outFile << title << " Habitat:" << endl;
            for (const auto& animalPtr : list) {
                outFile << animalPtr->getReportLine() << endl;
            }
            outFile << endl;
        }
        
        // no manual delete loops thanks to unique ptr

    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}