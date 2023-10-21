#include <iostream>
#include <sodium.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <algorithm>
#define LENGTH 20
#define DATABASE "passwords.db"

struct Password
{
    std::string service;
    std::string password;
};

int validate_input();
std::string trim(std::string &str);
bool initialize_database(SQLite::Database *db);
void generate_password(Password *new_password);
void enter_password(Password *new_password);
void get_service(Password *new_password);
void write_to_database(Password *new_password, SQLite::Database *db);
void view_passwords(SQLite::Database *db);

int main()
{
    if (sodium_init() < 0) {
        std::cerr << "Sodium library initialization failed" << std::endl;
        return 1;
    }

    SQLite::Database db("");

    if(!initialize_database(&db))
    {
        std::cerr << "Failed to open database" <<std::endl;
        return 1;
    }

    int loop = 1;
    while(loop)
    {
        std::cout << "\nPassword Manager\n";
        std::cout << "----------------\n";
        std::cout << "1: Generate new password\n";
        std::cout << "2: View passwords\n";
        std::cout << "3: Exit\n";

        int choice = validate_input();

        switch(choice)
        {
            case 1:
            {
                auto *new_password = new Password;
                get_service(new_password);
                generate_password(new_password);

                if(new_password->service.empty())
                {
                    std::cerr << "No service entered" << std::endl;
                    break;
                }

                write_to_database(new_password, &db);
                std::cout << new_password->service << ": " << new_password->password << std::endl;

                delete(new_password);
                break;
            }
            case 2:
                view_passwords(&db);
                break;
            case 3:
                std::cout << "\nExiting...";
                loop = 0;
                break;
            default:
                std::cout << "\nInvalid input";
        }
    }

}

bool initialize_database(SQLite::Database *db)
{
    try
    {
        *db = SQLite::Database(DATABASE, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement query(*db, "CREATE TABLE IF NOT EXISTS PASSWORDS ("
                                    "SERVICE TEXT NOT NULL,"
                                    "PASSWORD TEXT NOT NULL);");

        query.exec();

        return true;
    }
    catch(std::exception &e)
    {
        std::cout << "exception: " << e.what() << std::endl;
        return false;
    }
}

int validate_input()
{
    int choice;
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    if(std::cin.fail())
    {
        choice = 0;
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    return choice;
}

std::string trim(std::string &str)
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');

    if(first == std::string::npos)
    {
        return "";
    }

    return str.substr(first, (last - first + 1));
}

void generate_password(Password *new_password)
{
    std::string password;
    char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~`!@#$%^&*()-_=+[{]}\\;:'\"/?.>,<";
    int char_size = sizeof(characters) - 1;

    for(int i = 0; i < LENGTH; i++)
    {
        uint32_t num = randombytes_uniform(char_size);
        password += characters[num];
    }

    new_password->password = password;
}

void get_service(Password *new_password)
{
    std::string service;
    std::cin.ignore();

    std::cout << "\nEnter service: ";
    std::getline(std::cin, service, '\n');

    service = trim(service);

    new_password->service = service;
}

void write_to_database(Password *new_password, SQLite::Database *db)
{
    try
    {
        SQLite::Statement insert(*db, "INSERT INTO PASSWORDS (SERVICE, PASSWORD) VALUES (?, ?);");
        insert.bind(1, new_password->service);
        insert.bind(2, new_password->password);

        insert.exec();
    }
    catch(std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}

void view_passwords(SQLite::Database *db)
{
    try
    {
        SQLite::Statement   query(*db, "SELECT * FROM PASSWORDS");

        // Loop to execute the query step by step, to get rows of result
        for(int i = 1; query.executeStep(); i++)
        {
            // Demonstrate how to get some typed column value
            int         id          = i;
            std::string service     = query.getColumn(0);
            std::string password    = query.getColumn(1);

            std::cout << id << ": " << service << " | " << password << std::endl;
        }
    }
    catch (std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}