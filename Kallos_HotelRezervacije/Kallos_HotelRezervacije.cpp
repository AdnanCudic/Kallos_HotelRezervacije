#include <iostream>
#include <string>
#include <mariadb/conncpp.hpp>
using namespace std;

// konfiguracija baze

#include <iostream>
#include <string>
#include <mariadb/conncpp.hpp>
using namespace std;

// konfiguracija baze
const string DB_HOST = "tcp://localhost:3306";
const string DB_USER = "root";
const string DB_PASS = "password";
const string DB_NAME = "hotel";
/*
klasa Baza se koristi za konekciju na bazu podataka
kada se kreira objekat automatski se konektuje i kreira tabelu
*/
class Baza {
public:
    sql::Driver* driver;
    sql::Connection* konekcija;

    Baza() {
        try {
            driver = sql::mariadb::get_driver_instance();
            sql::SQLString url(DB_HOST);
            sql::Properties props({
                {"user", DB_USER},
                {"password", DB_PASS}
                });
            konekcija = driver->connect(url, props);
            konekcija->setSchema(DB_NAME);
            cout << "Konekcija uspjesna!" << endl;
            kreirajTabelu();
        }
        catch (sql::SQLException& e) {
            cout << "Greska: " << e.what() << endl;
            exit(1);
        }
    }

    ~Baza() {
        delete konekcija;
    }

    /*
    kreiramo tabelu rezervacije ako vec ne postoji
    */
    void kreirajTabelu() {
        try {
            sql::Statement* stmt = konekcija->createStatement();
            stmt->execute(
                "CREATE TABLE IF NOT EXISTS rezervacije ("
                "id INT AUTO_INCREMENT PRIMARY KEY,"
                "ime VARCHAR(100) NOT NULL,"
                "datum_dolaska VARCHAR(20) NOT NULL,"
                "datum_odlaska VARCHAR(20) NOT NULL,"
                "broj_sobe INT NOT NULL UNIQUE)"
            );
            delete stmt;
        }
        catch (sql::SQLException& e) {
            cout << "Greska: " << e.what() << endl;
            exit(1);
        }
    }
};

/*
Hotel klasa ima sve metode za rad sa rezervacijama
*/
class Hotel {
private:
    Baza& baza;

public:
    Hotel(Baza& b) : baza(b) {}

    /*
    dodajRezervaciju unosi novu rezervaciju u bazu
    */
    void dodajRezervaciju() {
        string ime, dolazak, odlazak;
        int soba;

        cout << "\nIme gosta: ";
        cin >> ime;
        cout << "Datum dolaska (DD.MM.YYYY): ";
        cin >> dolazak;
        cout << "Datum odlaska (DD.MM.YYYY): ";
        cin >> odlazak;
        cout << "Broj sobe: ";
        cin >> soba;

        try {
            sql::PreparedStatement* pstmt = baza.konekcija->prepareStatement(
                "INSERT INTO rezervacije (ime, datum_dolaska, datum_odlaska, broj_sobe) VALUES (?, ?, ?, ?)"
            );
            pstmt->setString(1, ime);
            pstmt->setString(2, dolazak);
            pstmt->setString(3, odlazak);
            pstmt->setInt(4, soba);
            pstmt->execute();

            sql::Statement* stmt = baza.konekcija->createStatement();
            sql::ResultSet* res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
            res->next();
            cout << "\nRezervacija uspjesno dodana! (ID: " << res->getInt(1) << ")" << endl;

            delete pstmt;
            delete stmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "\nGreska: Soba " << soba << " je mozda vec rezervisana." << endl;
        }
    }

    /*
    pregledRezervacija ispisuje sve rezervacije iz baze
    */
    void pregledRezervacija() {
        try {
            sql::Statement* stmt = baza.konekcija->createStatement();
            sql::ResultSet* res = stmt->executeQuery("SELECT * FROM rezervacije");

            if (!res->next()) {
                cout << "\nNema aktivnih rezervacija." << endl;
                delete stmt;
                delete res;
                return;
            }

            cout << "\n=== Sve rezervacije ===" << endl;
            int count = 0;
            do {
                count++;
                cout << "-------------------------------" << endl;
                cout << "ID rezervacije : " << res->getInt("id") << endl;
                cout << "Ime gosta      : " << res->getString("ime") << endl;
                cout << "Datum dolaska  : " << res->getString("datum_dolaska") << endl;
                cout << "Datum odlaska  : " << res->getString("datum_odlaska") << endl;
                cout << "Broj sobe      : " << res->getInt("broj_sobe") << endl;
            } while (res->next());
            cout << "-------------------------------" << endl;
            cout << "\nUkupno rezervacija: " << count << endl;

            delete stmt;
            delete res;
        }
        catch (sql::SQLException& e) {
            cout << "Greska: " << e.what() << endl;
        }
    }

    /*
    otkaziRezervaciju brise rezervaciju iz baze po ID-u
    */
    void otkaziRezervaciju() {
        int id;
        cout << "\nUnesite ID rezervacije za otkazivanje: ";
        cin >> id;
        cin.ignore();

        try {
            sql::PreparedStatement* pstmt = baza.konekcija->prepareStatement(
                "DELETE FROM rezervacije WHERE id = ?"
            );
            pstmt->setInt(1, id);
            int obrisano = pstmt->executeUpdate();

            if (obrisano > 0)
                cout << "Rezervacija sa ID " << id << " uspjesno otkazana." << endl;
            else
                cout << "Rezervacija sa ID " << id << " nije pronadjena." << endl;

            delete pstmt;
        }
        catch (sql::SQLException& e) {
            cout << "Greska: " << e.what() << endl;
        }
    }
};

/*
main pokrece program i vrti meni u petlji dok korisnik ne izadje
*/
int main() {
    Baza baza;
    Hotel hotel(baza);

    cout << "\n=== Sistem za rezervaciju hotela ===" << endl;

    while (true) {
        cout << "\n1. Dodaj rezervaciju";
        cout << "\n2. Pregled rezervacija";
        cout << "\n3. Otkazi rezervaciju";
        cout << "\n0. Izlaz";
        cout << "\n\nIzbor: ";

        int izbor;
        cin >> izbor;
        cin.ignore();

        switch (izbor) {
        case 1: hotel.dodajRezervaciju(); break;
        case 2: hotel.pregledRezervacija(); break;
        case 3: hotel.otkaziRezervaciju(); break;
        case 0:
            cout << "\nDovidenja!" << endl;
            return 0;
        default:
            cout << "\nPokusajte ponovo." << endl;
        }
    }
}