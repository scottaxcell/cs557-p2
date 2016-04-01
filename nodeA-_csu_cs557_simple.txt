#include <iostream>
#include <vector>
#include <algorithm>

#include "fshare.h"
#include "Producer.h"
#include "Consumer.h"

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
        if (std::string(argv[i]) == "-d" || std::string(argv[i]) == "-n") {
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
        if (std::string(argv[i]) == "-p" || std::string(argv[i]) == "-n") {
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

  // prepend node name to local_filename
  for (auto &fd : publishFiles) {
    fd.m_local_filename = nodename + '-' + fd.m_local_filename;

    // TODO create producers - these will need to be forked or threaded since start() does not exit
    Producer p(fd.m_filename, fd);
    p.start();
  }
  for (auto &fd : downloadFiles) {
    fd.m_local_filename = nodename + '-' + fd.m_local_filename;
    // TODO create consumers
    Consumer c(fd.m_filename, fd);
    c.start();
  }

  return 0;
}
