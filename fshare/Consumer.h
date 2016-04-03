#ifndef CONSUMER_H
#define CONSUMER_H

#include <iostream>

#include <ndn-cxx/face.hpp>

#include "fshare.h"

class Consumer
{
public:

  Consumer(const std::string& uri, FileData& filedata)
    : m_prefix(uri),
      m_filedata(filedata),
      m_debug(true),
      m_haveFinalBlockId(false),
      m_nextSegmentNum(0),
      m_lastSegmentNum(0),
      m_numTimeoutRetries(0),
      m_maxTimeoutRetries(3)
  {
  }

  void
  start()
  {
    std::shared_ptr<ndn::Interest> request(std::make_shared<ndn::Interest>(m_prefix));
    m_face.expressInterest(*request,
                           bind(&Consumer::onFirstData, this, _1, _2),
                           bind(&Consumer::onTimeout, this, _1));

    m_face.processEvents();
  }

protected:

  void
  onFirstData(const ndn::Interest& interest, const ndn::Data& data);

  bool
  getNextSegment(size_t segmentNum);

  void
  onSegmentData(const ndn::Interest& interest, const ndn::Data& data);

  void
  onSegmentTimeout(const ndn::Interest& interest);

  void
  writeBufferedData();

  void
  onTimeout(const ndn::Interest& interest)
  {
    std::cout << "Interest timed out " << interest << std::endl;
    exit(0);
  }


protected:
  const ndn::Name m_prefix;
  ndn::Face m_face;
  FileData m_filedata;
  std::map<uint64_t, std::shared_ptr<ndn::Data>> m_bufferedData;
  bool m_debug;
  bool m_haveFinalBlockId;
  uint64_t m_nextSegmentNum; 
  uint64_t m_lastSegmentNum; // last file segment to complete the transfer
  uint64_t m_numTimeoutRetries;
  uint64_t m_maxTimeoutRetries;
};

#endif
