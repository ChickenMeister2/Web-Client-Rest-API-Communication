#ifndef _COMMANDS_
#define _COMMANDS_

void register_handler(char* login_cookie);

void login_handler(char*& login_cookie);

void enter_library_handler(char *login_cookie, char*& library_cookie);

void get_books_handler(char* login_cookie,char *library_cookie);

void get_book_handler(char* login_cookie,char *library_cookie);

void add_book_handler(char* login_cookie,char *library_cookie);

void delete_book_handler(char *login_cookie, char *library_cookie);

void logout_handler(char*& login_cookie,char*& library_cookie);

#endif
