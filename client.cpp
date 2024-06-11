#include <iostream>
#include "commands.hpp"
using namespace std;

#define HOST_ADDR "54.170.241.232"
#define HOST_PORT 8080


int main(int argc, char *argv[]) {
	string command;
	char *login_cookie = nullptr;
	char *library_cookie = nullptr;

	while(1) {
		cin >> command;
		if(command == "register")
			register_handler(login_cookie);
		else if (command == "login")
			login_handler(login_cookie);
		else if (command == "enter_library")
			enter_library_handler(login_cookie,library_cookie);
		else if (command == "get_books")
			get_books_handler(login_cookie,library_cookie);
		else if (command == "get_book")
			get_book_handler(login_cookie,library_cookie);
		else if (command == "add_book")
			add_book_handler(login_cookie,library_cookie);
		else if (command == "delete_book")
			delete_book_handler(login_cookie,library_cookie);
		else if (command == "logout")
			logout_handler(login_cookie,library_cookie);
		else if(command == "exit")
			break;
		else
			cout << "Invalid command" << endl;
	}

}
