#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <fcntl.h>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <sstream>
#include <deque>

#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HttpService.h"
#include "HttpUtils.h"
#include "FileService.h"
#include "MySocket.h"
#include "MyServerSocket.h"
#include "dthread.h"

using namespace std;

int PORT = 8080;
int THREAD_POOL_SIZE = 1;
int BUFFER_SIZE = 1;
string BASEDIR = "static";
string SCHEDALG = "FIFO";
string LOGFILE = "/dev/null";

// create locks used in this file TODO
pthread_mutex_t generalLock = PTHREAD_MUTEX_INITIALIZER;

// create conditions
pthread_cond_t queueHasSpace = PTHREAD_COND_INITIALIZER;
pthread_cond_t queueIsNotEmpty = PTHREAD_COND_INITIALIZER;

// Queue
vector<MySocket *> queue; 


vector<HttpService *> services;

HttpService *find_service(HTTPRequest *request) {
   // find a service that is registered for this path prefix
  for (unsigned int idx = 0; idx < services.size(); idx++) {
    if (request->getPath().find(services[idx]->pathPrefix()) == 0) {
      return services[idx];
    }
  }

  return NULL;
}


void invoke_service_method(HttpService *service, HTTPRequest *request, HTTPResponse *response) {
  stringstream payload;

  // invoke the service if we found one
  if (service == NULL) {
    // not found status
    response->setStatus(404);
  } else if (request->isHead()) {
    service->head(request, response);
  } else if (request->isGet()) {
    service->get(request, response);
  } else {
    // The server doesn't know about this method
    response->setStatus(501);
  }
}

void handle_request(MySocket *client) {
  HTTPRequest *request = new HTTPRequest(client, PORT);
  HTTPResponse *response = new HTTPResponse();
  stringstream payload;
  
  // read in the request
  bool readResult = false;
  try {
    payload << "client: " << (void *) client;
    sync_print("read_request_enter", payload.str());
    readResult = request->readRequest();
    sync_print("read_request_return", payload.str());
  } catch (...) {
    // swallow it
  }    
    
  if (!readResult) {
    // there was a problem reading in the request, bail
    delete response;
    delete request;
    sync_print("read_request_error", payload.str());
    return;
  }
  
  HttpService *service = find_service(request);
  invoke_service_method(service, request, response);

  // send data back to the client and clean up
  payload.str(""); payload.clear();
  payload << " RESPONSE " << response->getStatus() << " client: " << (void *) client;
  sync_print("write_response", payload.str());
  cout << payload.str() << endl;
  client->write(response->response());
    
  delete response;
  delete request;

  payload.str(""); payload.clear();
  payload << " client: " << (void *) client;
  sync_print("close_connection", payload.str());
  client->close();
  delete client;
}

void* tempThreadFunct(void* arg)
{
	dthread_mutex_lock(&generalLock);
	while (true)
	{
	    //cout << "thrend reached here 1" << endl;
		while (queue.empty())
		{
			dthread_cond_wait(&queueIsNotEmpty, &generalLock);
		}
	    
	    //cout << "thrend reached here 2" << endl;

		MySocket* client = NULL;
		if (!queue.empty())
		{
			client = queue.back();
			queue.pop_back();
		}
		dthread_mutex_unlock(&generalLock); // allow another thread in
		dthread_cond_signal(&queueHasSpace);
	    //cout << "thrend reached here 3" << client << endl;
		if (client != NULL)
		{
			handle_request(client);
		}
		dthread_mutex_lock(&generalLock);
	}
	dthread_mutex_unlock(&generalLock);
	return NULL;
}

int main(int argc, char *argv[]) {

  signal(SIGPIPE, SIG_IGN);
  int option;

  while ((option = getopt(argc, argv, "d:p:t:b:s:l:")) != -1) {
    switch (option) {
    case 'd':
      BASEDIR = string(optarg);
      break;
    case 'p':
      PORT = atoi(optarg);
      break;
    case 't':
      THREAD_POOL_SIZE = atoi(optarg);
      break;
    case 'b':
      BUFFER_SIZE = atoi(optarg);
      break;
    case 's':
      SCHEDALG = string(optarg);
      break;
    case 'l':
      LOGFILE = string(optarg);
      break;
    default:
      cerr<< "usage: " << argv[0] << " [-p port] [-t threads] [-b buffers]" << endl;
      exit(1);
    }
  }

  set_log_file(LOGFILE);

  sync_print("init", "");
  MyServerSocket *server = new MyServerSocket(PORT);
  MySocket *client;

  // create necessary bools amd the queue
  pthread_t threadPool[THREAD_POOL_SIZE]; 

  // create the worker threads
  for (int i = 0; i < THREAD_POOL_SIZE; i++)
  {
	dthread_create(threadPool + i, NULL, tempThreadFunct, NULL);
  }


  services.push_back(new FileService(BASEDIR));
 
  dthread_mutex_lock(&generalLock);
  while(true) {
	    //cout << "manager 1" << endl;
	    
	    while((int)(queue.size()) >= BUFFER_SIZE) // wait until buffer has space to add requests
	    {
		    dthread_cond_wait(&queueHasSpace, &generalLock);
	    }
	    
	    //cout << "manager 2" << endl;

	    sync_print("waiting_to_accept", "");
	    
	    dthread_mutex_unlock(&generalLock); // allow another thread in
	    client = server->accept();
	    dthread_mutex_lock(&generalLock);

	    sync_print("client_accepted", "");
	    
	    //cout << "manager 3" << endl;
	    
	    queue.insert(queue.begin(), client);
	    //queue.push_back(client);

	    dthread_cond_signal(&queueIsNotEmpty);
	    
	    //cout << "manager 4" << endl;
	    
  }
  dthread_mutex_unlock(&generalLock);

  }
