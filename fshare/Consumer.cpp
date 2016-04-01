#include "Consumer.h"
#include <fstream>

void
Consumer::onFirstData(const ndn::Interest& interest, const ndn::Data& data)
{
  if (!data.getFinalBlockId().empty()) {
    // figure out the last segment number of the file transfer
    m_haveFinalBlockId = true;
    m_lastSegmentNum = data.getFinalBlockId().toSegment();
  }

  // save the first segment data we received
  const auto segmentNum = static_cast<size_t>(data.getName()[-1].toSegment());
  const ndn::Block& block = data.getContent();
  auto mydata = std::make_shared<ndn::Data>(ndn::Name(m_prefix).appendSegment(segmentNum));
  mydata->setContent(block.value(), block.value_size());
  m_bufferedData[segmentNum] = mydata;

  m_nextSegmentNum++;

  if (m_debug)
    std::cerr << "onFirstData m_lastSegmentNum = " << m_lastSegmentNum << std::endl;

  if (m_haveFinalBlockId && (m_bufferedData.size() - 1) == m_lastSegmentNum) {
    // received the last data segment so write the file and exit
    writeBufferedData();
    m_face.shutdown();
    exit(0);
  }

  size_t maxSegments = 1024; // TODO should this be m_lastSegmentNum instead?
  for (size_t i = 1; i < maxSegments; i++) { // start at 1 since for first segment already
    if (!getNextSegment(i))
      break;
  }
}

bool
Consumer::getNextSegment(size_t segmentNum) // TODO maybe don't need to pass in segmentNum
{
  if (m_haveFinalBlockId && m_nextSegmentNum > m_lastSegmentNum)
    return false; // we've shown interested in every segment for the file

  if (m_debug)
    std::cerr << "Requesting segment #" << m_nextSegmentNum << std::endl;

  ndn::Interest interest(ndn::Name(m_prefix).appendSegment(m_nextSegmentNum));
  m_face.expressInterest(interest,
                         bind(&Consumer::onSegmentData, this, _1, _2),
                         bind(&Consumer::onSegmentTimeout, this, _1));
  
  m_nextSegmentNum++;
  return true;
}

void
Consumer::onSegmentData(const ndn::Interest& interest, const ndn::Data& data)
{
  if (m_debug)
    std::cerr << "Received segment #" << data.getName()[-1].toSegment() << std::endl;

  if (!m_haveFinalBlockId && !data.getFinalBlockId().empty()) {
    m_lastSegmentNum = data.getFinalBlockId().toSegment();
    m_haveFinalBlockId = true;
  }

  //const ndn::Block& block = data.getContent();
  //m_bufferedData[data.getName()[-1].toSegment()] = reinterpret_cast<const char*>(block.value());
  const auto segmentNum = static_cast<size_t>(data.getName()[-1].toSegment());
  const ndn::Block& block = data.getContent();
  auto mydata = std::make_shared<ndn::Data>(ndn::Name(m_prefix).appendSegment(segmentNum));
  mydata->setContent(block.value(), block.value_size());
  m_bufferedData[segmentNum] = mydata;

  if (m_debug)
    std::cerr << "m_bufferedData.size() = " << m_bufferedData.size() << std::endl;

  if (m_haveFinalBlockId && (m_bufferedData.size() - 1) == m_lastSegmentNum) {
    // received the last data segment so write the file and exit
    writeBufferedData();
    m_face.shutdown();
    exit(0);
  }
}

void
Consumer::onSegmentTimeout(const ndn::Interest& interest)
{
  std::cout << "Segment interest timed out " << interest << std::endl;
  exit(0);
}

void
Consumer::writeBufferedData()
{
  if (m_debug)
    std::cerr << "Writing data out to file " << m_filedata.m_local_filename << std::endl;

  std::ofstream of(m_filedata.m_local_filename, std::ofstream::binary);
  if (!of) {
    std::cout << "ERROR: could not open " << m_filedata.m_local_filename << " for write" << std::endl;
    exit(1);
  }
  for (const auto &it : m_bufferedData) {
    of.write(reinterpret_cast<const char*>(it.second->getContent().value()), it.second->getContent().value_size());
  }
  of.close();

}

