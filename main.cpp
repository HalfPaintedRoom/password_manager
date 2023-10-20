#include <iostream>
#include <sodium.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#define LENGTH 20
#define DATABASE "passwords.db"

struct Password
{
    std::string service;
    std::string password;
};

void generate_password(Password *new_password);
void get_service(Password *new_password);
void write_to_database(Password *new_password);

int main()
{
    int loop = 1;
    while(loop)
    {
        std::cout << "\nPassword Manager\n";
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
                auto *new_password = new Password;
                get_service(new_password);
                generate_password(new_password);

                if(new_password->service.empty())
                {
                    std::cerr << "No service entered" << std::endl;
                    break;
                }

                write_to_database(new_password);
                std::cout << new_password->service << ": " << new_password->password << std::endl;
                break;
            }
            case 2:
                std::cout << "\nNot yet implemented\n";
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

    //makes sure string is not empty
    if(service.find_first_not_of(' ') == std::string::npos)
    {
        new_password->service = "";
    }

    new_password->service = service;
}

void write_to_database(Password *new_password)
{
    try
    {
        SQLite::Database db(DATABASE, SQLite::OPEN_READWRITE|SQLite::OPEN_CREATE);
        SQLite::Statement query(db, "CREATE TABLE IF NOT EXISTS PASSWORDS ("
                                    "SERVICE TEXT NOT NULL,"
                                    "PASSWORD TEXT NOT NULL);");

        query.exec();

        SQLite::Statement insert(db, "INSERT INTO PASSWORDS (SERVICE, PASSWORD) VALUES (?, ?);");
        insert.bind(1, new_password->service);
        insert.bind(2, new_password->password);

        insert.exec();
    }
    catch(std::exception& e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}