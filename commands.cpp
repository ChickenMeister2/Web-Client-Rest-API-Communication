#include <cstdlib>     /* exit, atoi, malloc, free */
#include <cstdio>
#include <cctype>
#include <iostream>
#include <string>
#include <cstring>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <sstream>
#include "helpers.hpp"
#include "requests.hpp"
#include "commands.hpp"
#include "json.hpp"

using namespace std;

#define HOST_ADDR "54.170.241.232"
#define HOST_PORT 8080

#define  BUF_SIZE 4096

int extractStatusCode(const std::string& response) {
	std::istringstream responseStream(response);
	std::string httpVersion;
	int statusCode;

	// parsing the first words from the response
	responseStream >> httpVersion >> statusCode;

	return statusCode;
}

void register_handler(char* login_cookie) {
	bool valid = true;

	// check if user is logged in
	if(login_cookie != nullptr) {
		cout << "ERROR - Already logged in!" << endl;
		return;
	}

	char *message;
	char *response;
	int sockfd;
	string username, password;

	cin.ignore();
	char buffer[4096];
	cout << "username=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("username", buffer)) {
		valid = false;
	}
	username = buffer;

	cout << "password=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("password", buffer)) {
		valid = false;
	}
	password = buffer;

	// validate input data
	if(!valid) {
		cout << "ERROR - Invalid username or password" << endl;
		return;
	}

	// create json object with user credentials
	nlohmann::json loginInfo;
	loginInfo["username"] = username;
	loginInfo["password"] = password;

	// converting to simple string
	string credentials = loginInfo.dump();
	char **body = new char*[1];
	body[0] = (char *)malloc(credentials.size() * sizeof(char));

	//converting to cstring
	strcpy(body[0], credentials.c_str());

	// connect to the server only when sending / receiving data
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	message = compute_post_request(HOST_ADDR, "/api/v1/tema/auth/register", "application/json", body, 1, nullptr, 0);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);

	// extracting status code from server response to check
	// if the request was successful or not
	int statusCode = extractStatusCode(response);

	// verify the status code and print the appropriate message
	if(statusCode == 400) {
		cout << "ERROR - " << extractError(response) << endl;
	} else if (statusCode == 201) {
		cout << "SUCCESS - User registered successfully!" << endl;
	}

	close_connection(sockfd);
	delete[] response;
	delete[] body[0];

}

void login_handler(char*& login_cookie) {
	bool valid = true;

	// similar checks to register function
	if(login_cookie != nullptr) {
		cout << "ERROR - Already logged in!" << endl;
		return;
	}

	char *message;
	char *response;
	int sockfd;
	string username, password;

	cin.ignore();
	char buffer[BUF_SIZE];
	cout << "username=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("username", buffer)) {
		valid = false;
	}
	username = buffer;

	cout << "password=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("password", buffer)) {
		valid = false;
	}
	password = buffer;

	if(!valid) {
		cout << "ERROR - Invalid username or password" << endl;
		return;
	}

	nlohmann::json loginInfo;
	loginInfo["username"] = username;
	loginInfo["password"] = password;
	string credentials = loginInfo.dump();
	char **body = new char*[1];
	body[0] = (char *)malloc(credentials.size() * sizeof(char));
	strcpy(body[0], credentials.c_str());
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	message = compute_post_request(HOST_ADDR, "/api/v1/tema/auth/login", "application/json", body, 1, nullptr, 0);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);

	int statusCode = extractStatusCode(response);
	if(statusCode != 200) {
		cout << "ERROR - " << extractError(response) << endl;
	} else {
		login_cookie = get_cookie(response);
		cout << "SUCCESS - Logged in successfully!" << endl;
	}

	close_connection(sockfd);
	delete[] response;
	delete[] body[0];

}

void enter_library_handler(char *login_cookie, char*& library_cookie) {

	// check if user already got access to library
	if(library_cookie != nullptr) {
		cout<< "ERROR - Already gained access to library!" << endl;
		return;
	}

	char *message;	
	char *response;
	int sockfd;
	char** cookies = new char*[1];
	cookies[0] = login_cookie;
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	message = compute_get_request(HOST_ADDR, "/api/v1/tema/library/access", nullptr, cookies, 1);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);
	if(extractStatusCode(response) != 200) {
		cout << "ERROR - " << extractError(response) << endl;
	} else  {
		cout << "SUCCESS - You have access to the library!" << endl;
		// extracting and storing library token from response
		library_cookie = parseLibraryCookie(response);
	}

	close_connection(sockfd);
	delete[] response;
}

void get_books_handler(char* login_cookie,char *library_cookie){
	char *message;
	char *response;
	int sockfd;
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	message = compute_get_request(HOST_ADDR, "/api/v1/tema/library/books", nullptr, &login_cookie, 1);
	message = addAuthHeader(library_cookie,message);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);

	//
	if(extractStatusCode(response) != 200) {
		cout << "ERROR - " << extractError(response) << endl;
	} else {
		extractBooks(response);
	}

	close_connection(sockfd);
	delete[] response;
}

void get_book_handler(char* login_cookie,char *library_cookie) {
	char *message;
	char *response;
	int sockfd;

	string bookId;
	cout << "id=";
	cin >> bookId;

	// check de inputted bookId
	bool isBookIdValid = std::all_of(bookId.begin(), bookId.end(), ::isdigit);
	if (!isBookIdValid ) {
		cout << "BookId can only contain numbers!" << endl;
		return;
	}

	// add the bookID to the end of the url
	char url[50];
	sprintf(url, "/api/v1/tema/library/books/%s", bookId.c_str());
	char** cookies = new char*[1];
	cookies[0] = login_cookie;
	message = compute_get_request(HOST_ADDR, url, nullptr, cookies, 1);
	char* newMessage = addAuthHeader(library_cookie,message);
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, newMessage);
	delete[] message;
	response = receive_from_server(sockfd);

	// if successful
	if(extractStatusCode(response) == 200)
		extractBook(response);
	else
		cout << "ERROR - " << extractError(response) << endl;

	close_connection(sockfd);
	delete[] response;
}

void add_book_handler(char* login_cookie,char *library_cookie){
	bool valid = true;
	char *message;
	char *response;
	int sockfd;
	string title, author, genre, publisher;
	int page_count;

	cin.ignore();
	char buffer[BUF_SIZE];
	cout << "title=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("title", buffer)) {
		valid = false;
	}
	title = buffer;

	cout << "author=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("author", buffer)) {
		valid = false;
	}
	author = buffer;

	cout << "genre=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("genre", buffer)) {
		valid = false;
	}
	genre = buffer;

	cout << "publisher=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("publisher", buffer)) {
		valid = false;
	}
	publisher = buffer;

	cout << "page_count=";
	cin.getline(buffer, BUF_SIZE);

	if(!is_valid_data("page_count", buffer)) {
		valid = false;
	}

	if(!valid) {
		cout << "ERROR - Invalid book data" << endl;
		return;
	}
	page_count = atoi(buffer);

	nlohmann::json bookInfo;
	bookInfo["title"] = title;
	bookInfo["author"] = author;
	bookInfo["genre"] = genre;
	bookInfo["publisher"] = publisher;
	bookInfo["page_count"] = page_count;
	string bookData = bookInfo.dump();

	char **body = new char*[1];
	body[0] = (char *)malloc(bookData.size() * sizeof(char));
	strcpy(body[0], bookData.c_str());

	if(library_cookie == nullptr || login_cookie == nullptr){
		cout<<"ERROR - Access denied"<<endl;
		return;
	}

	message = compute_post_request(HOST_ADDR, "/api/v1/tema/library/books", "application/json", body, 1, &login_cookie, 1);
	message = addAuthHeader(library_cookie,message);
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);

	if(extractStatusCode(response) != 200)
		cout << "ERROR - " << extractError(response) << endl;
	else
		cout << "SUCCESS - Book added successfully!" << endl;

	close_connection(sockfd);
	delete[] response;

}

void delete_book_handler(char *login_cookie, char *library_cookie){
	char *message;
	char *response;
	int sockfd;
	char buffer[BUF_SIZE];

	cin.ignore();
	cout << "id=";
	cin.getline(buffer, BUF_SIZE);
	if(!is_valid_data("page_count", buffer)) {
		cout << "ERROR - Invalid book ID" << endl;
		return;
	}

	char url[50];
	sprintf(url, "/api/v1/tema/library/books/%s", buffer);

	message = compute_delete_request(HOST_ADDR, url, "application/json", nullptr, 0, &login_cookie, 1);
	message = addAuthHeader(library_cookie,message);
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);

	if(extractStatusCode(response) != 200)
		cout << "ERROR - " << extractError(response) << endl;
	else
		cout << "SUCCESS - Book deleted successfully!" << endl;

	close_connection(sockfd);
	delete[] response;

}

void logout_handler(char*& login_cookie,char*& library_cookie){
	if(login_cookie == nullptr) {
		cout << "ERROR - Not logged in!" << endl;
		return;
	}

	char *message;
	char *response;
	int sockfd;

	message = compute_get_request(HOST_ADDR, "/api/v1/tema/auth/logout", "application/json",  &login_cookie, 1);
	if(library_cookie != nullptr)
		message = addAuthHeader(library_cookie,message);
	sockfd = open_connection(HOST_ADDR,HOST_PORT, AF_INET, SOCK_STREAM, 0);
	send_to_server(sockfd, message);
	delete[] message;
	response = receive_from_server(sockfd);

	if(extractStatusCode(response) != 200)
		cout << "ERROR - " << extractError(response) << endl;
	else {
		cout <<"SUCCESS - Logged out successfully!" << endl;
		login_cookie = nullptr;
		library_cookie = nullptr;
	}

	close_connection(sockfd);
	delete[] response;

}