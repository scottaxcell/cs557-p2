#ifndef FSHARE_H
#define FSHARE_H

/*
 * Small class for tracking NDN filename and the local filename
 */
class FileData
{

public:
  FileData(std::string &filename, std::string &local_filename)
   : m_filename(filename), m_local_filename(local_filename)
  {}

  std::string m_filename;
  std::string m_local_filename;
};

#endif
