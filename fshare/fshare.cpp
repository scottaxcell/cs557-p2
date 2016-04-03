#include <iostream>
#include <vector>
#include <algorithm>
#include <thread>

#include "fshare.h"
#include "Producer.h"
#include "Consumer.h"

#define debug 1

namespace {

void usage()
{
  std::cout << "./fshare -n <nodename> [-p <filename>] [-d <filename>]" << std::endl;
  exit(0);
}


std::string translateNameToLocalName(std::string &filename)
{
  std::string local_filename(filename);
  std::replace(local_filename.begin(), local_filename.end(), '/', '_');
  return local_filename;
}


void runProducer(FileData fd)
{
  Producer p(fd.m_filename, fd);
  p.start();
}

} // namespace


int
main(int argc, char** argv)
{
  // Node name is used to decipher what the downloaded file should be named
  // ex: hostname-prefix_filename
  std::string nodename("NOT_SET");

  // Filenames interested in sharing with others
  std::vector<FileData> publishFiles;

  // Filenames interested in downloading from others
  std::vector<FileData> downloadFiles;

  if (argc == 1) {
    usage();
  }

  int i = 1;
  while (i < argc) {
    if (std::string(argv[i]) == "-h")
      usage();
    if (std::string(argv[i]) == "-n") {
      i++;
      nodename = argv[i];
      i++;
    }
    else if (std::string(argv[i]) == "-p") {
      i++;
      while (i < argc) {
        if (std::string(argv[i]) == "-d" || std::string(argv[i]) == "-n" || std::string(argv[i]) == "-p") {
          i--;
          break;
        }
        std::string filename(argv[i]);
        std::string local_filename = translateNameToLocalName(filename);
        FileData fd(filename, local_filename);
        publishFiles.push_back(fd);
        i++;
      }
    }
    else if (std::string(argv[i]) == "-d") {
      i++;
      while (i < argc) {
        if (std::string(argv[i]) == "-d" || std::string(argv[i]) == "-n" || std::string(argv[i]) == "-p") {
          i--;
          break;
        }
        std::string filename(argv[i]);
        std::string local_filename = translateNameToLocalName(filename);
        FileData fd(filename, local_filename);
        downloadFiles.push_back(fd);
        i++;
      }
    } else {
      i++;
    }
  }

  if (nodename == "NOT_SET" || (publishFiles.empty() && downloadFiles.empty()))
    usage();

  // download files sequentially
  for (auto &fd : downloadFiles) {
    fd.m_local_filename = nodename + '-' + fd.m_local_filename;
    if (debug)
      std::cerr << "Consumer: " << fd.m_filename << " = " << fd.m_local_filename << std::endl;
    Consumer c(fd.m_filename, fd);
    c.start();
  }

  // create each producer on it's own thread
  std::vector<std::thread> threads;
  for (auto &fd : publishFiles) {
    // prepend node name to local_filename
    fd.m_local_filename = nodename + '-' + fd.m_local_filename;
    threads.push_back(std::thread(runProducer, fd));
    if (debug)
      std::cerr << "Producer: " << fd.m_filename << " = " << fd.m_local_filename << std::endl;
  }
  // run producers
  for (auto &t : threads)
    t.join();

  return 0;
}
