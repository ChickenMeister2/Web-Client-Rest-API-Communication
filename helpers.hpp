#ifndef _HELPERS_
#define _HELPERS_

#define BUFLEN 4096
#define LINELEN 1000

// shows the current error
void error(const char *msg);

// adds a line to a string message
void compute_message(char *message, const char *line);

// opens a connection with server host_ip on port portno, returns a socket
int open_connection(char *host_ip, int portno, int ip_type, int socket_type, int flag);

// closes a server connection on socket sockfd
void close_connection(int sockfd);

// send a message to a server
void send_to_server(int sockfd, char *message);

// receives and returns the message from a server
char *receive_from_server(int sockfd);

// extracts and returns a JSON from a server response
char *basic_extract_json_response(char *str);

// extracts and returns the cookie from a server response
char *get_cookie(char *response);

// extracts and returns the token from a server response
char *parseLibraryCookie(const char* result);

// adds the authentication header to a message
char* addAuthHeader(char* library_cookie,char* message);

// extracting the error message from a response
char* extractError(char* response);

// printing all books from a library
void extractBooks(const char* response);

// printing book details
void extractBook(char* response);

// check data validity for input fields
bool is_valid_data(const std::string& field, const std::string& buffer);

#endif
