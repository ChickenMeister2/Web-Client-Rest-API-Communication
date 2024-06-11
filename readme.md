<h1>Web client</h1>
<h2>Rest API communication</h2>

<h3>Possible client commands</h3>

<ul>
<li><h4>Register and Login</h4> The user is prompted to enter a username and password. The
client checks if the inputted data is valid by checking characters and if it's valid it opens
a connection to the server, sends the message and waits for a response. The client parses the 
status code from the response. If the status code is 200, the client logs the user in. If it's not 200
then it prints the string "ERROR" followed by the error message got from the server. <ul><li>
The parser for the <b>status code</b> and <b>error message</b> work by checking certain parts of the response message
that start with a certain word.</li><li>The login function also modifies the <b>Login cookie</b> variable by getting the 
line that starts with <b>"Set-Cookie: "</b> and ends with space. I also had to remove the last character from the
extracted string because that is a <b>;</b> .</li></ul>
</li>

<li><h4>Enter Library</h4>
The client sends a message to the server to enter the library. This message uses cookies to transmit the login cookie to the server
. The server responds with a status code and a message. If this operation was successful, the server sends a message
that contains a JWT for entering the library. I'm storing this token in a variable called <b>login_cookie</b>.
<ul><li>This token is parsed by using a separate function that is looking for a "{" inside the response. This means that
the response contains a JSON object containing the token.</li></ul>
</li>
<li><h4>Get books</h4>
This functions sends a GET message to the server. The message is computed with the login cookie. After this, I send the message
to another function that will add the Authentication header to the message. If the response from the server is OK then I print
the title and the ID of all the books inside the user's library.
</li>
<li><h4>Get book</h4>
Similarly to the last function, the message contains a cookie (login cookie) and the authentication header. The
URL that I want to access from the server changes based on the ID of the book. I use the sprintf function to add the ID of the
book to the URL. If the response
is OK, the program prints all the details about that certain book.
</li>
<li><h4>Delete book</h4></li>
This function reads an ID from the user then will make a <b>DELETE</b> request to the server.
The URL used is similar to the one used in <b>Get Book function</b>. The <b>DELETE</b> request is created
by using the slightly modified <b>POST</b> function. Just by changing the first line of the message.
<li><h4>Logout</h4>
This function sends a <b>POST</b> request to the server. The message contains the login cookie. If the user was logged in,
the login_cookie and the library_cookie are being set to <b>nullptr</b>. They will have a value when the user
signs in again.</li>
<li><h4>Exit</h4>
This function will stop the while loop where the program is running.</li>
</ul>
