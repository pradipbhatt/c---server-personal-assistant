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
        return "Hello! How can I assist you today? Feel free to ask me anything, from setting reminders to telling you jokes!";
    } else if (command.find("time") != string::npos) {
        return "The current time is: " + getCurrentTime() + ". Let me know if you need help with anything else!";
    } else if (command.find("open website") != string::npos) {
        return "Sure! Opening the website for you. Please hold on a moment while I fetch that for you.";
    } else if (command.find("joke") != string::npos) {
        return "Why don’t skeletons fight each other? They don’t have the guts! If you need more jokes, just ask!";
    } else if (command.find("weather") != string::npos) {
        return "I cannot fetch real-time weather at the moment, but I can help you find weather reports from other sources or even give tips on how to stay prepared for different weather conditions!";
    } else if (command.find("your name") != string::npos) {
        return "I am your personal assistant, created by jelina Bhatt. My purpose is to assist with various tasks, provide information, and make your day easier!";
    } else if (command.find("goodbye") != string::npos) {
        return "Goodbye! Have a great day ahead! Don't hesitate to come back whenever you need help!";
    } else if (command.find("who are you") != string::npos) {
        return "I am a voice assistant created by jelina Bhatt, a student at Far Western University. My main goal is to help you with tasks like setting reminders, answering questions, telling jokes, and much more!";
    } else if (command.find("set reminder") != string::npos) {
        return "Reminder set successfully! What would you like me to remind you of? I can help you with anything from meetings to taking breaks.";
    } else if (command.find("thank you") != string::npos) {
        return "You're welcome! I'm always here to help. If you need anything else, just ask!";
    } else if (command.find("What do you do") != string::npos) {
        return "I can assist you in various ways! I can tell you the time, help you set reminders, answer your questions, provide jokes, give information about current events, and more! Just ask me anything!";
    } else if (command.find("Who is jelina Bhatt") != string::npos) {
        return "jelina Bhatt is a dedicated student from Far Western University, Nepal, currently pursuing a B.Tech in Computer Science and Engineering. He is passionate about web development, AI, machine learning, and creating impactful solutions. In addition to studying, he enjoys photography, traveling, and exploring new things.";
    } else if (command.find("Tell me a fun fact") != string::npos) {
        return "Did you know? An octopus has three hearts: two pump blood to the gills, and one pumps it to the rest of the body! Isn't that fascinating?";
    } else if (command.find("set a reminder for tomorrow") != string::npos) {
        return "Reminder for tomorrow has been set! Don't forget to check back with me for any updates or new tasks you need assistance with!";
    } else if (command.find("what is the weather") != string::npos) {
        return "I can't fetch real-time weather, but I can tell you how to find accurate weather reports online. Websites like AccuWeather and weather apps on your phone can give you up-to-date weather information.";
    } else if (command.find("are you smart") != string::npos) {
        return "I try my best! But I always strive to learn more and improve my responses. If there’s something I don’t know, I’ll do my best to find it out!";
    } else if (command.find("tell me a riddle") != string::npos) {
        return "Alright! Here’s a riddle for you: What has keys but can’t open locks? (Answer: A piano!)";
    } else if (command.find("who is the president of Nepal") != string::npos) {
        return "The current president of Nepal is Bidya Devi Bhandari. She has been serving as the first female president of Nepal since 2015.";
    } else if (command.find("how to learn coding") != string::npos) {
        return "To learn coding, start with the basics like understanding programming concepts, syntax, and problem-solving. Websites like Codecademy, freeCodeCamp, and Coursera offer great courses for beginners. Consistent practice and building projects will help you improve faster!";
    } else if (command.find("what is AI") != string::npos) {
        return "AI, or Artificial Intelligence, refers to machines or systems designed to mimic human intelligence. This includes tasks like learning, problem-solving, and decision-making. AI is widely used in applications like self-driving cars, voice assistants, and more!";
    } else if (command.find("what is Machine Learning") != string::npos) {
        return "Machine Learning (ML) is a subset of AI that enables systems to automatically learn and improve from experience without being explicitly programmed. ML is widely used in applications such as image recognition, recommendation systems, and natural language processing.";
    } else if (command.find("how do you work") != string::npos) {
        return "I work by receiving commands from you, processing them, and then providing relevant responses. I utilize various APIs and built-in logic to perform tasks and fetch information. My capabilities depend on the information available to me!";
    } else if (command.find("help me with coding") != string::npos) {
        return "I'd be happy to help you with coding! Let me know what programming language or specific problem you're working on, and I'll provide guidance or solutions!";
    } else if (command.find("who is Elon Musk") != string::npos) {
        return "Elon Musk is the CEO of Tesla and SpaceX. He is known for his work in advancing electric vehicles, space exploration, and renewable energy. Musk is also known for his vision of colonizing Mars and creating sustainable technologies.";
    } else if (command.find("what is quantum computing") != string::npos) {
        return "Quantum computing is a field of computing that leverages the principles of quantum mechanics to process information in ways that classical computers cannot. It has the potential to revolutionize fields such as cryptography, optimization, and AI!";
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
