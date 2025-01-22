#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctime>
#include <curl/curl.h> // Include curl for HTTP requests

using namespace std;

// Function declarations
string fetchGeminiResponse(const string& userCommand);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

// Function to read file content
string readFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error: Unable to open file " << filePath << endl;
        return "";
    }
    stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to get the current time as a string
string getCurrentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    return string(dt);
}

// Function to handle voice commands
string handleCommand(const string& command) {
    if (command.find("hello") != string::npos) {
        return "Hello! How can I assist you today?";
    } else if (command.find("time") != string::npos) {
        return "The current time is: " + getCurrentTime();
    } else if (command.find("open website") != string::npos) {
        return "Sure! Opening the website for you.";
    } else if (command.find("joke") != string::npos) {
        return "Why don’t skeletons fight each other? They don’t have the guts!";
    } else if (command.find("weather") != string::npos) {
        return "I cannot fetch real-time weather yet, but I can help you with the weather report if needed.";
    } else if (command.find("your name") != string::npos) {
        return "I am your personal assistant!";
    } else if (command.find("goodbye") != string::npos) {
        return "Goodbye! Have a great day ahead!";
    } else if (command.find("who are you") != string::npos) {
        return "I am a voice assistant, created by jelina boss to help you with various tasks!";
    } else if (command.find("set reminder") != string::npos) {
        return "Reminder set successfully! What would you like me to remind you of?";
    } else if (command.find("thank you") != string::npos) {
        return "You're welcome!";
    } else {
        // Interact with the Gemini API if the command is unknown
        return fetchGeminiResponse(command);
    }
}

// Function to interact with Gemini API
string fetchGeminiResponse(const string& userCommand) {
    CURL *curl;
    CURLcode res;
    stringstream responseStream;
    
    // Initialize CURL
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        // Define the Gemini API endpoint and API key
        const string apiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key=AIzaSyCcwb1CgJ7pd_X6FqEbbare8zuKL1USQAQ";
        
        // Setup the request headers
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        // Setup the request data (send the user command in JSON format)
        string requestData = "{\"contents\": [{\"parts\":[{\"text\": \"" + userCommand + "\"}]}]}";
        
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestData.c_str());
        
        // Write response data to the responseStream
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseStream);
        
        // Perform the request
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            cerr << "CURL error: " << curl_easy_strerror(res) << endl;
            curl_easy_cleanup(curl);
            return "Error fetching response from Gemini API.";
        }
        
        // Cleanup
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        
        return responseStream.str();
    }
    
    return "Failed to initialize CURL.";
}

// Callback function for handling the response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((stringstream*)userp)->write((char*)contents, size * nmemb);
    return size * nmemb;
}

int main() {
    int serverSocket, clientSocket;
    sockaddr_in serverAddr{}, clientAddr{};

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        cerr << "Socket creation failed!" << endl;
        return -1;
    }

    // Configure server address
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    // Bind socket
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        cerr << "Bind failed!" << endl;
        close(serverSocket);
        return -1;
    }

    // Listen for connections
    if (listen(serverSocket, 5) < 0) {
        cerr << "Listen failed!" << endl;
        close(serverSocket);
        return -1;
    }

    cout << "Server running on http://localhost:8080" << endl;

    // Accept client connections
    while (true) {
        socklen_t clientLen = sizeof(clientAddr);
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            cerr << "Failed to accept client!" << endl;
            continue;
        }

        // Read client request
        char buffer[2048] = {0};
        read(clientSocket, buffer, sizeof(buffer));
        cout << "Request received:\n" << buffer << endl;

        // Parse request
        string request(buffer);
        string response;
        if (request.find("POST /") != string::npos) {
            // Extract JSON body
            size_t start = request.find("\r\n\r\n") + 4;
            string body = request.substr(start);

            // Extract "command" from JSON
            size_t cmdStart = body.find("\"command\":\"") + 11;
            size_t cmdEnd = body.find("\"", cmdStart);
            string command = body.substr(cmdStart, cmdEnd - cmdStart);

            // Handle command
            string commandResponse = handleCommand(command);
            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: " + to_string(commandResponse.size()) + "\r\n\r\n" +
                commandResponse;
        } else {
            string htmlContent = readFile("index.html");
            if (htmlContent.empty()) {
                cerr << "Error: Unable to serve index.html" << endl;
                close(clientSocket);
                continue;
            }
            response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: " + to_string(htmlContent.size()) + "\r\n\r\n" +
                htmlContent;
        }

        // Send response
        send(clientSocket, response.c_str(), response.size(), 0);

        // Close client connection
        close(clientSocket);
    }

    // Close server socket
    close(serverSocket);
    return 0;
}
