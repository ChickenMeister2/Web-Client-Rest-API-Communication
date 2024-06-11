#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <iostream>
#include <string>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <arpa/inet.h>
#include "helpers.hpp"
#include "buffer.hpp"
#include "json.hpp"

#define HEADER_TERMINATOR "\r\n\r\n"
#define HEADER_TERMINATOR_SIZE (sizeof(HEADER_TERMINATOR) - 1)
#define CONTENT_LENGTH "Content-Length: "
#define CONTENT_LENGTH_SIZE (sizeof(CONTENT_LENGTH) - 1)

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

void compute_message(char *message, const char *line)
{
    strcat(message, line);
    strcat(message, "\r\n");
}

int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag)
{
    struct sockaddr_in serv_addr;
    int sockfd = socket(ip_type, socket_type, flag);
    if (sockfd < 0)
        error("ERROR opening socket");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = ip_type;
    serv_addr.sin_port = htons(portno);
    inet_aton(host_ip, &serv_addr.sin_addr);

    /* connect the socket */
    if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    return sockfd;
}

void close_connection(int sockfd)
{
    close(sockfd);
}

void send_to_server(int sockfd, char *message)
{
    int bytes, sent = 0;
    int total = strlen(message);

    do
    {
        bytes = write(sockfd, message + sent, total - sent);
        if (bytes < 0) {
            error("ERROR writing message to socket");
        }

        if (bytes == 0) {
            break;
        }

        sent += bytes;
    } while (sent < total);
}

char *receive_from_server(int sockfd)
{
    char response[BUFLEN];
    buffer buffer = buffer_init();
    int header_end = 0;
    int content_length = 0;

    do {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0){
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);

        header_end = buffer_find(&buffer, HEADER_TERMINATOR, HEADER_TERMINATOR_SIZE);

        if (header_end >= 0) {
            header_end += HEADER_TERMINATOR_SIZE;

            int content_length_start = buffer_find_insensitive(&buffer, CONTENT_LENGTH, CONTENT_LENGTH_SIZE);

            if (content_length_start < 0) {
                continue;
            }

            content_length_start += CONTENT_LENGTH_SIZE;
            content_length = strtol(buffer.data + content_length_start, NULL, 10);
            break;
        }
    } while (1);
    size_t total = content_length + (size_t) header_end;

    while (buffer.size < total) {
        int bytes = read(sockfd, response, BUFLEN);

        if (bytes < 0) {
            error("ERROR reading response from socket");
        }

        if (bytes == 0) {
            break;
        }

        buffer_add(&buffer, response, (size_t) bytes);
    }
    buffer_add(&buffer, "", 1);
    return buffer.data;
}

char *basic_extract_json_response(char *str)
{
    return strstr(str, "{\"");
}

char *get_cookie(char *response) {
		char *cookie_line = strstr(response, "Set-Cookie: ");
    if(cookie_line != NULL) {
        // checking if there is a line that starts with "Set-Cookie: "
        char *cookie = strtok(cookie_line, " ");
        // get the cookie value from NULL to space
        cookie = strtok(nullptr, " ");
		// last char is a ; so we remove it
		cookie[strlen(cookie) - 1] = '\0';
        return cookie;
    } else {
        return nullptr;
    }
}

char *parseLibraryCookie(const char* result) {
	std::string response = result;

	std::size_t start = response.find("{");
	if (start == std::string::npos) {
		return nullptr;
	}

	std::string jsonBody = response.substr(start);
	nlohmann::json j = nlohmann::json::parse(jsonBody);

	// get library token from json object
	if(j.contains("token")){
		std::string token = j["token"];
		char* tokenCStr = new char[token.length() + 1];
		strcpy(tokenCStr, token.c_str());
		return tokenCStr;
	} else {
		return nullptr;
	}
}

char* addAuthHeader(char* library_cookie,char* message) {
	if(library_cookie == nullptr) {
		return message;
	}
	std::string authHeader = "\nAuthorization: Bearer ";
	authHeader += library_cookie;
	std::string messageStr = message;
	std::size_t pos = messageStr.find("Host");
	if (pos != std::string::npos) {
		// find end of line
		pos = messageStr.find("\n", pos);
		if (pos != std::string::npos) {
			// insert auth header
			messageStr.insert(pos, authHeader);
		}
	}

	// converting back to cstring
	strncpy(message, messageStr.c_str(), messageStr.size() + 1);
	return message;
}

char* extractError(char* response) {
	std::string responseStr = response;
	std::size_t start = responseStr.find("{");
	// if the response doesn't contain a json object
	if (start == std::string::npos) {
		return nullptr;
	}

	std::string jsonBody = responseStr.substr(start);
	nlohmann::json j = nlohmann::json::parse(jsonBody);

	// get error message from json object
	if(j.contains("error")) {
		std::string error = j["error"];
		char* errorCStr = new char[error.length() + 1];
		strcpy(errorCStr, error.c_str());
		return errorCStr;
	} else {
		return nullptr;
	}
}

void extractBooks(const char* result) {
	std::string response = result;

	// check if answer contains a json array
	std::size_t start = response.find("[");
	if (start == std::string::npos) {
		return;
	}

	std::string jsonBody = response.substr(start);
	nlohmann::json books = nlohmann::json::parse(jsonBody);
	nlohmann::json bookArray = nlohmann::json::array();

	// get every book from the response then add it
	// to the bookArray for printing
	for(auto& book : books) {
		nlohmann::json bookInfo = {
				{"id", book["id"]},
				{"title", book["title"]}
		};
		bookArray.push_back(bookInfo);
	}
	std::cout << bookArray.dump() << std::endl;
}

void extractBook(char* response) {
	std::string responseStr = response;

	// check for json body inside response
	std::size_t start = responseStr.find("{");
	if (start == std::string::npos) {
		return;
	}
	std::string jsonBody = responseStr.substr(start);
	nlohmann::json book = nlohmann::json::parse(jsonBody);

	// extract book info from json object for printing
	nlohmann::json bookInfo = {
			{"id", book["id"]},
			{"title", book["title"]},
			{"author", book["author"]},
			{"publisher", book["publisher"]},
			{"genre", book["genre"]},
			{"page_count", book["page_count"]}
	};

	std::cout << bookInfo.dump() << std::endl;
}

// function for data input validation
bool is_valid_data(const std::string& field, const std::string& buffer) {
	if (buffer.empty()) {
		return false;
	}
	else if (field == "page_count") {
		// verify that page_count is a number and does not contain any spaces
		for(const auto &c : buffer) {
			if (!isdigit(c)) {
				return false;
			}
		}
	}
	else if (field == "username" || field == "password") {
		// verify that username and password are alphanumeric and do not contain any spaces
		for(const auto &c : buffer) {
			if (!isalnum(c)) {
				return false;
			}
		}
	}
	return true;
}