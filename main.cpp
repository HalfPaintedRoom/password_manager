#include <iostream>
#include <sodium.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#define LENGTH 20

struct Password
{
    std::string service;
    std::string password;
};

std::string generate_password();
std::string get_service();
void write_to_database(const std::string& service, const std::string& password);

int main()
{
    std::cout << "Password Manager\n";
    std::cout << "----------------\n";
    std::cout << "1: Generate new password\n";
    std::cout << "2: View passwords\n";
    std::cout << "3: Exit\n";

    int choice;
    std::cout << "Enter your choice: ";
    std::cin >> choice;

    if(std::cin.fail())
    {
        choice = 0;
    }

    switch(choice)
    {
        case 1:
        {
            Password new_password = {get_service(), generate_password()};
            if(new_password.service.empty())
            {
                std::cerr << "No service entered" << std::endl;
                break;
            }
            write_to_database(new_password.service, new_password.password);
            std::cout << new_password.service << ": " << new_password.password;
            break;
        }
        case 2:
            std::cout << "Not yet implemented\n";
            break;
        case 3:
            std::cout << "Exiting...";
            break;
        default:
            std::cout << "Invalid input";
    }
}

std::string generate_password()
{
    std::string password;
    char characters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890~`!@#$%^&*()-_=+[{]}\\;:'\"/?.>,<";
    int char_size = sizeof(characters) - 1;

    for(int i = 0; i < LENGTH; i++)
    {
        uint32_t num = randombytes_uniform(char_size);
        password += characters[num];
    }

    return password;
}

std::string get_service()
{
    std::string service;
    std::cin.ignore();

    std::cout << "Enter service: ";
    std::getline(std::cin, service, '\n');

    if(service.find_first_not_of(' ') == std::string::npos)
    {
        return "";
    }

    return service;
}

void write_to_database(const std::string& service, const std::string& password)
{
    try
    {
        SQLite::Database db("test.db", SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        SQLite::Statement query(db, "CREATE TABLE IF NOT EXISTS PASSWORDS ("
                                    "SERVICE TEXT NOT NULL,"
                                    "PASSWORD TEXT NOT NULL);");

        query.exec();

        SQLite::Statement insert(db, "INSERT INTO PASSWORDS (SERVICE, PASSWORD) VALUES (?, ?);");
        insert.bind(1, service);
        insert.bind(2, password);

        insert.exec();
    }
    catch(std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}