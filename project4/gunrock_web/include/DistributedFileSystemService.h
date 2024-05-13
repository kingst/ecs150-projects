#ifndef _DISTRIBUTEDFILESYSTEMSERVICE_H_
#define _DISTRIBUTEDFILESYSTEMSERVICE_H_

#include "HttpService.h"
#include "LocalFileSystem.h"

#include <string>

class DistributedFileSystemService : public HttpService {
 public:
  DistributedFileSystemService(std::string driveFile);

  virtual void get(HTTPRequest *request, HTTPResponse *response);
  virtual void put(HTTPRequest *request, HTTPResponse *response);
  virtual void del(HTTPRequest *request, HTTPResponse *response);

private:
  LocalFileSystem *fileSystem;
};

#endif
