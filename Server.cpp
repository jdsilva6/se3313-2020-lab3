#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
std::string bob;
// This thread handles each client connection
class SocketThread : public Thread
{
private:
    // Reference to our connected socket
    Socket& socket;
    // The data we are receiving
    ByteArray data;
public:
    SocketThread(Socket& socket)
    : socket(socket)
    {}

    ~SocketThread()
    {}

    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain()
    {
        while(1)
        {
            try
            {
                // Wait for data
                socket.Read(data);
                std::cout << "Data read" << std::endl;
                // Perform operations on the data
                std::string str = data.ToString();
                if (str == "done"){
				    std::cout << "Socket terminated" << std::endl << "Press enter to terminate the server..." << std::endl;
                    break;
                }
                std::reverse(str.begin(), str.end());
                data = ByteArray(str);
                std::cout << "Sending " << str << std::endl;
                // Send it back
                socket.Write(data);
                std::cout << "Data sent" << std::endl;
            }
            catch (...)
            {
                // catch something?
            }
        }
		
	// ???

        return 0;
    }
};

// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    std::vector<SocketThread*> socketThreadVector;
    bool terminate = false;
public:
    ServerThread(SocketServer& server)
    : server(server)
    {}

    ~ServerThread()
    {
        // Cleanup
        for (int i = 0; i < socketThreadVector.size(); i++){
            Socket& socket = socketThreadVector[i]->GetSocket();
            socket.Close();
        }
    }

    virtual long ThreadMain()
    {
        while(true){
            // Wait for a client socket connection
            Socket* newConnection = new Socket(server.Accept());

            // Pass a reference to this pointer into a new socket thread
            Socket& socketReference = *newConnection;
            //SocketThread* socketThread = new SocketThread(socketReference);
            socketThreadVector.push_back(new SocketThread(socketReference)); 
            // changed original code as there will be multiple socket threads, and so a record of all of them is needed
        }
    }
};


int main(void)
{
    std::cout << "I am a server." << std::endl;
	std::cout << "Press enter to terminate the server..." << std::endl;
    std::cout.flush();
	
    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();
    std::cin.get();

    // Shut down and clean up the server
    server.Shutdown();

}
