#include <iostream>
#include <vector>
#include <sodium.h>
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Statement.h>
#include <algorithm>

#define LENGTH 20
#define DATABASE "passwords.db"

int num_passwords = 0;
std::string master_pass;

struct Password
{
    std::string service;
    std::string password;
};

//function prototypes
//function definitions should be in the same order that they are here
int validate_input();
std::string trim(std::string &str);
bool initialize_database(SQLite::Database *db);
void count_rows(SQLite::Database *db);
void show_passwords_menu(SQLite::Database *db);
void display_passwords(SQLite::Database *db);
void new_password_menu(SQLite::Database *db);
void delete_password(SQLite::Database *db);
void generate_password(Password *new_password);
void get_service(std::string *new_service);
void get_password(std::string *new_password);
void write_to_database(Password *new_password, SQLite::Database *db);
std::string encrypt_data(std::string &data_to_encrypt, std::string &key);
std::string decrypt_data(std::string &data_to_decrypt, std::string &key);
bool check_master_pass(SQLite::Database *db);

int main()
{
    if(sodium_init() < 0)
    {
        std::cout << "Sodium library initialization failed" << std::endl;
        return 1;
    }

    SQLite::Database db("");

    if(!initialize_database(&db))
    {
        std::cout << "Failed to open database" << std::endl;
        return 1;
    }

    if(!check_master_pass(&db))
    {
        std::cout << "Invalid password" << std::endl;
        return 1;
    }

    int loop = 1;
    while(loop)
    {
        std::cout << "\nPassword Manager\n";
        std::cout << "----------------\n";
        std::cout << "1: View passwords\n";
        std::cout << "2: New password\n";
        std::cout << "3: Exit\n";

        int choice = validate_input();

        switch(choice)
        {
            case 1:
            {
                show_passwords_menu(&db);
                break;
            }
            case 2:
                new_password_menu(&db);
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

bool initialize_database(SQLite::Database *db)
{
    try
    {

        *db = SQLite::Database(DATABASE, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        SQLite::Statement query(*db, "CREATE TABLE IF NOT EXISTS PASSWORDS ("
                                     "SERVICE TEXT NOT NULL,"
                                     "PASSWORD TEXT NOT NULL);");

        query.exec();

        SQLite::Statement user_auth(*db, "CREATE TABLE IF NOT EXISTS USER_AUTH ("
                                         "MASTER_PASS TEXT NOT NULL);");
        user_auth.exec();

        return true;
    }
    catch(std::exception &e)
    {
        std::cout << "exception: " << e.what() << std::endl;
        return false;
    }
}

void count_rows(SQLite::Database *db)
{
    SQLite::Statement query(*db, "SELECT COUNT(*) FROM PASSWORDS");

    if(query.executeStep())
    {
        num_passwords = query.getColumn(0).getInt();
    } else
    {
        std::cout << "Could not count rows" << std::endl;
        return;
    }
}

void show_passwords_menu(SQLite::Database *db)
{

    int loop = 1;
    while(loop)
    {
        display_passwords(db);

        std::cout << "\n1: Edit Password\n";
        std::cout << "2: Delete Password\n";
        std::cout << "3: Return to Main Menu\n";

        int choice = validate_input();
        switch(choice)
        {
            case 1:
                std::cout << "Not implemented yet" << std::endl;
                break;
            case 2:
                delete_password(db);
                break;
            case 3:
                loop = 0;
                break;
            default:
                std::cout << "Invalid input" << std::endl;
                break;
        }
    }
}

//displays all passwords in the menu
void display_passwords(SQLite::Database *db)
{
    try
    {
        SQLite::Statement query(*db, "SELECT * FROM PASSWORDS");

        count_rows(db);

        if(num_passwords == 0)
        {
            std::cout << "No passwords to display" << std::endl;
            return;
        }

        std::cout << "\n";

        for(int i = 1; query.executeStep(); i++)
        {
            int id = i;
            std::string service = query.getColumn(0);
            std::string password = query.getColumn(1);

            service = decrypt_data(service, master_pass);
            password = decrypt_data(password, master_pass);

            std::cout << id << ": " << service << " | " << password << std::endl;
        }

    }
    catch(std::exception &e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}

//deletes the selected password from the database
void delete_password(SQLite::Database *db)
{
    std::cout << "Which password would you like to delete\n";
    int choice = validate_input();

    if(choice == 0 || choice > num_passwords)
    {
        std::cout << "Not a valid input" << std::endl;
        return;
    }

    std::cout << choice << std::endl;

    SQLite::Statement query(*db, "DELETE FROM PASSWORDS WHERE ROWID = ?");

    query.bind(1, choice);

    if(query.exec() == 1)
    {
        // Deletion was successful
        SQLite::Statement update_ids(*db, "UPDATE PASSWORDS SET ROWID = ROWID - 1 WHERE ROWID > ?");
        update_ids.bind(1, choice);
        update_ids.exec();

        num_passwords--;
    } else
    {
        std::cout << "Failed to delete the password" << std::endl;
    }
}

//menu for user to select how they would like to enter a new password
void new_password_menu(SQLite::Database *db)
{
    auto *new_password = new Password;

    std::cout << "1: Generate new password" << std::endl;
    std::cout << "2: Enter password" << std::endl;
    std::cout << "3: Return to main menu" << std::endl;

    int choice = validate_input();

    switch(choice)
    {
        case 1:
        {
            get_service(&new_password->service);
            generate_password(new_password);

            if(new_password->service.empty())
            {
                std::cout << "No service entered" << std::endl;
                break;
            }

            write_to_database(new_password, db);

            delete (new_password);
            break;
        }
        case 2:
        {
            get_service(&new_password->service);
            get_password(&new_password->password);

            if(new_password->service.empty())
            {
                std::cout << "No service entered" << std::endl;
                break;
            }

            if(new_password->password.empty())
            {
                std::cout << "No password entered" << std::endl;
                break;
            }

            write_to_database(new_password, db);

            delete (new_password);
            break;
        }
        case 3:
            delete (new_password);
            break;
        default:
            std::cout << "\nInvalid Input";
    }
}

//generates a random password using libsodium
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

    std::cout << new_password->service << ": " << password << std::endl;

    new_password->password = password;
}

void get_service(std::string *new_service)
{
    std::string service;
    std::cin.ignore();

    std::cout << "\nEnter service: ";
    std::getline(std::cin, service, '\n');

    service = trim(service);

    *new_service = service;
}

void get_password(std::string *new_password)
{
    std::string password;

    std::cout << "Enter password: ";
    std::cin >> password;

    password = trim(password);

    *new_password = password;
}

void write_to_database(Password *new_password, SQLite::Database *db)
{
    try
    {
        SQLite::Statement insert(*db, "INSERT INTO PASSWORDS (SERVICE, PASSWORD) VALUES (?, ?);");

        std::string encrypted_service = encrypt_data(new_password->service, master_pass);
        std::string encrypted_password = encrypt_data(new_password->password, master_pass);

        insert.bind(1, encrypted_service);
        insert.bind(2, encrypted_password);

        insert.exec();
    }
    catch(std::exception &e)
    {
        std::cout << "exception: " << e.what() << std::endl;
    }
}

std::string encrypt_data(std::string &data_to_encrypt, std::string &key)
{
    std::vector<unsigned char> cipher_text(crypto_secretbox_MACBYTES + data_to_encrypt.size());
    std::vector<unsigned char> nonce(crypto_secretbox_NONCEBYTES);

    randombytes_buf(nonce.data(), nonce.size());

    crypto_secretbox_easy(
            cipher_text.data(),
            reinterpret_cast<const unsigned char*>(data_to_encrypt.c_str()),
            data_to_encrypt.size(),
            nonce.data(),
            reinterpret_cast<const unsigned char*>(key.c_str()));

    std::vector<unsigned char> combined_data(nonce.begin(), nonce.end());
    combined_data.insert(combined_data.end(), cipher_text.begin(), cipher_text.end());

    std::string encrypted(combined_data.begin(), combined_data.end());

    return encrypted;
}

std::string decrypt_data(std::string &data_to_decrypt, std::string &key)
{
    std::vector<unsigned char> combined_data(data_to_decrypt.begin(), data_to_decrypt.end());
    std::vector<unsigned char> nonce(combined_data.begin(), combined_data.begin() + crypto_secretbox_NONCEBYTES);
    std::vector<unsigned char> cipher_text(combined_data.begin() + crypto_secretbox_NONCEBYTES, combined_data.end());

    std::vector<unsigned char> decrypted_text(data_to_decrypt.size() - crypto_secretbox_NONCEBYTES);

    if(crypto_secretbox_open_easy(
            decrypted_text.data(),
            cipher_text.data(),
            cipher_text.size(),
            nonce.data(),
            reinterpret_cast<const unsigned char*>(key.c_str())
            ) != 0)
    {
        std::cout << "Failed to decrypt password";
        return "";
    }

    decrypted_text.push_back('\0');
    std::string data(reinterpret_cast<char*>(decrypted_text.data()));

    return data;
}

bool check_master_pass(SQLite::Database *db)
{
    SQLite::Statement query(*db, "SELECT MASTER_PASS FROM USER_AUTH");

    if(query.executeStep())
    {
        std::string hash = query.getColumn(0);

        get_password(&master_pass);

        if(master_pass == hash)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        std::cout << "No master password detected" << std::endl;
        get_password(&master_pass);

        SQLite::Statement update(*db, "INSERT INTO USER_AUTH (MASTER_PASS) VALUES (?)");

        update.bind(1, master_pass);
        update.exec();

        return true;
    }
}